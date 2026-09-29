#include "sentient_fields.h"
#include <universal/assertive.h>
#include <universal/q_shared.h>
#include <clientscript/cscr_vm.h>
#include "sentient.h"
#include <game_mp/g_spawn_mp.h>

// The script field table below binds sentient_s members by raw offset; keep the layout honest.
#ifndef KISAK_NX // nx-port: x86 offsets; the table below uses offsetof
static_assert(offsetof(sentient_s, eTeam) == 4, "sentient field offset");
static_assert(offsetof(sentient_s, scriptOwner) == 8, "sentient field offset");
static_assert(offsetof(sentient_s, iThreatBias) == 12, "sentient field offset");
static_assert(offsetof(sentient_s, iThreatBiasGroupIndex) == 16, "sentient field offset");
static_assert(offsetof(sentient_s, bIgnoreMe) == 20, "sentient field offset");
static_assert(offsetof(sentient_s, bIgnoreAll) == 21, "sentient field offset");
static_assert(offsetof(sentient_s, bIgnoreForFriendlyFire) == 22, "sentient field offset");
static_assert(offsetof(sentient_s, maxVisibleDist) == 36, "sentient field offset");
static_assert(offsetof(sentient_s, lastAttacker) == 48, "sentient field offset");
static_assert(offsetof(sentient_s, syncedMeleeEnt) == 52, "sentient field offset");
static_assert(offsetof(sentient_s, targetEnt) == 56, "sentient field offset");
static_assert(offsetof(sentient_s, scriptTargetEnt) == 60, "sentient field offset");
static_assert(offsetof(sentient_s, scriptTargetTag) == 64, "sentient field offset");
static_assert(offsetof(sentient_s, attackerAccuracy) == 88, "sentient field offset");
static_assert(offsetof(sentient_s, ignoreRandomBulletDamage) == 92, "sentient field offset");
static_assert(offsetof(sentient_s, turretInvulnerability) == 93, "sentient field offset");
static_assert(offsetof(sentient_s, pClaimedNode) == 96, "sentient field offset");
static_assert(offsetof(sentient_s, pPrevClaimedNode) == 100, "sentient field offset");
static_assert(offsetof(sentient_s, bInMeleeCharge) == 140, "sentient field offset");
#endif

const sentient_fields_s fields_2[20] =
{
#ifdef KISAK_SP
  // Retail SP's sentient field table at BlackOps.exe 0x00A55C78 names this
  // field "team" and routes it through SentientScr_SetTeam/GetTeam.
  { "team", (int)offsetof(sentient_s, eTeam), { 4 }, F_INT, SentientScr_SetTeam, SentientScr_GetTeam },
#else
  { "aiteam", (int)offsetof(sentient_s, eTeam), { 4 }, F_INT, SentientScr_SetTeam, SentientScr_GetTeam },
#endif
  { "script_owner", (int)offsetof(sentient_s, scriptOwner), { 4 }, F_ENTHANDLE, &SentientScr_ReadOnly, NULL },
  { "threatbias", (int)offsetof(sentient_s, iThreatBias), { 4 }, F_INT, NULL, NULL },
  { "threatbiasgroup", (int)offsetof(sentient_s, iThreatBiasGroupIndex), { 4 }, F_INT, &SentientScr_ReadOnly, NULL },
  { "attacker", (int)offsetof(sentient_s, lastAttacker), { 4 }, F_ENTHANDLE, &SentientScr_ReadOnly, NULL },
  { "node", (int)offsetof(sentient_s, pClaimedNode), { 4 }, F_PATHNODE, &SentientScr_ReadOnly, NULL },
  { "prevnode", (int)offsetof(sentient_s, pPrevClaimedNode), { 4 }, F_PATHNODE, &SentientScr_ReadOnly, NULL },
  { "enemy", (int)offsetof(sentient_s, targetEnt), { 4 }, F_ENTHANDLE, &SentientScr_ReadOnly, NULL },
  { "scriptenemy", (int)offsetof(sentient_s, scriptTargetEnt), { 4 }, F_ENTHANDLE, &SentientScr_ReadOnly, NULL },
  { "scriptenemytag", (int)offsetof(sentient_s, scriptTargetTag), { 2 }, F_STRING, &SentientScr_ReadOnly, NULL },
  { "syncedmeleetarget", (int)offsetof(sentient_s, syncedMeleeEnt), { 4 }, F_ENTHANDLE, NULL, NULL },
  { "ignoreme", (int)offsetof(sentient_s, bIgnoreMe), { 1 }, F_BYTE, NULL, NULL },
  { "ignoreall", (int)offsetof(sentient_s, bIgnoreAll), { 1 }, F_BYTE, NULL, NULL },
  { "ignoreforfriendlyfire", (int)offsetof(sentient_s, bIgnoreForFriendlyFire), { 1 }, F_BYTE, NULL, NULL },
  { "maxvisibledist", (int)offsetof(sentient_s, maxVisibleDist), { 4 }, F_FLOAT, NULL, NULL },
  { "attackeraccuracy", (int)offsetof(sentient_s, attackerAccuracy), { 4 }, F_FLOAT, NULL, NULL },
  { "ignorerandombulletdamage", (int)offsetof(sentient_s, ignoreRandomBulletDamage), { 1 }, F_BYTE, NULL, NULL },
  { "turretinvulnerability", (int)offsetof(sentient_s, turretInvulnerability), { 1 }, F_BYTE, NULL, NULL },
  { "inmeleecharge", (int)offsetof(sentient_s, bInMeleeCharge), { 1 }, F_BYTE, NULL, NULL },
  { NULL, 0, { 0 }, F_INT, NULL, NULL }
};



void __cdecl SentientScr_ReadOnly(sentient_s *pSelf, const sentient_fields_s *pField)
{
    const char *v2; // eax

    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp", 67, 0, "%s", "pSelf") )
        __debugbreak();
    v2 = va("sentient property '%s' is read-only", pField->name);
    Scr_Error(v2, 0);
}

void __cdecl SentientScr_SetTeam(sentient_s *pSelf, const sentient_fields_s *pField)
{
    const char *v1; // eax
    char *pszTeam; // [esp+0h] [ebp-4h]

    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp", 79, 0, "%s", "pSelf") )
        __debugbreak();
    pszTeam = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
#ifdef KISAK_SP
    // Retail SP 0x00819180 accepts exactly axis/allies/neutral and writes the
    // numeric sentient team through Sentient_SetTeam (0x0043F3A0).
    if ( !I_stricmp(pszTeam, "axis") )
    {
        Sentient_SetTeam(pSelf, TEAM_AXIS);
        return;
    }
    if ( !I_stricmp(pszTeam, "allies") )
    {
        Sentient_SetTeam(pSelf, TEAM_ALLIES);
        return;
    }
    if ( !I_stricmp(pszTeam, "neutral") )
    {
        Sentient_SetTeam(pSelf, TEAM_SPECTATOR);
        return;
    }
    v1 = va("unknown team '%s', should be axis, allies, or neutral\n", pszTeam);
    Scr_Error(v1, 0);
#else
    if ( I_stricmp(pszTeam, "axis") )
    {
        if ( I_stricmp(pszTeam, "allies") )
        {
            if ( I_stricmp(pszTeam, "spectator") )
            {
                if ( I_stricmp(pszTeam, "free") )
                {
                    v1 = va("unknown team '%s', should be axis, allies, or neutral\n", pszTeam);
                    Scr_Error(v1, 0);
                }
                else
                {
                    Sentient_SetTeam(pSelf, TEAM_FREE);
                }
            }
            else
            {
                Sentient_SetTeam(pSelf, TEAM_SPECTATOR);
            }
        }
        else
        {
            Sentient_SetTeam(pSelf, TEAM_ALLIES);
        }
    }
    else
    {
        Sentient_SetTeam(pSelf, TEAM_AXIS);
    }
#endif
}

void __cdecl SentientScr_GetTeam(sentient_s *pSelf, const sentient_fields_s *pField)
{
    if ( !pSelf
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp", 108, 0, "%s", "pSelf") )
    {
        __debugbreak();
    }
#ifdef KISAK_SP
    // Retail SP 0x00819210 has no TEAM_NUM_TEAMS assertion. Its jump table
    // maps 1/2/3/5 and returns without a string for 0, 4, or out-of-range.
    switch ( static_cast<int>(pSelf->eTeam) )
    {
        case 1:
            Scr_AddString("axis", SCRIPTINSTANCE_SERVER);
            break;
        case 2:
            Scr_AddString("allies", SCRIPTINSTANCE_SERVER);
            break;
        case 3:
            Scr_AddString("neutral", SCRIPTINSTANCE_SERVER);
            break;
        case 5:
            Scr_AddString("dead", SCRIPTINSTANCE_SERVER);
            break;
        default:
            break;
    }
#else
    if ( pSelf->eTeam >= (unsigned int)TEAM_NUM_TEAMS
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp",
                    110,
                    0,
                    "%s",
                    "pSelf->eTeam >= TEAM_FREE && pSelf->eTeam < TEAM_NUM_TEAMS") )
    {
        __debugbreak();
    }
    switch ( pSelf->eTeam )
    {
        case TEAM_FREE:
            Scr_AddString("free", SCRIPTINSTANCE_SERVER);
            break;
        case TEAM_AXIS:
            Scr_AddString("axis", SCRIPTINSTANCE_SERVER);
            break;
        case TEAM_ALLIES:
            Scr_AddString("allies", SCRIPTINSTANCE_SERVER);
            break;
        case TEAM_SPECTATOR:
            Scr_AddString("spectator", SCRIPTINSTANCE_SERVER);
            break;
        default:
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp",
                            139,
                            0,
                            "SentientScr_GetTeam: default case (shouldn't happen") )
                __debugbreak();
            break;
    }
#endif
}

void __cdecl GScr_AddFieldsForSentient()
{
    const sentient_fields_s *f; // [esp+4h] [ebp-4h]

    for ( f = fields_2; f->name; ++f )
    {
        if ( ((f - fields_2) & 0xC000) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp",
                        158,
                        0,
                        "%s",
                        "!((f - fields) & ENTFIELD_MASK)") )
        {
            __debugbreak();
        }
        if ( f - fields_2 != (unsigned __int16)(f - fields_2)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp",
                        159,
                        0,
                        "%s",
                        "(f - fields) == (unsigned short)( f - fields )") )
        {
            __debugbreak();
        }
        Scr_AddClassField(0, (char *)f->name, (unsigned __int16)(f - fields_2) | 0x4000, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl Scr_SetSentientField(sentient_s *sentient, unsigned int offset)
{
    const sentient_fields_s *f; // [esp+0h] [ebp-4h]

    if ( !sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp", 175, 0, "%s", "sentient") )
    {
        __debugbreak();
    }
    if ( offset >= 0x13
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp",
                    176,
                    0,
                    "%s",
                    "(unsigned)offset < ARRAY_COUNT( fields ) - 1") )
    {
        __debugbreak();
    }
    f = &fields_2[offset];
    if ( f->setter )
        f->setter(sentient, f);
    else
        GScr_SetGenericField((unsigned __int8 *)sentient, f->type, f->ofs, 0);
}

void __cdecl Scr_GetSentientField(sentient_s *sentient, unsigned int offset)
{
    const sentient_fields_s *f; // [esp+0h] [ebp-4h]

    if ( !sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp", 199, 0, "%s", "sentient") )
    {
        __debugbreak();
    }
    if ( offset >= 0x13
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp",
                    200,
                    0,
                    "%s",
                    "(unsigned)offset < ARRAY_COUNT( fields ) - 1") )
    {
        __debugbreak();
    }
    f = &fields_2[offset];
    if ( f->getter )
        f->getter(sentient, f);
    else
        GScr_GetGenericField((unsigned __int8 *)sentient, f->type, f->ofs, 0);
}

