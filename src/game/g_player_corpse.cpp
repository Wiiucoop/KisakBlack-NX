#include "g_player_corpse.h"
#include <game_mp/g_scr_main_mp.h>
#include <game_mp/g_utils_mp.h>
#include <clientscript/scr_const.h>
#include <game_mp/g_main_mp.h>
#include <cgame/cg_event.h>
#include <universal/com_math_anglevectors.h>
#include <server/sv_world.h>
#include <qcommon/dobj_management.h>

int __cdecl G_GetPlayerCorpseIndex(gentity_s *ent, const char *error_msg)
{
    const char *EntityTypeName; // eax
    const char *v4; // eax
    int i; // [esp+4h] [ebp-4h]
    int ia; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 4; ++i )
    {
        if (g_scr_data.playerCorpseInfo[i].entnum == ent->s.number)
            return i; 
        //if ( g_scr_data.actorXAnimTrees[376 * i - 1495] == (XAnimTree_s *)ent->s.number )
        //    return i;
    }

    Com_Printf(14, "Corpse List:\n");

    for ( ia = 0; ia < 4; ++ia )
        Com_Printf(
            14,
            "Corpse %i entnum %i %i\n",
            ia,
            g_scr_data.playerCorpseInfo[i].entnum,
            g_scr_data.playerCorpseInfo[i].tree);

    EntityTypeName = G_GetEntityTypeName(ent);
    v4 = va(
                 "G_GetPlayerCorpseIndex called for non player corpse from %s entity num: %i type: %s",
                 error_msg,
                 ent->s.number,
                 EntityTypeName);
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_player_corpse.cpp", 111, 0, v4) )
        __debugbreak();
    return 0;
}

int __cdecl G_GetFreePlayerCorpseIndex()
{
    float playerPos[3]; // [esp+10h] [ebp-20h] BYREF
    int bestIndex; // [esp+1Ch] [ebp-14h]
    gentity_s *ent; // [esp+20h] [ebp-10h]
    int i; // [esp+24h] [ebp-Ch]
    float distSq; // [esp+28h] [ebp-8h]
    float bestDistSq; // [esp+2Ch] [ebp-4h]

    bestDistSq = -1.0f;
    bestIndex = 0;
    ent = G_Find(0, G_FIND_CLASSNAME, scr_const.player);
    if ( !ent && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_player_corpse.cpp", 135, 0, "%s", "ent") )
        __debugbreak();
    playerPos[0] = ent->s.lerp.pos.trBase[0];
    playerPos[1] = ent->s.lerp.pos.trBase[1];
    playerPos[2] = ent->s.lerp.pos.trBase[2];
    for ( i = 0; i < 4; ++i )
    {
        //if ( g_scr_data.actorXAnimTrees[376 * i - 1495] == (XAnimTree_s *)-1 )
        if (g_scr_data.playerCorpseInfo[i].entnum == -1)
            return i;
        ent = &level.gentities[g_scr_data.playerCorpseInfo[i].entnum];
        distSq = Vec3DistanceSq(ent->r.currentOrigin, playerPos);
        if ( distSq > bestDistSq )
        {
            bestDistSq = distSq;
            bestIndex = i;
        }
    }
    Com_Printf(
        14,
        "Freeing Corpse Entity %i entnum %i %i %i\n",
        bestIndex,
        g_scr_data.playerCorpseInfo[i].entnum,
        g_scr_data.playerCorpseInfo[i].tree,
        level.time);

    ent = &level.gentities[g_scr_data.playerCorpseInfo[bestIndex].entnum];

    G_FreeEntity(ent);

    g_scr_data.playerCorpseInfo[bestIndex].entnum = -1;

    return bestIndex;
}

void __cdecl PlayerCorpse_Free(gentity_s *ent)
{
    int playerCorpseIndex; // [esp+0h] [ebp-4h]

    playerCorpseIndex = G_GetPlayerCorpseIndex(ent, "PlayerCorpse_Free");

    iassert(g_scr_data.playerCorpseInfo[playerCorpseIndex].entnum == ent->s.number);

    Com_Printf(
        14,
        "Freeing Corpse %i entnum %i %i %i\n",
        playerCorpseIndex,
        g_scr_data.playerCorpseInfo[playerCorpseIndex].entnum,
        g_scr_data.playerCorpseInfo[playerCorpseIndex].tree,
        level.time);

    g_scr_data.playerCorpseInfo[playerCorpseIndex].entnum = -1;
}

void __cdecl G_RunCorpseMove(gentity_s *ent)
{
    int v1; // [esp+8h] [ebp-18Ch]
    int v2; // [esp+Ch] [ebp-188h]
    int passEntityNum; // [esp+10h] [ebp-184h]
    bool v4; // [esp+14h] [ebp-180h]
    float v5; // [esp+24h] [ebp-170h]
    float v6; // [esp+30h] [ebp-164h]
    col_context_t v7; // [esp+6Ch] [ebp-128h] BYREF
    float DROP_CHECK_TRACE_START_HEIGHT; // [esp+94h] [ebp-100h]
    int hitnum; // [esp+98h] [ebp-FCh] BYREF
    float deltaChange[3]; // [esp+C8h] [ebp-CCh] BYREF
    bool isRagdoll; // [esp+D7h] [ebp-BDh]
    col_context_t context; // [esp+D8h] [ebp-BCh] BYREF
    float origin[3]; // [esp+100h] [ebp-94h] BYREF
    float right[3]; // [esp+10Ch] [ebp-88h] BYREF
    float start[3]; // [esp+118h] [ebp-7Ch] BYREF
    float forward[3]; // [esp+124h] [ebp-70h] BYREF
    float left[3]; // [esp+130h] [ebp-64h] BYREF
    float endpos[3]; // [esp+13Ch] [ebp-58h] BYREF
    int corpseIndex; // [esp+148h] [ebp-4Ch]
    bool haveDelta; // [esp+14Fh] [ebp-45h]
    trace_t tr; // [esp+150h] [ebp-44h] BYREF
    int mask; // [esp+18Ch] [ebp-8h]
    corpseInfo_t *corpseInfo; // [esp+190h] [ebp-4h]
    int savedregs; // [esp+194h] [ebp+0h] BYREF

    isRagdoll = Com_IsRagdollTrajectory(&ent->s.lerp.pos);
    //col_context_t::col_context_t(&context);
    if ( ent->s.eType != 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_player_corpse.cpp",
                    223,
                    0,
                    "%s",
                    "ent->s.eType == ET_PLAYER_CORPSE") )
    {
        __debugbreak();
    }
    corpseIndex = G_GetPlayerCorpseIndex(ent, "G_RunCorpseMove");
    //corpseInfo = (corpseInfo_t *)&g_scr_data.actorXAnimTrees[376 * corpseIndex - 1496];
    corpseInfo = &g_scr_data.playerCorpseInfo[corpseIndex];
    haveDelta = G_GetAnimDeltaForCorpse(ent, deltaChange);
    v4 = haveDelta
        && (float)((float)((float)(deltaChange[0] * deltaChange[0]) + (float)(deltaChange[1] * deltaChange[1]))
                         + (float)(deltaChange[2] * deltaChange[2])) > 1.0;
    haveDelta = v4;
    if ( corpseInfo->falling || haveDelta )
    {
        if ( corpseInfo->falling )
            AssignToSmallerType<unsigned char>(&ent->s.lerp.pos.trType, isRagdoll ? 13 : 6);
        else
            AssignToSmallerType<unsigned char>(&ent->s.lerp.pos.trType, isRagdoll ? 14 : 1);
        BG_EvaluateTrajectory(&ent->s.lerp.pos, level.time, origin);
        if ( haveDelta )
        {
            AngleVectors(ent->r.currentAngles, forward, right, 0);
            left[0] = -1.0 * right[0];
            left[1] = -1.0 * right[1];
            left[2] = -1.0 * right[2];
            Vec3Normalize(forward);
            Vec3Normalize(left);
            origin[0] = (float)(deltaChange[0] * forward[0]) + origin[0];
            origin[1] = (float)(deltaChange[0] * forward[1]) + origin[1];
            origin[2] = (float)(deltaChange[0] * forward[2]) + origin[2];
            origin[0] = (float)(deltaChange[1] * left[0]) + origin[0];
            origin[1] = (float)(deltaChange[1] * left[1]) + origin[1];
            origin[2] = (float)(deltaChange[1] * left[2]) + origin[2];
        }
        else
        {
            memset(left, 0, sizeof(left));
        }
        if ( !ent->clipmask
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_player_corpse.cpp",
                        262,
                        0,
                        "%s",
                        "ent->clipmask") )
        {
            __debugbreak();
        }
        mask = ent->clipmask;
        if ( (mask & ent->r.contents) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_player_corpse.cpp",
                        266,
                        0,
                        "%s",
                        "!( ent->r.contents & mask )") )
        {
            __debugbreak();
        }
        //if ( EntHandle::isDefined(&ent->r.ownerNum) )
        if ( ent->r.ownerNum.isDefined() )
        {
            //passEntityNum = EntHandle::entnum(&ent->r.ownerNum);
            passEntityNum = ent->r.ownerNum.entnum();
            G_TraceCapsule(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, passEntityNum, mask, &context);
        }
        else
        {
            G_TraceCapsule(&tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, 1023, mask, &context);
        }
        Vec3Lerp(ent->r.currentOrigin, origin, tr.fraction, endpos);
        ent->r.currentOrigin[0] = endpos[0];
        ent->r.currentOrigin[1] = endpos[1];
        ent->r.currentOrigin[2] = endpos[2];
        if ( tr.startsolid )
            tr.fraction = 0.0f;
        SV_LinkEntity(ent);
        G_RunThink(ent);
        if ( ent->r.inuse )
        {
            if ( tr.fraction == 1.0 )
            {
                if ( !corpseInfo->falling || haveDelta )
                {
                    AssignToSmallerType<unsigned char>(&ent->s.lerp.pos.trType, isRagdoll ? 14 : 1);
                    ent->s.lerp.pos.trBase[0] = endpos[0];
                    ent->s.lerp.pos.trBase[1] = endpos[1];
                    ent->s.lerp.pos.trBase[2] = endpos[2];
                    ent->s.lerp.pos.trTime = 0;
                    ent->s.lerp.pos.trDuration = 0;
                    ent->s.lerp.pos.trDelta[0] = 0.0f;
                    ent->s.lerp.pos.trDelta[1] = 0.0f;
                    ent->s.lerp.pos.trDelta[2] = 0.0f;
                    origin[2] = origin[2] - 1.0;
                    //col_context_t::col_context_t(&v10, mask);
                    col_context_t v10(mask); //[esp+9Ch] [ebp-F8h] BYREF
                    
                    //if ( EntHandle::isDefined(&ent->r.ownerNum) )
                    if (ent->r.ownerNum.isDefined())
                    {
                        //v2 = EntHandle::entnum(&ent->r.ownerNum);
                        v2 = ent->r.ownerNum.entnum();
                    }
                    else
                    {
                        v2 = 1023;
                    }

                    v10.passEntityNum0 = v2;
                    hitnum = -1;
                    if ( SV_SightTraceCapsule(&hitnum, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, &v10) )
                    {
                        corpseInfo->falling = 1;
                        AssignToSmallerType<unsigned char>(&ent->s.lerp.pos.trType, isRagdoll ? 13 : 6);
                        ent->s.lerp.pos.trBase[0] = endpos[0];
                        ent->s.lerp.pos.trBase[1] = endpos[1];
                        ent->s.lerp.pos.trBase[2] = endpos[2];
                        ent->s.lerp.pos.trDelta[0] = 0.0f;
                        ent->s.lerp.pos.trDelta[1] = 0.0f;
                        ent->s.lerp.pos.trDelta[2] = 0.0f;
                        v6 = deltaChange[0];
                        ent->s.lerp.pos.trDelta[0] = (float)(deltaChange[0] * forward[0]) + ent->s.lerp.pos.trDelta[0];
                        ent->s.lerp.pos.trDelta[1] = (float)(v6 * forward[1]) + ent->s.lerp.pos.trDelta[1];
                        ent->s.lerp.pos.trDelta[2] = (float)(v6 * forward[2]) + ent->s.lerp.pos.trDelta[2];
                        v5 = deltaChange[1];
                        ent->s.lerp.pos.trDelta[0] = (float)(deltaChange[1] * left[0]) + ent->s.lerp.pos.trDelta[0];
                        ent->s.lerp.pos.trDelta[1] = (float)(v5 * left[1]) + ent->s.lerp.pos.trDelta[1];
                        ent->s.lerp.pos.trDelta[2] = (float)(v5 * left[2]) + ent->s.lerp.pos.trDelta[2];
                        ent->s.lerp.pos.trTime = level.time;
                        ent->s.lerp.pos.trDuration = 0;
                    }
                }
            }
            else if ( SV_PointContents(ent->r.currentOrigin, -1, 0x80000000) )
            {
                G_FreeEntity(ent);
            }
            else if ( corpseInfo->falling )
            {
                if ( tr.allsolid )
                {
                    DROP_CHECK_TRACE_START_HEIGHT = 32.0f;
                    start[0] = ent->r.currentOrigin[0];
                    start[1] = ent->r.currentOrigin[1];
                    start[2] = ent->r.currentOrigin[2];
                    start[2] = start[2] + 32.0;
                    //col_context_t::col_context_t(&v7);
                    //if ( EntHandle::isDefined(&ent->r.ownerNum) )
                    if ( ent->r.ownerNum.isDefined() )
                    {
                        //v1 = EntHandle::entnum(&ent->r.ownerNum);
                        v1 = ent->r.ownerNum.entnum();
                        G_TraceCapsule(&tr, start, ent->r.mins, ent->r.maxs, origin, v1, mask & 0xFFFEFFFF, &v7);
                    }
                    else
                    {
                        G_TraceCapsule(&tr, start, ent->r.mins, ent->r.maxs, origin, 1023, mask & 0xFFFEFFFF, &v7);
                    }
                    if ( !tr.allsolid )
                    {
                        Vec3Lerp(start, ent->r.currentOrigin, tr.fraction, endpos);
                        ent->r.currentOrigin[0] = endpos[0];
                        ent->r.currentOrigin[1] = endpos[1];
                        ent->r.currentOrigin[2] = endpos[2];
                    }
                }
                G_BounceCorpse(ent, corpseInfo, &tr, endpos);
            }
        }
    }
}

void __cdecl G_BounceCorpse(gentity_s *ent, corpseInfo_t *corpseInfo, trace_t *trace, float *endpos)
{
    float vAngles[3]; // [esp+20h] [ebp-34h] BYREF
    float vAxis[3][3]; // [esp+2Ch] [ebp-28h] BYREF
    bool isRagdoll; // [esp+53h] [ebp-1h]
    int savedregs; // [esp+54h] [ebp+0h] BYREF

    isRagdoll = Com_IsRagdollTrajectory(&ent->s.lerp.pos);
    ent->s.lerp.pos.trDelta[0] = 0.0f;
    ent->s.lerp.pos.trDelta[1] = 0.0f;
    ent->s.lerp.pos.trDelta[2] = 0.0f;
    if ( trace->allsolid || trace->normal.vec.v[2] > 0.0 )
    {
        corpseInfo->falling = 0;
        AssignToSmallerType<unsigned char>(&ent->s.lerp.pos.trType, isRagdoll ? 14 : 1);
        ent->s.lerp.pos.trDelta[0] = 0.0f;
        ent->s.lerp.pos.trDelta[1] = 0.0f;
        ent->s.lerp.pos.trDelta[2] = 0.0f;
        ent->s.lerp.pos.trBase[0] = *endpos;
        ent->s.lerp.pos.trBase[1] = endpos[1];
        ent->s.lerp.pos.trBase[2] = endpos[2];
        ent->s.lerp.pos.trTime = 0;
        ent->s.lerp.pos.trDuration = 0;
        ent->s.groundEntityNum = Trace_GetEntityHitId(trace);
        g_entities[ent->s.groundEntityNum].flags |= 0x100000u;
        if ( trace->allsolid )
        {
            G_SetAngle(ent, ent->r.currentAngles);
        }
        else
        {
            vAxis[2][0] = trace->normal.vec.v[0];
            vAxis[2][1] = trace->normal.vec.v[1];
            vAxis[2][2] = trace->normal.vec.v[2];
            AngleVectors(ent->r.currentAngles, vAxis[0], 0, 0);
            Vec3Cross(vAxis[2], vAxis[0], vAxis[1]);
            Vec3Cross(vAxis[1], vAxis[2], vAxis[0]);
            AxisToAngles(vAxis, vAngles);
            G_SetAngle(ent, vAngles);
        }
        SV_LinkEntity(ent);
    }
    else
    {
        ent->r.currentOrigin[0] = ent->r.currentOrigin[0] + trace->normal.vec.v[0];
        ent->r.currentOrigin[1] = ent->r.currentOrigin[1] + trace->normal.vec.v[1];
        ent->r.currentOrigin[2] = ent->r.currentOrigin[2] + trace->normal.vec.v[2];
        ent->s.lerp.pos.trBase[0] = ent->r.currentOrigin[0];
        ent->s.lerp.pos.trBase[1] = ent->r.currentOrigin[1];
        ent->s.lerp.pos.trBase[2] = ent->r.currentOrigin[2];
        if ( ent->s.lerp.pos.trType != 6
            && !isRagdoll
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_player_corpse.cpp",
                        83,
                        0,
                        "%s",
                        "ent->s.lerp.pos.trType == TR_GRAVITY || isRagdoll") )
        {
            __debugbreak();
        }
        ent->s.lerp.pos.trTime = level.time;
    }
}

char __cdecl G_GetAnimDeltaForCorpse(gentity_s *ent, float *originChange)
{
    DObj *obj; // [esp+18h] [ebp-10h]
    float rot[3]; // [esp+1Ch] [ebp-Ch] BYREF

    obj = Com_GetServerDObj(ent->s.number);
    if ( !obj )
        return 0;
    XAnimCalcDelta(obj, 0, rot, originChange, 1);
    if ( anim_deltas_debug->current.enabled && *originChange != 0.0 )
        Com_Printf(19, "got anim delta for this frame of ( %f, %f, %f )\n", *originChange, originChange[1], originChange[2]);
    return 1;
}

void __cdecl G_RunCorpse(gentity_s *ent)
{
    G_RunCorpseMove(ent);
    G_RunCorpseAnimate(ent);
    G_RunThink(ent);
}

void __cdecl G_RunCorpseAnimate(gentity_s *ent)
{
    DObj *ServerDObj; // eax
    int corpseIndex; // [esp+4h] [ebp-8h]

    corpseIndex = G_GetPlayerCorpseIndex(ent, "G_RunCorpseAnimate");
    if ( corpseIndex < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_player_corpse.cpp",
                    354,
                    0,
                    "%s",
                    "corpseIndex >= 0") )
    {
        __debugbreak();
    }
    if ( corpseIndex >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\g_player_corpse.cpp",
                    355,
                    0,
                    "%s",
                    "corpseIndex < MAX_CLIENT_CORPSES") )
    {
        __debugbreak();
    }
    ServerDObj = Com_GetServerDObj(ent->s.number);
    BG_UpdatePlayerDObj(-1, ServerDObj, &ent->s, &g_scr_data.playerCorpseInfo[corpseIndex].ci, 0);
    if ( Com_GetServerDObj(ent->s.number) )
        BG_PlayerAnimation(-1, &ent->s, &g_scr_data.playerCorpseInfo[corpseIndex].ci);
}

