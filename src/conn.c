#include "module.h"
#include "except.h"
#include "cookie.h"
#include "response.h"
#include "event.h"
#include "error.h"
#include "extkey.h"
#include "ext.h"
#include "conn.h"

/*
 * Helpers
 */
int
xpybConn_invalid(xpybConn *self)
{
    if (self->conn == NULL) {
	PyErr_SetString(xpybExcept_base, "Invalid connection.");
	return 1;
    }
    return 0;
}

PyObject *
xpybConn_make_core(xpybConn *self)
{
    /* Make sure core was set. */
    if (xpybModule_core == NULL) {
	PyErr_SetString(xpybExcept_base, "No core protocol object has been set.  Did you import xcb.xproto?");
	return NULL;
    }

    /* Call the object to get a new xcb.Extension object. */
    return PyObject_CallFunctionObjArgs((PyObject *)xpybModule_core, self, NULL);
}

static PyObject *
xpybConn_make_ext(xpybConn *self, PyObject *key)
{
    PyObject *result;
    xpybExt *ext;
    const xcb_query_extension_reply_t *reply;

    /* Look up the callable object in the global dictionary. */
    result = PyDict_GetItem(xpybModule_extdict, key);
    if (result == NULL) {
	PyErr_SetString(xpybExcept_ext, "No extension found for that key.");
	return NULL;
    }

    /* Call the object to get a new xcb.Extension object. */
    ext = (xpybExt *)PyObject_CallFunctionObjArgs(result, self, key, NULL);
    if (ext == NULL)
	return NULL;

    /* Get the opcode and base numbers. */
    reply = xcb_get_extension_data(self->conn, &((xpybExtkey *)key)->key);
    if (!reply->present) {
	PyErr_SetString(xpybExcept_ext, "Extension not present on server.");
	Py_DECREF(ext);
	return NULL;
    }
    ext->major_opcode = reply->major_opcode;
    ext->first_event = reply->first_event;
    ext->first_error = reply->first_error;

    return (PyObject *)ext;
}

static int
xpybConn_setup_ext(xpybConn *self, xpybExt *ext, PyObject *events, PyObject *errors)
{
    Py_ssize_t j = 0;
    unsigned char opcode, newlen;
    PyObject *num, *type, **newmem;

    while (PyDict_Next(events, &j, &num, &type)) {
	opcode = ext->first_event + PyInt_AS_LONG(num);
	if (opcode >= self->events_len) {
	    newlen = opcode + 1;
	    newmem = realloc(self->events, newlen * sizeof(PyObject *));
	    if (newmem == NULL)
		return -1;
	    memset(newmem + self->events_len, 0, (newlen - self->events_len) * sizeof(PyObject *));
	    self->events = newmem;
	    self->events_len = newlen;
	}
	Py_INCREF(self->events[opcode] = type);
    }

    j = 0;
    while (PyDict_Next(errors, &j, &num, &type)) {
	opcode = ext->first_error + PyInt_AS_LONG(num);
	if (opcode >= self->errors_len) {
	    newlen = opcode + 1;
	    newmem = realloc(self->errors, newlen * sizeof(PyObject *));
	    if (newmem == NULL)
		return -1;
	    memset(newmem + self->errors_len, 0, (newlen - self->errors_len) * sizeof(PyObject *));
	    self->errors = newmem;
	    self->errors_len = newlen;
	}
	Py_INCREF(self->errors[opcode] = type);
    }

    return 0;
}

int
xpybConn_setup(xpybConn *self)
{
    PyObject *key, *events, *errors;
    xpybExt *ext = NULL;
    Py_ssize_t i = 0;
    int rc = -1;

    ext = (xpybExt *)xpybModule_core;
    events = xpybModule_core_events;
    errors = xpybModule_core_errors;
    if (xpybConn_setup_ext(self, ext, events, errors) < 0)
	return -1;

    while (PyDict_Next(xpybModule_ext_events, &i, &key, &events)) {
	errors = PyDict_GetItem(xpybModule_ext_errors, key);
	if (errors == NULL)
	    goto out;

	Py_XDECREF(ext);
	ext = (xpybExt *)PyObject_CallFunctionObjArgs((PyObject *)self, key, NULL);
	if (ext == NULL)
	    goto out;

	if (xpybConn_setup_ext(self, ext, events, errors) < 0)
	    goto out;
    }

    rc = 0;
out:
    Py_XDECREF(ext);
    return rc;
}

/*
 * Infrastructure
 */

static PyObject *
xpybConn_new(PyTypeObject *self, PyObject *args, PyObject *kw)
{
    return PyType_GenericNew(self, args, kw);
}

static void
xpybConn_dealloc(xpybConn *self)
{
    int i;

    Py_CLEAR(self->core);
    Py_CLEAR(self->extcache);

    if (self->conn)
	xcb_disconnect(self->conn);

    for (i = 0; i < self->events_len; i++)
	Py_XDECREF(self->events[i]);
    for (i = 0; i < self->errors_len; i++)
	Py_XDECREF(self->errors[i]);

    free(self->events);
    free(self->errors);
    self->ob_type->tp_free((PyObject *)self);
}

static PyObject *
xpybConn_getattro(xpybConn *self, PyObject *obj)
{
    PyObject *result, *core;

    result = PyObject_GenericGetAttr((PyObject *)self, obj);
    if (result != NULL || xpybModule_core == NULL)
	return result;

    core = self->core;
    return core->ob_type->tp_getattro(core, obj);
}

static PyObject *
xpybConn_call(xpybConn *self, PyObject *args, PyObject *kw)
{
    static char *kwlist[] = { "key", NULL };
    PyObject *ext, *key;

    /* Parse the extension key argument and check connection. */
    if (!PyArg_ParseTupleAndKeywords(args, kw, "O!", kwlist, &xpybExtkey_type, &key))
	return NULL;
    if (xpybConn_invalid(self))
	return NULL;

    /* Check our dictionary of cached values */
    ext = PyDict_GetItem(self->extcache, key);
    if (ext == NULL) {
	ext = xpybConn_make_ext(self, key);
	if (ext == NULL)
	    return NULL;
	if (PyDict_SetItem(self->extcache, key, ext) < 0)
	    return NULL;
    }

    Py_INCREF(ext);
    return ext;
}


/*
 * Members
 */

static PyMemberDef xpybConn_members[] = {
    { "pref_screen",
      T_INT,
      offsetof(xpybConn, pref_screen),
      READONLY,
      "Preferred display screen" },

    { NULL } /* terminator */
};


/*
 * Methods
 */

static PyObject *
xpybConn_flush(xpybConn *self, PyObject *args)
{
    if (xpybConn_invalid(self))
	return NULL;

    xcb_flush(self->conn);
    Py_RETURN_NONE;
}

static PyObject *
xpybConn_disconnect(xpybConn *self, PyObject *args)
{
    if (self->conn)
	xcb_disconnect(self->conn);
    self->conn = NULL;
    Py_RETURN_NONE;
}

static PyObject *
xpybConn_get_maximum_request_length(xpybConn *self, PyObject *args)
{
    if (xpybConn_invalid(self))
	return NULL;

    return Py_BuildValue("I", xcb_get_maximum_request_length(self->conn));
}

static PyObject *
xpybConn_prefetch_maximum_request_length(xpybConn *self, PyObject *args)
{
    if (xpybConn_invalid(self))
	return NULL;

    xcb_prefetch_maximum_request_length(self->conn);
    Py_RETURN_NONE;
}

static PyObject *
xpybConn_wait_for_event(xpybConn *self, PyObject *args)
{
    xcb_generic_event_t *data;

    if (xpybConn_invalid(self))
	return NULL;

    data = xcb_wait_for_event(self->conn);

    if (data == NULL) {
	PyErr_SetString(PyExc_IOError, "I/O error on X server connection.");
	return NULL;
    }

    if (data->response_type == 0) {
	xpybError_set(self, (xcb_generic_error_t *)data);
	return NULL;
    }

    return xpybEvent_create(self, data);
}

static PyObject *
xpybConn_poll_for_event(xpybConn *self, PyObject *args)
{
    xcb_generic_event_t *data;

    if (xpybConn_invalid(self))
	return NULL;

    data = xcb_poll_for_event(self->conn);

    if (data == NULL) {
	PyErr_SetString(PyExc_IOError, "I/O error on X server connection.");
	return NULL;
    }

    if (data->response_type == 0) {
	xpybError_set(self, (xcb_generic_error_t *)data);
	return NULL;
    }

    return xpybEvent_create(self, data);
}

static PyMethodDef xpybConn_methods[] = {
    { "flush",
      (PyCFunction)xpybConn_flush,
      METH_NOARGS,
      "Forces any buffered output to be written to the server." },

    { "disconnect",
      (PyCFunction)xpybConn_disconnect,
      METH_NOARGS,
      "Disconnects from the X server." },

    { "get_maximum_request_length",
      (PyCFunction)xpybConn_get_maximum_request_length,
      METH_NOARGS,
      "Returns the maximum request length that this server accepts." },

    { "prefetch_maximum_request_length",
      (PyCFunction)xpybConn_prefetch_maximum_request_length,
      METH_NOARGS,
      "Prefetch the maximum request length without blocking." },

    { "wait_for_event",
      (PyCFunction)xpybConn_wait_for_event,
      METH_NOARGS,
      "Returns the next event or raises the next error from the server." },

    { "poll_for_event",
      (PyCFunction)xpybConn_poll_for_event,
      METH_NOARGS,
      "Returns the next event or raises the next error from the server." },

    { NULL } /* terminator */
};


/*
 * Definition
 */

PyTypeObject xpybConn_type = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "xcb.Connection",
    .tp_basicsize = sizeof(xpybConn),
    .tp_new = xpybConn_new,
    .tp_dealloc = (destructor)xpybConn_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "XCB connection object",
    .tp_methods = xpybConn_methods,
    .tp_members = xpybConn_members,
    .tp_call = (ternaryfunc)xpybConn_call,
    .tp_getattro = (getattrofunc)xpybConn_getattro
};


/*
 * Module init
 */
int xpybConn_modinit(PyObject *m)
{
    if (PyType_Ready(&xpybConn_type) < 0)
        return -1;
    Py_INCREF(&xpybConn_type);
    if (PyModule_AddObject(m, "Connection", (PyObject *)&xpybConn_type) < 0)
	return -1;

    return 0;
}
