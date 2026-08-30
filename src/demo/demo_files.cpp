#include "demo_files.h"
#include "demo_common.h"
#include <universal/com_memory.h>
#include <universal/com_files.h>
#include <qcommon/files.h>
#include <qcommon/common.h>
#include "demo_recording.h"

demoFileHandler s_fileHandler;

void __cdecl Demo_InitFileHandlerSystem()
{
    if ( !demo_usefilesystem->current.enabled )
    {
        s_fileHandler.ptr = 0;
        s_fileHandler.size = 0;
        s_fileHandler.offset = 0;
    }
}

void __cdecl Demo_AllocateMemoryFromStreamBuffer(int size)
{
    if ( !demo_usefilesystem->current.enabled )
    {
        if ( s_fileHandler.ptr )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_files.cpp",
                            63,
                            0,
                            "%s",
                            "s_fileHandler.ptr == NULL") )
                __debugbreak();
        }
        s_fileHandler.ptr = (unsigned __int8 *)Z_VirtualAlloc(size, "demoDownloadBuffer", 11);
        s_fileHandler.offset = 0;
        s_fileHandler.size = size;
    }
}

void __cdecl Demo_ReturnStreamBufferMemory()
{
    if ( !demo_usefilesystem->current.enabled )
    {
        if ( !s_fileHandler.ptr
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_files.cpp",
                        90,
                        0,
                        "%s",
                        "s_fileHandler.ptr != NULL") )
        {
            __debugbreak();
        }
        s_fileHandler.ptr = 0;
    }
}

bool __cdecl Demo_IsStreamBufferAllocated()
{
    return !demo_usefilesystem->current.enabled && s_fileHandler.ptr != 0;
}

int __cdecl Demo_GetCurrentAllocatedFileSize()
{
    return s_fileHandler.size;
}

char *__cdecl Demo_GetStreamAllocatedBuffer()
{
    return (char *)s_fileHandler.ptr;
}

void __cdecl Demo_GetDemoPath(char *demoPath)
{
    if ( demo_usefilesystem->current.enabled )
        FS_BuildOSPath((char *)fs_homepath->current.string, (char*)"demos", (char *)"", demoPath);
}

int __cdecl Demo_OpenFileWrite(const char *filename, const char *dir, bool supressErrors)
{
    int handle; // [esp+0h] [ebp-10Ch]
    char name[260]; // [esp+4h] [ebp-108h] BYREF

    if ( !demo_usefilesystem->current.enabled )
        return 1;
    Com_sprintf(name, 0x100u, "%s", filename);
    handle = FS_SV_FOpenFileWrite(name, (char *)"demos");
    if ( handle )
        return handle;
    if ( !supressErrors )
        Com_PrintError(0, "ERROR: couldn't open file for write.\n");
    return 0;
}

int __cdecl Demo_OpenFileRead(const char *filename, const char *dir, bool supressErrors)
{
    int handle; // [esp+0h] [ebp-10Ch] BYREF
    char ospath[256]; // [esp+4h] [ebp-108h] BYREF
    int size; // [esp+108h] [ebp-4h]

    if ( demo_usefilesystem->current.enabled )
    {
        Com_sprintf(ospath, 0x100u, "%s", filename);
        size = FS_SV_FOpenFileRead(ospath, (char *)"demos", &handle);
        if ( handle && size )
        {
            return handle;
        }
        else
        {
            if ( !supressErrors )
                Com_PrintError(0, "ERROR: couldn't open file for read.\n");
            if ( handle )
                FS_FCloseFile(handle);
            return 0;
        }
    }
    else
    {
        if ( !s_fileHandler.ptr
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_files.cpp",
                        255,
                        0,
                        "%s",
                        "s_fileHandler.ptr != NULL") )
        {
            __debugbreak();
        }
        return 1;
    }
}

unsigned int __cdecl Demo_Write(char *buffer, unsigned int len, int handle)
{
    if ( demo_usefilesystem->current.enabled )
        return FS_WriteToDemo(buffer, len, handle);
    if ( Demo_IsRecording() )
    {
        Demo_SaveToStreamBuffer((unsigned __int8 *)buffer, len);
    }
    else
    {
        if ( !s_fileHandler.ptr )
            return 0;
        memcpy(&s_fileHandler.ptr[s_fileHandler.offset], (unsigned __int8 *)buffer, len);
        s_fileHandler.offset += len;
    }
    return len;
}

unsigned int __cdecl Demo_Read(unsigned __int8 *buffer, unsigned int len, int handle)
{
    if ( demo_usefilesystem->current.enabled )
        return FS_Read(buffer, len, handle);
    memcpy(buffer, &s_fileHandler.ptr[s_fileHandler.offset], len);
    s_fileHandler.offset += len;
    return len;
}

void __cdecl Demo_CloseFile(int handle)
{
    if ( demo_usefilesystem->current.enabled )
        FS_FCloseFile(handle);
    else
        s_fileHandler.offset = 0;
}

int __cdecl Demo_SeekFile(int handle, int offset, int origin)
{
    const char *v3; // eax

    if ( demo_usefilesystem->current.enabled )
        return FS_Seek(handle, offset, origin);
    switch ( origin )
    {
        case 0:
            s_fileHandler.offset += offset;
            return offset;
        case 1:
            s_fileHandler.offset = offset + s_fileHandler.size;
            return offset;
        case 2:
            s_fileHandler.offset = offset;
            return offset;
    }
    v3 = va("Bad origin %i in FS_Seek", origin);
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\demo\\demo_files.cpp", 448, 0, v3) )
        __debugbreak();
    return 0;
}

