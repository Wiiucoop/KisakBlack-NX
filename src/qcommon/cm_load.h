#pragma once

#include "cm_trace.h"

void __cdecl CM_LoadMap(const char *name, int *checksum);
void CM_InitAllThreadData();
void __cdecl CM_InitThreadData(unsigned int threadContext);
void __cdecl CM_LoadMapData(const char *name);
void __cdecl CM_LoadMapData_LoadObj(const char *name);
void __cdecl CM_LoadMapData_FastFile(const char *name);
void __cdecl Rope_InitRopes();
void __cdecl ShutdownRopes();
void __cdecl CM_Shutdown();
void __cdecl CM_Unload();
int __cdecl CM_LeafCluster(unsigned int leafnum);
void __cdecl CM_ModelBounds(unsigned int model, float *mins, float *maxs);
int __cdecl CM_ModelSurfaceFlags(unsigned int model);
unsigned __int8 *__cdecl CM_Hunk_Alloc(unsigned int size, const char *name, int type);
void __cdecl CM_Hunk_CheckTempMemoryHighClear();
void *__cdecl CM_Hunk_AllocateTempMemoryHigh(int size, const char *name);
void __cdecl CM_Hunk_ClearTempMemoryHigh();


extern clipMap_t cm;

extern int num_occlusion_overrides;
extern float occlusion_overrides[32][2][3];

extern int num_undertow_volumes;
extern int undertow_volumes[32];
extern float undertow_origins[32][3];
extern float undertow_forces[32][4];