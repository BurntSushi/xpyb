#ifndef XPYB_CONN_H
#define XPYB_CONN_H

typedef struct {
    PyObject_HEAD
    int pref_screen;
    xcb_connection_t *conn;
    PyObject *extcache;
    PyObject *core;
} xpybConn;

extern PyTypeObject xpybConn_type;

int xpybConn_invalid(xpybConn *self);
PyObject *xpybConn_make_core(xpybConn *self);

int xpybConn_modinit(PyObject *m);

#endif
