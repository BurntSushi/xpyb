#ifndef XPYB_CONN_H
#define XPYB_CONN_H

#include "xpyb.h"

extern PyTypeObject xpybConn_type;

int xpybConn_invalid(xpybConn *self);
xpybConn *xpybConn_create(PyObject *core_type);
int xpybConn_setup(xpybConn *self);

int xpybConn_modinit(PyObject *m);

#endif
