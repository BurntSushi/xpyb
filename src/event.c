#include "module.h"
#include "except.h"
#include "response.h"
#include "event.h"

/*
 * Helpers
 */


/*
 * Infrastructure
 */


/*
 * Members
 */


/*
 * Definition
 */

PyTypeObject xpybEvent_type = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "xcb.Event",
    .tp_basicsize = sizeof(xpybEvent),
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "XCB generic event object",
    .tp_base = &xpybResponse_type
};


/*
 * Module init
 */
int xpybEvent_modinit(PyObject *m)
{
    if (PyType_Ready(&xpybEvent_type) < 0)
        return -1;
    Py_INCREF(&xpybEvent_type);
    if (PyModule_AddObject(m, "Event", (PyObject *)&xpybEvent_type) < 0)
	return -1;

    return 0;
}
