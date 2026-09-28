#pragma once
#include <xanim/xanim.h>

#include "mem_largelocal.h"

struct __declspec(align(4)) fileData_s // sizeof=0xC
{                                       // XREF: fileData_t/r
    void *data;
    fileData_s *next;
    unsigned __int8 type;
    char name[1];
    // padding byte
    // padding byte
};

struct hunkHeader_t // sizeof=0x10
{
    unsigned int magic;
    int size;
    const char *name;
    int dummy;
};

struct hunkUsed_t // sizeof=0x8
{                                       // XREF: .data:hunk_high/r
                                        // .data:hunk_low/r
    int permanent;                      // XREF: _Com_TouchMemory+53/r
                                        // _Com_TouchMemory+93/r ...
    int temp;                           // XREF: _Hunk_CheckTempMemoryClear:loc_7B2B10/r
                                        // _Hunk_CheckTempMemoryHighClear:loc_7B2BC0/r ...
};

void *__cdecl Z_VirtualReserve(int size);
void __cdecl Z_VirtualDecommitInternal(void *ptr, int size);
void __cdecl Z_VirtualFreeInternal(void *ptr);
void *__cdecl Z_TryVirtualAllocInternal(int size);
bool __cdecl Z_TryVirtualCommitInternal(void *ptr, int size);
void __cdecl Z_VirtualCommitInternal(void *ptr, int size);
void Com_TouchMemory();
void Com_InitHunkMemory();
void __cdecl Com_AllMemInfo_f();
char Hunk_CheckTempMemoryClear();
char Hunk_CheckTempMemoryHighClear();
void *__cdecl Hunk_FindDataForFile(int type, const char *name);
void *__cdecl Hunk_FindDataForFileInternal(int type, const char *name, int hash);
bool __cdecl Hunk_DataOnHunk(unsigned __int8 *data);
char *__cdecl Hunk_SetDataForFile(int type, const char *name, void *data, void *(__cdecl *alloc)(int));
void __cdecl Hunk_AddData(int type, void *data, void *(__cdecl *alloc)(int));
void __cdecl Hunk_OverrideDataForFile(int type, const char *name, void *data);
int __cdecl DB_GetAllXAssetOfType(XAssetType type, XAssetHeader *assets, int maxCount);
int __cdecl DB_GetAllXAssetOfType_LoadObj(XAssetType type, XAssetHeader *assets, int maxCount);
void __cdecl Hunk_AddAsset(XAssetHeader header, unsigned int *data);
void __cdecl DB_EnumXAssets(
                XAssetType type,
                void (__cdecl *func)(XAssetHeader, void *),
                void *inData,
                bool includeOverride);
void __cdecl DB_EnumXAssets_LoadObj(XAssetType type, void (__cdecl *func)(void *, void *), void *inData);
void __cdecl DB_EnumXAssetsFor(
                fileData_s *fileData,
                int fileDataType,
                void (__cdecl *func)(void *, void *),
                void *inData);
int __cdecl Hunk_SetMark();
void __cdecl Hunk_ClearToMark(int mark);
void Hunk_ClearData();
void __cdecl Hunk_ClearDataFor(fileData_s **pFileData, unsigned __int8 *low, unsigned __int8 *high);
void __cdecl Hunk_ClearToMarkLow(int mark);
void Hunk_Clear();
int __cdecl Hunk_Used();
unsigned __int8 *__cdecl Hunk_Alloc(unsigned int size, const char *name, int type);
unsigned __int8 *__cdecl Hunk_AllocAlign(unsigned int size, int alignment, const char *name, int type);
void *__cdecl Hunk_AllocateTempMemoryHigh(int size, const char *name);
void Hunk_ClearTempMemoryHigh();
unsigned __int8 *__cdecl Hunk_AllocLow(unsigned int size, const char *name, int type);
unsigned __int8 *__cdecl Hunk_AllocLowAlign(unsigned int size, int alignment, const char *name, int type);
unsigned int *__cdecl Hunk_AllocateTempMemory(int size, const char *name);
void __cdecl Hunk_FreeTempMemory(char *buf);
void Hunk_ClearTempMemory();
void __cdecl Com_TempMeminfo_f();
char *__cdecl Z_TryVirtualAlloc(int size, const char *name, int type);
char *__cdecl Z_VirtualAlloc(int size, const char *name, int type);
void __cdecl Z_VirtualCommit(char *ptr, int size, int type);
void __cdecl Z_VirtualFree(void *ptr, int type);
void __cdecl Z_VirtualDecommit(char *ptr, int size, int type);
void __cdecl Z_Free(char *ptr, int type);
unsigned int *__cdecl Z_TryMalloc(int size, const char *name, int type);
char *__cdecl Z_TryMallocGarbage(int size, const char *name, int type);
unsigned int *__cdecl Z_Malloc(int size, const char *name, int type);
void __cdecl    Z_MallocFailed(int size);
char *__cdecl Z_MallocGarbage(int size, const char *name, int type);
char *__cdecl CopyString(char *in, const char *name, int type, scriptInstance_t inst);
void __cdecl ReplaceString(const char **str, char *in, const char *name, int type, scriptInstance_t inst);
void __cdecl FreeString(const char *str, int type, scriptInstance_t inst);


