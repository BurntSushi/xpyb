#include "module.h"
#include "except.h"
#include "list.h"

/*
 * Helpers
 */

static PyObject *
xpybList_build(PyObject *str, Py_ssize_t size, const char *data)
{
    switch (PyString_AS_STRING(str)[0]) {
    case 'b':
	return Py_BuildValue("b", *data);
    case 'B':
	return Py_BuildValue("B", *(unsigned char *)data);
    case 'h':
	return Py_BuildValue("h", *(short *)data);
    case 'H':
	return Py_BuildValue("H", *(unsigned short *)data);
    case 'i':
	return Py_BuildValue("i", *(int *)data);
    case 'I':
	return Py_BuildValue("I", *(unsigned int *)data);
    case 'L':
	return Py_BuildValue("L", *(long long *)data);
    case 'K':
	return Py_BuildValue("K", *(unsigned long long *)data);
    case 'f':
	return Py_BuildValue("f", *(float *)data);
    case 'd':
	return Py_BuildValue("d", *(double *)data);
    default:
	PyErr_SetString(xpybExcept_base, "Invalid format character.");
    }

    return NULL;
}


/*
 * Infrastructure
 */

static int
xpybList_init(xpybList *self, PyObject *args, PyObject *kw)
{
    static char *kwlist[] = { "parent", "offset", "length", "type", "size", NULL };
    Py_ssize_t i, datalen, cur, offset, length, size = -1;
    xpybProtobj *parent, *sobj;
    PyObject *type, *obj, *arglist;
    const char *data;

    if (!PyArg_ParseTupleAndKeywords(args, kw, "O!iiO|i", kwlist,
				     &xpybProtobj_type, &parent,
				     &offset, &length, &type, &size))
	return -1;

    self->list = PyList_New(0);
    if (self->list == NULL)
	return -1;

    if (PyObject_AsReadBuffer(parent->buf, (const void **)&data, &datalen) < 0)
	return -1;
    if (length * size - offset > datalen) {
	PyErr_SetString(xpybExcept_base, "Protocol object buffer too short.");
	return -1;
    }

    cur = offset;

    for (i = 0; i < length; i++) {
	if (PyString_CheckExact(type)) {
	    obj = xpybList_build(type, length, data + cur);
	    if (obj == NULL)
		return -1;
	    cur += size;
	} else if (size > 0) {
	    arglist = Py_BuildValue("(Oii)", parent, cur, size);
	    obj = PyEval_CallObject(type, arglist);
	    Py_DECREF(arglist);
	    if (obj == NULL)
		return -1;
	    cur += size;
	} else {
	    arglist = Py_BuildValue("(Oi)", parent, cur);
	    obj = PyEval_CallObject(type, arglist);
	    Py_DECREF(arglist);
	    if (obj == NULL)
		return -1;
	    datalen = PySequence_Size(obj);
	    if (datalen < 0)
		return -1;
	    cur += datalen;
	}

	if (PyList_Append(self->list, obj) < 0)
	    return -1;
    }

    sobj = (xpybProtobj *)self;
    sobj->buf = PyBuffer_FromObject(parent->buf, offset, cur);
    if (sobj->buf == NULL)
	return -1;

    Py_INCREF(sobj->parent = (PyObject *)parent);
    return 0;
}

static void
xpybList_dealloc(xpybList *self)
{
    Py_CLEAR(self->list);
    xpybList_type.tp_base->tp_dealloc((PyObject *)self);
}


/*
 * Members
 */


/*
 * Definition
 */

PyTypeObject xpybList_type = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "xcb.List",
    .tp_basicsize = sizeof(xpybList),
    .tp_init = (initproc)xpybList_init,
    .tp_dealloc = (destructor)xpybList_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "XCB generic list object",
    .tp_base = &xpybProtobj_type,
};


/*
 * Module init
 */
int xpybList_modinit(PyObject *m)
{
    if (PyType_Ready(&xpybList_type) < 0)
        return -1;
    Py_INCREF(&xpybList_type);
    if (PyModule_AddObject(m, "List", (PyObject *)&xpybList_type) < 0)
	return -1;

    return 0;
}
