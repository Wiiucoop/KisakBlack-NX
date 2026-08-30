#ifndef NX_COMPAT_DXERR_H
#define NX_COMPAT_DXERR_H

#include "windows.h"

const char *DXGetErrorDescriptionA(HRESULT hr);
const char *DXGetErrorStringA(HRESULT hr);

#endif
