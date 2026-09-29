#include "cg_laser.h"
#include <universal/q_shared.h>
#include <clientscript/scr_const.h>
#include <xanim/dobj_utils.h>
#include <cgame_mp/cg_ents_mp.h>
#include <cgame_mp/cg_main_mp.h>
#include <EffectsCore/fx_beam.h>
#include <EffectsCore/fx_postlight.h>

void __cdecl CG_Laser_Add(centity_s *cent, DObj *obj, cpose_t *pose, const float *viewerPos, LaserOwnerEnum laserOwner)
{
    unsigned __int8 boneIndex; // [esp+3h] [ebp-31h] BYREF
    orientation_t orient; // [esp+4h] [ebp-30h] BYREF

    boneIndex = -2;
    if ( DObjGetBoneIndex(obj, scr_const.tag_laser, &boneIndex, -1)
        || (boneIndex = -2, DObjGetBoneIndex(obj, scr_const.tag_flash, &boneIndex, -1)) )
    {
        CG_DObjGetWorldBoneMatrix(pose, obj, boneIndex, orient.axis, orient.origin);
        CG_Laser_Add_Core(cent, obj, &orient, viewerPos, laserOwner);
    }
}

void __cdecl CG_Laser_Add_Core(
                centity_s *cent,
                DObj *obj,
                orientation_t *orient,
                const float *viewerPos,
                LaserOwnerEnum laserOwner)
{
    int v5; // [esp+20h] [ebp-154h]
    int v6; // [esp+24h] [ebp-150h]
    float v7; // [esp+2Ch] [ebp-148h]
    float laserLightAverageDist; // [esp+34h] [ebp-140h]
    float laserLength; // [esp+38h] [ebp-13Ch]
    float laserLightBeginDist; // [esp+64h] [ebp-110h]
    float mins[3]; // [esp+68h] [ebp-10Ch] BYREF
    FxBeam beam; // [esp+74h] [ebp-100h] BYREF
    float laserLightEndDist; // [esp+C0h] [ebp-B4h]
    float laserRange; // [esp+C4h] [ebp-B0h]
    float distanceBetweenViewerAndLaserEnd; // [esp+C8h] [ebp-ACh]
    trace_t traceResults; // [esp+CCh] [ebp-A8h] BYREF
    float maxs[3]; // [esp+104h] [ebp-70h] BYREF
    float endBrightness; // [esp+110h] [ebp-64h]
    float laserBegin[3]; // [esp+114h] [ebp-60h] BYREF
    float laserBeginLightPos[3]; // [esp+120h] [ebp-54h]
    float laserEndWidenScale; // [esp+12Ch] [ebp-48h]
    float laserEndLightPos[3]; // [esp+130h] [ebp-44h]
    unsigned __int8 endColorByte; // [esp+13Fh] [ebp-35h]
    FxPostLight postLight; // [esp+140h] [ebp-34h] BYREF
    float laserEnd[3]; // [esp+168h] [ebp-Ch] BYREF

    Com_Memset((unsigned int *)&traceResults, 0, sizeof(traceResults));   // nx-port: was 56
    if ( (unsigned int)laserOwner >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_laser.cpp",
                    37,
                    0,
                    "%s\n\t(laserOwner) = %i",
                    "(laserOwner == LASER_OWNER_PLAYER || laserOwner == LASER_OWNER_NON_PLAYER)",
                    laserOwner) )
    {
        __debugbreak();
    }
    if ( laserOwner == LASER_OWNER_PLAYER )
        laserRange = cg_laserRangePlayer->current.value;
    else
        laserRange = cg_laserRange->current.value;
    memset(mins, 0, sizeof(mins));
    memset(maxs, 0, sizeof(maxs));
    laserBegin[0] = orient->origin[0];
    laserBegin[1] = orient->origin[1];
    laserBegin[2] = orient->origin[2];
    laserEnd[0] = (float)(laserRange * orient->axis[0][0]) + orient->origin[0];
    laserEnd[1] = (float)(laserRange * orient->axis[0][1]) + orient->origin[1];
    laserEnd[2] = (float)(laserRange * orient->axis[0][2]) + orient->origin[2];

    col_context_t context; // [esp+3Ch] [ebp-138h] BYREF

    ////col_context_t::col_context_t(&context);
    CG_TraceCapsule(&traceResults, laserBegin, mins, maxs, laserEnd, cent->nextState.number, 0x280E833, &context);
    laserLength = traceResults.fraction * laserRange;
    v7 = (float)(traceResults.fraction * laserRange) - cg_laserEndOffset->current.value;
    laserEnd[0] = (float)(v7 * orient->axis[0][0]) + orient->origin[0];
    laserEnd[1] = (float)(v7 * orient->axis[0][1]) + orient->origin[1];
    laserEnd[2] = (float)(v7 * orient->axis[0][2]) + orient->origin[2];
    if ( traceResults.fraction > 1.0001
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\cgame\\cg_laser.cpp",
                    55,
                    0,
                    "%s",
                    "traceResults.fraction <= 1.0001f") )
    {
        __debugbreak();
    }
    beam.begin[0] = laserBegin[0];
    beam.begin[1] = laserBegin[1];
    beam.begin[2] = laserBegin[2];
    beam.end[0] = laserEnd[0];
    beam.end[1] = laserEnd[1];
    beam.end[2] = laserEnd[2];
    beam.beginColor.packed = -1;
    endBrightness = 1.0 - traceResults.fraction;
    v6 = (int)(float)(255.0 * (float)(1.0 - traceResults.fraction));
    if ( v6 >= 0 )
    {
        if (v6 <= 255)
        {
            v5 = (int)(float)(255.0 * (float)(1.0 - traceResults.fraction));
        }
        else
        {
            //LOBYTE(v5) = -1;
            v5 = -1;
        }
    }
    else
    {
        //LOBYTE(v5) = 0;
        v5 = 0;
    }
    endColorByte = v5;
    beam.endColor.packed = ((unsigned __int8)v5 << 24) | 0xFFFFFF;
    distanceBetweenViewerAndLaserEnd = Vec3Distance(laserEnd, viewerPos);
    laserEndWidenScale = (float)((float)(distanceBetweenViewerAndLaserEnd * cg_laserFlarePct->current.value) * 0.0099999998)
                                         + 1.0;
    LODWORD(beam.beginRadius) = cg_laserRadius->current.integer;
    beam.endRadius = cg_laserRadius->current.value * laserEndWidenScale;
    beam.material = cgMedia.laserMaterial;
    beam.segmentCount = 1;
    beam.wiggleDist = 0.0f;
    FX_Beam_Add(&beam);
    if ( cg_laserLight->current.enabled )
    {
        laserLightBeginDist = cg_laserLightBeginOffset->current.value;
        laserLightEndDist = laserLength - cg_laserLightEndOffset->current.value;
        if ((traceResults.cflags & 0x200C000) != 0)
            laserLightEndDist = laserLightEndDist + cg_laserLightBodyTweak->current.value;
        if ( (float)(laserLightEndDist - laserLightBeginDist) < 4.0 )
        {
            laserLightAverageDist = (float)(laserLightBeginDist + laserLightEndDist) * 0.5;
            laserLightEndDist = laserLightAverageDist + 2.0;
            laserLightBeginDist = laserLightAverageDist - 2.0;
        }
        laserBeginLightPos[0] = (float)(laserLightBeginDist * orient->axis[0][0]) + orient->origin[0];
        laserBeginLightPos[1] = (float)(laserLightBeginDist * orient->axis[0][1]) + orient->origin[1];
        laserBeginLightPos[2] = (float)(laserLightBeginDist * orient->axis[0][2]) + orient->origin[2];
        laserEndLightPos[0] = (float)(laserLightEndDist * orient->axis[0][0]) + orient->origin[0];
        laserEndLightPos[1] = (float)(laserLightEndDist * orient->axis[0][1]) + orient->origin[1];
        laserEndLightPos[2] = (float)(laserLightEndDist * orient->axis[0][2]) + orient->origin[2];
        postLight.begin[0] = laserBeginLightPos[0];
        postLight.begin[1] = laserBeginLightPos[1];
        postLight.begin[2] = laserBeginLightPos[2];
        postLight.end[0] = laserEndLightPos[0];
        postLight.end[1] = laserEndLightPos[1];
        postLight.end[2] = laserEndLightPos[2];
        postLight.radius = cg_laserLightRadius->current.value * laserEndWidenScale;
        postLight.color.packed = -1;
        postLight.material = cgMedia.laserLightMaterial;
        FX_PostLight_Add(&postLight);
    }
}

