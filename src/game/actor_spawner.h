#pragma once
#include <bgame/bg_public.h>
#include <qcommon/common.h>

enum enumForceSpawn : __int32
{                                       // XREF: ?SpawnActor@@YAPAUgentity_s@@PAU1@IW4enumForceSpawn@@H@Z/r
    CHECK_SPAWN = 0x0,
    FORCE_SPAWN = 0x1,
};

int __cdecl SpotWouldTelefrag(gentity_s *spot);
int __cdecl PointCouldSeeSpawn(const float *vEyePos, const float *vSpawnPos, int iIgnoreEnt1, int iIgnoreEnt2);
gentity_s *__cdecl SpawnActor(gentity_s *ent, unsigned int targetname, enumForceSpawn forceSpawn, int getEnemyInfo);
void __cdecl G_DropActorSpawnersToFloor();
int __cdecl SP_actor_spawner(gentity_s *pEnt, SpawnVar *spawnVar);

#ifdef KISAK_SP
void G_SetAILimit_SP(int limit);
int G_GetAILimit_SP();
#endif
