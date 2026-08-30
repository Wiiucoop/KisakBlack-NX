// nvapi.h -- NVAPI stub declarations for the Switch build (shadows
// src/nvapi/nvapi.h via include-path priority). All functions are
// implemented in src/nx/nx_stubs.cpp and fail, which routes the renderer
// down its vanilla non-NVIDIA paths.
#ifndef NX_COMPAT_NVAPI_H
#define NX_COMPAT_NVAPI_H

#include "../windows.h"

typedef unsigned char  NvU8;
typedef unsigned short NvU16;
typedef unsigned int   NvU32;

typedef enum _NvAPI_Status {
    NVAPI_OK = 0,
    NVAPI_ERROR = -1,
    NVAPI_LIBRARY_NOT_FOUND = -2,
    NVAPI_NO_IMPLEMENTATION = -3,
    NVAPI_API_NOT_INITIALIZED = -4,
    NVAPI_INVALID_ARGUMENT = -5,
} NvAPI_Status;

#define NVAPI_INTERFACE NvAPI_Status

typedef void *StereoHandle;

typedef struct NVDX_ObjectHandle__ { int unused; } *NVDX_ObjectHandle;
#define NVDX_OBJECT_NONE ((NVDX_ObjectHandle)0)

#define NVAPI_MAX_PHYSICAL_GPUS 64
#define NVAPI_SHORT_STRING_MAX 64

typedef char NvAPI_ShortString[NVAPI_SHORT_STRING_MAX];

typedef struct {
    NvU32 version;
    NvU32 drvVersion;
    NvU32 bldChangeListNum;
    NvAPI_ShortString szBuildBranchString;
    NvAPI_ShortString szAdapterString;
} NV_DISPLAY_DRIVER_VERSION;
#define NV_DISPLAY_DRIVER_VERSION_VER (sizeof(NV_DISPLAY_DRIVER_VERSION) | (1u << 16))

typedef struct {
    NvU32 version;
    NvU32 maxNumAFRGroups;
    NvU32 numAFRGroups;
    NvU32 currentAFRIndex;
    NvU32 nextFrameAFRIndex;
    NvU32 previousFrameAFRIndex;
    NvU32 bIsCurAFRGroupNew;
} NV_GET_CURRENT_SLI_STATE;
#define NV_GET_CURRENT_SLI_STATE_VER (sizeof(NV_GET_CURRENT_SLI_STATE) | (1u << 16))

NVAPI_INTERFACE NvAPI_Initialize(void);
NVAPI_INTERFACE NvAPI_Unload(void);
NVAPI_INTERFACE NvAPI_GetDisplayDriverVersion(void *displayHandle, NV_DISPLAY_DRIVER_VERSION *version);
NVAPI_INTERFACE NvAPI_Stereo_CreateHandleFromIUnknown(void *device, StereoHandle *handle);
NVAPI_INTERFACE NvAPI_Stereo_DestroyHandle(StereoHandle handle);
NVAPI_INTERFACE NvAPI_Stereo_IsActivated(StereoHandle handle, NvU8 *activated);
NVAPI_INTERFACE NvAPI_Stereo_IsEnabled(NvU8 *enabled);
NVAPI_INTERFACE NvAPI_Stereo_SetConvergence(StereoHandle handle, float convergence);
NVAPI_INTERFACE NvAPI_D3D9_GetTextureHandle(void *texture, NVDX_ObjectHandle *handle);
NVAPI_INTERFACE NvAPI_D3D9_GetCurrentZBufferHandle(void *device, NVDX_ObjectHandle *handle);
NVAPI_INTERFACE NvAPI_D3D9_StretchRect(void *device, NVDX_ObjectHandle src, const RECT *srcRect,
                                       NVDX_ObjectHandle dst, const RECT *dstRect, int filter);
NVAPI_INTERFACE NvAPI_D3D_GetCurrentSLIState(void *device, NV_GET_CURRENT_SLI_STATE *sliState);

#endif // NX_COMPAT_NVAPI_H
