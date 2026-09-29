#include "r_dpvs_entity.h"
#include "r_dpvs_sceneent.h"
#include "r_model_pose.h"
#include <cgame/cg_pose_utils.h>
#include "r_dobj_skin.h"

void __cdecl R_AddEntitySurfacesInFrustumCmd(unsigned __int16 *data)
{
    int v1; // [esp+4h] [ebp-30h]
    int v2; // [esp+8h] [ebp-2Ch]
    const float *minmax; // [esp+Ch] [ebp-28h]
    DpvsPlane *plane; // [esp+14h] [ebp-20h]
    int v5; // [esp+18h] [ebp-1Ch]
    DObjAnimMat *boneMatrix; // [esp+1Ch] [ebp-18h]
    const DObj *obj; // [esp+24h] [ebp-10h] BYREF
    GfxSceneEntity *localSceneEnt; // [esp+28h] [ebp-Ch] BYREF
    const DpvsPlane *planes; // [esp+2Ch] [ebp-8h]
    GfxSceneEntity *sceneEnt; // [esp+30h] [ebp-4h]

    sceneEnt = *(GfxSceneEntity **)data;
    boneMatrix = R_UpdateSceneEntBounds(sceneEnt, &localSceneEnt, &obj, 1);
    if ( boneMatrix )
    {
        if ( !localSceneEnt
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_entity.cpp",
                        416,
                        0,
                        "%s",
                        "localSceneEnt") )
        {
            __debugbreak();
        }
        // nx-port: data is a DpvsEntityCmd; it was read at x86 offsets
        planes = ((const DpvsEntityCmd *)data)->planes;
        v2 = ((const DpvsEntityCmd *)data)->planeCount;
        minmax = localSceneEnt->cull.mins;
        v5 = 0;
        plane = (DpvsPlane *)planes;
        while ( v5 < v2 )
        {
            if ( R_DpvsPlaneMaxSignedDistToBox(plane, minmax) <= 0.0 )
            {
                v1 = 1;
                goto LABEL_14;
            }
            ++v5;
            ++plane;
        }
        v1 = 0;
LABEL_14:
        if ( !v1
            && R_BoundsInCell(
                     (mnode_t *)g_worldDpvsPlanes->nodes,
                     ((const DpvsEntityCmd *)data)->cellIndex,
                     localSceneEnt->cull.mins,
                     localSceneEnt->cull.maxs) )
        {
            CG_CullIn(localSceneEnt->info.pose);
            R_SkinSceneDObj(sceneEnt, localSceneEnt, obj, boneMatrix, 0);
            if ( localSceneEnt->entnum == gfxCfg.entnumNone
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_entity.cpp",
                            480,
                            0,
                            "%s",
                            "localSceneEnt->entnum != gfxCfg.entnumNone") )
            {
                __debugbreak();
            }
            ((const DpvsEntityCmd *)data)->entVisData[localSceneEnt->entnum] = 1;
        }
        else
        {
            CG_UsedDObjCalcPose(localSceneEnt->info.pose);
        }
    }
    else if ( localSceneEnt )
    {
        CG_UsedDObjCalcPose(localSceneEnt->info.pose);
    }
}

bool __cdecl R_BoundsInCell(mnode_t *node, int findCellIndex, const float *mins, const float *maxs)
{
    return R_BoundsInCell_r(node, findCellIndex, mins, maxs);
}

bool __cdecl R_BoundsInCell_r(mnode_t *node, int findCellIndex, const float *mins, const float *maxs)
{
    float localmaxs[3]; // [esp+0h] [ebp-58h]
    float dist; // [esp+Ch] [ebp-4Ch]
    float localmins[3]; // [esp+10h] [ebp-48h] BYREF
    unsigned int type; // [esp+1Ch] [ebp-3Ch]
    int side; // [esp+20h] [ebp-38h]
    cplane_s *plane; // [esp+24h] [ebp-34h]
    int cellIndex; // [esp+28h] [ebp-30h]
    mnode_t *leftNode; // [esp+30h] [ebp-28h]
    float mins2[3]; // [esp+34h] [ebp-24h] BYREF
    int cellCount; // [esp+40h] [ebp-18h]
    float maxs2[3]; // [esp+44h] [ebp-14h] BYREF
    mnode_t *rightNode; // [esp+50h] [ebp-8h]
    int planeIndex; // [esp+54h] [ebp-4h]

    cellCount = g_worldDpvsPlanes->cellCount + 1;
    mins2[0] = *mins;
    mins2[1] = mins[1];
    mins2[2] = mins[2];
    maxs2[0] = *maxs;
    maxs2[1] = maxs[1];
    maxs2[2] = maxs[2];
    while ( 1 )
    {
        cellIndex = node->cellIndex;
        planeIndex = cellIndex - cellCount;
        if ( cellIndex - cellCount < 0 )
            break;
        plane = &g_worldDpvsPlanes->planes[planeIndex];
        side = BoxOnPlaneSide(
                         mins2,
                         maxs2,
                         plane);
        if ( side == 3 )
        {
            type = plane->type;
            rightNode = (mnode_t *)((char *)node + 2 * node->rightChildOffset);
            if ( type >= 3 )
            {
                leftNode = node + 1;
                if ( R_BoundsInCell_r(node + 1, findCellIndex, mins2, maxs2) )
                    return 1;
            }
            else
            {
                dist = plane->dist;
                localmins[0] = mins2[0];
                localmins[1] = mins2[1];
                localmins[2] = mins2[2];
                localmins[type] = dist;
                localmaxs[0] = maxs2[0];
                localmaxs[1] = maxs2[1];
                localmaxs[2] = maxs2[2];
                localmaxs[type] = dist;
                if ( BoxOnPlaneSide(
                             localmins,
                             maxs2,
                             plane) != 1
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_entity.cpp",
                                201,
                                0,
                                "%s",
                                "BoxOnPlaneSide( localmins, maxs2, plane ) == BOXSIDE_FRONT") )
                {
                    __debugbreak();
                }
                if ( maxs2[type] > dist )
                {
                    leftNode = node + 1;
                    if ( R_BoundsInCell_r(node + 1, findCellIndex, localmins, maxs2) )
                        return 1;
                }
                maxs2[0] = localmaxs[0];
                maxs2[1] = localmaxs[1];
                maxs2[2] = localmaxs[2];
            }
            node = rightNode;
        }
        else
        {
            if ( side != 1
                && side != 2
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_entity.cpp",
                            294,
                            0,
                            "%s\n\t(side) = %i",
                            "((side == (1 << 0)) || (side == (1 << 1)))",
                            side) )
            {
                __debugbreak();
            }
            node = (mnode_t *)((char *)node + 2 * (side - 1) * (node->rightChildOffset - 2) + 4);
        }
    }
    return cellIndex && cellIndex - 1 == findCellIndex;
}

void __cdecl R_ForceAddEntitySurfacesInFrustumCmd(GfxSceneEntity *sceneEnt, unsigned __int8 *entVisData)
{
    DObjAnimMat *boneMatrix; // [esp+0h] [ebp-Ch]
    const DObj *obj; // [esp+4h] [ebp-8h] BYREF
    GfxSceneEntity *localSceneEnt; // [esp+8h] [ebp-4h] BYREF
    int savedregs; // [esp+Ch] [ebp+0h] BYREF

    boneMatrix = R_UpdateSceneEntBounds(sceneEnt, &localSceneEnt, &obj, 1);
    if ( boneMatrix )
    {
        if ( !localSceneEnt
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_entity.cpp",
                        506,
                        0,
                        "%s",
                        "localSceneEnt") )
        {
            __debugbreak();
        }
        CG_CullIn(localSceneEnt->info.pose);
        R_SkinSceneDObj(sceneEnt, localSceneEnt, obj, boneMatrix, 0);
        if ( localSceneEnt->entnum == gfxCfg.entnumNone
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_entity.cpp",
                        524,
                        0,
                        "%s",
                        "localSceneEnt->entnum != gfxCfg.entnumNone") )
        {
            __debugbreak();
        }
        entVisData[localSceneEnt->entnum] = 1;
    }
    else if ( localSceneEnt )
    {
        CG_UsedDObjCalcPose(localSceneEnt->info.pose);
    }
}

