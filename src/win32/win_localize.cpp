#include "win_localize.h"
#include <universal/com_fileaccess.h>
#include <universal/assertive.h>
#include <stringed/stringed_hooks.h>
#include <universal/q_parse.h>
#include <universal/q_shared.h>
#include <cstring>

struct// $8CB265A9D3778DFC1F2AA7A5F0192391 // sizeof=0x8
{                                       // XREF: .data:localization/r
    char *language;                     // XREF: Win_InitLocalization(void)+6/w
    char *strings;                      // XREF: Win_InitLocalization(void)+10/w
} localization;

char language_buffer[4096];

int Win_InitLocalization()
{
    signed int size; // [esp+0h] [ebp-10h]
    int sizea; // [esp+0h] [ebp-10h]
    FILE *fp; // [esp+4h] [ebp-Ch]
    int i; // [esp+8h] [ebp-8h]
    int lang; // [esp+Ch] [ebp-4h] BYREF

    localization.language = 0;
    localization.strings = 0;
    fp = FS_FileOpenReadText("localization.txt");

    if (!fp)
    {
        iassert(0); // LWSS ADD: you probably need to change the working dir!
        return 0;
    }

    size = FS_FileGetFileSize(fp);
    if (size >= 4096
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_localize.cpp",
            39,
            0,
            "%s",
            "size < LANGUAGE_BUF_SIZE"))
    {
        __debugbreak();
    }
    localization.language = language_buffer;
    sizea = FS_FileRead(language_buffer, size, fp);
    FS_FileClose(fp);
#ifdef KISAK_NX
    // Opened with "rt": on Windows the CRT strips CR in text mode. newlib
    // has no text mode, so strip them here -- otherwise the language parses
    // as "english\r" and every zone\<lang>\*.ff path misses.
    {
        int rd = 0, wr = 0;
        for (; rd < sizea; ++rd)
            if (language_buffer[rd] != 0x0D)
                language_buffer[wr++] = language_buffer[rd];
        sizea = wr;
    }
#endif
    if (sizea)
    {
        localization.language[sizea] = 0;
        lang = 0;
        for (i = 0; localization.language[i]; ++i)
        {
            if (localization.language[i] == 10)
            {
                localization.language[i] = 0;
                localization.strings = &localization.language[i + 1];
                SEH_GetLanguageIndexForName(localization.language, &lang);
                return lang;
            }
        }
        return lang;
    }
    else
    {
        localization.language = 0;
        return 0;
    }
}

void __cdecl Win_ShutdownLocalization()
{
    localization.language = 0;
    localization.strings = 0;
}

char *__cdecl Win_LocalizeRef(const char *ref)
{
    const char *v1; // eax
    const char *v3; // eax
    const char *strings; // [esp+14h] [ebp-Ch] BYREF
    int useRef; // [esp+18h] [ebp-8h]
    const char *token; // [esp+1Ch] [ebp-4h]

    Com_BeginParseSession("localization");
    strings = localization.strings;
    do
    {
        token = (const char *)Com_Parse(&strings);
        if ( !*token )
        {
            Com_EndParseSession();
            v1 = va("unlocalized: %s", ref);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_localize.cpp", 112, 0, v1) )
                __debugbreak();
            return Win_CopyLocalizationString(ref);
        }
        useRef = strcmp(token, ref) == 0;
        token = (const char *)Com_Parse(&strings);
        if ( !*token )
        {
            Com_EndParseSession();
            v3 = va("missing value: %s", ref);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_localize.cpp", 121, 0, v3) )
                __debugbreak();
            return Win_CopyLocalizationString(ref);
        }
    }
    while ( !useRef );
    Com_EndParseSession();
    return Win_CopyLocalizationString(token);
}

char *__cdecl Win_CopyLocalizationString(const char *string)
{
    return va("%s", string);
}

char *__cdecl Win_GetLanguage()
{
    if ( !localization.language
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\win32\\win_localize.cpp",
                    140,
                    0,
                    "%s",
                    "localization.language") )
    {
        __debugbreak();
    }
    return localization.language;
}

