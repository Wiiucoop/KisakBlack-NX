#include "actor_spawner.h"
#include <qcommon/cm_world.h>
#include <game_mp/g_main_mp.h>
#include <clientscript/cscr_stringlist.h>
#include <game_mp/g_active_mp.h>
#include <clientscript/cscr_vm.h>
#include <game_mp/g_utils_mp.h>
#include <game_mp/g_spawn_mp.h>
#include <game_mp/actor_mp.h>
#include "actor_events.h"
#include "actor_senses.h"
#include <clientscript/scr_const.h>

const float g_vSpawnCheckPoints[11][3] =
{
  { 0.5, 0.5, 0.80000001 },
  { 0.5, 0.5, 0.5 },
  { 0.5, 0.5, 0.2 },
  { 0.0, 0.0, 1.0 },
  { 0.0, 1.0, 1.0 },
  { 1.0, 1.0, 1.0 },
  { 1.0, 0.0, 1.0 },
  { 1.0, 0.0, 0.0 },
  { 1.0, 1.0, 0.0 },
  { 0.0, 1.0, 0.0 },
  { 0.0, 0.0, 0.0 }
};

#ifdef KISAK_SP
// Retail SP's setailimit/getailimit/resetailimit handlers at
// 0x00804AD0/0x00804B10/0x00804D60 share the dword at 0x01C08AE4.
// SpawnActor reads it before allocating an actor and refuses the spawn once
// the number of in-use actors reaches the non-zero limit.
static int s_aiLimit;

void G_SetAILimit_SP(int limit)
{
    s_aiLimit = limit;
}

int G_GetAILimit_SP()
{
    return s_aiLimit;
}
#endif


int __cdecl SpotWouldTelefrag(gentity_s *spot)
{
    int entityList[1025]; // [esp+8h] [ebp-1028h] BYREF
    float mins[3]; // [esp+100Ch] [ebp-24h] BYREF
    gentity_s *v4; // [esp+1018h] [ebp-18h]
    float maxs[3]; // [esp+101Ch] [ebp-14h] BYREF
    int v6; // [esp+1028h] [ebp-8h]
    int i; // [esp+102Ch] [ebp-4h]

    mins[0] = spot->r.currentOrigin[0] + -15.0;
    mins[1] = spot->r.currentOrigin[1] + -15.0;
    mins[2] = spot->r.currentOrigin[2] + 0.0;
    maxs[0] = spot->r.currentOrigin[0] + 15.0;
    maxs[1] = spot->r.currentOrigin[1] + 15.0;
    maxs[2] = spot->r.currentOrigin[2] + 70.0;
    v6 = CM_AreaEntities(mins, maxs, entityList, 1024, 0x2008000);
    for ( i = 0; i < v6; ++i )
    {
        v4 = &g_entities[entityList[i]];
        if ( v4->client && v4->client->ps.pm_type < 9 )
            return 1;
        if ( v4->actor && v4->health > 0 )
            return 1;
    }
    return 0;
}

int __cdecl PointCouldSeeSpawn(const float *vEyePos, const float *vSpawnPos, int iIgnoreEnt1, int iIgnoreEnt2)
{
    col_context_t context; // [esp+0h] [ebp-4Ch] BYREF
    float fDistSqrd; // [esp+28h] [ebp-24h]
    unsigned int i; // [esp+2Ch] [ebp-20h]
    float vCheckPos[3]; // [esp+30h] [ebp-1Ch] BYREF
    int hitNum; // [esp+3Ch] [ebp-10h] BYREF
    float vDelta[3]; // [esp+40h] [ebp-Ch]

    //col_context_t::col_context_t(&context);
    vDelta[0] = *vEyePos - *vSpawnPos;
    vDelta[1] = vEyePos[1] - vSpawnPos[1];
    vDelta[2] = vEyePos[2] - vSpawnPos[2];
    fDistSqrd = (float)((float)(vDelta[0] * vDelta[0]) + (float)(vDelta[1] * vDelta[1])) + (float)(vDelta[2] * vDelta[2]);
    if ( (float)(fDistSqrd * 1.0) > level.fFogOpaqueDistSqrd )
        return 0;
    hitNum = 0;
    context.mask = 6145;
    context.passEntityNum0 = iIgnoreEnt1;
    context.passEntityNum1 = iIgnoreEnt2;
    for ( i = 0; i < 0xB; ++i )
    {
        vCheckPos[0] = (float)(*vSpawnPos + -15.0) + (float)((float)(15.0 - -15.0) * (float)g_vSpawnCheckPoints[i][0]);
        vCheckPos[1] = (float)(vSpawnPos[1] + -15.0) + (float)((float)(15.0 - -15.0) * (float)g_vSpawnCheckPoints[i][1]);
        vCheckPos[2] = (float)(vSpawnPos[2] + 0.0) + (float)((float)(48.0 - 0.0) * (float)g_vSpawnCheckPoints[i][2]);
        SV_SightTracePoint(&hitNum, vEyePos, vCheckPos, &context);
        if ( !hitNum )
            return 1;
    }
    return 0;
}

gentity_s *__cdecl SpawnActor(gentity_s *ent, unsigned int targetname, enumForceSpawn forceSpawn, int getEnemyInfo)
{
    const char *v5; // [esp+18h] [ebp-38h]
    const char *v6; // [esp+1Ch] [ebp-34h]
    const char *v7; // [esp+20h] [ebp-30h]
    int clientNum; // [esp+24h] [ebp-2Ch]
    actor_s *pSelf; // [esp+28h] [ebp-28h]
    actor_s *pActor; // [esp+2Ch] [ebp-24h]
    sentient_s *pEnemy; // [esp+30h] [ebp-20h]
    gentity_s *spawn; // [esp+34h] [ebp-1Ch]
    float vEyePos[3]; // [esp+38h] [ebp-18h] BYREF
    unsigned int i; // [esp+44h] [ebp-Ch]
    team_t eTeam; // [esp+48h] [ebp-8h]
    gentity_s *player; // [esp+4Ch] [ebp-4h]

    if ( ai_disableSpawn->current.enabled )
    {
        Com_DPrintf(18, "Attempted spawn prevented by ai_disableSpawn.\n");
        return 0;
    }
#ifdef KISAK_SP
    const int aiLimit = G_GetAILimit_SP();
    if ( aiLimit )
    {
        int actorCount = 0;
        for ( int actorIndex = 0; actorIndex < MAX_ACTORS; ++actorIndex )
        {
            if ( level.actors[actorIndex].inuse )
                ++actorCount;
        }
        if ( actorCount >= aiLimit )
            return 0;
    }
#endif
    if ( forceSpawn == CHECK_SPAWN )
    {
        if ( SpotWouldTelefrag(ent) )
        {
            if ( ent->targetname )
                v7 = SL_ConvertToString(ent->targetname, SCRIPTINSTANCE_SERVER);
            else
                v7 = "<unnamed>";
            Com_DPrintf(
                18,
                "^3couldn't spawn from %s because spawnpoint (%7.3f %7.3f %7.3f) would telefrag\n",
                v7,
                ent->r.currentOrigin[0],
                ent->r.currentOrigin[1],
                ent->r.currentOrigin[2]);
            return 0;
        }
        for ( clientNum = 0; clientNum < com_maxclients->current.integer; ++clientNum )
        {
            player = G_GetPlayer(clientNum);
            if ( !player )
                Scr_Error("Attempt to spawn actor before player setup.", 0);
            if ( player->sentient )
            {
                if ( player->sentient->eTeam == TEAM_ALLIES )
                {
                    Sentient_GetEyePosition(player->sentient, vEyePos);
                    if ( PointCouldSeeSpawn(vEyePos, ent->r.currentOrigin, player->s.number, ent->s.number) )
                    {
                        if ( ent->targetname )
                            v6 = SL_ConvertToString(ent->targetname, SCRIPTINSTANCE_SERVER);
                        else
                            v6 = "<unnamed>";
                        Com_DPrintf(
                            18,
                            "^3couldn't spawn from %s because player can see spawnpoint (%7.3f %7.3f %7.3f)\n",
                            v6,
                            ent->r.currentOrigin[0],
                            ent->r.currentOrigin[1],
                            ent->r.currentOrigin[2]);
                        return 0;
                    }
                }
            }
        }
    }
    spawn = G_Spawn();
    G_DuplicateEntityFields(spawn, ent);
    G_DuplicateScriptFields(spawn, ent);
    Scr_SetString(&spawn->targetname, targetname, SCRIPTINSTANCE_SERVER);
    spawn->spawnflags &= ~1u;
    if ( SP_actor(spawn, 0) )
    {
        if ( !spawn->actor
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_spawner.cpp", 211, 0, "%s", "spawn->actor") )
        {
            __debugbreak();
        }
        if ( (ent->spawnflags & 8) != 0 )
        {
            for ( pEnemy = Sentient_FirstSentient(-1); pEnemy; pEnemy = Sentient_NextSentient(pEnemy, -1) )
                Actor_GetPerfectInfo(spawn->actor, pEnemy);
        }
#ifdef KISAK_SP
        // Runs the spawned AI's aitype/<name>.gsc main(), which is where its weapon, sidearm,
        // team, health and self.type come from. Actor_FinishSpawning had NO CALLER anywhere in
        // this tree -- Actor_FinishSpawningAll (actor_mp.cpp:944) calls Actor_InitAnimScript
        // instead, so nothing ever ran an aitype script and every spawned actor reached
        // animscripts/init with its weapon fields undefined:
        //     undefined is not an array index: animscripts/init.gsc:37
        //       self.weaponInfo[weapon] = SpawnStruct();   <- from initWeapon(self.sidearm)
        //
        // POSITION IS RETAIL'S, not chosen: SpawnActor at 0x00526e50 calls Actor_FinishSpawning
        // (0x0048a750) exactly here -- after the spawnflags & 8 perfect-info loop and before
        // the getEnemyInfo block reads ent->pActor. 0x0048a750 is identified by its body, not
        // by position: it resolves the classname, skips the "actor_" prefix, looks the scripts
        // up with Hunk_FindDataForFile(0, classname + 6) and runs typeScript->main through
        // Scr_ExecEntThread/Scr_FreeThread -- statement for statement this tree's
        // Actor_FinishSpawning head. The type-0 registry key is the same one
        // GScr_LoadScriptsAndAnimsForEntities now writes.
        //
        // COUPLING worth knowing: Actor_FinishSpawning asserts typeScript and typeScript->main
        // are non-null, and Assert_MyHandler __debugbreak()s in Release. The entity walk loads
        // aitype scripts with bEnforceExists = 0, so a map that names an actor type whose
        // script did not ship trades a soft "Could not find script" at load for a hard stop at
        // spawn. That is retail's own shape (its two-pass loader Com_Errors on a null handle),
        // and a missing aitype is a content error that should be loud, so it is left alone.
        //
        // NOT changed, and deliberately not guessed: retail's 0x0048a750 has a tail this
        // tree's Actor_FinishSpawning lacks -- a memset of a 0x47c per-actor block plus four
        // field writes (two zeros, two 0x3ff) and an eFlags |= 0x800 under one condition.
        // Those offsets are retail actor_s offsets and are not mapped here.
        Actor_FinishSpawning(spawn->actor);
#endif
        pSelf = spawn->actor;
        if ( getEnemyInfo )
        {
            eTeam = spawn->sentient->eTeam;
            for ( pActor = Actor_FirstActor(1 << eTeam); pActor; pActor = Actor_NextActor(pActor, 1 << eTeam) )
            {
                if ( pSelf != pActor )
                {
                    for ( i = 0; i < 0x30; ++i )
                        SentientInfo_Copy(pSelf, pActor, i);
                }
            }
        }
        Actor_UpdateSight(pSelf);
        Actor_UpdateThreat(pSelf);
        Actor_InitAnimScript(spawn->actor);
        Scr_AddEntity(spawn, SCRIPTINSTANCE_SERVER);
        Scr_Notify(ent, scr_const.spawned, 1u);
        if ( ent->count > 0 )
            --ent->count;
        return spawn;
    }
    else
    {
        if ( ent->targetname )
        {
            v5 = SL_ConvertToString(ent->targetname, SCRIPTINSTANCE_SERVER);
            Com_DPrintf(18, "^3couldn't spawn from %s because there are no free actors\n", v5);
        }
        else
        {
            Com_DPrintf(18, "^3couldn't spawn from %s because there are no free actors\n", "<unnamed>");
        }
        return 0;
    }
}

void __cdecl G_DropActorSpawnersToFloor()
{
    unsigned int v0[1025]; // [esp+20h] [ebp-1018h]
    float v1; // [esp+1024h] [ebp-14h]
    float v2; // [esp+1028h] [ebp-10h]
    float v3; // [esp+102Ch] [ebp-Ch]
    gentity_s *ent; // [esp+1030h] [ebp-8h]
    int i; // [esp+1034h] [ebp-4h]

    for ( i = 0; i < level.num_entities; ++i )
    {
        ent = &level.gentities[i];
        if ( ent->r.inuse )
        {
            v0[i] = ent->r.contents;
            if ( Path_IsDynamicBlockingEntity(ent) )
                ent->r.contents = 0;
        }
    }
    for ( i = 0; i < level.num_entities; ++i )
    {
        ent = &level.gentities[i];
        if ( ent->r.inuse )
        {
            if ( ent->s.eType == 18 )
            {
                v1 = ent->r.currentOrigin[0];
                v2 = ent->r.currentOrigin[1];
                v3 = ent->r.currentOrigin[2];
                if ( Actor_droptofloor(ent) )
                {
                    Com_Printf(18, "^3Spawner at (%g %g %g) is in solid\n", v1, v2, v3);
                    ent->r.svFlags &= ~1u;
                }
            }
        }
    }
    for ( i = 0; i < level.num_entities; ++i )
    {
        ent = &level.gentities[i];
        if ( ent->r.inuse )
            ent->r.contents = v0[i];
    }
}

int __cdecl SP_actor_spawner(gentity_s *pEnt, SpawnVar *spawnVar)
{
    if ( !spawnVar->spawnVarsValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_spawner.cpp",
                    324,
                    0,
                    "%s",
                    "spawnVar.spawnVarsValid") )
    {
        __debugbreak();
    }
    pEnt->clipmask = 0;
    pEnt->r.contents = 0;
    pEnt->r.svFlags = 1;
    pEnt->s.eType = ET_ACTOR_SPAWNER;
    pEnt->item[0].clipAmmoCount = -1;
    pEnt->item[0].ammoCount = 0;
    return 1;
}

