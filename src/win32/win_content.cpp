#include "win_content.h"

#include <universal/dvar.h>
#include <universal/q_shared.h>
#include <database/db_registry.h>
#include <steam/steam_api.h>

struct contentPack_t // sizeof=0x9C
{
    char name[128];
    char label[16];
    int mask;
    bool available;
    bool enabled;
    const dvar_t *dvar;
};

static contentPack_t s_contentPacks[8]; // retail SP 0x0276AE38
static int s_contentPackCount;          // retail SP 0x0276B1E4

const dvar_t *xblive_anymappacks;
const dvar_t *xblive_allmappacks;
const dvar_t *xblive_contentpath;

// retail SP FUN_006471a0
static unsigned int Content_GetSteamAppIdForPack(int mask)
{
    switch ( mask )
    {
    case 4:
    case 0x40:
        return 0xA6DF;
    case 8:
        return 0xA6DC;
    case 0x10:
        return 0xA6DE;
    case 0x20:
        return 0xA6E3;
    default:
        return 0;
    }
}

// Not retail: used when the Steam interface is unavailable so that DLC which is present on disk
// still shows up in the menus.
static const char *Content_GetProbeZoneForPack(int mask)
{
    switch ( mask )
    {
    case 4:
        return "zombie_cod5_prototype";
    case 8:
        return "zombie_cosmodrome";
    case 0x10:
        return "zombie_coast";
    case 0x20:
        return "zombie_temple";
    case 0x40:
        return "zombie_moon";
    default:
        return NULL;
    }
}

// retail SP FUN_0044fe10
static bool Content_IsPackInstalled(int mask)
{
    unsigned int appId; // eax
    const char *zone;

    appId = Content_GetSteamAppIdForPack(mask);
    if ( !appId )
        return 0;
#ifndef KISAK_NX   // no Steam: the zone probe below decides
    if ( SteamApps() )
    {
        if ( SteamApps()->BIsDlcInstalled(appId) )
            return 1;
    }
#endif
    zone = Content_GetProbeZoneForPack(mask);
    return zone && DB_FileExists(zone, FFD_DEFAULT);
}

// retail SP FUN_00866b80
static const dvar_t *Content_RegisterPackDvar(int mask)
{
    switch ( mask )
    {
    case 4:
        return _Dvar_RegisterBool("dlc1", 1, 0, "");
    case 8:
        return _Dvar_RegisterBool("dlc2", 1, 0, "");
    case 0x10:
        return _Dvar_RegisterBool("dlc3", 1, 0, "");
    case 0x20:
        return _Dvar_RegisterBool("dlc4", 1, 0, "");
    case 0x40:
        _Dvar_RegisterBool("dlc1", 1, 0, "");
        return _Dvar_RegisterBool("dlc5", 1, 0, "");
    default:
        return NULL;
    }
}

// retail SP FUN_00866c80
static void Content_AddPack(int mask)
{
    contentPack_t *pack;
    const char *label;
    int i;

    if ( !Content_IsPackInstalled(mask) )
        return;
    if ( s_contentPackCount >= (int)(sizeof(s_contentPacks) / sizeof(s_contentPacks[0])) )
        return;

    pack = &s_contentPacks[s_contentPackCount];
    pack->mask = mask;
    pack->available = 1;
    pack->enabled = 1;
    pack->dvar = Content_RegisterPackDvar(mask);

    label = mask == 2 ? "ORIGINAL_MAPS" : "DLC_UNKNOWN";
    for ( i = 1; i < 7; ++i )
    {
        if ( mask == 2 << i )
        {
            label = va("DLC_%d", i);
            break;
        }
    }
    I_strncpyz(pack->name, label, sizeof(pack->name));
    I_strncpyz(pack->label, label, sizeof(pack->label));
    ++s_contentPackCount;
}

// retail SP FUN_004e8350
void __cdecl Content_Init()
{
    s_contentPackCount = 0;
    s_contentPacks[0].mask = 2;
    s_contentPacks[0].available = 1;
    s_contentPacks[0].enabled = 1;
    I_strncpyz(s_contentPacks[0].name, "MP_ORIGINAL_MAPS", sizeof(s_contentPacks[0].name));
    I_strncpyz(s_contentPacks[0].label, "ORIGINAL_MAPS", sizeof(s_contentPacks[0].label));
    ++s_contentPackCount;

    xblive_anymappacks = _Dvar_RegisterBool("xblive_anymappacks", 0, 0x40u, "");
    xblive_allmappacks = _Dvar_RegisterBool("xblive_allmappacks", 0, 0x40u, "");
    xblive_contentpath = _Dvar_RegisterString("xblive_contentpath", "", 0x40u, "");

    Content_AddPack(4);
    Content_AddPack(8);
    Content_AddPack(0x10);
    Content_AddPack(0x20);
    Content_AddPack(0x40);
}

// retail SP FUN_00574510
int __cdecl Content_GetAvailableContentPacks()
{
    int packs; // eax
    int i;

    packs = 2;
    for ( i = 1; i < s_contentPackCount; ++i )
    {
        if ( s_contentPacks[i].available )
            packs |= s_contentPacks[i].mask;
    }
    return packs;
}

// retail SP FUN_0041eec0
bool __cdecl Content_IsPackAvailable(int mask)
{
    int i;

    if ( mask == 2 )
        return 1;
    for ( i = 0; i < s_contentPackCount; ++i )
    {
        if ( s_contentPacks[i].mask == mask )
            return s_contentPacks[i].available;
    }
    return 0;
}
