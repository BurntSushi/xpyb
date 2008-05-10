#include "module.h"
#include "except.h"
#include "protobj.h"

/*
 * Helpers
 */

PyObject *
xpybProtobj_create(PyTypeObject *type, void *data, Py_ssize_t size)
{
    xpybProtobj *obj;
    PyObject *buf;

    obj = PyObject_New(xpybProtobj, type);
    if (obj == NULL)
	return NULL;
    buf = PyBuffer_FromMemory(data, size);
    if (buf == NULL)
	return NULL;

    obj->parent = NULL;
    obj->buf = buf;
    obj->data = data;
    return (PyObject *)obj;
}


/*
 * Infrastructure
 */

static PyObject *
xpybProtobj_new(PyTypeObject *self, PyObject *args, PyObject *kw)
{
    return PyType_GenericNew(self, args, kw);
}

static int
xpybProtobj_init(xpybProtobj *self, PyObject *args, PyObject *kw)
{
    static char *kwlist[] = { "parent", "offset", "size", NULL };
    Py_ssize_t offset, size = Py_END_OF_BUFFER;
    xpybProtobj *parent;

    if (!PyArg_ParseTupleAndKeywords(args, kw, "O!i|i", kwlist,
				     &xpybProtobj_type, &parent,
				     &offset, &size))
	return -1;

    self->buf = PyBuffer_FromObject(parent->buf, offset, size);
    if (self->buf == NULL)
	return -1;

    Py_INCREF(self->parent = (PyObject *)parent);
    return 0;
}

static void
xpybProtobj_dealloc(xpybProtobj *self)
{
    Py_CLEAR(self->buf);
    Py_CLEAR(self->parent);
    free(self->data);
    self->ob_type->tp_free((PyObject *)self);
}

static Py_ssize_t
xpybProtobj_readbuf(xpybProtobj *self, Py_ssize_t s, void **p)
{
    return PyBuffer_Type.tp_as_buffer->bf_getreadbuffer(self->buf, s, p);
}

static Py_ssize_t
xpybProtobj_segcount(xpybProtobj *self, Py_ssize_t *s)
{
    return PyBuffer_Type.tp_as_buffer->bf_getsegcount(self->buf, s);
}

static Py_ssize_t
xpybProtobj_charbuf(xpybProtobj *self, Py_ssize_t s, char **p)
{
    return PyBuffer_Type.tp_as_buffer->bf_getcharbuffer(self->buf, s, p);
}

/*
 * Members
 */


/*
 * Methods
 */


/*
 * Definition
 */

static PyBufferProcs xpybProtobj_bufops = {
    .bf_getreadbuffer = (readbufferproc)xpybProtobj_readbuf,
    .bf_getsegcount = (segcountproc)xpybProtobj_segcount,
    .bf_getcharbuffer = (charbufferproc)xpybProtobj_charbuf
};

PyTypeObject xpybProtobj_type = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "xcb.Protobj",
    .tp_basicsize = sizeof(xpybProtobj),
    .tp_init = (initproc)xpybProtobj_init,
    .tp_new = xpybProtobj_new,
    .tp_dealloc = (destructor)xpybProtobj_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "XCB generic X protocol object",
    .tp_as_buffer = &xpybProtobj_bufops
};


/*
 * Module init
 */
int xpybProtobj_modinit(PyObject *m)
{
    if (PyType_Ready(&xpybProtobj_type) < 0)
        return -1;
    Py_INCREF(&xpybProtobj_type);
    if (PyModule_AddObject(m, "Protobj", (PyObject *)&xpybProtobj_type) < 0)
	return -1;

    return 0;
}
