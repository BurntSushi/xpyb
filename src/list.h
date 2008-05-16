#ifndef XPYB_LIST_H
#define XPYB_LIST_H

#include "protobj.h"

typedef struct {
    xpybProtobj base;
    PyObject *list;
} xpybList;

extern PyTypeObject xpybList_type;

int xpybList_modinit(PyObject *m);

#endif
