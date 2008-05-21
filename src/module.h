#ifndef XPYB_MODULE_H
#define XPYB_MODULE_H

#include <Python.h>
#include <structmember.h>

#include <xcb/xcb.h>
#include <xcb/xcbext.h>

extern PyObject *xpybModule_extdict;
extern PyTypeObject *xpybModule_core;

PyMODINIT_FUNC initxcb(void);

#endif
