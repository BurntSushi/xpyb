#include "module.h"
#include "except.h"
#include "conn.h"
#include "cookie.h"
#include "error.h"
#include "reply.h"

/*
 * Helpers
 */


/*
 * Infrastructure
 */

static PyObject *
xpybCookie_new(PyTypeObject *self, PyObject *args, PyObject *kw)
{
    return PyType_GenericNew(self, args, kw);
}

static void
xpybCookie_dealloc(xpybCookie *self)
{
    Py_CLEAR(self->reply);
    Py_CLEAR(self->request);
    Py_CLEAR(self->conn);
    self->ob_type->tp_free((PyObject *)self);
}


/*
 * Members
 */


/*
 * Methods
 */

static PyObject *
xpybCookie_check(xpybCookie *self, PyObject *args)
{
    xcb_generic_error_t *error;

    if (!(self->request->is_void && self->request->is_checked)) {
	PyErr_SetString(xpybExcept_base, "Request is not void and checked.");
	return NULL;
    }
    if (xpybConn_invalid(self->conn))
	return NULL;

    error = xcb_request_check(self->conn->conn, self->cookie);
    if (xpybError_set(error))
	return NULL;

    Py_RETURN_NONE;
}

static PyObject *
xpybCookie_reply(xpybCookie *self, PyObject *args)
{
    xcb_generic_error_t *error;
    void *data;

    if (self->request->is_void) {
	PyErr_SetString(xpybExcept_base, "Request has no reply.");
	return NULL;
    }
    if (xpybConn_invalid(self->conn))
	return NULL;

    data = xcb_wait_for_reply(self->conn->conn, self->cookie.sequence, &error);
    if (xpybError_set(error))
	return NULL;

    if (xpybReply_populate(self->reply, data) < 0)
	return NULL;

    Py_INCREF((PyObject *)self->reply);
    return (PyObject *)self->reply;
}

static PyMethodDef xpybCookie_methods[] = {
    { "check",
      (PyCFunction)xpybCookie_check,
      METH_NOARGS,
      "Raise an error if one occurred on the request." },

    { "reply",
      (PyCFunction)xpybCookie_reply,
      METH_NOARGS,
      "Return the reply or raise an error." },

    { NULL } /* terminator */
};


/*
 * Definition
 */

PyTypeObject xpybCookie_type = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "xcb.Cookie",
    .tp_basicsize = sizeof(xpybCookie),
    .tp_new = xpybCookie_new,
    .tp_dealloc = (destructor)xpybCookie_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "XCB generic cookie object",
    .tp_methods = xpybCookie_methods
};


/*
 * Module init
 */
int xpybCookie_modinit(PyObject *m)
{
    if (PyType_Ready(&xpybCookie_type) < 0)
        return -1;
    Py_INCREF(&xpybCookie_type);
    if (PyModule_AddObject(m, "Cookie", (PyObject *)&xpybCookie_type) < 0)
	return -1;

    return 0;
}
