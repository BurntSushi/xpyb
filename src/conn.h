#ifndef XPYB_CONN_H
#define XPYB_CONN_H

typedef struct {
    PyObject_HEAD
    xcb_connection_t *conn;
    int pref_screen;
    PyObject *core;
    PyObject *setup;
    PyObject *extcache;
    PyObject **events;
    int events_len;
    PyObject **errors;
    int errors_len;
} xpybConn;

extern PyTypeObject xpybConn_type;

int xpybConn_invalid(xpybConn *self);
int xpybConn_setup(xpybConn *self);
PyObject *xpybConn_make_core(xpybConn *self);

int xpybConn_modinit(PyObject *m);

#endif
