#include "g_trigger_mp.h"
#include <clientscript/cscr_vm.h>
#include "g_main_mp.h"
#include "g_spawn_mp.h"
#include <clientscript/scr_const.h>
#include <server/sv_game.h>
#include "g_utils_mp.h"
#include <server/sv_world.h>
#include "g_combat_mp.h"
#include <game/g_load_utils.h>
#include "actor_mp.h"
#include <bgame/bg_perks.h>
#include <qcommon/cm_world.h>

void __cdecl G_Trigger(gentity_s *self, gentity_s *other)
{
    trigger_info_t *trigger_info; // [esp+0h] [ebp-4h]

    if ( !self->r.inuse
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_trigger_mp.cpp", 11, 0, "%s", "self->r.inuse") )
    {
        __debugbreak();
    }
    if ( !other->r.inuse
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_trigger_mp.cpp", 12, 0, "%s", "other->r.inuse") )
    {
        __debugbreak();
    }
    if ( Scr_IsSystemActive(1u, SCRIPTINSTANCE_SERVER) )
    {
        if ( level.pendingTriggerListSize == 256 )
        {
            Scr_AddEntity(other, SCRIPTINSTANCE_SERVER);
            Scr_Notify(self, scr_const.trigger, 1u);
        }
        else
        {
            trigger_info = &level.pendingTriggerList[level.pendingTriggerListSize++];
            trigger_info->entnum = self->s.number;
            trigger_info->otherEntnum = other->s.number;
            trigger_info->useCount = self->useCount;
            trigger_info->otherUseCount = other->useCount;
        }
    }
}

char __cdecl InitTrigger(gentity_s *self)
{
    if ( SV_SetBrushModel(self) )
    {
        self->r.contents = 1079771144;
        self->r.svFlags = 1;
        self->s.lerp.eFlags |= 1u;
        if ( !self->model )
            self->s.lerp.eFlags |= 0x20u;
        return 1;
    }
    else
    {
        Com_PrintError(
            1,
            "Killing trigger at (%f %f %f) because the brush model is invalid.\n",
            self->s.lerp.pos.trBase[0],
            self->s.lerp.pos.trBase[1],
            self->s.lerp.pos.trBase[2]);
        G_FreeEntity(self);
        return 0;
    }
}

void __cdecl InitSentientTrigger(gentity_s *self)
{
    if ( !self && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_trigger_mp.cpp", 70, 0, "%s", "self") )
        __debugbreak();
    self->r.contents = 0;
    if ( (self->spawnflags & 8) == 0 )
        self->r.contents |= 0x40000000u;
    if ( (self->spawnflags & 1) != 0 )
        self->r.contents |= 0x40000u;
    if ( (self->spawnflags & 2) != 0 )
        self->r.contents |= 0x80000u;
    if ( (self->spawnflags & 4) != 0 )
        self->r.contents |= 0x100000u;
    if ( (self->spawnflags & 0x10) != 0 )
        self->r.contents |= 8u;
}

void __cdecl multi_trigger(gentity_s *ent)
{
    if ( (ent->spawnflags & 0x40) != 0 )
        G_FreeEntityDelay(ent);
}

void __cdecl Touch_Multi(gentity_s *self, gentity_s *other, int __formal)
{
    G_Trigger(self, other);
    multi_trigger(self);
}

void __cdecl SP_trigger_multiple(gentity_s *ent, SpawnVar *spawnVar)
{
    int savedregs; // [esp+0h] [ebp+0h] BYREF

    if ( (ent->spawnflags & 0x80) != 0 )
    {
        G_FreeEntity(ent);
    }
    else
    {
        ent->handler = 4;
        InitTriggerWait(ent, 64, spawnVar);
        if ( InitTrigger(ent) )
        {
            InitSentientTrigger(ent);
            SV_LinkEntity(ent);
        }
    }
}

void __cdecl InitTriggerWait(gentity_s *ent, int spawnflag, SpawnVar *spawnVar)
{
    float wait; // [esp+0h] [ebp-4h] BYREF

    if ( spawnVar && G_SpawnFloat(spawnVar, "wait", "", &wait) && wait <= 0.0 )
        ent->spawnflags |= spawnflag;
}

void __cdecl SP_trigger_radius(gentity_s *ent, SpawnVar *spawnVar)
{
    const char *v2; // eax
    const char *v3; // eax
    float height; // [esp+18h] [ebp-8h] BYREF
    float radius; // [esp+1Ch] [ebp-4h] BYREF
    int savedregs; // [esp+20h] [ebp+0h] BYREF

    if ( spawnVar )
    {
        if ( !G_SpawnFloat(spawnVar, "radius", "", &radius) )
        {
            v2 = va(
                         "radius not specified for trigger_radius at (%g %g %g)",
                         ent->r.currentOrigin[0],
                         ent->r.currentOrigin[1],
                         ent->r.currentOrigin[2]);
            Com_Error(ERR_DROP, v2);
        }
        if ( !G_SpawnFloat(spawnVar, "height", "", &height) )
        {
            v3 = va(
                         "height not specified for trigger_radius at (%g %g %g)",
                         ent->r.currentOrigin[0],
                         ent->r.currentOrigin[1],
                         ent->r.currentOrigin[2]);
            Com_Error(ERR_DROP, v3);
        }
    }
    else
    {
        if ( (unsigned int)Scr_GetNumParam(SCRIPTINSTANCE_SERVER) < 5 )
            Scr_Error("USAGE: spawn( \"trigger_radius\", <origin>, <spawnflags>, <radius>, <height> )", 0);
        radius = Scr_GetFloat(3u, SCRIPTINSTANCE_SERVER);
        height = Scr_GetFloat(4u, SCRIPTINSTANCE_SERVER);
    }
    init_trigger_radius(ent, radius, height);
    ent->handler = 4;
    InitTriggerWait(ent, 64, spawnVar);
    SV_LinkEntity(ent);
}

void __cdecl init_trigger_radius(gentity_s *ent, float radius, float height)
{
    ent->r.mins[0] = -radius;
    ent->r.mins[1] = -radius;
    ent->r.mins[2] = 0.0f;
    ent->r.maxs[0] = radius;
    ent->r.maxs[1] = radius;
    ent->r.maxs[2] = height;
    ent->r.svFlags = 33;
    InitSentientTrigger(ent);
    ent->r.contents |= 0x200000u;
    ent->item[1].ammoCount = 1023;
#ifdef KISAK_SP
    ent->trigger.perk = BG_SP_PERK_COUNT;
#else
    ent->trigger.perk = 52;
#endif
    ent->s.un3.item = 0;
    ent->s.un1.scale = -1;
    ent->s.otherEntityNum = 1023;
}

void __cdecl SP_trigger_radius_use(gentity_s *ent, SpawnVar *v)
{
    int savedregs; // [esp+8h] [ebp+0h] BYREF

    init_trigger_radius(ent, 128.0, 96.0);
    ent->handler = 22;
    SV_LinkEntity(ent);
}

void __cdecl SP_trigger_disk(gentity_s *ent, SpawnVar *spawnVar)
{
    const char *v2; // eax
    float radius; // [esp+18h] [ebp-4h] BYREF
    int savedregs; // [esp+1Ch] [ebp+0h] BYREF

    if ( !G_SpawnFloat(spawnVar, "radius", "", &radius) )
    {
        v2 = va(
                     "radius not specified for trigger_radius at (%g %g %g)",
                     ent->r.currentOrigin[0],
                     ent->r.currentOrigin[1],
                     ent->r.currentOrigin[2]);
        Com_Error(ERR_DROP, v2);
    }
    if ( (ent->spawnflags & 0x80) != 0 )
    {
        G_FreeEntity(ent);
    }
    else
    {
        ent->handler = 4;
        radius = radius + 64.0;
        ent->r.mins[0] = -radius;
        ent->r.mins[1] = -radius;
        ent->r.mins[2] = -100000.0f;
        ent->r.maxs[0] = radius;
        ent->r.maxs[1] = radius;
        ent->r.maxs[2] = 100000.0f;
        ent->r.svFlags = 65;
        InitTriggerWait(ent, 64, spawnVar);
        InitSentientTrigger(ent);
        SV_LinkEntity(ent);
    }
}

void __cdecl hurt_touch(gentity_s *self, gentity_s *other, int __formal)
{
    if ( other->takedamage && self->item[0].index <= level.time )
    {
        G_Trigger(self, other);
        self->item[0].index = (self->spawnflags & 0x10) != 0 ? level.time + 1000 : level.time + 50;
        G_Damage(other, self, self, 0, 0, self->damage, 0, 14, 0xFFFFFFFF, HITLOC_NONE, 0, 0, 0);
        if ( (self->spawnflags & 0x20) != 0 )
        {
            if ( self->handler != 6
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_trigger_mp.cpp",
                            276,
                            0,
                            "%s",
                            "self->handler == ENT_HANDLER_TRIGGER_HURT_TOUCH") )
            {
                __debugbreak();
            }
            self->handler = 5;
        }
    }
}

void __cdecl hurt_use(gentity_s *self, gentity_s *__formal, gentity_s *__formal2)
{
    if ( self->handler == 6 )
    {
        self->handler = 5;
    }
    else
    {
        if ( self->handler != 5
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_trigger_mp.cpp",
                        290,
                        0,
                        "%s",
                        "self->handler == ENT_HANDLER_TRIGGER_HURT") )
        {
            __debugbreak();
        }
        self->handler = 6;
    }
}

void __cdecl SP_trigger_hurt(gentity_s *self, SpawnVar *spawnVar)
{
    const char *sound; // [esp+0h] [ebp-4h] BYREF

    if ( InitTrigger(self) )
    {
        G_SpawnString(spawnVar, "sound", "world_hurt_me", &sound);
        if ( !self->damage )
            self->damage = 5;
        self->r.contents = 1079771144;
        if ( (self->spawnflags & 1) != 0 )
            self->handler = 5;
        else
            self->handler = 6;
    }
}

void __cdecl SP_trigger_once(gentity_s *ent, SpawnVar *spawnVar)
{
    int savedregs; // [esp+0h] [ebp+0h] BYREF

    if ( (ent->spawnflags & 0x80) != 0 )
    {
        G_FreeEntity(ent);
    }
    else
    {
        ent->handler = 4;
        ent->spawnflags |= 0x40u;
        if ( InitTrigger(ent) )
        {
            InitSentientTrigger(ent);
            SV_LinkEntity(ent);
        }
    }
}

bool __cdecl Respond_trigger_damage(gentity_s *pEnt, int iMOD)
{
    if ( (pEnt->spawnflags & 1) != 0 && iMOD == 1 )
        return 0;
    if ( (pEnt->spawnflags & 2) != 0 && iMOD == 2 )
        return 0;
    if ( (pEnt->spawnflags & 4) != 0 && iMOD >= 3 && iMOD <= 6 )
        return 0;
    if ( (pEnt->spawnflags & 8) != 0 && iMOD >= 3 && (iMOD <= 6 || iMOD == 15) )
        return 0;
    if ( (pEnt->spawnflags & 0x10) != 0 && (iMOD == 4 || iMOD == 6) )
        return 0;
    if ( (pEnt->spawnflags & 0x20) != 0 && iMOD == 7 )
        return 0;
    return (pEnt->spawnflags & 0x100) == 0 || iMOD && (iMOD <= 9 || iMOD > 14);
}

void __cdecl Activate_trigger_damage(gentity_s *pEnt, gentity_s *pOther, int iDamage, int iMOD)
{
    if ( pEnt->item[0].ammoCount <= 0 || iDamage >= pEnt->item[0].ammoCount )
    {
        if ( Respond_trigger_damage(pEnt, iMOD) )
        {
            pEnt->health -= iDamage;
            if ( !pEnt->item[0].clipAmmoCount || 32000 - pEnt->health >= pEnt->item[0].clipAmmoCount )
            {
                if ( iMOD != -1 )
                    G_Trigger(pEnt, pOther);
                pEnt->health = 32000;
                if ( (pEnt->spawnflags & 0x200) != 0 )
                    G_FreeEntityDelay(pEnt);
            }
        }
    }
}

void __cdecl Use_trigger_damage(gentity_s *pEnt, gentity_s *pOther, gentity_s *__formal)
{
    Activate_trigger_damage(pEnt, pOther, pEnt->item[0].clipAmmoCount + 1, -1);
}

void __cdecl Pain_trigger_damage(gentity_s *pSelf, gentity_s *pAttacker, int iDamage, const float *vPoint, const int iMod, const float *__formal, const hitLocation_t __formal2, const int __formal3)
{
    Activate_trigger_damage(pSelf, pAttacker, iDamage, iMod);
}

void __cdecl Flame_trigger_damage(
                gentity_s *pSelf,
                gentity_s *pAttacker,
                int iDamage,
                float *vPoint,
                int iMod,
                float *vDir)
{
    if ( pSelf->classname == scr_const.trigger_damage && (pSelf->spawnflags & 0x40) == 0 )
    {
        Scr_AddConstString(*modNames[iMod], SCRIPTINSTANCE_SERVER);
        Scr_AddVector(vPoint, SCRIPTINSTANCE_SERVER);
        Scr_AddVector(vDir, SCRIPTINSTANCE_SERVER);
        Scr_AddEntity(pAttacker, SCRIPTINSTANCE_SERVER);
        Scr_AddInt(iDamage, SCRIPTINSTANCE_SERVER);
        Scr_Notify(pSelf, scr_const.damage, 5u);
        Activate_trigger_damage(pSelf, pAttacker, iDamage, iMod);
    }
}

void __cdecl Die_trigger_damage(gentity_s *pSelf, gentity_s *pInflictor, gentity_s *pAttacker, int iDamage, int iMod, const int __formal, const float *__formal2, const hitLocation_t __formal3, int __formal4)
{
    Activate_trigger_damage(pSelf, pAttacker, iDamage, iMod);
}

void __cdecl SP_trigger_damage(gentity_s *pSelf, SpawnVar *spawnVar)
{
    int savedregs; // [esp+0h] [ebp+0h] BYREF

    G_SpawnInt(spawnVar, "accumulate", "0", &pSelf->trigger.accumulate);
    G_SpawnInt(spawnVar, "threshold", "0", &pSelf->trigger.threshold);

    pSelf->health = 32000;
    pSelf->takedamage = 1;
    pSelf->handler = 7;
    InitTriggerWait(pSelf, 512, spawnVar);
    if ( InitTrigger(pSelf) )
        SV_LinkEntity(pSelf);
}

void __cdecl G_CheckHitTriggerDamage(gentity_s *pActivator, float *vStart, float *vEnd, int iDamage, unsigned int iMOD)
{
    int v5; // [esp+Ch] [ebp-107Ch]
    int entityList[1024]; // [esp+10h] [ebp-1078h] BYREF
    sightclip_t clip; // [esp+1010h] [ebp-78h] BYREF
    float value[3]; // [esp+105Ch] [ebp-2Ch] BYREF
    float mins[3]; // [esp+1068h] [ebp-20h] BYREF
    int i; // [esp+1074h] [ebp-14h]
    gentity_s *pEnt; // [esp+1078h] [ebp-10h]
    float maxs[3]; // [esp+107Ch] [ebp-Ch] BYREF

    if ( iMOD >= 0x15
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_trigger_mp.cpp",
                    565,
                    0,
                    "iMOD doesn't index MOD_NUM\n\t%i not in [0, %i)",
                    iMOD,
                    21) )
    {
        __debugbreak();
    }
    if ( !*modNames[iMOD]
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_trigger_mp.cpp",
                    566,
                    0,
                    "%s",
                    "*modNames[iMOD]") )
    {
        __debugbreak();
    }
    if ( !vStart
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_trigger_mp.cpp", 567, 0, "%s", "vStart") )
    {
        __debugbreak();
    }
    if ( !vEnd && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game_mp\\g_trigger_mp.cpp", 568, 0, "%s", "vEnd") )
        __debugbreak();
    mins[0] = *vStart;
    mins[1] = vStart[1];
    mins[2] = vStart[2];
    maxs[0] = *vStart;
    maxs[1] = vStart[1];
    maxs[2] = vStart[2];
    AddPointToBounds(vEnd, mins, maxs);
    value[0] = *vEnd - *vStart;
    value[1] = vEnd[1] - vStart[1];
    value[2] = vEnd[2] - vStart[2];
    Vec3Normalize(value);
    v5 = CM_AreaEntities(mins, maxs, entityList, 1024, 0x400000);
    clip.start[0] = *vStart;
    clip.start[1] = vStart[1];
    clip.start[2] = vStart[2];
    clip.end[0] = *vEnd;
    clip.end[1] = vEnd[1];
    clip.end[2] = vEnd[2];
    memset(&clip, 0, 24);
    clip.contentmask = -1;
    clip.passEntityNum[1] = 1023;
    clip.passEntityNum[0] = 1023;
    for ( i = 0; i < v5; ++i )
    {
        pEnt = &g_entities[entityList[i]];
        if ( pEnt->classname == scr_const.trigger_damage && SV_SightTraceCapsuleToEntity(&clip, pEnt->s.number) )
        {
            Scr_AddConstString(*modNames[iMOD], SCRIPTINSTANCE_SERVER);
            Scr_AddVector((float *)vec3_origin, SCRIPTINSTANCE_SERVER);
            Scr_AddVector(value, SCRIPTINSTANCE_SERVER);
            Scr_AddEntity(pActivator, SCRIPTINSTANCE_SERVER);
            Scr_AddInt(iDamage, SCRIPTINSTANCE_SERVER);
            Scr_Notify(pEnt, scr_const.damage, 5u);
            Activate_trigger_damage(pEnt, pActivator, iDamage, iMOD);
            if ( !pEnt->item[0].clipAmmoCount )
                pEnt->health = 32000;
        }
    }
}

void __cdecl G_GrenadeTouchTriggerDamage(gentity_s *pActivator, float *vStart, float *vEnd, int iDamage, int iMOD)
{
    int v5; // [esp+Ch] [ebp-107Ch]
    int entityList[1024]; // [esp+10h] [ebp-1078h] BYREF
    sightclip_t clip; // [esp+1010h] [ebp-78h] BYREF
    float value[3]; // [esp+105Ch] [ebp-2Ch] BYREF
    float mins[3]; // [esp+1068h] [ebp-20h] BYREF
    int i; // [esp+1074h] [ebp-14h]
    gentity_s *pEnt; // [esp+1078h] [ebp-10h]
    float maxs[3]; // [esp+107Ch] [ebp-Ch] BYREF

    mins[0] = *vStart;
    mins[1] = vStart[1];
    mins[2] = vStart[2];
    maxs[0] = *vStart;
    maxs[1] = vStart[1];
    maxs[2] = vStart[2];
    AddPointToBounds(vEnd, mins, maxs);
    value[0] = *vEnd - *vStart;
    value[1] = vEnd[1] - vStart[1];
    value[2] = vEnd[2] - vStart[2];
    Vec3Normalize(value);
    v5 = CM_AreaEntities(mins, maxs, entityList, 1024, 0x400000);
    clip.start[0] = *vStart;
    clip.start[1] = vStart[1];
    clip.start[2] = vStart[2];
    clip.end[0] = *vEnd;
    clip.end[1] = vEnd[1];
    clip.end[2] = vEnd[2];
    memset(&clip, 0, 24);
    clip.contentmask = -1;
    clip.passEntityNum[1] = 1023;
    clip.passEntityNum[0] = 1023;
    for ( i = 0; i < v5; ++i )
    {
        pEnt = &g_entities[entityList[i]];
        if ( pEnt->classname == scr_const.trigger_damage
            && (pEnt->flags & 0x4000) != 0
            && SV_SightTraceCapsuleToEntity(&clip, pEnt->s.number) )
        {
            Scr_AddConstString(*modNames[iMOD], SCRIPTINSTANCE_SERVER);
            Scr_AddVector((float *)vec3_origin, SCRIPTINSTANCE_SERVER);
            Scr_AddVector(value, SCRIPTINSTANCE_SERVER);
            Scr_AddEntity(pActivator, SCRIPTINSTANCE_SERVER);
            Scr_AddInt(iDamage, SCRIPTINSTANCE_SERVER);
            Scr_Notify(pEnt, scr_const.damage, 5u);
            Activate_trigger_damage(pEnt, pActivator, iDamage, iMOD);
            if ( !pEnt->item[0].clipAmmoCount )
                pEnt->health = 32000;
        }
    }
}

void __cdecl SP_trigger_lookat(gentity_s *self, SpawnVar *v)
{
    if ( SV_SetBrushModel(self) )
    {
        self->r.contents = 0x20000000;
        self->r.svFlags = 1;
        self->s.lerp.eFlags |= 1u;
        if ( !self->model )
            self->s.lerp.eFlags |= 0x20u;
    }
    else
    {
        Com_PrintError(
            1,
            "Killing trigger_lookat at (%f %f %f) because the brush model is invalid.\n",
            self->s.lerp.pos.trBase[0],
            self->s.lerp.pos.trBase[1],
            self->s.lerp.pos.trBase[2]);
        G_FreeEntity(self);
    }
}

void __cdecl trigger_ik_playerclip_terrain_touch(gentity_s *ent, gentity_s *other, int __formal)
{
    if ( other->client || other->actor )
    {
        other->s.lerp.eFlags2 |= 0x10000u;
        other->ikPlayerclipTerrainTime = level.time;
    }
}

void __cdecl SP_trigger_ik_playerclip_terrain(gentity_s *self, SpawnVar *v)
{
    int savedregs; // [esp+0h] [ebp+0h] BYREF

    self->handler = 29;
    if ( InitTrigger(self) )
        SV_LinkEntity(self);
}

void __cdecl trigger_ik_disable_terrain_mapping_touch(gentity_s *ent, gentity_s *other, int __formal)
{
    if ( other->client || other->actor )
    {
        other->s.lerp.eFlags2 |= 0x100000u;
        other->ikDisableTerrainMappingTime = level.time;
    }
}

