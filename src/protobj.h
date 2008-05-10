#ifndef XPYB_PROTOBJ_H
#define XPYB_PROTOBJ_H

typedef struct {
    PyObject_HEAD
    PyObject *parent;
    PyObject *buf;
    void *data;
} xpybProtobj;

extern PyTypeObject xpybProtobj_type;

PyObject *xpybProtobj_create(PyTypeObject *type, void *data, Py_ssize_t size);

int xpybProtobj_modinit(PyObject *m);

#endif
