#include "win_common.h"

#include <Windows.h>
#include <io.h>
#include <direct.h>
#include <universal/q_shared.h>
#include <universal/com_shared.h>

char cwd[256];
char exePath[256];

_RTL_CRITICAL_SECTION s_criticalSection[75];

void __cdecl Sys_Mkdir(const char *path)
{
    _mkdir(path);
}

char *__cdecl Sys_Cwd()
{
    _getcwd(cwd, 255);
    cwd[255] = 0;
    return cwd;
}

const char *__cdecl Sys_DefaultCDPath()
{
    return "";
}

char *__cdecl Sys_DefaultInstallPath()
{
    char *v0; // eax
    unsigned int len; // [esp+0h] [ebp-8h]
    HINSTANCE__ *hinst; // [esp+4h] [ebp-4h]

    if ( !exePath[0] )
    {
        if ( IsDebuggerPresent() )
        {
            v0 = Sys_Cwd();
            I_strncpyz(exePath, v0, 256);
        }
        else
        {
            hinst = GetModuleHandleA(0);
            len = GetModuleFileNameA(hinst, exePath, 0x100u);
            if ( len == 256 )
                len = 255;
            while ( len && exePath[len] != 92 && exePath[len] != 47 && exePath[len] != 58 )
                --len;
            exePath[len] = 0;
        }
    }
    return exePath;
}

HunkUser **__cdecl Sys_ListFiles(char *directory, char *extension, char *filter, int *numfiles, int wantsubs)
{
    char *v6; // eax
    _finddata64i32_t pfd; // [esp+0h] [ebp-10248h] BYREF
    int v8; // [esp+12Ch] [ebp-1011Ch]
    HunkUser **v9; // [esp+130h] [ebp-10118h]
    int hFile; // [esp+134h] [ebp-10114h]
    char *list[16384]; // [esp+138h] [ebp-10110h] BYREF
    int numfilesa; // [esp+10138h] [ebp-110h] BYREF
    HunkUser *user; // [esp+1013Ch] [ebp-10Ch]
    char dest[256]; // [esp+10140h] [ebp-108h] BYREF
    int i; // [esp+10244h] [ebp-4h]

    if ( filter )
    {
        user = Hunk_UserCreate(0x20000, HU_SCHEME_DEFAULT, 0, 0, "Sys_ListFiles", 3);
        numfilesa = 0;
        Sys_ListFilteredFiles(user, directory, "", filter, list, &numfilesa);
        list[numfilesa] = 0;
        *numfiles = numfilesa;
        if ( numfilesa )
        {
            v9 = (HunkUser **)Hunk_UserAlloc(user, sizeof(char *) * (numfilesa + 2), sizeof(char *), 0);
            *v9++ = user;
            for ( i = 0; i < numfilesa; ++i )
                v9[i] = (HunkUser *)list[i];
            v9[i] = 0;
            return v9;
        }
        else
        {
            Hunk_UserDestroy(user);
            return 0;
        }
    }
    else
    {
        if ( !extension )
            extension = (char *)"";
        if ( *extension != 47 || extension[1] )
        {
            v8 = 16;
        }
        else
        {
            extension = (char *)"";
            v8 = 0;
        }
        if ( *extension )
            Com_sprintf(dest, 0x100u, "%s\\*.%s", directory, extension);
        else
            Com_sprintf(dest, 0x100u, "%s\\*", directory);
        numfilesa = 0;
        hFile = _findfirst64i32(dest, &pfd);
        if ( hFile == -1 )
        {
            *numfiles = 0;
            return 0;
        }
        else
        {
            user = Hunk_UserCreate(0x20000, HU_SCHEME_DEFAULT, 0, 0, "Sys_ListFiles", 3);
            do
            {
                if ( (!wantsubs && v8 != (pfd.attrib & 0x10) || wantsubs && (pfd.attrib & 0x10) != 0)
                    && ((pfd.attrib & 0x10) == 0
                     || I_stricmp(pfd.name, ".") && I_stricmp(pfd.name, "..") && I_stricmp(pfd.name, "CVS"))
                    && (!*extension || HasFileExtension(pfd.name, extension)) )
                {
                    v6 = Hunk_CopyString(user, pfd.name);
                    list[numfilesa++] = v6;
                    if ( numfilesa == 0x3FFF )
                        break;
                }
            }
            while ( _findnext64i32(hFile, &pfd) != -1 );
            list[numfilesa] = 0;
            _findclose(hFile);
            *numfiles = numfilesa;
            if ( numfilesa )
            {
                v9 = (HunkUser **)Hunk_UserAlloc(user, sizeof(char *) * (numfilesa + 2), sizeof(char *), 0);
                *v9++ = user;
                for ( i = 0; i < numfilesa; ++i )
                    v9[i] = (HunkUser *)list[i];
                v9[i] = 0;
                return v9;
            }
            else
            {
                Hunk_UserDestroy(user);
                return 0;
            }
        }
    }
}

void __cdecl Sys_ListFilteredFiles(
                HunkUser *user,
                const char *basedir,
                const char *subdirs,
                const char *filter,
                char **list,
                int *numfiles)
{
    char filename[256]; // [esp+10h] [ebp-338h] BYREF
    _finddata64i32_t findinfo; // [esp+110h] [ebp-238h] BYREF
    int findhandle; // [esp+23Ch] [ebp-10Ch]
    char search[260]; // [esp+240h] [ebp-108h] BYREF

    if ( *numfiles < 0x3FFF )
    {
        if ( strlen(subdirs) )
            Com_sprintf(search, 0x100u, "%s\\%s\\*", basedir, subdirs);
        else
            Com_sprintf(search, 0x100u, "%s\\*", basedir);
        findhandle = _findfirst64i32(search, &findinfo);
        if ( findhandle != -1 )
        {
            do
            {
                if ( (findinfo.attrib & 0x10) == 0
                    || I_stricmp(findinfo.name, ".") && I_stricmp(findinfo.name, "..") && I_stricmp(findinfo.name, "CVS") )
                {
                    if ( *numfiles >= 0x3FFF )
                        break;
                    if ( subdirs )
                        Com_sprintf(filename, 0x100u, "%s\\%s", subdirs, findinfo.name);
                    else
                        Com_sprintf(filename, 0x100u, "%s", findinfo.name);
                    if ( Com_FilterPath(filter, filename, 0) )
                        list[(*numfiles)++] = Hunk_CopyString(user, filename);
                }
            }
            while ( _findnext64i32(findhandle, &findinfo) != -1 );
            _findclose(findhandle);
        }
    }
}

bool __cdecl HasFileExtension(const char *name, const char *extension)
{
    char search[260]; // [esp+0h] [ebp-108h] BYREF

    Com_sprintf(search, 0x100u, "*.%s", extension);
    return I_stricmpwild(search, name) == 0;
}

int __cdecl Sys_DirectoryHasContents(const char *directory)
{
    _finddata64i32_t findinfo; // [esp+0h] [ebp-238h] BYREF
    int findhandle; // [esp+12Ch] [ebp-10Ch]
    char search[260]; // [esp+130h] [ebp-108h] BYREF

    Com_sprintf(search, 0x100u, "%s\\*", directory);
    findhandle = _findfirst64i32(search, &findinfo);
    if ( findhandle == -1 )
        return 0;
    do
    {
        if ( (findinfo.attrib & 0x10) == 0
            || I_stricmp(findinfo.name, ".") && I_stricmp(findinfo.name, "..") && I_stricmp(findinfo.name, "CVS") )
        {
            _findclose(findhandle);
            return 1;
        }
    }
    while ( _findnext64i32(findhandle, &findinfo) != -1 );
    _findclose(findhandle);
    return 0;
}

void __cdecl Sys_InitializeCriticalSections()
{
    int critSect; // [esp+0h] [ebp-4h]

    static int inited_1 = 0;
    if ( !inited_1 )
    {
        inited_1 = 1;
        for ( critSect = 0; critSect < 75; ++critSect )
            InitializeCriticalSection(&s_criticalSection[critSect]);
    }
}

void __cdecl Sys_EnterCriticalSection(CriticalSection critSect)
{
    if ( (unsigned int)critSect >= CRITSECT_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\win_common.cpp",
                    523,
                    0,
                    "critSect doesn't index CRITSECT_COUNT\n\t%i not in [0, %i)",
                    critSect,
                    75) )
    {
        __debugbreak();
    }
    EnterCriticalSection(&s_criticalSection[critSect]);
}

bool __cdecl Sys_TryEnterCriticalSection(CriticalSection critSect)
{
    if ( (unsigned int)critSect >= CRITSECT_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\win_common.cpp",
                    529,
                    0,
                    "critSect doesn't index CRITSECT_COUNT\n\t%i not in [0, %i)",
                    critSect,
                    75) )
    {
        __debugbreak();
    }
    return TryEnterCriticalSection(&s_criticalSection[critSect]);
}

void __cdecl Sys_LeaveCriticalSection(CriticalSection critSect)
{
    if ( (unsigned int)critSect >= CRITSECT_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\win_common.cpp",
                    535,
                    0,
                    "critSect doesn't index CRITSECT_COUNT\n\t%i not in [0, %i)",
                    critSect,
                    75) )
    {
        __debugbreak();
    }
    LeaveCriticalSection(&s_criticalSection[critSect]);
}

