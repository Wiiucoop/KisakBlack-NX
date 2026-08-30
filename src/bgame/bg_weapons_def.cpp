#include "bg_weapons_def.h"
#include "bg_weapons.h"

#include <universal/assertive.h>
#include <universal/com_files.h>
#include "bg_weapons_load_obj.h"
#include <clientscript/cscr_stringlist.h>
#include <universal/com_stringtable_obj.h>
#include "bg_weapons_ammo.h"
#include <game/g_items.h>
#include <qcommon/common.h>
#include <ui/ui_shared.h>
#include <game_mp/g_main_mp.h>
#include "bg_unlockable_items.h"

unsigned int bg_firstWeaponTableIndex;
unsigned int bg_lastParsedWeaponIndex;
unsigned int bg_lastWeaponTableIndex;
WeaponVariantDef *bg_weaponVariantDefs[2048];
WeaponVariantDefHash bg_weaponVariantNameHashTable[2048];
bool bg_weaponVariantNameHashTableSorted;

gitem_s bg_itemlist[2048];

unsigned int __cdecl BG_GetNumWeapons()
{
    return bg_lastParsedWeaponIndex + 1;
}

const WeaponVariantDef *__cdecl BG_GetWeaponVariantDef(unsigned int weaponIndex)
{
    if ( weaponIndex > bg_lastParsedWeaponIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_def.cpp",
                    63,
                    0,
                    "weaponIndex not in [0, bg_lastParsedWeaponIndex]\n\t%i not in [%i, %i]",
                    weaponIndex,
                    0,
                    bg_lastParsedWeaponIndex) )
    {
        __debugbreak();
    }
    return bg_weaponVariantDefs[weaponIndex];
}

const WeaponDef *__cdecl BG_GetWeaponDef(unsigned int weaponIndex)
{
    if ( weaponIndex > bg_lastParsedWeaponIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_def.cpp",
                    75,
                    0,
                    "weaponIndex not in [0, bg_lastParsedWeaponIndex]\n\t%i not in [%i, %i]",
                    weaponIndex,
                    0,
                    bg_lastParsedWeaponIndex) )
    {
        __debugbreak();
    }
    return bg_weaponVariantDefs[weaponIndex]->weapDef;
}

unsigned int __cdecl BG_GetWeaponIndex(const WeaponVariantDef *weapVariantDef)
{
    unsigned int weapIndex; // [esp+0h] [ebp-4h]

    if ( !weapVariantDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_def.cpp", 87, 0, "%s", "weapVariantDef") )
    {
        __debugbreak();
    }
    for ( weapIndex = 0; weapIndex <= bg_lastParsedWeaponIndex; ++weapIndex )
    {
        if ( weapVariantDef == bg_weaponVariantDefs[weapIndex] )
            return weapIndex;
    }
    return 0;
}

void __cdecl BG_ShutdownWeaponDefFiles()
{
    if ( fs_gameDirVar && *(_BYTE *)fs_gameDirVar->current.string )
    {
        BG_ClearWeaponDefInternal();
        BG_FreeWeaponDefStrings();
    }
    bg_lastParsedWeaponIndex = 0;
    bg_firstWeaponTableIndex = 0;
    bg_lastWeaponTableIndex = 0;
}

void BG_FreeWeaponDefStrings()
{
    unsigned int j; // [esp+0h] [ebp-10h]
    unsigned int ja; // [esp+0h] [ebp-10h]
    const WeaponVariantDef *weapVariantDef; // [esp+4h] [ebp-Ch]
    unsigned int i; // [esp+8h] [ebp-8h]
    const WeaponDef *weapDef; // [esp+Ch] [ebp-4h]

    for ( i = 1; i <= bg_lastParsedWeaponIndex; ++i )
    {
        weapVariantDef = bg_weaponVariantDefs[i];
        weapDef = weapVariantDef->weapDef;
        if ( (!weapVariantDef || !weapDef)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_def.cpp",
                        110,
                        0,
                        "%s",
                        "weapVariantDef && weapDef") )
        {
            __debugbreak();
        }
        for ( j = 0; j < 0x20; ++j )
        {
            if ( weapVariantDef->hideTags[j] )
                SL_RemoveRefToString(SCRIPTINSTANCE_SERVER, weapVariantDef->hideTags[j]);
        }
        for ( ja = 0; ja < 0x14; ++ja )
        {
            if ( weapDef->notetrackSoundMapKeys[ja] )
                SL_RemoveRefToString(SCRIPTINSTANCE_SERVER, weapDef->notetrackSoundMapKeys[ja]);
            if ( weapDef->notetrackSoundMapValues[ja] )
                SL_RemoveRefToString(SCRIPTINSTANCE_SERVER, weapDef->notetrackSoundMapValues[ja]);
        }
    }
}

void __cdecl BG_InitDefaultWeaponDef()
{
    if ( bg_lastParsedWeaponIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_def.cpp",
                    266,
                    0,
                    "%s",
                    "bg_lastParsedWeaponIndex == 0") )
    {
        __debugbreak();
    }
    bg_weaponVariantDefs[0] = BG_LoadDefaultWeaponVariantDef();
    bg_weaponVariantNameHashTable[0].weaponIndex = 0;
    bg_weaponVariantNameHashTable[0].hash = StringTable_HashString(bg_weaponVariantDefs[0]->szInternalName);
    bg_weaponVariantNameHashTableSorted = 0;
}

void __cdecl BG_ClearWeaponDef()
{
    int itemIdx; // [esp+0h] [ebp-4h]

    BG_InitDefaultWeaponDef();
    for ( itemIdx = 1; itemIdx < 2048; ++itemIdx )
        bg_itemlist[itemIdx].giType = IT_BAD;
    BG_ClearWeaponDefAmmo();
    BG_LoadPlayerAnimTypes();
    BG_InitWeaponStrings();
    BG_LoadWeaponMergeSupport();
    BG_ClearWeaponDefInternal();
}

void __cdecl BG_FillInAllWeaponItems()
{
    unsigned int weaponIndex; // [esp+0h] [ebp-8h]
    unsigned int weaponCount; // [esp+4h] [ebp-4h]

    //BLOPS_NULLSUB();
    weaponCount = BG_GetNumWeapons();
    for ( weaponIndex = 1; weaponIndex < weaponCount; ++weaponIndex )
        BG_SetupWeaponIndex(weaponIndex);
}

void __cdecl BG_SetupWeaponIndex(unsigned int weapIndex)
{
    BG_SetUpAmmoForWeapon(weapIndex);
    BG_FillInWeaponItems(weapIndex);
}

void __cdecl BG_FillInWeaponItems(unsigned int weapIndex)
{
    bg_itemlist[weapIndex].giType = IT_WEAPON;
}

void __cdecl BG_SetUpAmmoForWeapon(unsigned int weapIndex)
{
    BG_SetupAmmoIndexes(weapIndex);
    BG_SetupSharedAmmoIndexes(weapIndex);
    BG_SetupClipIndexes(weapIndex);
}

void __cdecl BG_SetupAmmoIndexes(unsigned int weapIndex)
{
    BG_SetupWeaponDefAmmoIndexes(weapIndex, bg_weaponVariantDefs[weapIndex]->weapDef, bg_weaponVariantDefs[weapIndex]);
}

void __cdecl BG_SetupSharedAmmoIndexes(unsigned int weapIndex)
{
    BG_SetupWeaponDefSharedAmmoIndexes(weapIndex, bg_weaponVariantDefs[weapIndex]->weapDef);
}

void __cdecl BG_SetupClipIndexes(unsigned int weapIndex)
{
    BG_SetupWeaponDefClipIndexes(bg_weaponVariantDefs[weapIndex]->weapDef, bg_weaponVariantDefs[weapIndex]);
}

int __cdecl BG_FindWeaponIndexForName(const char *name)
{
    WeaponVariantDefHash *ret; // [esp+0h] [ebp-10h]
    int nameHash; // [esp+4h] [ebp-Ch]
    WeaponVariantDefHash key; // [esp+8h] [ebp-8h] BYREF

    if ( !name )
        return 0;
    nameHash = StringTable_HashString(name);
    if ( !bg_weaponVariantNameHashTableSorted )
    {
        qsort(
            bg_weaponVariantNameHashTable,
            bg_lastParsedWeaponIndex + 1,
            8u,
            (int (__cdecl *)(const void *, const void *))BG_WeaponVariantNameHashCompare);
        bg_weaponVariantNameHashTableSorted = 1;
    }
    key.hash = nameHash;
    ret = (WeaponVariantDefHash *)bsearch(
                                                                    &key,
                                                                    bg_weaponVariantNameHashTable,
                                                                    bg_lastParsedWeaponIndex + 1,
                                                                    8u,
                                                                    (int (__cdecl *)(const void *, const void *))BG_WeaponVariantNameHashCompare);
    if ( ret )
        return ret->weaponIndex;
    else
        return 0;
}

int __cdecl BG_WeaponVariantNameHashCompare(unsigned int *a, unsigned int *b)
{
    if ( *a > *b )
        return 1;
    if ( *a >= *b )
        return 0;
    return -1;
}

int __cdecl BG_GetWeaponIndexForName(const char *name, void (__cdecl *regWeap)(unsigned int))
{
    WeaponVariantDef *weapVariantDef; // [esp+0h] [ebp-8h]
    unsigned int weapIndex; // [esp+4h] [ebp-4h]

    if ( !*name || !I_stricmp(name, "none") )
        return 0;
    weapIndex = BG_FindWeaponIndexForName(name);
    if ( weapIndex )
        return weapIndex;
    weapVariantDef = BG_LoadWeaponVariantDef(name);
    if ( weapVariantDef )
    {
        if ( BG_IsDefaultWeapon(name) )
            return 0;
        else
            return BG_SetupWeaponVariantDef(weapVariantDef, regWeap);
    }
    else
    {
        Com_DPrintf(17, "Couldn't find weapon \"%s\"\n", name);
        return 0;
    }
}

int __cdecl BG_SetupWeaponVariantDef(WeaponVariantDef *weapVariantDef, void (__cdecl *regWeap)(unsigned int))
{
    int weapIndex; // [esp+4h] [ebp-4h]

    if ( ++bg_lastParsedWeaponIndex >= 0x800
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_def.cpp",
                    309,
                    0,
                    "bg_lastParsedWeaponIndex doesn't index ARRAY_COUNT( bg_weaponVariantDefs )\n\t%i not in [0, %i)",
                    bg_lastParsedWeaponIndex,
                    2048) )
    {
        __debugbreak();
    }
    weapIndex = bg_lastParsedWeaponIndex;
    bg_weaponVariantDefs[bg_lastParsedWeaponIndex] = weapVariantDef;
    bg_weaponVariantNameHashTable[weapIndex].weaponIndex = weapIndex;
    bg_weaponVariantNameHashTable[weapIndex].hash = StringTable_HashString(bg_weaponVariantDefs[weapIndex]->szInternalName);
    bg_weaponVariantNameHashTableSorted = 0;
    BG_SetupWeaponIndex(weapIndex);
    BG_SetupWeaponAlts(weapIndex, regWeap);
    BG_SetupDualWieldAlts(weapIndex, regWeap);
    BG_SetupWeaponMountedVersions(weapIndex, regWeap);
    if ( regWeap )
        regWeap(weapIndex);
    return weapIndex;
}

void __cdecl BG_SetupWeaponAlts(unsigned int weapIndex, void (__cdecl *regWeap)(unsigned int))
{
    const char *v2; // eax
    int altWeaponIndex; // [esp+0h] [ebp-8h]
    WeaponVariantDef *weapVariantDef; // [esp+4h] [ebp-4h]

    if ( weapIndex > bg_lastParsedWeaponIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_def.cpp",
                    183,
                    0,
                    "weapIndex not in [0, bg_lastParsedWeaponIndex]\n\t%i not in [%i, %i]",
                    weapIndex,
                    0,
                    bg_lastParsedWeaponIndex) )
    {
        __debugbreak();
    }
    weapVariantDef = bg_weaponVariantDefs[weapIndex];
    weapVariantDef->altWeaponIndex = 0;
    if ( *weapVariantDef->szAltWeaponName )
    {
        altWeaponIndex = BG_GetWeaponIndexForName((char*)weapVariantDef->szAltWeaponName, regWeap);
        if ( !altWeaponIndex )
        {
            v2 = BG_WeaponName(weapIndex);
            Com_Error(ERR_DROP, "could not find altWeapon '%s' for weapon '%s'", weapVariantDef->szAltWeaponName, v2);
        }
        weapVariantDef->altWeaponIndex = altWeaponIndex;
    }
}

void __cdecl BG_SetupDualWieldAlts(unsigned int weapIndex, void (__cdecl *regWeap)(unsigned int))
{
    const char *v2; // eax
    int altWeaponIndex; // [esp+0h] [ebp-8h]
    WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    weapDef = bg_weaponVariantDefs[weapIndex]->weapDef;
    if ( weapDef->bDualWield )
    {
        weapDef->dualWieldWeaponIndex = 0;
        if ( *weapDef->szDualWieldWeaponName )
        {
            altWeaponIndex = BG_GetWeaponIndexForName((char*)weapDef->szDualWieldWeaponName, regWeap);
            if ( !altWeaponIndex )
            {
                v2 = BG_WeaponName(weapIndex);
                Com_Error(ERR_DROP, "could not find alt Dual Wield Weapon '%s' for weapon '%s'", weapDef->szDualWieldWeaponName, v2);
            }
            weapDef->dualWieldWeaponIndex = altWeaponIndex;
        }
    }
}

void __cdecl BG_SetupWeaponMountedVersions(unsigned int weaponIndex, void (__cdecl *regWeap)(unsigned int))
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *standMountedWeapdef; // [esp-4h] [ebp-Ch]
    const char *crouchMountedWeapdef; // [esp-4h] [ebp-Ch]
    const char *proneMountedWeapdef; // [esp-4h] [ebp-Ch]
    int mountedWeaponIndex; // [esp+0h] [ebp-8h]
    int mountedWeaponIndexa; // [esp+0h] [ebp-8h]
    int mountedWeaponIndexb; // [esp+0h] [ebp-8h]
    WeaponDef *weapDef; // [esp+4h] [ebp-4h]

    weapDef = bg_weaponVariantDefs[weaponIndex]->weapDef;
    weapDef->standMountedIndex = 0;
    weapDef->crouchMountedIndex = 0;
    weapDef->proneMountedIndex = 0;
    if ( *weapDef->standMountedWeapdef )
    {
        mountedWeaponIndex = BG_GetWeaponIndexForName(weapDef->standMountedWeapdef, regWeap);
        if ( !mountedWeaponIndex )
        {
            standMountedWeapdef = weapDef->standMountedWeapdef;
            v2 = BG_WeaponName(weaponIndex);
            Com_Error(ERR_DROP, "could not find standingWeapdef '%s' for weapon '%s' Please add the line 'weapon,sp/%s' to your level csv", weapDef->standMountedWeapdef, v2, standMountedWeapdef);
        }
        weapDef->standMountedIndex = mountedWeaponIndex;
    }
    if ( *weapDef->crouchMountedWeapdef )
    {
        mountedWeaponIndexa = BG_GetWeaponIndexForName(weapDef->crouchMountedWeapdef, regWeap);
        if ( !mountedWeaponIndexa )
        {
            crouchMountedWeapdef = weapDef->crouchMountedWeapdef;
            v3 = BG_WeaponName(weaponIndex);
            Com_Error(ERR_DROP, "could not find crouchingWeapdef '%s' for weapon '%s' Please add the line 'weapon,sp/%s' to your level csv", weapDef->crouchMountedWeapdef, v3, crouchMountedWeapdef);
        }
        weapDef->crouchMountedIndex = mountedWeaponIndexa;
    }
    if ( *weapDef->proneMountedWeapdef )
    {
        mountedWeaponIndexb = BG_GetWeaponIndexForName(weapDef->proneMountedWeapdef, regWeap);
        if ( !mountedWeaponIndexb )
        {
            proneMountedWeapdef = weapDef->proneMountedWeapdef;
            v4 = BG_WeaponName(weaponIndex);
            Com_Error(ERR_DROP, "could not find proneWeapdef '%s' for weapon '%s' Please add the line 'weapon,sp/%s' to your level csv", weapDef->proneMountedWeapdef, v4, proneMountedWeapdef);
        }
        weapDef->proneMountedIndex = mountedWeaponIndexb;
    }
}

bool __cdecl BG_IsDefaultWeapon(const char *name)
{
    bool v2; // [esp+0h] [ebp-8h]

    v2 = fs_gameDirVar && *(_BYTE *)fs_gameDirVar->current.string;
    return !v2 && useFastFile->current.enabled && DB_IsXAssetDefault(ASSET_TYPE_WEAPON, name);
}

int __cdecl BG_GetWeaponIndexForName(const char *name)
{
    return BG_GetWeaponIndexForName(name, 0);
}

int __cdecl BG_GetWeaponTableItemIndex(int weaponIndex)
{
    int v2; // [esp+0h] [ebp-Ch]
    const WeaponVariantDef *weapVarDef; // [esp+4h] [ebp-8h]
    WeaponVariantDef *baseWeaponVariantDef; // [esp+8h] [ebp-4h]

    if ( weaponIndex >= 2048
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_def.cpp",
                    598,
                    0,
                    "%s",
                    "weaponIndex < MAX_WEAPONS") )
    {
        __debugbreak();
    }
    weapVarDef = BG_GetWeaponVariantDef(weaponIndex);
    if ( weapVarDef->iVariantCount >= 0 )
        v2 = weaponIndex;
    else
        v2 = -weapVarDef->iVariantCount;
    baseWeaponVariantDef = bg_weaponVariantDefs[v2];
    if ( (!baseWeaponVariantDef || !baseWeaponVariantDef->weapDef)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_def.cpp",
                    601,
                    0,
                    "%s",
                    "baseWeaponVariantDef && baseWeaponVariantDef->weapDef") )
    {
        __debugbreak();
    }
    if ( baseWeaponVariantDef && baseWeaponVariantDef->weapDef )
        return baseWeaponVariantDef->weapDef->itemIndex;
    else
        return 0;
}

char __cdecl BG_LoadWeaponTable(const char *weaponNamePostfix, void (__cdecl *regWeap)(unsigned int))
{
    int itemIndex; // [esp+0h] [ebp-18h]
    int item; // [esp+4h] [ebp-14h]
    int itemsInGroup; // [esp+8h] [ebp-10h]
    itemGroup_t itemGroupIndex; // [esp+10h] [ebp-8h]
    int nullTableWeapon; // [esp+14h] [ebp-4h]

    if ( G_ExitAfterToolComplete() )
        return 1;
    if ( bg_firstWeaponTableIndex )
        return 1;
    if ( bg_lastParsedWeaponIndex != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_def.cpp",
                    626,
                    0,
                    "%s",
                    "bg_lastParsedWeaponIndex == 1") )
    {
        __debugbreak();
    }
    bg_firstWeaponTableIndex = bg_lastParsedWeaponIndex + 1;
    nullTableWeapon = BG_UnlockablesGetDefaultWeaponItem();
    for ( itemGroupIndex = ITEMGROUP_SMG; itemGroupIndex < ITEMGROUP_COUNT; ++itemGroupIndex )
    {
        if ( itemGroupIndex == ITEMGROUP_PISTOL
            || (unsigned int)itemGroupIndex <= ITEMGROUP_CQB
            || itemGroupIndex == ITEMGROUP_LAUNCHER
            || itemGroupIndex == ITEMGROUP_SPECIAL
            || itemGroupIndex == ITEMGROUP_SNIPER
            || itemGroupIndex == ITEMGROUP_SHOTGUN
            || itemGroupIndex == ITEMGROUP_EXPLOSIVE
            || itemGroupIndex == ITEMGROUP_GRENADE
            || itemGroupIndex == ITEMGROUP_SPECIALGRENADE
            || itemGroupIndex == ITEMGROUP_INVENTORY
            || itemGroupIndex == ITEMGROUP_LMG )
        {
            itemsInGroup = BG_UnlockablesBuildItemListForGroupForWeaponTable(itemGroupIndex);
            for ( item = 0; item < itemsInGroup; ++item )
            {
                itemIndex = BG_UnlockablesGetItemIndexInSlot(item);
                if ( itemIndex != nullTableWeapon )
                    BG_LoadWeaponTableBaseAndVariants(itemIndex, weaponNamePostfix, regWeap);
            }
        }
    }
    bg_lastWeaponTableIndex = bg_lastParsedWeaponIndex;
    Com_DPrintf(
        17,
        "BG_LoadWeaponTable: Loaded %d weapons from table\n",
        bg_lastParsedWeaponIndex - bg_firstWeaponTableIndex);
    return 1;
}

char __cdecl BG_LoadWeaponTableBaseAndVariants(
                int weaponTableItemIndex,
                const char *weaponNamePostfix,
                void (__cdecl *regWeap)(unsigned int))
{
    char *v4; // eax
    int currentWeaponIndex; // [esp+0h] [ebp-14h]
    WeaponVariantDef *baseWeaponVariantDef; // [esp+4h] [ebp-10h]
    const char *weaponReference; // [esp+8h] [ebp-Ch]
    char success; // [esp+Fh] [ebp-5h]
    unsigned int baseWeaponIndex; // [esp+10h] [ebp-4h]

    weaponReference = BG_UnlockablesGetItemRef(weaponTableItemIndex);
    if ( !*weaponReference )
        return 0;
    currentWeaponIndex = bg_lastParsedWeaponIndex;
    v4 = va("%s%s", weaponReference, weaponNamePostfix);
    baseWeaponIndex = BG_GetWeaponIndexForName(v4, regWeap);
    if ( baseWeaponIndex )
    {
        success = BG_WeaponVariantPrecache_r(
                                weaponTableItemIndex,
                                baseWeaponIndex,
                                regWeap,
                                weaponReference,
                                weaponNamePostfix,
                                ATTACHMENT_POINT_TOP,
                                0,
                                0);
        if ( !success )
            Com_PrintWarning(
                17,
                "WARNING: Could not load all weapon variants for '%s%s'\n",
                weaponReference,
                weaponNamePostfix);
        baseWeaponVariantDef = bg_weaponVariantDefs[baseWeaponIndex];
        if ( (!baseWeaponVariantDef || !baseWeaponVariantDef->weapDef)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_def.cpp",
                        586,
                        0,
                        "%s",
                        "baseWeaponVariantDef && baseWeaponVariantDef->weapDef") )
        {
            __debugbreak();
        }
        baseWeaponVariantDef->iVariantCount = bg_lastParsedWeaponIndex - currentWeaponIndex - 1;
        baseWeaponVariantDef->weapDef->itemIndex = weaponTableItemIndex;
        return success;
    }
    else
    {
        Com_Error(
            ERR_DROP,
            "Could not load base weapon '%s%s' defined in the weapon table",
            weaponReference,
            weaponNamePostfix);
        return 0;
    }
}

char __cdecl BG_WeaponVariantPrecache_r(
                int weaponTableItemIndex,
                int baseWeaponIndex,
                void (__cdecl *regWeap)(unsigned int),
                const char *namePrefix,
                const char *namePostfix,
                eAttachmentPoint point,
                int index,
                bool alreadyRegistered)
{
    const char *AttachmentName; // eax
    char weaponName[68]; // [esp+8h] [ebp-50h] BYREF
    eAttachment attachment; // [esp+50h] [ebp-8h]
    bool success; // [esp+57h] [ebp-1h]

    if ( !alreadyRegistered )
    {
        alreadyRegistered = 1;
        BG_WeaponVariantPrecache(baseWeaponIndex, regWeap, namePrefix, namePostfix);
    }
    if ( point == ATTACHMENT_POINT_COUNT )
        return 1;
    success = 1;
    if ( !index )
        success = BG_WeaponVariantPrecache_r(
                                weaponTableItemIndex,
                                baseWeaponIndex,
                                regWeap,
                                namePrefix,
                                namePostfix,
                                (eAttachmentPoint)(point + 1),
                                0,
                                alreadyRegistered);
    if ( index == BG_UnlockablesGetNumItemAttachmentsAtPoint(weaponTableItemIndex, point) )
        return success;
    BG_WeaponVariantPrecache_r(
        weaponTableItemIndex,
        baseWeaponIndex,
        regWeap,
        namePrefix,
        namePostfix,
        point,
        index + 1,
        alreadyRegistered);
    attachment = BG_UnlockablesGetItemAttachmentAtPointByIndex(weaponTableItemIndex, point, index);
    if ( attachment == ATTACHMENT_NONE || attachment > ATTACHMENT_SNUB )
        return success;
    AttachmentName = BG_GetAttachmentName(attachment);
    Com_sprintf(weaponName, 0x40u, "%s_%s", namePrefix, AttachmentName);
    return BG_WeaponVariantPrecache_r(
                     weaponTableItemIndex,
                     baseWeaponIndex,
                     regWeap,
                     weaponName,
                     namePostfix,
                     (eAttachmentPoint)(point + 1),
                     0,
                     0)
            && success;
}

void __cdecl BG_WeaponVariantPrecache(
                int baseWeaponIndex,
                void (__cdecl *regWeap)(unsigned int),
                const char *namePrefix,
                const char *namePostfix)
{
    char *v4; // eax
    WeaponVariantDef *dualWeapVariantDef; // [esp+0h] [ebp-1Ch]
    unsigned int dualWeaponIndex; // [esp+4h] [ebp-18h]
    WeaponVariantDef *altWeapVariantDef; // [esp+8h] [ebp-14h]
    unsigned int altWeaponIndex; // [esp+Ch] [ebp-10h]
    const WeaponDef *weapDef; // [esp+10h] [ebp-Ch]
    WeaponVariantDef *weapVariantDef; // [esp+14h] [ebp-8h]
    unsigned int weaponIndex; // [esp+18h] [ebp-4h]

    v4 = va("%s%s", namePrefix, namePostfix);
    weaponIndex = BG_GetWeaponIndexForName(v4, regWeap);
    if ( weaponIndex )
    {
        weapVariantDef = bg_weaponVariantDefs[weaponIndex];
        if ( weapVariantDef && weapVariantDef->weapDef )
        {
            weapVariantDef->iVariantCount = -baseWeaponIndex;
            weapDef = weapVariantDef->weapDef;
            if ( *weapVariantDef->szAltWeaponName )
            {
                altWeaponIndex = BG_GetWeaponIndexForName((char *)weapVariantDef->szAltWeaponName, regWeap);
                if ( altWeaponIndex )
                {
                    altWeapVariantDef = bg_weaponVariantDefs[altWeaponIndex];
                    if ( altWeapVariantDef )
                    {
                        altWeapVariantDef->iVariantCount = -baseWeaponIndex;
                        if ( altWeapVariantDef->weapDef->inventoryType == WEAPINVENTORY_ALTMODE )
                            altWeapVariantDef->altWeaponIndex = 0;
                    }
                }
                else
                {
                    Com_PrintWarning(17, "WARNING: Could not load alt weapon file '%s'\n", weapVariantDef->szAltWeaponName);
                }
            }
            if ( *weapDef->szDualWieldWeaponName )
            {
                dualWeaponIndex = BG_GetWeaponIndexForName((char *)weapDef->szDualWieldWeaponName, regWeap);
                if ( dualWeaponIndex )
                {
                    dualWeapVariantDef = bg_weaponVariantDefs[dualWeaponIndex];
                    if ( dualWeapVariantDef )
                        dualWeapVariantDef->iVariantCount = -baseWeaponIndex;
                }
                else
                {
                    Com_PrintWarning(17, "WARNING: Could not load dual wield weapon file '%s'\n", weapDef->szDualWieldWeaponName);
                }
            }
        }
    }
    else
    {
        Com_PrintWarning(17, "WARNING: Could not load weapon variant file '%s%s'\n", namePrefix, namePostfix);
    }
}

