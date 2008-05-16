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

PyObject *xpybModule_extdict;


/*
 * Module functions
 */

static PyObject *
xpyb_connect(PyObject *self, PyObject *args, PyObject *kw)
{
    static char *kwlist[] = { "display", NULL };
    const char *displayname = NULL;
    int screenp;
    xcb_connection_t *c;
    xpybConn *conn = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kw, "|z", kwlist, &displayname))
	return NULL;

    c = xcb_connect(displayname, &screenp);
    if (xcb_connection_has_error(c))
	PyErr_SetString(xpybExcept_conn, "Failed to connect to X server.");
    else
	conn = PyObject_New(xpybConn, &xpybConn_type);

    if (conn != NULL) {
	conn->pref_screen = screenp;
	conn->conn = c;
    }
    return (PyObject *)conn;
}

static PyObject *
xpyb_add_core(PyObject *self, PyObject *args)
{
    PyObject *value;

    if (!PyArg_ParseTuple(args, "O!", &PyType_Type, &value))
	return NULL;

    if (PyDict_SetItem(xpybModule_extdict, Py_None, value) < 0)
	return NULL;

    Py_RETURN_NONE;
}

static PyObject *
xpyb_add_ext(PyObject *self, PyObject *args)
{
    xpybExtkey *key;
    PyObject *value;

    if (!PyArg_ParseTuple(args, "O!O!", &xpybExtkey_type, &key, &PyType_Type, &value))
	return NULL;

    if (PyDict_SetItem(xpybModule_extdict, (PyObject *)key, value) < 0)
	return NULL;

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
