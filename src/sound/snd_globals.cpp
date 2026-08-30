#include "snd_globals.h"
#include "snd_dvar.h"
#include "snd_utils.h"
#include <qcommon/common.h>

snd_category_t __cdecl SND_GroupCategory(unsigned int group)
{
    if ( group >= SND_GetGroupCount()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_globals.cpp",
                    13,
                    0,
                    "%s\n\t(group) = %i",
                    "(group >= 0 && group < SND_GetGroupCount())",
                    group) )
    {
        __debugbreak();
    }
    return SND_GetGroupByIndex(group)->category;
}

double __cdecl SND_GroupGetAttenuation(unsigned int group)
{
    return (double)SND_GetGroupByIndex(group)->attenuationMp / 65535.0;
}

unsigned int __cdecl SND_GetGroupCount()
{
    return g_snd.global_constants->groupCount;
}

const snd_group *__cdecl SND_GetGroupByIndex(unsigned int index)
{
    if ( index >= g_snd.global_constants->groupCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_globals.cpp",
                    37,
                    0,
                    "index doesn't index g_snd.global_constants->groupCount\n\t%i not in [0, %i)",
                    index,
                    g_snd.global_constants->groupCount) )
    {
        __debugbreak();
    }
    return &g_snd.global_constants->groups[index];
}

const snd_master *__cdecl SND_GetMaster(unsigned int id)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    if ( !g_snd.global_constants
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_globals.cpp",
                    44,
                    0,
                    "%s",
                    "g_snd.global_constants") )
    {
        __debugbreak();
    }
    if ( g_snd.global_constants )
    {
        for ( i = 0; i < g_snd.global_constants->masterCount; ++i )
        {
            if ( g_snd.global_constants->masters[i].id == id )
                return &g_snd.global_constants->masters[i];
        }
    }
    if ( g_snd.defaultHash == id )
        return 0;
    else
        return SND_GetMaster(g_snd.defaultHash);
}

const snd_master *__cdecl SND_GetMasterByConfig()
{
    unsigned int v0; // eax
    unsigned int setup; // [esp+0h] [ebp-30h]
    const char *snames[3][3]; // [esp+4h] [ebp-2Ch] BYREF
    const char *sname; // [esp+28h] [ebp-8h]
    int level; // [esp+2Ch] [ebp-4h]

    setup = snd_menu_speaker_setup->current.unsignedInt;
    level = snd_menu_listen_level->current.integer;
    snames[0][0] = "tv_quiet";
    snames[0][1] = "tv_loud";
    snames[0][2] = 0;
    snames[1][0] = "surround_quiet";
    snames[1][1] = "surround_loud";
    snames[1][2] = 0;
    snames[2][0] = "headphone_quiet";
    snames[2][1] = "headphone_loud";
    snames[2][2] = 0;
    if ( setup >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_globals.cpp",
                    77,
                    0,
                    "setup not in [0, ARRAY_COUNT(snames)]\n\t%i not in [%i, %i]",
                    setup,
                    0,
                    3) )
    {
        __debugbreak();
    }
    if ( (unsigned int)level >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_globals.cpp",
                    78,
                    0,
                    "level not in [0, ARRAY_COUNT(snames[0])]\n\t%i not in [%i, %i]",
                    level,
                    0,
                    3) )
    {
        __debugbreak();
    }
    sname = snames[setup][level];
    v0 = SND_HashName(sname);
    return SND_GetMaster(v0);
}

const snd_master *__cdecl SND_GetMasterCurrent()
{
    unsigned int v0; // eax

    if ( !snd_master_override->current.string || !*(_BYTE *)snd_master_override->current.string )
        return SND_GetMasterByConfig();
    v0 = SND_HashName(snd_master_override->current.string);
    return SND_GetMaster(v0);
}

const snd_curve *__cdecl SND_GetCurveById(unsigned int id)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    if ( !g_snd.global_constants
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_globals.cpp",
                    103,
                    0,
                    "%s",
                    "g_snd.global_constants") )
    {
        __debugbreak();
    }
    if ( g_snd.global_constants )
    {
        for ( i = 0; i < g_snd.global_constants->curveCount; ++i )
        {
            if ( g_snd.global_constants->curves[i].id == id )
                return &g_snd.global_constants->curves[i];
        }
    }
    if ( g_snd.defaultHash == id )
        return g_snd.global_constants->curves;
    else
        return SND_GetCurveById(g_snd.defaultHash);
}

const snd_curve *__cdecl SND_GetCurveByIndex(unsigned int i)
{
    if ( !g_snd.global_constants
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_globals.cpp",
                    127,
                    0,
                    "%s",
                    "g_snd.global_constants") )
    {
        __debugbreak();
    }
    if ( i == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_globals.cpp",
                    128,
                    0,
                    "%s",
                    "i != SND_INVALID_CURVE") )
    {
        __debugbreak();
    }
    if ( i >= g_snd.global_constants->curveCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_globals.cpp",
                    129,
                    0,
                    "%s",
                    "i < g_snd.global_constants->curveCount") )
    {
        __debugbreak();
    }
    return &g_snd.global_constants->curves[i];
}

const snd_pan *__cdecl SND_GetPanByIndex(unsigned int i)
{
    if ( !g_snd.global_constants
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_globals.cpp",
                    138,
                    0,
                    "%s",
                    "g_snd.global_constants") )
    {
        __debugbreak();
    }
    if ( i >= g_snd.global_constants->panCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_globals.cpp",
                    139,
                    0,
                    "i doesn't index g_snd.global_constants->panCount\n\t%i not in [0, %i)",
                    i,
                    g_snd.global_constants->panCount) )
    {
        __debugbreak();
    }
    return &g_snd.global_constants->pans[i];
}

unsigned int __cdecl SND_FindContextIndex(unsigned int type)
{
    unsigned int i; // [esp+0h] [ebp-4h]

    if ( !g_snd.global_constants
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_globals.cpp",
                    147,
                    0,
                    "%s",
                    "g_snd.global_constants") )
    {
        __debugbreak();
    }
    for ( i = 0; i < g_snd.global_constants->contextCount; ++i )
    {
        if ( g_snd.global_constants->contexts[i].type == type )
            return i;
    }
    return -1;
}

int __cdecl SND_FindContextValueIndex(unsigned int type, unsigned int value)
{
    int i; // [esp+0h] [ebp-8h]
    unsigned int t; // [esp+4h] [ebp-4h]

    t = SND_FindContextIndex(type);
    for ( i = 0; i < 8; ++i )
    {
        if ( g_snd.global_constants->contexts[t].values[i] == value )
            return i;
    }
    return -1;
}

double __cdecl Snd_CurveEval(const snd_curve *curve, float x)
{
    float t; // [esp+8h] [ebp-Ch]
    int i; // [esp+10h] [ebp-4h]

    if ( x < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_globals.cpp", 181, 0, "%s", "x>=0.0f") )
    {
        __debugbreak();
    }
    if ( x > 1.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_globals.cpp", 182, 0, "%s", "x<=1.0f") )
    {
        __debugbreak();
    }
    if ( !curve && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_globals.cpp", 183, 0, "%s", "curve") )
        __debugbreak();
    if ( curve->points[0][0] >= x )
        return curve->points[0][1];
    if ( x >= curve->points[7][0] )
        return curve->points[7][1];
    for ( i = 1; i < 8; ++i )
    {
        if ( curve->points[i][0] >= x && x >= *(float *)&curve->name[8 * i + 28] )
        {
            t = (float)(x - *(float *)&curve->name[8 * i + 28])
                / (float)(curve->points[i][0] - *(float *)&curve->name[8 * i + 28]);
            return (1.0 - t) * *(float *)&curve->name[8 * i + 32] + t * curve->points[i][1];
        }
    }
    Com_PrintError(9, "invalid curve data in curve '%s'\n", curve->name);
    return 0.0;
}

double __cdecl Snd_CurveEvalOverRange(const snd_curve *curve, float x, float minimum, float maximum)
{
    float v5; // [esp+8h] [ebp-8h]
    float v6; // [esp+Ch] [ebp-4h]

    v5 = (float)(x - minimum) / (float)(maximum - minimum);
    if ( (float)(v5 - 1.0) < 0.0 )
        v6 = (float)(x - minimum) / (float)(maximum - minimum);
    else
        v6 = 1.0f;
    if ( (float)(0.0 - v5) < 0.0 )
        return Snd_CurveEval(curve, v6);
    else
        return Snd_CurveEval(curve, 0.0);
}

