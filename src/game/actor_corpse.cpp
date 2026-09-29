#include "actor_corpse.h"
#include <game_mp/g_utils_mp.h>
#include <clientscript/scr_const.h>
#include "sentient.h"
#include <client_mp/g_client_mp.h>
#include <game_mp/g_main_mp.h>
#include <gfx_d3d/r_dobj_skin.h>
#include <server/sv_world.h>
#include "g_player_corpse.h"
#include <game_mp/actor_mp.h>
#include "actor_event_listeners.h"
#include <qcommon/dobj_management.h>
#include <game_mp/g_misc_mp.h>
#include <game_mp/g_spawn_mp.h>
#include <cgame/cg_event.h>

int __cdecl G_GetActorCorpseIndex(gentity_s *ent)
{
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 8; ++i )
    {
        if ( g_scr_data.actorCorpseInfo[i].entnum == ent->s.number )
            return i;
    }
    if ( !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_corpse.cpp",
                    60,
                    0,
                    "G_GetActorCorpseIndex called for non actor corpse") )
        __debugbreak();
    return 0;
}

int __cdecl G_GetFreeActorCorpseIndex(int reuse)
{
    float vRefDir[3]; // [esp+10h] [ebp-58h] BYREF
    float farthestBehindDistSq; // [esp+1Ch] [ebp-4Ch]
    float vRefPos[3]; // [esp+20h] [ebp-48h] BYREF
    int farthestIndex; // [esp+2Ch] [ebp-3Ch]
    int bestIndex; // [esp+30h] [ebp-38h]
    bool use2DDist; // [esp+37h] [ebp-31h]
    float farthestDistSq; // [esp+38h] [ebp-30h]
    gentity_s *ent; // [esp+3Ch] [ebp-2Ch]
    int farthest90DegreeIndex; // [esp+40h] [ebp-28h]
    int i; // [esp+44h] [ebp-24h]
    float distSq; // [esp+48h] [ebp-20h]
    float farthest90DegreeDistSq; // [esp+4Ch] [ebp-1Ch]
    float dot; // [esp+50h] [ebp-18h]
    float vDelta[3]; // [esp+54h] [ebp-14h]
    int farthestBehindIndex; // [esp+60h] [ebp-8h]
    int found; // [esp+64h] [ebp-4h]

    farthestBehindDistSq = -1.0f;
    farthestBehindIndex = 0;
    farthest90DegreeDistSq = -1.0f;
    farthest90DegreeIndex = 0;
    farthestDistSq = -1.0f;
    farthestIndex = 0;
    ent = G_Find(0, 356, scr_const.player);
    if ( ent )
    {
        Sentient_GetEyePosition(ent->sentient, vRefPos);
        G_GetPlayerViewDirection(ent, vRefDir, 0, 0);
        use2DDist = 1;
        vRefDir[2] = 0.0f;
        if ( Vec3Normalize(vRefDir) == 0.0 )
        {
            use2DDist = 0;
            G_GetPlayerViewDirection(ent, vRefDir, 0, 0);
        }
    }
    else
    {
        use2DDist = 1;
        vRefDir[2] = 0.0f;
        vRefDir[0] = 0.0f;
        vRefDir[1] = 1.0f;
        memset(vRefPos, 0, sizeof(vRefPos));
    }
    found = 0;
    for ( i = 0; i < level.actorCorpseCount; ++i )
    {
        if ( g_scr_data.actorCorpseInfo[i].entnum == -1 )
        {
            if ( reuse )
                return i;
        }
        else
        {
            found = 1;
            ent = &level.gentities[g_scr_data.actorCorpseInfo[i].entnum];
            vDelta[0] = ent->r.currentOrigin[0] - vRefPos[0];
            vDelta[1] = ent->r.currentOrigin[1] - vRefPos[1];
            vDelta[2] = ent->r.currentOrigin[2] - vRefPos[2];
            if ( use2DDist )
                vDelta[2] = 0.0f;
            distSq = (float)((float)(vDelta[0] * vDelta[0]) + (float)(vDelta[1] * vDelta[1])) + (float)(vDelta[2] * vDelta[2]);
            dot = (float)((float)(vDelta[0] * vRefDir[0]) + (float)(vDelta[1] * vRefDir[1])) + (float)(vDelta[2] * vRefDir[2]);
            if ( dot < 0.0 && distSq > farthestBehindDistSq )
            {
                farthestBehindDistSq = distSq;
                farthestBehindIndex = i;
            }
            if ( (float)(0.5 * distSq) >= (float)(dot * dot) && distSq > farthest90DegreeDistSq )
            {
                farthest90DegreeDistSq = distSq;
                farthest90DegreeIndex = i;
            }
            if ( distSq > farthestDistSq )
            {
                farthestDistSq = distSq;
                farthestIndex = i;
            }
        }
    }
    if ( found )
    {
        if ( farthestBehindDistSq == -1.0 )
        {
            if ( farthest90DegreeDistSq == -1.0 )
                bestIndex = farthestIndex;
            else
                bestIndex = farthest90DegreeIndex;
        }
        else
        {
            bestIndex = farthestBehindIndex;
        }
        ent = &level.gentities[g_scr_data.actorCorpseInfo[bestIndex].entnum];
        G_FreeEntity(ent);
        g_scr_data.actorCorpseInfo[bestIndex].entnum = -1;
        return bestIndex;
    }
    else
    {
        if ( reuse
            && level.actorCorpseCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_corpse.cpp",
                        165,
                        0,
                        "%s",
                        "!reuse || !level.actorCorpseCount") )
        {
            __debugbreak();
        }
        return -1;
    }
}

void __cdecl G_RemoveOneActorCorpse()
{
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < level.actorCorpseCount; ++i )
    {
        if ( g_scr_data.actorCorpseInfo[i].entnum >= 0 )
        {
            G_FreeEntity(&level.gentities[g_scr_data.actorCorpseInfo[i].entnum]);
            return;
        }
    }
}

void __cdecl G_RemoveAllActorCorpses()
{
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < level.actorCorpseCount; ++i )
    {
        if ( g_scr_data.actorCorpseInfo[i].entnum >= 0 )
            G_FreeEntity(&level.gentities[g_scr_data.actorCorpseInfo[i].entnum]);
    }
}

void __cdecl G_RemoveActorCorpses(unsigned int allowedCorpseCount)
{
    signed int i; // [esp+4h] [ebp-4h]

    if ( allowedCorpseCount > 8
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_corpse.cpp",
                    232,
                    0,
                    "%s\n\t(allowedCorpseCount) = %i",
                    "(allowedCorpseCount >= 0 && allowedCorpseCount <= 8)",
                    allowedCorpseCount) )
    {
        __debugbreak();
    }
    for ( i = allowedCorpseCount; i < level.actorCorpseCount; ++i )
    {
        if ( g_scr_data.actorCorpseInfo[i].entnum >= 0 )
            G_FreeEntity(&level.gentities[g_scr_data.actorCorpseInfo[i].entnum]);
    }
    level.actorCorpseCount = allowedCorpseCount;
}

void __cdecl G_UpdateActorCorpses()
{
    signed int actorCorpseCount; // [esp+0h] [ebp-8h]
    int zombietronCorpseCount; // [esp+4h] [ebp-4h]

    if ( ((g_skinErrorFlags.allbits >> 1) & 1) != 0 )
    {
        if ( zombiemode->current.enabled )
            G_RemoveAllActorCorpses();
        else
            G_RemoveOneActorCorpse();
    }
    if (!g_skinErrorFlags.outOfSkinnedCachedVerts)
    {
        actorCorpseCount = ai_corpseCount->current.integer;
        zombietronCorpseCount = Dvar_GetInt("ai_zombietronCorpseCount");
        if ( Dvar_GetBool("zombietron") && zombietronCorpseCount > 0 )
            actorCorpseCount = zombietronCorpseCount;
        if ( (actorCorpseCount < 1 || actorCorpseCount > 8)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_corpse.cpp",
                        300,
                        0,
                        "%s\n\t(actorCorpseCount) = %i",
                        "(actorCorpseCount >= 1 && actorCorpseCount <= 8)",
                        actorCorpseCount) )
        {
            __debugbreak();
        }
        if ( level.actorCorpseCount != actorCorpseCount )
            G_RemoveActorCorpses(actorCorpseCount);
    }
}

void __cdecl ActorCorpse_Free(gentity_s *ent)
{
    int actorCorpseIndex; // [esp+0h] [ebp-4h]

    actorCorpseIndex = G_GetActorCorpseIndex(ent);
    if ( g_scr_data.actorCorpseInfo[actorCorpseIndex].entnum != ent->s.number
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_corpse.cpp",
                    401,
                    0,
                    "%s",
                    "g_scr_data.actorCorpseInfo[actorCorpseIndex].entnum == ent->s.number") )
    {
        __debugbreak();
    }
#ifdef KISAK_SP
    // Retail SP moves the actor's per-entity small tree into the corpse slot.
    // G_FreeEntity has already detached the DObj and cleared the tree by the
    // time it reaches this callback; release the small-tree allocation here.
    if ( g_scr_data.actorCorpseInfo[actorCorpseIndex].tree )
    {
        Com_XAnimFreeSmallTree(g_scr_data.actorCorpseInfo[actorCorpseIndex].tree);
        g_scr_data.actorCorpseInfo[actorCorpseIndex].tree = 0;
    }
#endif
    g_scr_data.actorCorpseInfo[actorCorpseIndex].entnum = -1;
}

void __cdecl Actor_GetBodyPlantAngles(
                int iEntNum,
                int iClipMask,
                const float *vOrigin,
                float fYaw,
                float *pfPitch,
                float *pfRoll,
                float *pfHeight)
{
    double v7; // st7
    int hitnum; // [esp+10h] [ebp-68h] BYREF
    float vEnd[3]; // [esp+3Ch] [ebp-3Ch] BYREF
    float vRight[3]; // [esp+48h] [ebp-30h] BYREF
    float fEndDown; // [esp+54h] [ebp-24h]
    float fHeight; // [esp+58h] [ebp-20h]
    float vForward[3]; // [esp+5Ch] [ebp-1Ch] BYREF
    float vStart[3]; // [esp+68h] [ebp-10h] BYREF
    float fStartUp; // [esp+74h] [ebp-4h]

    fStartUp = 30.0f;
    fEndDown = 30.0f;
    if ( !pfPitch
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_corpse.cpp", 529, 0, "%s", "pfPitch") )
    {
        __debugbreak();
    }
    vStart[0] = *vOrigin;
    vStart[1] = vOrigin[1];
    vStart[2] = vOrigin[2];
    vStart[2] = vStart[2] + fStartUp;
    vEnd[0] = *vOrigin;
    vEnd[1] = vOrigin[1];
    vEnd[2] = vOrigin[2] - fEndDown;
    YawVectors(fYaw, vForward, vRight);
    fHeight = Actor_SetBodyPlantAngle(iEntNum, iClipMask, vOrigin, vOrigin, vForward, pfPitch);
    if ( pfRoll )
    {
        if ( fabs(*pfPitch) >= 30.0 )
        {
            *pfRoll = 0.0f;
        }
        else
        {
            v7 = Actor_SetBodyPlantAngle(iEntNum, iClipMask, vOrigin, vOrigin, vRight, pfRoll);
            fHeight = v7 + fHeight;
            fHeight = fHeight * 0.5;
        }
    }
    if ( pfHeight )
    {
        fHeight = fHeight - vOrigin[2];
        if ( fHeight < 0.0 )
        {
            vStart[0] = *vOrigin;
            vStart[1] = vOrigin[1];
            vStart[2] = vOrigin[2];
            vEnd[0] = *vOrigin;
            vEnd[1] = vOrigin[1];
            vEnd[2] = vOrigin[2];
            vEnd[2] = vEnd[2] - 1.0;
            //col_context_t::col_context_t(&context, iClipMask);
            col_context_t context(iClipMask); // [esp+14h] [ebp-64h] BYREF
            context.passEntityNum0 = iEntNum;
            hitnum = -1;
            if ( SV_SightTraceCapsule(&hitnum, vStart, actorMins, actorMaxs, vEnd, &context) )
                fHeight = 0.0f;
        }
        *pfHeight = fHeight;
    }
}

double __cdecl Actor_SetBodyPlantAngle(
                int iEntNum,
                int iClipMask,
                const float *vOrigin,
                const float *vCenter,
                const float *vDir,
                float *pfAngle)
{
    float angle; // [esp+4h] [ebp-D8h]
    col_context_t context; // [esp+18h] [ebp-C4h] BYREF
    float vPointA[3]; // [esp+40h] [ebp-9Ch] BYREF
    float vEnd[3]; // [esp+4Ch] [ebp-90h] BYREF
    float fEndDown; // [esp+58h] [ebp-84h]
    float fSize; // [esp+5Ch] [ebp-80h]
    trace_t trace; // [esp+60h] [ebp-7Ch] BYREF
    float vMins[3]; // [esp+9Ch] [ebp-40h] BYREF
    float vPointB[3]; // [esp+A8h] [ebp-34h] BYREF
    float vStart[3]; // [esp+B4h] [ebp-28h] BYREF
    float fStartUp; // [esp+C0h] [ebp-1Ch]
    float vDelta[3]; // [esp+C4h] [ebp-18h] BYREF
    float vMaxs[3]; // [esp+D0h] [ebp-Ch] BYREF

    fStartUp = 30.0f;
    fEndDown = 30.0f;
    fSize = 4.0999999f;

    vMins[0] = -4.0f;
    vMins[1] = -4.0f;
    vMins[2] = 0.0f;

    vMaxs[0] = 4.0f;
    vMaxs[1] = 4.0f;
    vMaxs[2] = 8.0f;

    //col_context_t::col_context_t(&context);
    if ( g_entities[iEntNum].actor && Flame_GetLocalClientSourceRange() )
        fEndDown = 45.0f;
    vStart[0] = *vOrigin;
    vStart[1] = vOrigin[1];
    vStart[2] = vOrigin[2];
    vStart[2] = vStart[2] + fStartUp;
    vEnd[0] = (float)((float)(15.0 - fSize) * *vDir) + vStart[0];
    vEnd[1] = (float)((float)(15.0 - fSize) * vDir[1]) + vStart[1];
    vEnd[2] = (float)((float)(15.0 - fSize) * vDir[2]) + vStart[2];
    G_TraceCapsule(&trace, vStart, vMins, vMaxs, vEnd, iEntNum, iClipMask, &context);
    if ( trace.fraction == 0.0 )
    {
        *pfAngle = 0.0f;
        return vCenter[2];
    }
    else
    {
        Vec3Lerp(vStart, vEnd, trace.fraction, vStart);
        vEnd[0] = vStart[0];
        vEnd[1] = vStart[1];
        vEnd[2] = vStart[2];
        vEnd[2] = vOrigin[2] - fEndDown;
        G_TraceCapsule(&trace, vStart, vMins, vMaxs, vEnd, iEntNum, iClipMask, &context);
        if ( trace.startsolid || trace.fraction >= 1.0 )
        {
            vPointA[0] = *vCenter;
            vPointA[1] = vCenter[1];
            vPointA[2] = vCenter[2];
        }
        else
        {
            Vec3Lerp(vStart, vEnd, trace.fraction, vPointA);
        }
        vStart[0] = *vOrigin;
        vStart[1] = vOrigin[1];
        vStart[2] = vOrigin[2];
        vStart[2] = vStart[2] + fStartUp;
        vEnd[0] = (float)((-(15.0 - fSize)) * *vDir) + vStart[0];
        vEnd[1] = (float)((-(15.0 - fSize)) * vDir[1]) + vStart[1];
        vEnd[2] = (float)((-(15.0 - fSize)) * vDir[2]) + vStart[2];
        G_TraceCapsule(&trace, vStart, vMins, vMaxs, vEnd, iEntNum, iClipMask, &context);
        if ( trace.fraction == 0.0 )
        {
            *pfAngle = 0.0f;
            return vCenter[2];
        }
        else
        {
            Vec3Lerp(vStart, vEnd, trace.fraction, vStart);
            vEnd[0] = vStart[0];
            vEnd[1] = vStart[1];
            vEnd[2] = vStart[2];
            vEnd[2] = vOrigin[2] - fEndDown;
            G_TraceCapsule(&trace, vStart, vMins, vMaxs, vEnd, iEntNum, iClipMask, &context);
            if ( trace.startsolid || trace.fraction >= 1.0 )
            {
                vPointB[0] = *vCenter;
                vPointB[1] = vCenter[1];
                vPointB[2] = vCenter[2];
            }
            else
            {
                Vec3Lerp(vStart, vEnd, trace.fraction, vPointB);
            }
            if ( vPointA[0] == vPointB[0] && vPointA[1] == vPointB[1] && vPointA[2] == vPointB[2] )
            {
                *pfAngle = 0.0f;
            }
            else
            {
                vDelta[0] = vPointA[0] - vPointB[0];
                vDelta[1] = vPointA[1] - vPointB[1];
                vDelta[2] = vPointA[2] - vPointB[2];
                angle = vectopitch(vDelta);
                *pfAngle = AngleNormalize180(angle);
            }
            return (vCenter[2] + vPointA[2] + vPointB[2]) / 3.0;
        }
    }
}

void __cdecl Actor_OrientCorpseToGround(gentity_s *self, int bLerp)
{
    float v2; // [esp+10h] [ebp-74h]
    float v3; // [esp+14h] [ebp-70h]
    float v4; // [esp+18h] [ebp-6Ch]
    float v5; // [esp+1Ch] [ebp-68h]
    float v6; // [esp+28h] [ebp-5Ch]
    float v7; // [esp+30h] [ebp-54h]
    float v8; // [esp+40h] [ebp-44h]
    float v9; // [esp+50h] [ebp-34h]
    float v10; // [esp+60h] [ebp-24h]
    float fNewRoll; // [esp+70h] [ebp-14h] BYREF
    float fNewPitch; // [esp+74h] [ebp-10h] BYREF
    float fNewHeight; // [esp+78h] [ebp-Ch] BYREF
    actor_prone_info_s *pProneInfo; // [esp+7Ch] [ebp-8h]
    int iClipMask; // [esp+80h] [ebp-4h]

    if ( self->s.eType != 19
        && self->s.eType != 17
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_corpse.cpp",
                    601,
                    0,
                    "%s",
                    "(self->s.eType == ET_ACTOR_CORPSE) || (self->s.eType == ET_ACTOR)") )
    {
        __debugbreak();
    }
    pProneInfo = &self->actor->ProneInfo;
    if ( pProneInfo->bCorpseOrientation )
    {
        iClipMask = self->clipmask & 0xFDFF7FFF;
        if ( bLerp )
        {
            Actor_GetBodyPlantAngles(
                self->s.number,
                iClipMask,
                self->r.currentOrigin,
                self->r.currentAngles[1],
                &fNewPitch,
                &fNewRoll,
                &fNewHeight);
            v10 = AngleNormalize180(fNewPitch - pProneInfo->fTorsoPitch);
            if ( fabs(v10) <= 12.0 )
            {
                v9 = fNewPitch;
            }
            else
            {
                if ( v10 < 0.0 )
                    v4 = -1.0f;
                else
                    v4 = 1.0f;
                v9 = (float)(v4 * 12.0) + pProneInfo->fTorsoPitch;
            }
            pProneInfo->fTorsoPitch = v9;
            v8 = AngleNormalize180(fNewRoll - pProneInfo->fWaistPitch);
            if ( fabs(v8) <= 12.0 )
            {
                v7 = fNewRoll;
            }
            else
            {
                if ( v8 < 0.0 )
                    v3 = -1.0f;
                else
                    v3 = 1.0f;
                v7 = (float)(v3 * 12.0) + pProneInfo->fWaistPitch;
            }
            pProneInfo->fWaistPitch = v7;
            v6 = fNewHeight - pProneInfo->fBodyHeight;
            if ( fabs(v6) <= 0.60000002 )
            {
                v5 = fNewHeight;
            }
            else
            {
                if ( v6 < 0.0 )
                    v2 = -1.0f;
                else
                    v2 = 1.0f;
                v5 = (float)(v2 * 0.60000002) + pProneInfo->fBodyHeight;
            }
            pProneInfo->fBodyHeight = v5;
        }
        else
        {
            Actor_GetBodyPlantAngles(
                self->s.number,
                iClipMask,
                self->r.currentOrigin,
                self->r.currentAngles[1],
                &pProneInfo->fTorsoPitch,
                &pProneInfo->fWaistPitch,
                &pProneInfo->fBodyHeight);
        }
    }
}

void __cdecl Actor_OrientPitchToGround(gentity_s *self, int bLerp)
{
    float v2; // [esp+10h] [ebp-54h]
    float v3; // [esp+14h] [ebp-50h]
    float v4; // [esp+18h] [ebp-4Ch]
    float v5; // [esp+20h] [ebp-44h]
    float v6; // [esp+2Ch] [ebp-38h]
    float v7; // [esp+34h] [ebp-30h]
    float v8; // [esp+44h] [ebp-20h]
    float fNewPitch; // [esp+54h] [ebp-10h] BYREF
    float fNewHeight; // [esp+58h] [ebp-Ch] BYREF
    actor_prone_info_s *pProneInfo; // [esp+5Ch] [ebp-8h]
    int iClipMask; // [esp+60h] [ebp-4h]

    if ( self->s.eType != 17
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_corpse.cpp",
                    648,
                    0,
                    "%s",
                    "self->s.eType == ET_ACTOR") )
    {
        __debugbreak();
    }
    pProneInfo = &self->actor->ProneInfo;
    if ( pProneInfo->orientPitch )
    {
        iClipMask = self->clipmask & 0xFDFF7FFF;
        if ( bLerp )
        {
            Actor_GetBodyPlantAngles(
                self->s.number,
                iClipMask,
                self->r.currentOrigin,
                self->r.currentAngles[1],
                &fNewPitch,
                0,
                &fNewHeight);
            v8 = AngleNormalize180(fNewPitch - pProneInfo->fTorsoPitch);
            if ( fabs(v8) <= 12.0 )
            {
                v7 = fNewPitch;
            }
            else
            {
                if ( v8 < 0.0 )
                    v4 = -1.0f;
                else
                    v4 = 1.0f;
                v7 = (float)(v4 * 12.0) + pProneInfo->fTorsoPitch;
            }
            pProneInfo->fTorsoPitch = v7;
            v6 = fNewHeight - pProneInfo->fBodyHeight;
            if ( fabs(v6) <= 0.60000002 )
            {
                v5 = fNewHeight;
            }
            else
            {
                if ( v6 < 0.0 )
                    v3 = -1.0f;
                else
                    v3 = 1.0f;
                v5 = (float)(v3 * 0.60000002) + pProneInfo->fBodyHeight;
            }
            pProneInfo->fBodyHeight = v5;
            v2 = floor(pProneInfo->fTorsoPitch * 182.04445 + 0.5);
            self->s.lerp.u.actor.proneInfo.fBodyPitch = (int)v2;
        }
        else
        {
            Actor_GetBodyPlantAngles(
                self->s.number,
                iClipMask,
                self->r.currentOrigin,
                self->r.currentAngles[1],
                &pProneInfo->fTorsoPitch,
                0,
                &pProneInfo->fBodyHeight);
        }
    }
}

int __cdecl Actor_BecomeCorpse(gentity_s *self)
{
#ifdef KISAK_SP
    // Retail SP FUN_00642b80 converts the actor entity in place.  It swaps the
    // actor's exact XAnimTree into actorCorpseInfo and never calls
    // XAnimCloneAnimTree.  That ownership transfer is required for zombies:
    // useAnimTree installs a species-specific tree whose layout cannot be
    // cloned into MP's preallocated generic/dog corpse tree.
    const int corpseIndex = G_GetFreeActorCorpseIndex(1);
    if ( corpseIndex < 0 )
        return 0;

    actor_s *actor = self->actor;
    const int actorIndex = G_GetActorIndex(actor);
    XAnimTree_s *tree = self->pAnimTree;
    if ( !tree )
    {
        if ( !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_corpse.cpp",
                    840,
                    0,
                    "%s",
                    "self->pAnimTree") )
        {
            __debugbreak();
        }
        return 0;
    }

    const trType_t origType = (trType_t)self->s.lerp.pos.trType;
    float origin[3];
    Vec3Copy(self->r.currentOrigin, origin);

    Actor_EventListener_RemoveEntity(self->s.number);

    // Detach the live tree before Actor_Free so normal actor teardown does not
    // destroy it.  G_DObjUpdate below rebuilds the same entity's DObj around
    // the transferred tree after its actor/sentient ownership is gone.
    DObj *dobj = Com_GetServerDObj(self->s.number);
    if ( dobj )
        DObjSetTree(dobj, 0);
    self->pAnimTree = 0;
    g_scr_data.actorXAnimTrees[actorIndex] = 0;
    Actor_Free(actor);

    corpseInfo_t *corpseInfo = &g_scr_data.actorCorpseInfo[corpseIndex];
    corpseInfo->tree = tree;
    corpseInfo->entnum = self->s.number;
    corpseInfo->time = level.time;
    corpseInfo->falling = true;
    self->pAnimTree = tree;

    self->handler = 3;
    self->r.contents = 0x4000000;
    self->s.eType = ET_ACTOR_CORPSE;
    self->clipmask = 0x820011;
    self->physicsObject = 1;
    self->s.lerp.eFlags |= 0x40010;
    self->s.groundEntityNum = 1023;
    self->item[0].ammoCount = level.time;

    G_SetOrigin(self, origin);
    if ( origType >= TR_FIRST_RAGDOLL && origType <= TR_LAST_RAGDOLL )
        AssignToSmallerType<unsigned char>(&self->s.lerp.pos.trType, origType);
    else
        self->s.lerp.pos.trType = TR_STATIONARY;

    self->s.lerp.u.actor.actorNum = corpseIndex;
    ScrNotify_FaceEvent(self, scr_const.death);
    Scr_Notify(self, scr_const.death, 0);
    Scr_FreeEntityNum(self->s.number, 0, SCRIPTINSTANCE_SERVER);
    G_DObjUpdate(self);
    SV_LinkEntity(self);
    return 1;
#else
    XAnimTree_s *tree; // [esp+40h] [ebp-44h]
    const DObj *dobj; // [esp+44h] [ebp-40h]
    trType_t origType; // [esp+64h] [ebp-20h]
    gentity_s *body; // [esp+6Ch] [ebp-18h]
    actor_s *actor; // [esp+78h] [ebp-Ch]
    corpseInfo_t *corpseInfo; // [esp+7Ch] [ebp-8h]
    int axis; // [esp+80h] [ebp-4h]
    int savedregs; // [esp+84h] [ebp+0h] BYREF

    actor = self->actor;
    G_GetActorIndex(actor);
    BG_ActorIsProne(&actor->ProneInfo, level.time);
    Actor_EventListener_RemoveEntity(self->s.number);
    body = G_SpawnActorClone();
    AssignToSmallerType<int>(&body->s.lerp.useCount, body->useCount + 1);
    body->s.lerp.eFlags = body->s.lerp.eFlags & 2 | self->s.lerp.eFlags & 0xFFFFFFFD | 0x40010;
    G_SetOrigin(body, self->r.currentOrigin);
    G_SetAngle(body, self->r.currentAngles);
    body->s.lerp.pos.trType = 6;
    body->s.lerp.pos.trTime = level.time;
    body->s.lerp.pos.trDelta[0] = self->s.lerp.pos.trDelta[0];
    body->s.lerp.pos.trDelta[1] = self->s.lerp.pos.trDelta[1];
    body->s.lerp.pos.trDelta[2] = self->s.lerp.pos.trDelta[2];
    if ( ((LODWORD(body->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(body->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(body->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_corpse.cpp",
                    840,
                    0,
                    "%s",
                    "!IS_NAN((body->s.lerp.pos.trDelta)[0]) && !IS_NAN((body->s.lerp.pos.trDelta)[1]) && !IS_NAN((body->s.lerp.pos.trDelta)[2])") )
    {
        __debugbreak();
    }
    body->physicsObject = 1;
    body->s.eType = ET_ACTOR_CORPSE;
    dobj = Com_GetServerDObj(self->s.number);
    tree = DObjGetTree(dobj);
    for ( axis = 0; axis < 2; ++axis )
    {
        if ( body->s.lerp.pos.trDelta[axis] > g_clonePlayerMaxVelocity->current.value )
            body->s.lerp.pos.trDelta[axis] = g_clonePlayerMaxVelocity->current.value;
    }
    if ( ((LODWORD(body->s.lerp.pos.trDelta[0]) & 0x7F800000) == 0x7F800000
         || (LODWORD(body->s.lerp.pos.trDelta[1]) & 0x7F800000) == 0x7F800000
         || (LODWORD(body->s.lerp.pos.trDelta[2]) & 0x7F800000) == 0x7F800000)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_corpse.cpp",
                    853,
                    0,
                    "%s",
                    "!IS_NAN((body->s.lerp.pos.trDelta)[0]) && !IS_NAN((body->s.lerp.pos.trDelta)[1]) && !IS_NAN((body->s.lerp.pos.trDelta)[2])") )
    {
        __debugbreak();
    }
    body->item[0].ammoCount = level.time;
    corpseInfo = &g_scr_data.actorCorpseInfo[G_GetFreeActorCorpseIndex(1)];
    corpseInfo->entnum = body->s.number;
    corpseInfo->time = level.time;
    corpseInfo->falling = 1;
    XAnimCloneAnimTree(tree, corpseInfo->tree);
    body->s.groundEntityNum = 1023;
    if ( body->r.svFlags
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_corpse.cpp", 869, 0, "%s", "!body->r.svFlags") )
    {
        __debugbreak();
    }
    body->r.svFlags = 2;
    body->r.mins[0] = self->r.mins[0];
    body->r.mins[1] = self->r.mins[1];
    body->r.mins[2] = self->r.mins[2];
    body->r.maxs[0] = self->r.maxs[0];
    body->r.maxs[1] = self->r.maxs[1];
    body->r.maxs[2] = self->r.maxs[2];
    body->r.absmin[0] = self->r.absmin[0];
    body->r.absmin[1] = self->r.absmin[1];
    body->r.absmin[2] = self->r.absmin[2];
    body->r.absmax[0] = self->r.absmax[0];
    body->r.absmax[1] = self->r.absmax[1];
    body->r.absmax[2] = self->r.absmax[2];
    //body->s.un2 = self->s.un2;
    body->s.animState = self->s.animState;
    body->clipmask = 65537;
    body->r.contents = 0x4004000 /* nx-port: decompiled as (int)&objBuf[1890][6] */;
    body->nextthink = level.time + 300;
    body->handler = 3;
    body->model = self->model;
    body->s.enemyModel = self->s.enemyModel;
    body->s.lerp.u.actor.actorNum = self->s.lerp.u.actor.actorNum;
    body->s.lerp.u.actor.team = self->s.lerp.u.actor.team;
    self->physicsObject = 1;
    if ( Com_IsRagdollTrajectory(&body->s.lerp.pos) )
    {
        origType = (trType_t)body->s.lerp.pos.trType;
        if ( (unsigned int)origType >= NUM_TRTYPES
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_corpse.cpp",
                        895,
                        0,
                        "%s",
                        "origType >= 0 && origType < NUM_TRTYPES") )
        {
            __debugbreak();
        }
        G_SetOrigin(self, body->r.currentOrigin);
        AssignToSmallerType<unsigned char>(&body->s.lerp.pos.trType, origType);
        if ( self->s.lerp.pos.trType >= 0xFu
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_corpse.cpp",
                        898,
                        0,
                        "%s",
                        "self->s.lerp.pos.trType < NUM_TRTYPES") )
        {
            __debugbreak();
        }
    }
    else
    {
        G_SetOrigin(self, body->r.currentOrigin);
        body->s.lerp.pos.trType = 0;
    }
    Actor_Free(actor);
    if ( self->actor
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_corpse.cpp",
                    908,
                    0,
                    "%s",
                    "self->actor == NULL") )
    {
        __debugbreak();
    }
    if ( self->sentient
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_corpse.cpp",
                    909,
                    0,
                    "%s",
                    "self->sentient == NULL") )
    {
        __debugbreak();
    }
    ScrNotify_FaceEvent(self, scr_const.death);
    Scr_Notify(self, scr_const.death, 0);
    Scr_FreeEntityNum(self->s.number, 0, SCRIPTINSTANCE_SERVER);
    G_DObjUpdate(body);
    SV_LinkEntity(body);
    return 0;
#endif
}

