#include "com_memory.h"
#include "assertive.h"

#include <Windows.h> // VirtualAlloc
#include <win32/win_main.h>
#include <qcommon/threads.h>

// nx-port: page arithmetic on hunk addresses, pointer-sized. The decompiled
// form was ((unsigned int)ptr & 0xFFFFF000), which on LP64 committed memory at
// the truncated address -- the first map load died in Z_VirtualCommit on it.
#define HUNK_PAGE_DOWN(p) ((unsigned __int8 *)((uintptr_t)(p) & ~(uintptr_t)0xFFF))
#include <win32/win_shared.h>
#include "com_files.h"
#include <qcommon/common.h>
#include <qcommon/mem_track.h>
#include <gfx_d3d/r_reflection_probe.h>
#include <qcommon/cmd.h>
#include <database/db_registry.h>
#include <gfx_d3d/r_image.h>
#include <xanim/xmodel.h>
#include <clientscript/cscr_stringlist.h>
#include <gfx_d3d/r_dvars.h>

hunkUsed_t hunk_low;
unsigned __int8 *s_hunkData;
int s_hunkTotal;
hunkUsed_t hunk_high;
unsigned __int8 *s_origHunkData;
fileData_s *com_fileDataHashTable[1024];
fileData_s *com_hunkData;


struct AssetList // sizeof=0xC
{                                       // XREF: DB_GetAllXAssetOfType_LoadObj/r
    int assetCount;                     // XREF: DB_GetAllXAssetOfType_LoadObj+C/w
    int maxCount;                       // XREF: DB_GetAllXAssetOfType_LoadObj+16/w
    XAssetHeader *assets;               // XREF: DB_GetAllXAssetOfType_LoadObj+9/w
};



void *__cdecl Z_VirtualReserve(int size)
{
    void *buf; // [esp+0h] [ebp-4h]

    if ( size <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    177,
                    0,
                    "%s\n\t(size) = %i",
                    "(size > 0)",
                    size) )
    {
        __debugbreak();
    }
    buf = VirtualAlloc(0, size, (size > 0x20000 ? 0 : 0x100000) | 0x2000, 4u);
    if ( !buf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 309, 0, "%s", "buf") )
        __debugbreak();
    return buf;
}

void __cdecl Z_VirtualDecommitInternal(void *ptr, int size)
{
    if ( size < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    340,
                    0,
                    "%s\n\t(size) = %i",
                    "(size >= 0)",
                    size) )
    {
        __debugbreak();
    }
    VirtualFree(ptr, size, 0x4000u);
}

void __cdecl Z_VirtualFreeInternal(void *ptr)
{
    VirtualFree(ptr, 0, 0x8000u);
}

void *__cdecl Z_TryVirtualAllocInternal(int size)
{
    void *ptr; // [esp+0h] [ebp-4h]

#ifdef KISAK_NX
    // Reserving is backed by real memory here and can fail; this is the path
    // whose callers handle a NULL (R_DebugAlloc), so do not assert.
    ptr = VirtualAlloc(0, size, MEM_RESERVE, PAGE_READWRITE);
#else
    ptr = Z_VirtualReserve(size);
#endif
    if ( ptr )
    {
        if ( Z_TryVirtualCommitInternal(ptr, size) )
            return ptr;
        Z_VirtualFree(ptr, 0);
    }
    return 0;
}

bool __cdecl Z_TryVirtualCommitInternal(void *ptr, int size)
{
    if ( size < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    318,
                    0,
                    "%s\n\t(size) = %i",
                    "(size >= 0)",
                    size) )
    {
        __debugbreak();
    }
    return VirtualAlloc(ptr, size, (size > 0x20000 ? 0 : 0x100000) | 0x1000, 4u) != 0;
}

void __cdecl Z_VirtualCommitInternal(void *ptr, int size)
{
    if ( !Z_TryVirtualCommitInternal(ptr, size) )
        Sys_OutOfMemErrorInternal("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 420);
}

void Com_TouchMemory()
{
    int sum; // [esp+4h] [ebp-10h]
    DWORD start; // [esp+8h] [ebp-Ch]
    DWORD end; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]
    int ia; // [esp+10h] [ebp-4h]

    if (!Sys_IsMainThread()
        && !Sys_IsRenderThread()
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
            518,
            0,
            "%s",
            "Sys_IsMainThread() || Sys_IsRenderThread()"))
    {
        __debugbreak();
    }
    start = Sys_Milliseconds();
    sum = 0;
    for (i = 0; i < hunk_low.permanent >> 2; i += 64)
        sum += *(_DWORD *)&s_hunkData[4 * i];
    for (ia = (s_hunkTotal - hunk_high.permanent) >> 2; ia < hunk_high.permanent >> 2; ia += 64)
        sum += *(_DWORD *)&s_hunkData[4 * ia];
    end = Sys_Milliseconds();
    Com_Printf(16, "Com_TouchMemory: %i msec. Using sum: %d\n", end - start, sum);
}

cmd_function_s Com_AllMemInfo_f_VAR;
cmd_function_s Com_TempMeminfo_f_VAR;
void Com_InitHunkMemory()
{
    if (!Sys_IsMainThread()
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
            548,
            0,
            "%s",
            "Sys_IsMainThread()"))
    {
        __debugbreak();
    }
    if (s_hunkData
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 549, 0, "%s", "!s_hunkData"))
    {
        __debugbreak();
    }
    if (FS_LoadStack())
        Com_Error(ERR_FATAL, "Hunk initialization failed. File system load stack not zero");
    //if (!useFastFile->current.enabled)
    //    s_hunkTotal = 0xA000000;
    //if (useFastFile->current.enabled)
    //    s_hunkTotal = 0xF00000;

    if (IsFastFileLoad())
    {
        //s_hunkTotal = IsDedicatedServer() ? 0xF00000 : 0x500000;
        s_hunkTotal = 0xF00000; // LWSS: im just gonna give it the higher cap
    }
    else
    {
        s_hunkTotal = 0xA000000;
    }

    R_ReflectionProbeRegisterDvars();
    if (r_reflectionProbeGenerate->current.enabled)
        s_hunkTotal = 0x12C00000;
    s_hunkData = (unsigned __int8 *)Z_VirtualReserve(s_hunkTotal);
    if (!s_hunkData)
        Sys_OutOfMemErrorInternal("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 596);
    s_origHunkData = s_hunkData;
    track_set_hunk_size(s_hunkTotal);
    track_physical_alloc(s_hunkTotal, "hunk", 1, 0);
    Hunk_Clear();
    Cmd_AddCommandInternal("meminfo", Com_AllMemInfo_f, &Com_AllMemInfo_f_VAR);
    Cmd_AddCommandInternal("tempmeminfo", Com_TempMeminfo_f, &Com_TempMeminfo_f_VAR);
}

void __cdecl Com_AllMemInfo_f()
{
    track_PrintAllInfo();
}

char Hunk_CheckTempMemoryClear()
{
    char result; // al

    if ( !Sys_IsMainThread()
        && !Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    797,
                    0,
                    "%s",
                    "Sys_IsMainThread() || Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    if ( !s_hunkData
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 801, 0, "%s", "s_hunkData") )
    {
        __debugbreak();
    }
    result = hunk_low.temp;
    if ( hunk_low.temp != hunk_low.permanent )
    {
        result = Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                             802,
                             0,
                             "%s",
                             "hunk_low.temp == hunk_low.permanent");
        if ( !result )
            __debugbreak();
    }
    return result;
}

char Hunk_CheckTempMemoryHighClear()
{
    char result; // al

    if ( !Sys_IsMainThread()
        && !Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    807,
                    0,
                    "%s",
                    "Sys_IsMainThread() || Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    if ( !s_hunkData
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 811, 0, "%s", "s_hunkData") )
    {
        __debugbreak();
    }
    result = hunk_high.temp;
    if ( hunk_high.temp != hunk_high.permanent )
    {
        result = Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                             812,
                             0,
                             "%s",
                             "hunk_high.temp == hunk_high.permanent");
        if ( !result )
            __debugbreak();
    }
    return result;
}

void *__cdecl Hunk_FindDataForFile(int type, const char *name)
{
    int hash; // [esp+0h] [ebp-4h]

    hash = FS_HashFileName(name, 1024);
    return Hunk_FindDataForFileInternal(type, name, hash);
}

void *__cdecl Hunk_FindDataForFileInternal(int type, const char *name, int hash)
{
    fileData_s *searchFileData; // [esp+0h] [ebp-4h]

    for ( searchFileData = com_fileDataHashTable[hash]; searchFileData; searchFileData = searchFileData->next )
    {
        if ( searchFileData->type == type && !I_stricmp(searchFileData->name, name) )
            return searchFileData->data;
    }
    return 0;
}

bool __cdecl Hunk_DataOnHunk(unsigned __int8 *data)
{
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    841,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( !s_hunkData
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 842, 0, "%s", "s_hunkData") )
    {
        __debugbreak();
    }
    return data >= s_hunkData && data < &s_hunkData[s_hunkTotal];
}

char *__cdecl Hunk_SetDataForFile(int type, const char *name, void *data, void *(__cdecl *alloc)(int))
{
    char v5; // [esp+3h] [ebp-25h]
    char *v6; // [esp+8h] [ebp-20h]
    const char *v7; // [esp+Ch] [ebp-1Ch]
    int hash; // [esp+20h] [ebp-8h]
    fileData_s *fileData; // [esp+24h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    856,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    hash = FS_HashFileName(name, 1024);
    if ( Hunk_FindDataForFileInternal(type, name, hash)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    859,
                    0,
                    "%s",
                    "!Hunk_FindDataForFileInternal( type, name, hash )") )
    {
        __debugbreak();
    }
    fileData = (fileData_s *)alloc((int)(offsetof(fileData_s, name) + strlen(name) + 1)); // nx-port: was strlen + 10 (x86 header 9)
    if ( !Hunk_DataOnHunk((unsigned __int8 *)fileData)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    864,
                    0,
                    "%s",
                    "Hunk_DataOnHunk( fileData )") )
    {
        __debugbreak();
    }
    fileData->data = data;
    fileData->type = type;
    if ( type != fileData->type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    869,
                    0,
                    "%s",
                    "type == fileData->type") )
    {
        __debugbreak();
    }
    v7 = name;
    v6 = fileData->name;
    do
    {
        v5 = *v7;
        *v6++ = *v7++;
    }
    while ( v5 );
    fileData->next = com_fileDataHashTable[hash];
    com_fileDataHashTable[hash] = fileData;
    return fileData->name;
}

void __cdecl Hunk_AddData(int type, void *data, void *(__cdecl *alloc)(int))
{
    fileData_s *fileData; // [esp+0h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    880,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    fileData = (fileData_s *)alloc((int)offsetof(fileData_s, name)); // nx-port: was 9 (x86 header)
    if ( !Hunk_DataOnHunk((unsigned __int8 *)fileData)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    885,
                    0,
                    "%s",
                    "Hunk_DataOnHunk( fileData )") )
    {
        __debugbreak();
    }
    fileData->data = data;
    fileData->type = type;
    if ( type != fileData->type
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    890,
                    0,
                    "%s",
                    "type == fileData->type") )
    {
        __debugbreak();
    }
    fileData->next = com_hunkData;
    com_hunkData = fileData;
}

void __cdecl Hunk_OverrideDataForFile(int type, const char *name, void *data)
{
    fileData_s *searchFileData; // [esp+4h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    969,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    for ( searchFileData = com_fileDataHashTable[FS_HashFileName(name, 1024)];
                searchFileData;
                searchFileData = searchFileData->next )
    {
        if ( searchFileData->type == type && !I_stricmp(searchFileData->name, name) )
        {
            searchFileData->data = data;
            return;
        }
    }
    if ( !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    984,
                    0,
                    "Hunk_OverrideDataForFile: could not find data") )
        __debugbreak();
}

int __cdecl DB_GetAllXAssetOfType(XAssetType type, XAssetHeader *assets, int maxCount)
{
    if ( useFastFile->current.enabled )
        return DB_GetAllXAssetOfType_FastFile(type, assets, maxCount);
    else
        return DB_GetAllXAssetOfType_LoadObj(type, assets, maxCount);
}

int __cdecl DB_GetAllXAssetOfType_LoadObj(XAssetType type, XAssetHeader *assets, int maxCount)
{
    AssetList assetList; // [esp+0h] [ebp-Ch] BYREF

    assetList.assets = assets;
    assetList.assetCount = 0;
    assetList.maxCount = maxCount;
    DB_EnumXAssets(type, (void (__cdecl *)(XAssetHeader, void *))Hunk_AddAsset, &assetList, 0);
    return assetList.assetCount;
}

void __cdecl Hunk_AddAsset(XAssetHeader header, unsigned int *data)
{
    if ( !data && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 1071, 0, "%s", "data") )
        __debugbreak();
    AssetList *assetList = (AssetList *)data;   // nx-port: was read as three 32-bit words
    if ( assetList->assetCount >= assetList->maxCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1075,
                    0,
                    "%s",
                    "assetList->assetCount < assetList->maxCount") )
    {
        __debugbreak();
    }
    assetList->assets[assetList->assetCount++] = header;
}

void __cdecl DB_EnumXAssets(
                XAssetType type,
                void (__cdecl *func)(XAssetHeader, void *),
                void *inData,
                bool includeOverride)
{
    if ( useFastFile->current.enabled )
        DB_EnumXAssets_FastFile(type, func, inData, includeOverride);
    else
        ((void (__cdecl *)(XAssetType, void (__cdecl *)(XAssetHeader, void *), void *, bool))DB_EnumXAssets_LoadObj)(
            type,
            func,
            inData,
            includeOverride);
}

void __cdecl DB_EnumXAssets_LoadObj(XAssetType type, void (__cdecl *func)(void *, void *), void *inData)
{
    unsigned int hash; // [esp+4h] [ebp-Ch]

    switch ( type )
    {
        case ASSET_TYPE_XMODEL:
            for ( hash = 0; hash < 0x400; ++hash )
                DB_EnumXAssetsFor(com_fileDataHashTable[hash], 5, func, inData);
            break;
        case ASSET_TYPE_MATERIAL:
            R_EnumMaterials((void (__cdecl *)(Material *, void *))func, inData);
            break;
        case ASSET_TYPE_TECHNIQUE_SET:
            R_EnumTechniqueSets((void (__cdecl *)(MaterialTechniqueSet *, void *))func, inData);
            break;
        case ASSET_TYPE_IMAGE:
            R_EnumImages((void (__cdecl *)(GfxImage *, void *))func, inData);
            break;
        default:
            return;
    }
}

void __cdecl DB_EnumXAssetsFor(
                fileData_s *fileData,
                int fileDataType,
                void (__cdecl *func)(void *, void *),
                void *inData)
{
    while ( fileData )
    {
        if ( fileData->type == fileDataType && fileData->type == 5 )
            func(fileData->data, inData);
        fileData = fileData->next;
    }
}

int __cdecl Hunk_SetMark()
{
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1106,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    Hunk_CheckTempMemoryHighClear();
    return hunk_high.permanent;
}

void __cdecl Hunk_ClearToMark(int mark)
{
    unsigned int commitSize; // [esp+4h] [ebp-8h]
    unsigned __int8 *beginBuf; // [esp+8h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1121,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    Hunk_CheckTempMemoryHighClear();
    beginBuf = HUNK_PAGE_DOWN(&s_hunkData[s_hunkTotal - hunk_high.temp]);
    hunk_high.temp = mark;
    hunk_high.permanent = mark;
    Hunk_ClearData();
    commitSize = (int)(HUNK_PAGE_DOWN(&s_hunkData[s_hunkTotal - hunk_high.temp]) - beginBuf);
    if ( commitSize )
        Z_VirtualDecommit((char *)beginBuf, commitSize, 11);
    track_hunk_ClearToMarkHigh(mark);
}

void Hunk_ClearData()
{
    unsigned __int8 *low; // [esp+0h] [ebp-10h]
    unsigned int hash; // [esp+4h] [ebp-Ch]
    unsigned __int8 *high; // [esp+Ch] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    941,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    low = &s_hunkData[hunk_low.permanent];
    high = &s_hunkData[s_hunkTotal - hunk_high.permanent];
    for ( hash = 0; hash < 0x400; ++hash )
        Hunk_ClearDataFor(&com_fileDataHashTable[hash], low, high);
    Hunk_ClearDataFor(&com_hunkData, low, high);
}

void __cdecl Hunk_ClearDataFor(fileData_s **pFileData, unsigned __int8 *low, unsigned __int8 *high)
{
    unsigned __int8 type; // [esp+0h] [ebp-Ch]
    XAnim_s *data; // [esp+4h] [ebp-8h]
    fileData_s *fileData; // [esp+8h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    900,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    while ( *pFileData )
    {
        fileData = *pFileData;
        if ( *pFileData >= (fileData_s *)low && fileData < (fileData_s *)high )
        {
            *pFileData = fileData->next;
            data = (XAnim_s *)fileData->data;
            type = fileData->type;
            switch ( type )
            {
                case 2u:
                    XAnimFreeList(data);
                    break;
                case 4u:
                    XModelPartsFree((XModelPartsLoad *)data);
                    break;
                case 6u:
                    XAnimFree((XAnimParts *)data, SCRIPTINSTANCE_SERVER);
                    break;
            }
        }
        else
        {
            pFileData = &fileData->next;
        }
    }
}

void __cdecl Hunk_ClearToMarkLow(int mark)
{
    unsigned __int8 *endBuf; // [esp+0h] [ebp-Ch]
    unsigned __int8 *beginBuf; // [esp+8h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1162,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    Hunk_CheckTempMemoryClear();
    endBuf = HUNK_PAGE_DOWN(&s_hunkData[hunk_low.temp + 4095]);
    hunk_low.temp = mark;
    hunk_low.permanent = mark;
    Hunk_ClearData();
    beginBuf = HUNK_PAGE_DOWN(&s_hunkData[hunk_low.temp + 4095]);
    if ( endBuf != beginBuf )
        Z_VirtualDecommit((char *)beginBuf, endBuf - beginBuf, 11);
    track_hunk_ClearToMarkLow(mark);
}

void Hunk_Clear()
{
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1188,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    hunk_low.permanent = 0;
    hunk_low.temp = 0;
    hunk_high.permanent = 0;
    hunk_high.temp = 0;
    Hunk_ClearData();
    Z_VirtualDecommit((char *)s_hunkData, s_hunkTotal, 11);
    track_hunk_ClearToMarkLow(0);
    track_hunk_ClearToMarkHigh(0);
    track_hunk_ClearToStart();
}

int __cdecl Hunk_Used()
{
    if ( !Sys_IsMainThread()
        && !Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1213,
                    0,
                    "%s",
                    "Sys_IsMainThread() || Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    return hunk_high.permanent + hunk_low.permanent;
}

unsigned __int8 *__cdecl Hunk_Alloc(unsigned int size, const char *name, int type)
{
    if ( !Sys_IsMainThread()
        && !Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1222,
                    0,
                    "%s",
                    "Sys_IsMainThread() || Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    return Hunk_AllocAlign(size, 32, name, type);
}

unsigned __int8 *__cdecl Hunk_AllocAlign(unsigned int size, int alignment, const char *name, int type)
{
    int old_permanent; // [esp+0h] [ebp-14h]
    unsigned __int8 *buf; // [esp+4h] [ebp-10h]
    unsigned __int8 *endBuf; // [esp+8h] [ebp-Ch]
    int alignmenta; // [esp+20h] [ebp+Ch]

    if ( !Sys_IsMainThread()
        && !Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1255,
                    0,
                    "%s",
                    "Sys_IsMainThread() || Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    if ( !s_hunkData
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 1259, 0, "%s", "s_hunkData") )
    {
        __debugbreak();
    }
    if ( (alignment & (alignment - 1)) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1266,
                    0,
                    "%s",
                    "!(alignment & (alignment - 1))") )
    {
        __debugbreak();
    }
    if ( alignment > 4096
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1267,
                    0,
                    "%s",
                    "alignment <= HUNK_MAX_ALIGNMENT") )
    {
        __debugbreak();
    }
    alignmenta = alignment - 1;
    Hunk_CheckTempMemoryHighClear();
    old_permanent = hunk_high.permanent;
    endBuf = HUNK_PAGE_DOWN(&s_hunkData[s_hunkTotal - hunk_high.permanent]);
    hunk_high.permanent += size;
    hunk_high.permanent = ~alignmenta & (alignmenta + hunk_high.permanent);
    hunk_high.temp = hunk_high.permanent;
    if ( hunk_high.permanent + hunk_low.temp > s_hunkTotal )
    {
        track_PrintAllInfo();
        Com_Error(ERR_DROP, "Hunk_AllocAlign failed on %i bytes (total %i MB, low %i MB, high %i MB)", size, s_hunkTotal / 0x100000, hunk_low.temp / 0x100000, hunk_high.temp / 0x100000);
    }
    buf = &s_hunkData[s_hunkTotal - hunk_high.permanent];
    if ( (alignmenta & (uintptr_t)buf) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1294,
                    0,
                    "%s",
                    "!(((psize_int)buf) & alignment)") )
    {
        __debugbreak();
    }
    if ( endBuf != HUNK_PAGE_DOWN(buf) )
        Z_VirtualCommit((char *)HUNK_PAGE_DOWN(buf), (int)(endBuf - HUNK_PAGE_DOWN(buf)), 11);
    track_hunk_alloc(hunk_high.permanent - old_permanent, hunk_high.temp, name, type);
    memset(buf, 0, size);
    return buf;
}

void *__cdecl Hunk_AllocateTempMemoryHigh(int size, const char *name)
{
    unsigned __int8 *buf; // [esp+0h] [ebp-10h]  nx-port: was unsigned int, the address itself
    unsigned __int8 *endBuf; // [esp+4h] [ebp-Ch]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1328,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( !s_hunkData
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 1329, 0, "%s", "s_hunkData") )
    {
        __debugbreak();
    }
    endBuf = HUNK_PAGE_DOWN(&s_hunkData[s_hunkTotal - hunk_high.temp]);
    hunk_high.temp += size;
    hunk_high.temp = (hunk_high.temp + 15) & 0xFFFFFFF0;
    if ( hunk_high.temp + hunk_low.temp > s_hunkTotal )
    {
        track_PrintAllInfo();
        Com_Error(ERR_DROP, "Hunk_AllocateTempMemoryHigh: failed on %i bytes (total %i MB, low %i MB, high %i MB)", size, s_hunkTotal / 0x100000, hunk_low.temp / 0x100000, hunk_high.temp / 0x100000);
    }
    buf = &s_hunkData[s_hunkTotal - hunk_high.temp];
    if ( ((uintptr_t)buf & 0xF) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1352,
                    0,
                    "%s",
                    "!(((psize_int)buf) & 15)") )
    {
        __debugbreak();
    }
    if ( endBuf != HUNK_PAGE_DOWN(buf) )
        Z_VirtualCommit((char *)HUNK_PAGE_DOWN(buf), (int)(endBuf - HUNK_PAGE_DOWN(buf)), 11);
    track_temp_high_alloc(size, hunk_high.temp + hunk_low.temp, hunk_high.permanent, name);
    return buf;
}

void Hunk_ClearTempMemoryHigh()
{
    unsigned int commitSize; // [esp+4h] [ebp-8h]
    unsigned __int8 *beginBuf; // [esp+8h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1379,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    beginBuf = HUNK_PAGE_DOWN(&s_hunkData[s_hunkTotal - hunk_high.temp]);
    hunk_high.temp = hunk_high.permanent;
    commitSize = (int)(HUNK_PAGE_DOWN(&s_hunkData[s_hunkTotal - hunk_high.permanent]) - beginBuf);
    if ( commitSize )
        Z_VirtualDecommit((char *)beginBuf, commitSize, 11);
    track_temp_high_clear(hunk_high.permanent);
}

unsigned __int8 *__cdecl Hunk_AllocLow(unsigned int size, const char *name, int type)
{
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1403,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    return Hunk_AllocLowAlign(size, 32, name, type);
}

unsigned __int8 *__cdecl Hunk_AllocLowAlign(unsigned int size, int alignment, const char *name, int type)
{
    int old_permanent; // [esp+0h] [ebp-14h]
    unsigned __int8 *buf; // [esp+4h] [ebp-10h]
    unsigned int commitSize; // [esp+Ch] [ebp-8h]
    unsigned __int8 *beginBuf; // [esp+10h] [ebp-4h]
    int alignmenta; // [esp+20h] [ebp+Ch]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1433,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( !s_hunkData
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 1434, 0, "%s", "s_hunkData") )
    {
        __debugbreak();
    }
    if ( (alignment & (alignment - 1)) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1441,
                    0,
                    "%s",
                    "!(alignment & (alignment - 1))") )
    {
        __debugbreak();
    }
    if ( alignment > 4096
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1442,
                    0,
                    "%s",
                    "alignment <= HUNK_MAX_ALIGNMENT") )
    {
        __debugbreak();
    }
    alignmenta = alignment - 1;
    Hunk_CheckTempMemoryClear();
    old_permanent = hunk_low.permanent;
    beginBuf = HUNK_PAGE_DOWN(&s_hunkData[hunk_low.permanent + 4095]);
    hunk_low.permanent = ~alignmenta & (alignmenta + hunk_low.permanent);
    buf = &s_hunkData[hunk_low.permanent];
    if ( (alignmenta & (uintptr_t)&s_hunkData[hunk_low.permanent]) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1458,
                    0,
                    "%s",
                    "!(((psize_int)buf) & alignment)") )
    {
        __debugbreak();
    }
    hunk_low.permanent += size;
    hunk_low.temp = hunk_low.permanent;
    if ( hunk_high.temp + hunk_low.permanent > s_hunkTotal )
    {
        track_PrintAllInfo();
        Com_Error(ERR_DROP, "Hunk_AllocLowAlign failed on %i bytes (total %i MB, low %i MB, high %i MB)", size, s_hunkTotal / 0x100000, hunk_low.temp / 0x100000, hunk_high.temp / 0x100000);
    }
    commitSize = (int)(HUNK_PAGE_DOWN(&s_hunkData[hunk_low.permanent + 4095]) - beginBuf);
    if ( commitSize )
        Z_VirtualCommit((char *)beginBuf, commitSize, 11);
    track_hunk_allocLow(hunk_low.permanent - old_permanent, hunk_low.permanent, name, type);
    memset(buf, 0, size);
    return buf;
}

unsigned int *__cdecl Hunk_AllocateTempMemory(int size, const char *name)
{
    hunkHeader_t *hdr; // [esp+0h] [ebp-18h]
    unsigned __int8 *buf; // [esp+4h] [ebp-14h]
    void *bufa; // [esp+4h] [ebp-14h]
    int prev_temp; // [esp+8h] [ebp-10h]
    unsigned int commitSize; // [esp+10h] [ebp-8h]
    unsigned __int8 *beginBuf; // [esp+14h] [ebp-4h]
    int sizea; // [esp+20h] [ebp+8h]

    if ( !Sys_IsMainThread()
        && !Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1507,
                    0,
                    "%s",
                    "Sys_IsMainThread() || Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    if ( !s_hunkData )
        return Z_Malloc(size, name, 11);
    sizea = size + 16;
    prev_temp = hunk_low.temp;
    beginBuf = HUNK_PAGE_DOWN(&s_hunkData[hunk_low.temp + 4095]);
    hunk_low.temp = (hunk_low.temp + 15) & 0xFFFFFFF0;
    buf = &s_hunkData[hunk_low.temp];
    hunk_low.temp += sizea;
    if ( hunk_high.temp + hunk_low.temp > s_hunkTotal )
    {
        track_PrintAllInfo();
        Com_Error(
            ERR_DROP,
            "Hunk_AllocateTempMemory: failed on %i bytes (total %i MB, low %i MB, high %i MB), needs %i more hunk bytes",
            sizea,
            s_hunkTotal / 0x100000,
            hunk_low.temp / 0x100000,
            hunk_high.temp / 0x100000,
            hunk_high.temp + hunk_low.temp - s_hunkTotal);
    }
    hdr = (hunkHeader_t *)buf;
    bufa = buf + 16;
    if ( ((uintptr_t)bufa & 0xF) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1550,
                    0,
                    "%s",
                    "!(((psize_int)buf) & 15)") )
    {
        __debugbreak();
    }
    commitSize = (int)(HUNK_PAGE_DOWN(&s_hunkData[hunk_low.temp + 4095]) - beginBuf);
    if ( commitSize )
        Z_VirtualCommit((char *)beginBuf, commitSize, 11);
    hdr->magic = 0x89537892;
    hdr->size = hunk_low.temp - prev_temp;
    track_temp_alloc(hdr->size, hunk_high.temp + hunk_low.temp, hunk_low.permanent, name);
    hdr->name = name;
    return (unsigned int *)bufa;
}

void __cdecl Hunk_FreeTempMemory(char *buf)
{
    hunkHeader_t *hdr; // [esp+0h] [ebp-10h]
    unsigned __int8 *endBuf; // [esp+4h] [ebp-Ch]
    unsigned __int8 *beginBuf; // [esp+Ch] [ebp-4h]

    if (!Sys_IsMainThread()
        && !Sys_IsRenderThread()
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
            1583,
            0,
            "%s",
            "Sys_IsMainThread() || Sys_IsRenderThread()"))
    {
        __debugbreak();
    }
    if (s_hunkData)
    {
        if (!buf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 1600, 0, "%s", "buf"))
            __debugbreak();
        hdr = (hunkHeader_t *)(buf - 16);
        if (*((_DWORD *)buf - 4) != 0x89537892)
            Com_Error(ERR_FATAL, "Hunk_FreeTempMemory: bad magic");
        hdr->magic = 0x89537893;
        if (hdr != (hunkHeader_t *)&s_hunkData[(hunk_low.temp - hdr->size + 15) & 0xFFFFFFF0]
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                1609,
                0,
                "%s",
                "hdr == (void *)( s_hunkData + ((hunk_low.temp - hdr->size + 15) & ~15) )"))
        {
            __debugbreak();
        }
        endBuf = HUNK_PAGE_DOWN(&s_hunkData[hunk_low.temp + 4095]);
        hunk_low.temp -= hdr->size;
        track_temp_free(hdr->size, hunk_low.permanent, hdr->name);
        beginBuf = HUNK_PAGE_DOWN(&s_hunkData[hunk_low.temp + 4095]);
        if (endBuf != beginBuf)
            Z_VirtualDecommit((char *)beginBuf, endBuf - beginBuf, 11);
    }
    else
    {
        Z_Free(buf, 11);
    }
}

void Hunk_ClearTempMemory()
{
    unsigned __int8 *endBuf; // [esp+0h] [ebp-Ch]
    unsigned __int8 *beginBuf; // [esp+8h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1637,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( !s_hunkData
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 1638, 0, "%s", "s_hunkData") )
    {
        __debugbreak();
    }
    endBuf = HUNK_PAGE_DOWN(&s_hunkData[hunk_low.temp + 4095]);
    hunk_low.temp = hunk_low.permanent;
    beginBuf = HUNK_PAGE_DOWN(&s_hunkData[hunk_low.permanent + 4095]);
    if ( endBuf != beginBuf )
        Z_VirtualDecommit((char *)beginBuf, endBuf - beginBuf, 11);
    track_temp_clear(hunk_low.permanent);
}

void __cdecl Com_TempMeminfo_f()
{
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                    1714,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    //BLOPS_NULLSUB();
    Com_Printf(0, "Related commands: meminfo, imagelist, gfx_world, gfx_model, cg_drawfps, com_statmon, tempmeminfo\n");
}

char *__cdecl Z_TryVirtualAlloc(int size, const char *name, int type)
{
    char *buf; // [esp+0h] [ebp-4h]

    buf = (char *)Z_TryVirtualAllocInternal(size);
    if ( buf )
        track_z_commit((size + 4095) & 0xFFFFF000, type);
    return buf;
}

char *__cdecl Z_VirtualAlloc(int size, const char *name, int type)
{
    char *buf; // [esp+0h] [ebp-4h]

    buf = Z_TryVirtualAlloc(size, name, type);
    if ( !buf )
        Sys_OutOfMemErrorInternal("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 1745);
    return buf;
}

void __cdecl Z_VirtualCommit(char *ptr, int size, int type)
{
    _MEMORY_BASIC_INFORMATION info; // [esp+0h] [ebp-2Ch] BYREF
    int byteCount; // [esp+1Ch] [ebp-10h]
    int allocSize; // [esp+20h] [ebp-Ch]
    char *base; // [esp+24h] [ebp-8h]
    int i; // [esp+28h] [ebp-4h]

    if ( !ptr && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 1763, 0, "%s", "ptr") )
        __debugbreak();
    if ( !size && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 1764, 0, "%s", "size") )
        __debugbreak();
    allocSize = 0;
    byteCount = VirtualQuery(ptr, &info, 0x1Cu);
    if ( byteCount )
    {
        base = (char *)info.AllocationBase;
        for ( i = 0; i < size; i += 4096 )
        {
            byteCount = VirtualQuery(&ptr[i], &info, 0x1Cu);
            if ( !byteCount
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 1776, 0, "%s", "byteCount") )
            {
                __debugbreak();
            }
            if ( info.AllocationBase != base
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                            1777,
                            0,
                            "%s",
                            "info.AllocationBase == base") )
            {
                __debugbreak();
            }
            if ( info.State == 0x2000 )
                allocSize += 4096;
        }
        if ( allocSize )
            track_z_commit(allocSize, type);
    }
    Z_VirtualCommitInternal(ptr, size);
}

void __cdecl Z_VirtualFree(void *ptr, int type)
{
    _MEMORY_BASIC_INFORMATION info; // [esp+0h] [ebp-2Ch] BYREF
    int byteCount; // [esp+1Ch] [ebp-10h]
    char *rangePtr; // [esp+20h] [ebp-Ch]
    int freeSize; // [esp+24h] [ebp-8h]
    char *base; // [esp+28h] [ebp-4h]

    if ( ptr )
    {
        freeSize = 0;
        byteCount = VirtualQuery(ptr, &info, 0x1Cu);
        if ( byteCount )
        {
            base = (char *)info.AllocationBase;
            if ( info.AllocationBase != ptr
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                            1807,
                            0,
                            "%s",
                            "base == ptr") )
            {
                __debugbreak();
            }
            if ( info.BaseAddress != ptr
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                            1808,
                            0,
                            "%s",
                            "info.BaseAddress == ptr") )
            {
                __debugbreak();
            }
            for ( rangePtr = (char *)ptr; ; rangePtr += info.RegionSize )
            {
                byteCount = VirtualQuery(rangePtr, &info, 0x1Cu);
                if ( !byteCount )
                    break;
                if ( info.BaseAddress != rangePtr
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                                1818,
                                0,
                                "%s",
                                "info.BaseAddress == rangePtr") )
                {
                    __debugbreak();
                }
                if ( info.AllocationBase != base )
                    break;
                if ( !info.RegionSize
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                                1823,
                                0,
                                "%s\n\t(info.RegionSize) = %i",
                                "(info.RegionSize)",
                                0) )
                {
                    __debugbreak();
                }
                if ( info.State == 4096 )
                    freeSize += info.RegionSize;
            }
            if ( freeSize )
                track_z_decommit(freeSize, type);
        }
    }
    Z_VirtualFreeInternal(ptr);
}

void __cdecl Z_VirtualDecommit(char *ptr, int size, int type)
{
    _MEMORY_BASIC_INFORMATION info; // [esp+0h] [ebp-2Ch] BYREF
    int byteCount; // [esp+1Ch] [ebp-10h]
    int freeSize; // [esp+20h] [ebp-Ch]
    char *base; // [esp+24h] [ebp-8h]
    int i; // [esp+28h] [ebp-4h]

    if ( !ptr && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 1849, 0, "%s", "ptr") )
        __debugbreak();
    if ( !size && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 1850, 0, "%s", "size") )
        __debugbreak();
    freeSize = 0;
    byteCount = VirtualQuery(ptr, &info, 0x1Cu);
    if ( byteCount )
    {
        if ( info.BaseAddress != ptr
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                        1856,
                        0,
                        "%s",
                        "info.BaseAddress == ptr") )
        {
            __debugbreak();
        }
        base = (char *)info.AllocationBase;
        for ( i = 0; i < size; i += 4096 )
        {
            byteCount = VirtualQuery(&ptr[i], &info, 0x1Cu);
            if ( !byteCount
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 1864, 0, "%s", "byteCount") )
            {
                __debugbreak();
            }
            if ( info.AllocationBase != base
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                            1865,
                            0,
                            "%s",
                            "info.AllocationBase == base") )
            {
                __debugbreak();
            }
            if ( info.BaseAddress != &ptr[i]
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp",
                            1866,
                            0,
                            "%s",
                            "info.BaseAddress == static_cast< byte * >( ptr ) + i") )
            {
                __debugbreak();
            }
            if ( info.State == 4096 )
                freeSize += 4096;
        }
        if ( freeSize )
            track_z_decommit(freeSize, type);
    }
    Z_VirtualDecommitInternal(ptr, size);
}

void __cdecl Z_Free(char *ptr, int type)
{
    if ( ptr )
    {
        track_z_free(type, ptr, 164);
        free(ptr - 164);
    }
}

unsigned int *__cdecl Z_TryMalloc(int size, const char *name, int type)
{
    unsigned int *buf; // [esp+0h] [ebp-4h]

    buf = (unsigned int *)Z_TryMallocGarbage(size, name, type);
    if ( buf )
        Com_Memset(buf, 0, size);
    return buf;
}

char *__cdecl Z_TryMallocGarbage(int size, const char *name, int type)
{
    char *buf; // [esp+0h] [ebp-4h]

    buf = (char *)malloc(size + 164);
    if ( buf )
    {
        buf += 164;
        track_z_alloc(size + 72, name, type, buf, 0, 164);
    }
    return buf;
}

unsigned int *__cdecl Z_Malloc(int size, const char *name, int type)
{
    unsigned int *buf; // [esp+0h] [ebp-4h]

    buf = Z_TryMalloc(size, name, type);
    if ( !buf )
        Z_MallocFailed(size + 164);
    return buf;
}

void __cdecl    Z_MallocFailed(int size)
{
    Com_PrintError(16, "Failed to Z_Malloc %i bytes\n", size);
    Sys_OutOfMemErrorInternal("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 436);
}

char *__cdecl Z_MallocGarbage(int size, const char *name, int type)
{
    char *buf; // [esp+0h] [ebp-4h]

    buf = Z_TryMallocGarbage(size, name, type);
    if ( !buf )
        Z_MallocFailed(size + 164);
    return buf;
}

char *__cdecl CopyString(char *in, const char *name, int type, scriptInstance_t inst)
{
    unsigned int out; // [esp+0h] [ebp-4h]

    if ( !in && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 2100, 0, "%s", "in") )
        __debugbreak();
    out = SL_GetString_(inst, in, 0, 22);
    return SL_ConvertToString(out, inst);
}

void __cdecl ReplaceString(const char **str, char *in, const char *name, int type, scriptInstance_t inst)
{
    char *newStr; // [esp+0h] [ebp-4h]

    if ( !str && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 2109, 0, "%s", "str") )
        __debugbreak();
    if ( !in && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 2110, 0, "%s", "in") )
        __debugbreak();
    newStr = CopyString(in, name, type, inst);
    if ( *str )
        FreeString(*str, type, inst);
    *str = newStr;
}

void __cdecl FreeString(const char *str, int type, scriptInstance_t inst)
{
    unsigned int out; // [esp+0h] [ebp-4h]

    if ( !str && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 2121, 0, "%s", "str") )
        __debugbreak();
    out = SL_FindString(str, SCRIPTINSTANCE_SERVER);
    if ( !out && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_memory.cpp", 2123, 0, "%s", "out") )
        __debugbreak();
    SL_RemoveRefToString(inst, out);
}

