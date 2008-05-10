#ifndef XPYB_ERROR_H
#define XPYB_ERROR_H

#include "response.h"

typedef struct {
    xpybResponse response;
} xpybError;

extern PyTypeObject xpybError_type;

int xpybError_set(xcb_generic_error_t *e);

int xpybError_modinit(PyObject *m);

#endif
