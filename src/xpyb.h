#ifndef XPYB_H
#define XPYB_H

#include <Python.h>
#include <xcb/xcb.h>

typedef struct {
    PyObject_HEAD
    xcb_connection_t *conn;
    int wrapped;
    PyObject *dict;
    int pref_screen;
    PyObject *core;
    PyObject *setup;
    PyObject *extcache;
    PyObject **events;
    int events_len;
    PyObject **errors;
    int errors_len;
} xpybConn;

typedef struct {
    PyTypeObject *xpybConn_type;
} xpyb_CAPI_t;

#define xpyb_IMPORT \
    xpyb_CAPI = (xpyb_CAPI_t *) PyCObject_Import("xcb", "CAPI")

#endif
