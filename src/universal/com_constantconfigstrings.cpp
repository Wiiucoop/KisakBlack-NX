#include "com_constantconfigstrings.h"
#include "com_stringtable.h"
#include <qcommon/common.h>
#include <qcommon/com_gamemodes.h>
#include <stdlib.h>

const StringTable *configStringTable;
int configStringTableChecksum;

void __cdecl CCS_ClearConstantConfigStrings()
{
    configStringTable = 0;
    configStringTableChecksum = 0;
    Com_DPrintf(30, "CCS: Cleared\n");
}

void __cdecl CCS_ValidateChecksums(int serverChecksum, int clientChecksum)
{
    const char *v2; // [esp+4h] [ebp-10h]
    const char *v3; // [esp+Ch] [ebp-8h]
    bool bOnlinegame; // [esp+12h] [ebp-2h]
    bool bSystemlink; // [esp+13h] [ebp-1h]

    bSystemlink = Com_GameMode_IsGameMode(GAMEMODE_SYSTEMLINK);
    bOnlinegame = Com_GameMode_IsOnlineGame();
    if ( serverChecksum || !clientChecksum )
    {
        if ( clientChecksum || !serverChecksum )
        {
            if ( serverChecksum != clientChecksum )
            {
                Com_PrintError(
                    14,
                    "Our config string checksum %i doesn't match the server's checksum %i\n",
                    clientChecksum,
                    serverChecksum);
                Com_Error(ERR_DROP, "EXE_CONFIGSTRINGMISMATCH");
            }
        }
        else
        {
            if ( bSystemlink )
                v2 = "on";
            else
                v2 = "off";
            if ( bOnlinegame )
                Com_PrintError(
                    14,
                    "The server sent us a config checksum (%i) but we have don't have one, (onlinegame is %s)(systemlink is %s)\n",
                    serverChecksum,
                    "on",
                    v2);
            else
                Com_PrintError(
                    14,
                    "The server sent us a config checksum (%i) but we have don't have one, (onlinegame is %s)(systemlink is %s)\n",
                    serverChecksum,
                    "off",
                    v2);
            Com_Error(ERR_DROP, "EXE_CONFIGSTRINGMISMATCH3");
        }
    }
    else
    {
        if ( bSystemlink )
            v3 = "on";
        else
            v3 = "off";
        if ( bOnlinegame )
            Com_PrintError(
                14,
                "The server did not send us a config checksum but we have one (%i), (onlinegame is %s)(systemlink is %s)\n",
                clientChecksum,
                "on",
                v3);
        else
            Com_PrintError(
                14,
                "The server did not send us a config checksum but we have one (%i), (onlinegame is %s)(systemlink is %s)\n",
                clientChecksum,
                "off",
                v3);
        Com_Error(ERR_DROP, "EXE_CONFIGSTRINGMISMATCH2");
    }
}

int __cdecl CCS_GetChecksum()
{
    Com_DPrintf(30, "CCS: CCS_GetChecksum: %d\n", configStringTableChecksum);
    return configStringTableChecksum;
}

char __cdecl CCS_ShouldLoadConstConfigStrings(int party)
{
    if ( Com_GameMode_IsOnlineGame() && party )
        return 1;
    else
        return 1;
}

void __cdecl CCS_LoadConstantConfigStrings(const char *mapname, const char *gametype)
{
    char filename[256]; // [esp+0h] [ebp-108h] BYREF
    const char *dir; // [esp+104h] [ebp-4h]

    if ( !mapname
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_constantconfigstrings.cpp",
                    96,
                    0,
                    "%s",
                    "mapname") )
    {
        __debugbreak();
    }
    if ( !gametype
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_constantconfigstrings.cpp",
                    97,
                    0,
                    "%s",
                    "gametype") )
    {
        __debugbreak();
    }
    if ( !*mapname
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_constantconfigstrings.cpp",
                    98,
                    0,
                    "%s",
                    "mapname[0]") )
    {
        __debugbreak();
    }
    if ( !*gametype
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_constantconfigstrings.cpp",
                    99,
                    0,
                    "%s",
                    "gametype[0]") )
    {
        __debugbreak();
    }
    // SP's constant-config-string tables live under "sp/configStrings/", MP's under
    // "mp/configStrings/". Note the local `dir` is assigned and then NOT used - the literal is
    // passed directly - so the argument is what has to change, not just `dir`.
    // Evidence (zone scan of all 139 shipped .ff files):
    //   mp/configstrings/* -> common_mp.ff (168 entries) and patch_mp.ff (312). Zero in ANY SP zone.
    //   sp/configstrings/* -> patch.ff (5: configstrings_pc_zombie_{coast,cosmodrome,pentagon,
    //                        theater}_zom.csv + zombietron), plus zombie_pentagon.ff,
    //                        zombie_theater.ff, zombietron.ff.
    //   The retail SP exe keeps the same format string with the %s directory slot
    //   ("%s/configstrings/configstrings_pc_%s_%s.csv", 0x5d6eac), i.e. the prefix really is a
    //   parameter rather than baked in.
    // Non-fatal either way (a missing STRINGTABLE resolves to the default empty table, and both
    // the client and the server of a listen server compute the same checksum from it, so
    // CCS_CompareChecksums' EXE_CONFIGSTRINGMISMATCH2 Com_Error cannot trip). For "frontend"
    // neither prefix has a table, so this changes nothing today; it matters for the zombie maps
    // whose tables do ship. Audit finding C9 (asset-availability audit).
#ifdef KISAK_SP
    dir = "sp";
    Com_sprintf(filename, 0x100u, "%s/configStrings/configStrings_pc_%s_%s.csv", "sp", mapname, gametype);
#else
    dir = "mp";
    Com_sprintf(filename, 0x100u, "%s/configStrings/configStrings_pc_%s_%s.csv", "mp", mapname, gametype);
#endif
    StringTable_GetAsset(filename, (XAssetHeader *)&configStringTable);
    configStringTableChecksum = StringTable_Checksum(configStringTable, 0);
    Com_DPrintf(30, "CCS: CCS_LoadConstantConfigStrings %s: %d\n", filename, configStringTableChecksum);
}

const char *__cdecl CCS_GetConfigStringValue(int rowNum)
{
    if ( configStringTable )
        return StringTable_GetColumnValueForRow(configStringTable, rowNum, 1);
    else
        return "";
}

int __cdecl CCS_GetConfigStringNum(int rowNum)
{
    const char *ColumnValueForRow; // eax

    if ( !configStringTable )
        return 0;
    ColumnValueForRow = StringTable_GetColumnValueForRow(configStringTable, rowNum, 0);
    return atoi(ColumnValueForRow);
}

