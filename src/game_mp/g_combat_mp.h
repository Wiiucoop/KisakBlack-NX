#pragma once
#include <bgame/bg_public.h>
#include <qcommon/common.h>
#include <game/teams.h>

void __cdecl G_ParseHitLocDmgTable();
void __cdecl LookAtKiller(gentity_s *self, gentity_s *inflictor, gentity_s *attacker);
int __cdecl G_MeansOfDeathFromScriptParam(unsigned int scrParam);
void __cdecl player_die(
    gentity_s *self,
    gentity_s *inflictor,
    gentity_s *attacker,
    int damage,
    int meansOfDeath,
    const int iWeapon,
    const float *vDir,
    const hitLocation_t hitLoc,
    int psTimeOffset);
void __cdecl DeathGrenadeDrop(gentity_s *self);
double __cdecl G_GetWeaponHitLocationMultiplier(hitLocation_t hitLoc, unsigned int weapon);
void __cdecl G_DamageClient(
                gentity_s *targ,
                gentity_s *inflictor,
                gentity_s *attacker,
                float *dir,
                float *point,
                int damage,
                int dflags,
                unsigned int mod,
                unsigned int weapon,
                hitLocation_t hitLoc,
#ifdef KISAK_SP
                int modelIndex,
#endif
                int timeOffset);
unsigned int __cdecl G_GetWeaponIndexForEntity(const gentity_s *ent);
void __cdecl G_DamageActor(
                gentity_s *targ,
                gentity_s *inflictor,
                gentity_s *attacker,
                float *dir,
                float *point,
                int damage,
                int dflags,
                unsigned int mod,
                unsigned int weapon,
                hitLocation_t hitLoc,
#ifdef KISAK_SP
                unsigned int modelIndex,
#endif
                int timeOffset);
void __cdecl G_DamageVehicle(
                gentity_s *targ,
                gentity_s *inflictor,
                gentity_s *attacker,
                float *dir,
                float *point,
                int damage,
                int dflags,
                int mod,
                unsigned int weapon,
                hitLocation_t hitLoc,
                int timeOffset,
                unsigned int modelIndex,
                unsigned int partName);
void __cdecl G_DamageNotify(
                unsigned __int16 notify,
                gentity_s *targ,
                gentity_s *attacker,
                float *dir,
                float *point,
                int damage,
                int mod,
                int dFlags,
                unsigned int modelIndex,
                unsigned int partName,
                char *weaponName);
void __cdecl G_Damage(
                gentity_s *targ,
                gentity_s *inflictor,
                gentity_s *attacker,
                const float *dir,
                const float *point,
                int damage,
                int dFlags,
                int mod,
                unsigned int weapon,
                hitLocation_t hitLoc,
                unsigned int modelIndex,
                unsigned int partName,
                int timeOffset);
double __cdecl CanDamage(
                gentity_s *targ,
                gentity_s *inflictor,
                const float *centerPos,
                float coneAngleCos,
                float *coneDirection,
                int contentMask);
void __cdecl G_FlashbangBlast(float *origin, float radius_max, float radius_min, gentity_s *attacker, team_t team);
void __cdecl GetEntListForRadius(
                const float *origin,
                float radius_max,
                float radius_min,
                int *entList,
                int *entListCount);
void __cdecl FlashbangBlastEnt(
                gentity_s *ent,
                const float *blastOrigin,
                float radius_max,
                float radius_min,
                gentity_s *attacker,
                team_t team);
double __cdecl EntDistToPoint(const float *origin, gentity_s *ent);
void __cdecl AddScrTeamName(team_t team);
bool __cdecl CanEntityBeFlashbanged(gentity_s *ent);
void __cdecl GetFlashbangViewPos(gentity_s *ent, float *origin);
void __cdecl GetFlashbangViewDirection(gentity_s *ent, float *dir);
bool __cdecl G_WithinDamageRadius(const float *damageOrigin, float radiusSquared, gentity_s *ent);
double __cdecl G_GetRadiusDamageDistanceSquared(const float *damageOrigin, gentity_s *ent);
int __cdecl G_RadiusDamage(
                float *origin,
                gentity_s *inflictor,
                gentity_s *attacker,
                float fInnerDamage,
                float fOuterDamage,
                float radius,
                float coneAngleCos,
                float *coneDirection,
                gentity_s *ignore,
                int mod,
                unsigned int weapon);
unsigned __int16 __cdecl G_GetHitLocationString(hitLocation_t hitLoc);
int __cdecl G_GetHitLocationIndexFromString(unsigned __int16 sString);

extern unsigned char bulletPriorityMap[20];
