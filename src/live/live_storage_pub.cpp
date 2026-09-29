#include "live_storage_pub.h"
#include <DW/dwLogOn_pc.h>
#include <stringed/stringed_hooks.h>
#include <cgame_mp/cg_main_mp.h>
#include <universal/com_files.h>
#include <universal/com_tasks.h>
#include <universal/mem_largelocal.h>
#include "live_fileshare.h"
#include "live_ticker.h"
#include <ui/ui_playlists.h>
#include <server/sv_autoplaylist.h>
#include "live_contracts.h"
#include "live_storage_win.h"
#include <zlib/zlib.h>

bool s_haveContracts;
bool s_havePlaylists;
bool s_haveMotd;

char wadFileName[128];

dwFileOperationInfo s_onlineWADFileInfo;
unsigned __int8 s_onlineWADBuffer[65536];
bool s_completedOnlineWAD;

const TaskDefinition task_LiveFetchOnlineWAD[1] =
{ { 8uLL, "LiveFetchOnlineWAD", 0, NULL, NULL, NULL } };

bool __cdecl LiveStorage_DoWeHaveContracts()
{
    return s_haveContracts;
}

bool __cdecl LiveStorage_DoWeHavePlaylists()
{
    return s_havePlaylists;
}

void __cdecl LiveStorage_SetHavePlaylists(bool val)
{
    s_havePlaylists = val;
}

static void NULLSUB_LOCAL(const int, void *)
{

}

TaskRecord *__cdecl LiveStorage_FetchOnlineWAD(int controllerIndex)
{
#ifdef KISAK_LIVE
    signed int fileLen; // [esp+0h] [ebp-14h]
    void *fileBuffer; // [esp+4h] [ebp-10h] BYREF
    const char *language; // [esp+8h] [ebp-Ch]
    TaskRecord *nestedTask; // [esp+Ch] [ebp-8h]
    TaskRecord *task; // [esp+10h] [ebp-4h]

    if ( !SV_IsConnectedToDW() )
        return 0;
    task = 0;
    language = SEH_GetLanguageName(loc_language->current.unsignedInt);
    Com_sprintf(wadFileName, 0x80u, "%s%s.wad", "online_mp_", language);
    if ( s_onlineWADFileInfo.fetchCompleted
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_pub.cpp",
                    315,
                    0,
                    "%s",
                    "s_onlineWADFileInfo.fetchCompleted == false") )
    {
        __debugbreak();
    }
    if ( cg_readTitleStorageLocally->current.enabled )
    {
        s_onlineWADFileInfo.fetchCompleted = 1;
        fileLen = FS_ReadFile(wadFileName, &fileBuffer);
        if ( fileLen > 0 )
        {
            if ( fileLen <= 0x10000 )
            {
                if ( fileBuffer )
                {
                    memcpy(s_onlineWADBuffer, (unsigned __int8 *)fileBuffer, fileLen);
                    FS_FreeFile(fileBuffer);
                    //BLOPS_NULLSUB();
                    return 0;
                }
            }
            else
            {
                Com_PrintError(16, "The WAD file '%s' file is too large and cannot be loaded!\n", wadFileName);
            }
        }
        Com_PrintError(16, "Unable to load WAD file '%s' locally\n", wadFileName);
        LiveStorage_FetchOnlineWADNotFound();
        return 0;
    }
    else if ( TaskManager2_TaskIsInProgress(task_LiveFetchOnlineWAD) )
    {
        return 0;
    }
    else
    {
        s_onlineWADFileInfo.isUserFile = 0;
        s_onlineWADFileInfo.isCompressedFile = 0;
        s_onlineWADFileInfo.fileTask.m_filename = wadFileName;
        s_onlineWADFileInfo.fileBuffer = s_onlineWADBuffer;
        s_onlineWADFileInfo.bufferSize = 0x10000;
        s_onlineWADFileInfo.fileOperationSucessFunction = NULLSUB_LOCAL;
        s_onlineWADFileInfo.fileNotFoundFunction = (taskCompleteResults (__cdecl *)(const int, void *))LiveStorage_FetchOnlineWADNotFound;
        nestedTask = LiveStorage_ReadDWFile(controllerIndex, &s_onlineWADFileInfo);
        return LiveStorage_SetupNestedTask(task_LiveFetchOnlineWAD, controllerIndex, nestedTask, &s_onlineWADFileInfo);
    }
#else
    return 0;
#endif
}

int __cdecl LiveStorage_FetchOnlineWADNotFound()
{
    Com_Printf(16, "Could not find WAD '%s' in publisher storage\n", "online_mp_");
    return 2;
}

bool __cdecl LiveStorage_DoWeHaveOnlineWAD()
{
    return s_onlineWADFileInfo.fetchCompleted;
}

void __cdecl LiveStorage_RefetchOnlineWAD()
{
    s_completedOnlineWAD = 0;
    s_onlineWADFileInfo.fetchCompleted = 0;
}

bool __cdecl LiveStorage_CompletedOnlineWAD()
{
    return s_completedOnlineWAD;
}

bool __cdecl LiveStorage_FetchingOnlineWAD()
{
    return TaskManager2_TaskIsInProgress(task_LiveFetchOnlineWAD);
}

void __cdecl LiveStorage_ForceWADFetch()
{
    s_onlineWADFileInfo.fetchCompleted = 0;
    s_completedOnlineWAD = 0;
}

void __cdecl LiveStorage_HandlePublisherStorageTasks()
{
    LiveStorage_ProcessOnlineWAD();
}

void LiveStorage_ProcessOnlineWAD()
{
    const char *v0; // eax
    const char *v1; // eax
    unsigned int compressedSize; // [esp+Ch] [ebp-3Ch]
    const char *v3; // [esp+10h] [ebp-38h]
    WADEntry *entry; // [esp+24h] [ebp-24h]
    __int64 genTime; // [esp+28h] [ebp-20h] BYREF
    const char *language; // [esp+30h] [ebp-18h]
    unsigned __int8 *uncompressedFileBuffer; // [esp+34h] [ebp-14h]
    unsigned int idx; // [esp+38h] [ebp-10h]
    WADEntry *toc; // [esp+44h] [ebp-4h]

    language = SEH_GetLanguageName(loc_language->current.unsignedInt);
    if ( !LiveStorage_CompletedOnlineWAD() && LiveStorage_DoWeHaveOnlineWAD() )
    {
        LargeLocal uncompressedFileBuffer_large_local(0x40000); // [esp+3Ch] [ebp-Ch] BYREF

        //LargeLocal::LargeLocal(&uncompressedFileBuffer_large_local, 0x40000);
        uncompressedFileBuffer = uncompressedFileBuffer_large_local.GetBuf();// LargeLocal::GetBuf(&uncompressedFileBuffer_large_local);
        if ( !s_onlineWADFileInfo.fetchCompleted
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_pub.cpp",
                        172,
                        0,
                        "%s",
                        "s_onlineWADFileInfo.fetchCompleted == true") )
        {
            __debugbreak();
        }
        LiveStorage_SetHavePlaylists(0);
        s_haveContracts = 0;
        s_haveMotd = 0;
        toc = (WADEntry *)(s_onlineWADBuffer + 16);
        if ( *(unsigned int *)s_onlineWADBuffer != 1412659115 )
        {
            if ( *(unsigned int *)s_onlineWADBuffer != -1418251436 )
            {
                Com_PrintError(16, "Platform wad file is missing or invalid.\n");
                s_completedOnlineWAD = 1;
                //LargeLocal::~LargeLocal(&uncompressedFileBuffer_large_local);
                return;
            }
            Com_PrintWarning(0, "WAD: Big-endian WAD on Small-endian machine\n");
            tlEndianSwapMemory32((int *)s_onlineWADBuffer, 0x10u);
        }
        genTime = *(unsigned int *)&s_onlineWADBuffer[4];
        v3 = _ctime64(&genTime);
        Com_Printf(23, "Got Online WAD. %d entries, generated: %s", *(unsigned int *)&s_onlineWADBuffer[8], v3);
        for ( idx = 0; idx < *(unsigned int *)&s_onlineWADBuffer[8]; ++idx )
        {
            entry = &toc[idx];
            memset(uncompressedFileBuffer, 0, 0x40000u);
            compressedSize = entry->compressedSize;
            LiveStorage_DecompressBuffer(
                &s_onlineWADBuffer[(entry->offset << 24)
                                                 | ((entry->offset & 0xFF00) << 8)
                                                 | (entry->offset >> 8) & 0xFF00
                                                 | HIBYTE(entry->offset)],
                (compressedSize << 24)
            | ((compressedSize & 0xFF00) << 8)
            | (compressedSize >> 8) & 0xFF00
            | HIBYTE(compressedSize),
                uncompressedFileBuffer,
                0x40000u);
            if ( I_stricmp(entry->name, "contracts.dat") )
            {
                if ( I_stricmp(entry->name, "geogroups.info") )
                {
                    if ( I_stricmp(entry->name, "playlists_mp.info") )
                    {
                        v0 = va("motd-mp-%s.txt", language);
                        if ( I_stricmp(entry->name, v0) )
                        {
                            v1 = va("liveblurb-mp-%s.txt", language);
                            if ( I_stricmp(entry->name, v1) )
                                Com_PrintWarning(23, "Unhandled entry '%s' in online WAD\n", entry->name);
                            else
                                Live_FileShare_SetLiveBlurb((const char *)uncompressedFileBuffer);
                        }
                        else
                        {
                            LiveTicker_RemoveStream(TICKER_STREAM_COD);
                            LiveTicker_AddMessages((const char *)uncompressedFileBuffer, TICKER_STREAM_COD, 1);
                        }
                    }
                    else
                    {
                        Playlist_ParsePlaylists((const char *)uncompressedFileBuffer);
                        LiveStorage_SetHavePlaylists(1);
                        SV_AP_Start();
                    }
                }
                else
                {
                    SV_Groups_ParseGeos((const char *)uncompressedFileBuffer);
                }
            }
            else
            {
                LiveContracts_ParseContracts((char *)uncompressedFileBuffer);
                s_haveContracts = 1;
            }
        }
        SV_SetPlaylistFetchedTime();
        s_completedOnlineWAD = 1;
        //BLOPS_NULLSUB();
        //LargeLocal::~LargeLocal(&uncompressedFileBuffer_large_local);
    }
}

void __cdecl tlEndianSwapMemory32(int *Data, unsigned int Bytes)
{
    unsigned int i; // [esp+8h] [ebp-8h]

    if ( (Bytes & 3) != 0
        && _tlAssert(
                 "C:\\projects_pc\\cod\\codsrc\\tl\\base\\include\\tl_defs.h",
                 652,
                 "(Bytes&3) == 0",
                 "Size must be a multiple of 4.") )
    {
        __debugbreak();
    }
    for ( i = 0; i < Bytes >> 2; ++i )
    {
        *Data = (*Data << 24) | ((*Data & 0xFF00) << 8) | ((unsigned int)*Data >> 8) & 0xFF00 | HIBYTE(*Data);
        ++Data;
    }
}

bool __cdecl LiveStorage_DecompressBuffer(
                unsigned __int8 *compressedBuffer,
                unsigned int compressedSize,
                unsigned __int8 *uncompressedBuffer,
                unsigned int uncompressedSize)
{
    int err; // [esp+0h] [ebp-38h]
    z_stream_s stream; // [esp+4h] [ebp-34h] BYREF

    memset(&stream.zalloc, 0, 12);
    stream.avail_in = compressedSize;
    stream.next_in = compressedBuffer;
    inflateInit2_(&stream, 15, "1.2.3", (int)sizeof(z_stream_s))   /* nx-port: was 52, the x86 sizeof */;
    stream.avail_out = uncompressedSize;
    stream.next_out = uncompressedBuffer;
    err = inflate(&stream, 0);
    if ( err != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_storage_pub.cpp",
                    142,
                    0,
                    "%s",
                    "err == Z_STREAM_END") )
    {
        __debugbreak();
    }
    inflateEnd(&stream);
    return err == 1;
}

