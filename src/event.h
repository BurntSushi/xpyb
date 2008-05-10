#ifndef XPYB_EVENT_H
#define XPYB_EVENT_H

#include "response.h"

typedef struct {
    xpybResponse response;
} xpybEvent;

extern PyTypeObject xpybEvent_type;

int xpybEvent_modinit(PyObject *m);

#endif
