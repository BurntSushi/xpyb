#include "module.h"
#include "except.h"
#include "response.h"

/*
 * Helpers
 */


/*
 * Infrastructure
 */


/*
 * Members
 */

static PyObject *
xpybResponse_getattro(xpybProtobj *self, PyObject *obj)
{
    const char *name = PyString_AS_STRING(obj);
    xcb_generic_event_t *data = self->data;

    if (strcmp(name, "type") == 0)
	return Py_BuildValue("B", data->response_type);
    if (strcmp(name, "sequence") == 0)
	return Py_BuildValue("H", data->sequence);

    return xpybResponse_type.tp_base->tp_getattro((PyObject *)self, obj);
}


/*
 * Definition
 */

PyTypeObject xpybResponse_type = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "xcb.Response",
    .tp_basicsize = sizeof(xpybResponse),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "XCB generic response object",
    .tp_base = &xpybProtobj_type,
    .tp_getattro = (getattrofunc)xpybResponse_getattro
};


/*
 * Module init
 */
int xpybResponse_modinit(PyObject *m)
{
    if (PyType_Ready(&xpybResponse_type) < 0)
        return -1;
    Py_INCREF(&xpybResponse_type);
    if (PyModule_AddObject(m, "Response", (PyObject *)&xpybResponse_type) < 0)
	return -1;

    return 0;
}
