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

static PyObject *
xpybConn_make_ext(xpybConn *self, PyObject *key)
{
    PyObject *result, *arglist;
    xpybExt *ext;
    const xcb_query_extension_reply_t *reply;

    /* Look up the callable object in the global dictionary. */
    result = PyDict_GetItem(xpybModule_extdict, key);
    if (result == NULL) {
	PyErr_SetString(xpybExcept_ext, "No extension found for that key.");
	return NULL;
    }

    /* Call the object to get a new xcb.Extension object. */
    if (key == Py_None)
	arglist = Py_BuildValue("(O)", self);
    else
	arglist = Py_BuildValue("(OO)", self, key);

    if (arglist == NULL)
	return NULL;
    ext = (xpybExt *)PyEval_CallObject(result, arglist);
    Py_DECREF(arglist);
    if (ext == NULL)
	return NULL;

    /* Get the opcode and base numbers for actual (non-core) extensions. */
    if (key != Py_None) {
	reply = xcb_get_extension_data(self->conn, &((xpybExtkey *)key)->key);
	if (!reply->present) {
	    PyErr_SetString(xpybExcept_ext, "Extension not present on server.");
	    Py_DECREF(ext);
	    return NULL;
	}
	ext->major_opcode = reply->major_opcode;
	ext->first_event = reply->first_event;
	ext->first_error = reply->first_error;
    }

    return (PyObject *)ext;
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
    Py_CLEAR(self->extcache);

    if (self->conn)
	xcb_disconnect(self->conn);

    self->ob_type->tp_free((PyObject *)self);
}

static PyObject *
xpybConn_call(xpybConn *self, PyObject *args, PyObject *kw)
{
    static char *kwlist[] = { "key", NULL };
    PyObject *ext, *key = Py_None;

    /* Parse the extension key argument and check connection. */
    if (!PyArg_ParseTupleAndKeywords(args, kw, "|O!", kwlist, &xpybExtkey_type, &key))
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
	xpybError_set((xcb_generic_error_t *)data);
	return NULL;
    }

    return xpybProtobj_create(&xpybEvent_type, data, sizeof(*data));
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
	xpybError_set((xcb_generic_error_t *)data);
	return NULL;
    }

    return xpybProtobj_create(&xpybEvent_type, data, sizeof(*data));
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
    .tp_call = (ternaryfunc)xpybConn_call
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
