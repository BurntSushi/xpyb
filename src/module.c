#include "module.h"
#include "except.h"
#include "cookie.h"
#include "protobj.h"
#include "response.h"
#include "event.h"
#include "error.h"
#include "reply.h"
#include "request.h"
#include "struct.h"
#include "union.h"
#include "list.h"
#include "conn.h"
#include "extkey.h"
#include "ext.h"
#include "void.h"


/*
 * Globals
 */

PyTypeObject *xpybModule_core;
PyObject *xpybModule_core_events;
PyObject *xpybModule_core_errors;

PyObject *xpybModule_extdict;
PyObject *xpybModule_ext_events;
PyObject *xpybModule_ext_errors;


/*
 * Module functions
 */

static PyObject *
xpyb_connect(PyObject *self, PyObject *args, PyObject *kw)
{
    static char *kwlist[] = { "display", NULL };
    const char *displayname = NULL;
    xpybConn *conn;

    /* Parse arguments and allocate new objects */
    if (!PyArg_ParseTupleAndKeywords(args, kw, "|z", kwlist, &displayname))
	return NULL;
    if ((conn = PyObject_New(xpybConn, &xpybConn_type)) == NULL)
	return NULL;
    if ((conn->core = xpybConn_make_core(conn)) == NULL)
	goto err;
    if ((conn->extcache = PyDict_New()) == NULL)
	goto err;

    /* Connect to display */
    conn->conn = xcb_connect(displayname, &conn->pref_screen);
    if (xcb_connection_has_error(conn->conn)) {
	PyErr_SetString(xpybExcept_conn, "Failed to connect to X server.");
	goto err;
    }

    /* Set up events and errors */
    conn->events = NULL;
    conn->events_len = 0;
    conn->errors = NULL;
    conn->errors_len = 0;

    if (xpybConn_setup(conn) < 0)
	goto err;

    return (PyObject *)conn;
err:
    Py_DECREF(conn);
    return NULL;
}

static PyObject *
xpyb_add_core(PyObject *self, PyObject *args)
{
    PyTypeObject *value;
    PyObject *events, *errors;

    if (xpybModule_core != NULL)
	Py_RETURN_NONE;

    if (!PyArg_ParseTuple(args, "O!O!O!", &PyType_Type, &value,
			  &PyDict_Type, &events, &PyDict_Type, &errors))
	return NULL;

    if (!PyType_IsSubtype(value, &xpybExt_type)) {
	PyErr_SetString(xpybExcept_base, "Type not derived from xcb.Extension.");
	return NULL;
    }

    Py_INCREF(xpybModule_core = value);
    Py_INCREF(xpybModule_core_events = events);
    Py_INCREF(xpybModule_core_errors = errors);
    Py_RETURN_NONE;
}

static PyObject *
xpyb_add_ext(PyObject *self, PyObject *args)
{
    PyTypeObject *value;
    PyObject *key, *events, *errors;

    if (!PyArg_ParseTuple(args, "O!O!O!O!", &xpybExtkey_type, &key, &PyType_Type, &value,
			  &PyDict_Type, &events, &PyDict_Type, &errors))
	return NULL;

    if (!PyType_IsSubtype(value, &xpybExt_type)) {
	PyErr_SetString(xpybExcept_base, "Type not derived from xcb.Extension.");
	return NULL;
    }

    if (PyDict_SetItem(xpybModule_extdict, key, (PyObject *)value) < 0)
	return NULL;
    if (PyDict_SetItem(xpybModule_ext_events, key, events) < 0)
	return NULL;
    if (PyDict_SetItem(xpybModule_ext_errors, key, errors) < 0)
	return NULL;

    Py_RETURN_NONE;
}

static PyObject *
xpyb_resize_obj(PyObject *self, PyObject *args)
{
    xpybProtobj *obj;
    Py_ssize_t size;
    PyObject *buf;

    if (!PyArg_ParseTuple(args, "O!i", &xpybProtobj_type, &obj, &size))
	return NULL;

    buf = PyBuffer_FromObject(obj->buf, 0, size);
    if (buf == NULL)
	return NULL;

    Py_CLEAR(obj->buf);
    obj->buf = buf;

    Py_RETURN_NONE;
}

static PyMethodDef XCBMethods[] = {
    { "connect",
      (PyCFunction)xpyb_connect,
      METH_VARARGS | METH_KEYWORDS,
      "Connects to the X server." },

    { "_add_core",
      (PyCFunction)xpyb_add_core,
      METH_VARARGS,
      "Registers the core protocol class.  Not meant for end users." },

    { "_add_ext",
      (PyCFunction)xpyb_add_ext,
      METH_VARARGS,
      "Registers a new extension protocol class.  Not meant for end users." },

    { "_resize_obj",
      (PyCFunction)xpyb_resize_obj,
      METH_VARARGS,
      "Sizes a protocol object after size determination.  Not meant for end users." },

    { NULL } /* terminator */
};


/*
 * Module init
 */

PyMODINIT_FUNC
initxcb(void) 
{
    /* Create module object */
    PyObject *m = Py_InitModule3("xcb", XCBMethods, "XCB Python Binding.");
    if (m == NULL)
	return;

    /* Create other internal objects */
    if ((xpybModule_extdict = PyDict_New()) == NULL)
	return;
    if ((xpybModule_ext_events = PyDict_New()) == NULL)
	return;
    if ((xpybModule_ext_errors = PyDict_New()) == NULL)
	return;

    /* Set up all the types */
    if (xpybExcept_modinit(m) < 0)
	return;
    if (xpybConn_modinit(m) < 0)
	return;
    if (xpybCookie_modinit(m) < 0)
	return;

    if (xpybExtkey_modinit(m) < 0)
	return;
    if (xpybExt_modinit(m) < 0)
	return;

    if (xpybProtobj_modinit(m) < 0)
	return;
    if (xpybResponse_modinit(m) < 0)
	return;
    if (xpybEvent_modinit(m) < 0)
	return;
    if (xpybError_modinit(m) < 0)
	return;
    if (xpybReply_modinit(m) < 0)
	return;
    if (xpybRequest_modinit(m) < 0)
	return;
    if (xpybStruct_modinit(m) < 0)
	return;
    if (xpybUnion_modinit(m) < 0)
	return;
    if (xpybList_modinit(m) < 0)
	return;

    if (xpybVoid_modinit(m) < 0)
	return;
}
