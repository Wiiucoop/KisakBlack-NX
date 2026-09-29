#include "live_fileshare_cache.h"
#include <universal/com_memory.h>

fshCacheFileDetails_t *s_fshCache;
fshCacheRatingDetails_t *s_fshRatingCache;

int s_fshCacheFileCount;
int s_fshCacheRatingCount;

bool __cdecl Live_FileShare_CacheInit()
{
    if ( s_fshCache
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare_cache.cpp",
                    37,
                    0,
                    "%s",
                    "s_fshCache == NULL") )
    {
        __debugbreak();
    }
    if ( s_fshRatingCache
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\live\\live_fileshare_cache.cpp",
                    38,
                    0,
                    "%s",
                    "s_fshRatingCache == NULL") )
    {
        __debugbreak();
    }
    s_fshCache = (fshCacheFileDetails_t *)Hunk_Alloc(300 * sizeof(fshCacheFileDetails_t), "fileShareCache", 36);
    s_fshCacheFileCount = 0;
    s_fshRatingCache = (fshCacheRatingDetails_t *)Hunk_Alloc(300 * sizeof(fshCacheRatingDetails_t), "fileShareCache", 36);
    s_fshCacheRatingCount = 0;
    return s_fshCache && s_fshRatingCache;
}

void __cdecl Live_FileShare_CacheShutdown()
{
    s_fshCache = 0;
    s_fshCacheFileCount = 0;
    s_fshRatingCache = 0;
    s_fshCacheRatingCount = 0;
}

int __cdecl Live_FileShare_Hash(unsigned __int64 fileID, fileShareLocation location)
{
    return (int)(location ^ fileID ^ HIDWORD(fileID)) % 300;
}

char __cdecl Live_FileShare_IsCached(unsigned __int64 fileID, fileShareLocation location, int *index)
{
    int idx; // [esp+8h] [ebp-4h]

    if ( !s_fshCache )
        return 0;
    idx = Live_FileShare_Hash(fileID, location);
    if ( LODWORD(s_fshCache[idx].descriptor.m_fileID) != (unsigned int)fileID
        || __PAIR64__(s_fshCache[idx].location, HIDWORD(s_fshCache[idx].descriptor.m_fileID)) != __PAIR64__(
                                                                                                                                                                                             location,
                                                                                                                                                                                             HIDWORD(fileID)) )
    {
        return 0;
    }
    *index = idx;
    return 1;
}

void __cdecl Live_FileShareCache_AddDescriptor(
                unsigned __int64 fileID,
                fileShareLocation location,
                bdFileMetaData *descriptor)
{
    int v3; // ecx

    if ( s_fshCache || Live_FileShare_CacheInit() )
    {
        v3 = Live_FileShare_Hash(fileID, location);
        s_fshCache[v3].location = location;
        memcpy(&s_fshCache[v3], descriptor, 0x838u);
    }
}

char __cdecl Live_FileShareCache_GetDescriptor(
                unsigned __int64 fileID,
                fileShareLocation location,
                bdFileMetaData *descriptor)
{
    int index; // [esp+8h] [ebp-4h] BYREF

    index = 0;
    if ( !Live_FileShare_IsCached(fileID, location, &index) )
        return 0;
    memcpy(descriptor, &s_fshCache[index], sizeof(bdFileMetaData));
    return 1;
}

char __cdecl Live_FileShare_IsRatingCached(unsigned __int64 fileID, fileShareLocation location, int *index)
{
    int idx; // [esp+8h] [ebp-4h]

    if ( !s_fshRatingCache )
        return 0;
    idx = Live_FileShare_Hash(fileID, location);
    if ( LODWORD(s_fshRatingCache[idx].fileID) != (unsigned int)fileID
        || __PAIR64__(s_fshRatingCache[idx].location, HIDWORD(s_fshRatingCache[idx].fileID)) != __PAIR64__(
                                                                                                                                                                                            location,
                                                                                                                                                                                            HIDWORD(fileID)) )
    {
        return 0;
    }
    *index = idx;
    return 1;
}

void __cdecl Live_FileShareCache_AddRating(
                unsigned __int64 fileID,
                fileShareLocation location,
                bdVoteRankStatsInfo *rating)
{
    int index; // [esp+8h] [ebp-4h]

    if ( s_fshRatingCache || Live_FileShare_CacheInit() )
    {
        index = Live_FileShare_Hash(fileID, location);
        s_fshRatingCache[index].location = location;
        s_fshRatingCache[index].fileID = fileID;
        memcpy(&s_fshRatingCache[index].rating, rating, sizeof(s_fshRatingCache[index].rating));
    }
}

char __cdecl Live_FileShareCache_GetRating(
                unsigned __int64 fileID,
                fileShareLocation location,
                bdVoteRankStatsInfo *rating)
{
    int index; // [esp+8h] [ebp-4h] BYREF

    index = 0;
    if ( !Live_FileShare_IsRatingCached(fileID, location, &index) )
        return 0;
    memcpy(rating, &s_fshRatingCache[index].rating, sizeof(bdVoteRankStatsInfo));
    return 1;
}

