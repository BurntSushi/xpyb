#include "module.h"
#include "except.h"
#include "constant.h"

int xpybConstant_modinit(PyObject *m)
{
    /* Basic constants */
    PyModule_AddIntConstant(m, "X_PROTOCOL", X_PROTOCOL);
    PyModule_AddIntConstant(m, "X_PROTOCOL_REVISION", X_PROTOCOL_REVISION);
    PyModule_AddIntConstant(m, "X_TCP_PORT", X_TCP_PORT);
    PyModule_AddIntConstant(m, "NONE", XCB_NONE);
    PyModule_AddIntConstant(m, "CopyFromParent", XCB_COPY_FROM_PARENT);
    PyModule_AddIntConstant(m, "CurrentTime", XCB_CURRENT_TIME);
    PyModule_AddIntConstant(m, "NoSymbol", XCB_NO_SYMBOL);

    return 0;
}
