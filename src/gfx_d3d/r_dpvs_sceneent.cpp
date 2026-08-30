#include "r_dpvs_sceneent.h"
#include <universal/com_workercmds.h>
#include "r_dpvs_entity.h"
#include "r_scene.h"
#ifdef KISAK_NX
// This file carries a duplicate of r_dpvs.cpp's R_CullSphereDpvs. The header
// declares it extern, which overrides the `static` here on GCC; rename the
// local copy (and this file's calls to it).
#define R_CullSphereDpvs R_CullSphereDpvs_sceneent_local
#endif

#if 0
void    R_AddCellSceneEntSurfacesInFrustumCmd(GfxWorldDpvsPlanes *data)
{
    bool v2; // zf
    DWORD v3; // eax
    DWORD v4; // eax
    int v5; // [esp+18h] [ebp-C0h]
    const DpvsPlane *v6; // [esp+20h] [ebp-B8h]
    signed int v7; // [esp+24h] [ebp-B4h]
    const float *v8; // [esp+28h] [ebp-B0h]
    int v9; // [esp+30h] [ebp-A8h]
    const DpvsPlane *bmodel; // [esp+38h] [ebp-A0h]
    int v11; // [esp+3Ch] [ebp-9Ch]
    GfxSceneEntity *v12; // [esp+40h] [ebp-98h]
    GfxSceneEntity *sceneEnt; // [esp+4Ch] [ebp-8Ch]
    GfxSceneEntity *sceneEnta; // [esp+4Ch] [ebp-8Ch]
    GfxEntCellRefInfo *info; // [esp+54h] [ebp-84h]
    GfxEntCellRefInfo *infoa; // [esp+54h] [ebp-84h]
    unsigned int sceneEntIndex; // [esp+58h] [ebp-80h]
    unsigned int sceneEntIndexa; // [esp+58h] [ebp-80h]
    unsigned int bit; // [esp+5Ch] [ebp-7Ch]
    unsigned int bita; // [esp+5Ch] [ebp-7Ch]
    unsigned int entnum; // [esp+60h] [ebp-78h]
    unsigned int entnuma; // [esp+60h] [ebp-78h]
    signed int indexLow; // [esp+64h] [ebp-74h]
    const DpvsPlane *bits; // [esp+68h] [ebp-70h]
    unsigned int wordIndex; // [esp+6Ch] [ebp-6Ch]
    unsigned int wordIndexa; // [esp+6Ch] [ebp-6Ch]
    int innerPlaneCount; // [esp+70h] [ebp-68h]
    unsigned int *entCellBits; // [esp+78h] [ebp-60h]
    unsigned int *entCellBitsa; // [esp+78h] [ebp-60h]
    const DpvsPlane *planes; // [esp+7Ch] [ebp-5Ch] BYREF
    unsigned int *entCellBitsEA; // [esp+80h] [ebp-58h]
    unsigned int offset; // [esp+84h] [ebp-54h]
    DpvsEntityCmd dpvsEntity; // [esp+88h] [ebp-50h]
    int nodes_high; // [esp+98h] [ebp-40h]
    int v35; // [esp+9Ch] [ebp-3Ch]
    unsigned int *cellCount; // [esp+A0h] [ebp-38h]
    int frustumPlaneCount; // [esp+A4h] [ebp-34h]
    int planeCount; // [esp+A8h] [ebp-30h]
    const DpvsPlane *planesEA; // [esp+ACh] [ebp-2Ch]
    unsigned int cellIndex; // [esp+B0h] [ebp-28h]
    unsigned int viewIndex; // [esp+B4h] [ebp-24h]
    unsigned __int16 *sceneDObjIndex; // [esp+B8h] [ebp-20h]
    unsigned __int16 *sceneXModelIndex; // [esp+BCh] [ebp-1Ch]
    GfxEntCellRefInfo *entInfo; // [esp+C0h] [ebp-18h]
    unsigned int wordCount; // [esp+C4h] [ebp-14h]
    GfxWorldDpvsPlanes *worldDpvsPlanes; // [esp+C8h] [ebp-10h]
    unsigned int localClientNum; // [esp+CCh] [ebp-Ch]
    GfxSceneDpvs *sceneDpvs; // [esp+D0h] [ebp-8h]
    GfxSceneDpvs *retaddr; // [esp+D8h] [ebp+0h]

    localClientNum = a1;
    sceneDpvs = retaddr;
    worldDpvsPlanes = data;
    wordCount = (unsigned int)&scene.dpvs;
    entInfo = (GfxEntCellRefInfo *)scene.dpvs.localClientNum;
    sceneXModelIndex = (unsigned __int16 *)&rgp.world->dpvsPlanes;
    sceneDObjIndex = (unsigned __int16 *)(gfxCfg.entCount >> 5);
    if ( scene.dpvs.localClientNum >= gfxCfg.maxClientViews
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_sceneent.cpp",
                    213,
                    0,
                    "localClientNum doesn't index gfxCfg.maxClientViews\n\t%i not in [0, %i)",
                    entInfo,
                    gfxCfg.maxClientViews) )
    {
        __debugbreak();
    }
    viewIndex = scene.dynSModelVisBitsCamera[(unsigned int)entInfo - 4];
    cellIndex = (unsigned int)scene.dpvs.sceneXModelIndex;
    planesEA = (const DpvsPlane *)scene.dpvs.sceneDObjIndex;
    planeCount = LOWORD(worldDpvsPlanes->nodes);
    frustumPlaneCount = (int)worldDpvsPlanes->planes;
    cellCount = (unsigned int *)worldDpvsPlanes->cellCount;
    v35 = BYTE2(worldDpvsPlanes->nodes);
    nodes_high = HIBYTE(worldDpvsPlanes->nodes);
    if ( (unsigned int)frustumPlaneCount >= *(unsigned int *)sceneXModelIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_sceneent.cpp",
                    225,
                    0,
                    "cellIndex doesn't index worldDpvsPlanes->cellCount\n\t%i not in [0, %i)",
                    frustumPlaneCount,
                    *(unsigned int *)sceneXModelIndex) )
    {
        __debugbreak();
    }
    dpvsEntity.sceneEnt = *(GfxSceneEntity **)(wordCount + 4 * planeCount + 4);
    entCellBitsEA = cellCount;
    LOWORD(offset) = v35;
    HIWORD(offset) = frustumPlaneCount;
    entCellBits = (unsigned int *)((unsigned int)sceneDObjIndex * (unsigned int)entInfo);
    if ( (unsigned int)((unsigned int)sceneDObjIndex * (unsigned int)entInfo) >= 0x100
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_sceneent.cpp",
                    234,
                    0,
                    "offset doesn't index MAX_TOTAL_ENT_COUNT >> 5\n\t%i not in [0, %i)",
                    entCellBits,
                    256) )
    {
        __debugbreak();
    }
    entCellBitsa = &entCellBits[64 * frustumPlaneCount];
    innerPlaneCount = (int)cellCount;
    wordIndex = *((unsigned int *)sceneXModelIndex + 3) + 4 * (unsigned int)entCellBitsa;
    if ( nodes_high > v35
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_sceneent.cpp",
                    261,
                    0,
                    "%s",
                    "frustumPlaneCount <= planeCount") )
    {
        __debugbreak();
    }
    bits = (const DpvsPlane *)&cellCount[5 * nodes_high];
    indexLow = v35 - nodes_high;
    for ( entnum = 0; entnum < (unsigned int)sceneDObjIndex; ++entnum )
    {
        bit = *(unsigned int *)(wordIndex + 4 * entnum);
        while ( 1 )
        {
            v2 = !_BitScanReverse(&v3, bit);
            if (v2)
                v3 = 63;

            sceneEntIndex = v3 ^ 0x1F;

            if ((v3 ^ 31) >= 32)
                break;

            info = (GfxEntCellRefInfo *)(sceneEntIndex + 32 * entnum);
            if ( ((0x80000000 >> sceneEntIndex) & bit) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_sceneent.cpp",
                            278,
                            0,
                            "%s",
                            "bits & bit") )
            {
                __debugbreak();
            }
            bit &= ~(0x80000000 >> sceneEntIndex);
            if ( !*((_BYTE *)&info->radius + (unsigned int)dpvsEntity.sceneEnt) )
            {
                sceneEnt = (GfxSceneEntity *)*((unsigned __int16 *)planesEA->coeffs + (unsigned int)info);
                if ( sceneEnt == (GfxSceneEntity *)0xFFFF )
                {
                    sceneEnta = (GfxSceneEntity *)*(unsigned __int16 *)(cellIndex + 2 * (unsigned int)info);
                    if ( sceneEnta != (GfxSceneEntity *)0xFFFF
                        && !R_CullSphereDpvs(
                                    scene.sceneModel[(unsigned int)sceneEnta].placement.base.origin,
                                    *(float *)(viewIndex + 4 * (unsigned int)info),
                                    bits,
                                    indexLow) )
                    {
                        *((_BYTE *)&info->radius + (unsigned int)dpvsEntity.sceneEnt) = 1;
                    }
                }
                else
                {
                    v12 = &scene.sceneDObj[(unsigned int)sceneEnt];
                    if ( !R_CullSphereDpvs(v12->placement.base.origin, *(float *)(viewIndex + 4 * (unsigned int)info), bits, indexLow) )
                    {
                        if ( v12->cull.state < 2 )
                            goto LABEL_36;
                        v11 = 0;
                        bmodel = (const DpvsPlane *)innerPlaneCount;
                        while ( v11 < v35 )
                        {
                            if ( R_DpvsPlaneMaxSignedDistToBox(bmodel, v12->cull.mins) <= 0.0 )
                            {
                                v9 = 1;
                                goto LABEL_35;
                            }
                            ++v11;
                            ++bmodel;
                        }
                        v9 = 0;
LABEL_35:
                        if ( !v9 )
                        {
LABEL_36:
                            planes = (const DpvsPlane *)&scene.sceneDObj[(unsigned int)sceneEnt];
                            if ( v12->cull.state < 2 )
                                Sys_AddWorkerCmdInternal(&r_dpvs_entityWorkerCmd, (unsigned char*)&planes, 0);
                            else
                                R_AddEntitySurfacesInFrustumCmd((unsigned short*)&planes);
                        }
                    }
                }
            }
        }
    }
    wordIndexa = *((unsigned int *)sceneXModelIndex + 3) + 4 * (unsigned int)entCellBitsa + (*(unsigned int *)sceneXModelIndex << 10);
    for ( entnuma = 0; entnuma < (unsigned int)sceneDObjIndex; ++entnuma )
    {
        bita = *(unsigned int *)(wordIndexa + 4 * entnuma);
        while ( 1 )
        {
            v2 = !_BitScanReverse(&v4, bita);
            if (v2)
                v4 = 63;
            indexLow = v4 ^ 31;
            if ((v4 ^ 31) >= 32)
                break;
            infoa = (GfxEntCellRefInfo *)(sceneEntIndexa + 32 * entnuma);
            if ( ((0x80000000 >> sceneEntIndexa) & bita) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_sceneent.cpp",
                            382,
                            0,
                            "%s",
                            "bits & bit") )
            {
                __debugbreak();
            }
            bita &= ~(0x80000000 >> sceneEntIndexa);
            if ( !*((_BYTE *)&infoa->radius + (unsigned int)dpvsEntity.sceneEnt) )
            {
                v8 = *(const float **)(viewIndex + 4 * (unsigned int)infoa);
                v7 = 0;
                v6 = bits;
                while ( v7 < indexLow )
                {
                    if ( R_DpvsPlaneMaxSignedDistToBox(v6, v8) <= 0.0 )
                    {
                        v5 = 1;
                        goto LABEL_56;
                    }
                    ++v7;
                    ++v6;
                }
                v5 = 0;
LABEL_56:
                if ( !v5 )
                    *((_BYTE *)&infoa->radius + (unsigned int)dpvsEntity.sceneEnt) = 1;
            }
        }
    }
}

#endif


static int __cdecl R_CullSphereDpvs(const float *origin, float radius, const DpvsPlane *planes, int planeCount)
{
    int planeIndex; // [esp+8h] [ebp-4h]

    for (planeIndex = 0; planeIndex < planeCount; ++planeIndex)
    {
        if ((float)((float)((float)((float)((float)(planes->coeffs[0] * *origin) + (float)(planes->coeffs[1] * origin[1]))
            + (float)(planes->coeffs[2] * origin[2]))
            + planes->coeffs[3])
            + radius) <= 0.0)
            return 1;
        ++planes;
    }
    return 0;
}

void R_AddCellSceneEntSurfacesInFrustumCmd(GfxWorldDpvsPlanes *data)
{
    bool v2; // zf
    DWORD v3; // eax
    DWORD v4; // eax
    int v5; // [esp+18h] [ebp-C0h]
    const DpvsPlane *v6; // [esp+20h] [ebp-B8h]
    int v7; // [esp+24h] [ebp-B4h]
    const float *mins; // [esp+28h] [ebp-B0h]
    int v9; // [esp+30h] [ebp-A8h]
    const DpvsPlane *v10; // [esp+38h] [ebp-A0h]
    int v11; // [esp+3Ch] [ebp-9Ch]
    GfxSceneEntity *sceneEnt; // [esp+40h] [ebp-98h]
    unsigned int sceneEntIndex; // [esp+4Ch] [ebp-8Ch]
    unsigned int sceneEntIndexa; // [esp+4Ch] [ebp-8Ch]
    unsigned int entnum; // [esp+54h] [ebp-84h]
    unsigned int entnuma; // [esp+54h] [ebp-84h]
    unsigned int indexLow; // [esp+58h] [ebp-80h]
    unsigned int indexLowa; // [esp+58h] [ebp-80h]
    unsigned int bits; // [esp+5Ch] [ebp-7Ch]
    unsigned int bitsa; // [esp+5Ch] [ebp-7Ch]
    unsigned int wordIndex; // [esp+60h] [ebp-78h]
    unsigned int wordIndexa; // [esp+60h] [ebp-78h]
    int innerPlaneCount; // [esp+64h] [ebp-74h]
    const DpvsPlane *innerPlanes; // [esp+68h] [ebp-70h]
    unsigned int *entCellBits; // [esp+6Ch] [ebp-6Ch]
    unsigned int *entCellBitsa; // [esp+6Ch] [ebp-6Ch]
    const DpvsPlane *planes; // [esp+70h] [ebp-68h]
    unsigned int offset; // [esp+78h] [ebp-60h]
    unsigned int offseta; // [esp+78h] [ebp-60h]
    DpvsEntityCmd dpvsEntity; // [esp+7Ch] [ebp-5Ch] BYREF
    int frustumPlaneCount; // [esp+98h] [ebp-40h]
    int planeCount; // [esp+9Ch] [ebp-3Ch]
    const DpvsPlane *planesEA; // [esp+A0h] [ebp-38h]
    unsigned int cellIndex; // [esp+A4h] [ebp-34h]
    unsigned int viewIndex; // [esp+A8h] [ebp-30h]
    unsigned __int16 *sceneDObjIndex; // [esp+ACh] [ebp-2Ch]
    unsigned __int16 *sceneXModelIndex; // [esp+B0h] [ebp-28h]
    GfxEntCellRefInfo *entInfo; // [esp+B4h] [ebp-24h]
    unsigned int wordCount; // [esp+B8h] [ebp-20h]
    GfxWorldDpvsPlanes *worldDpvsPlanes; // [esp+BCh] [ebp-1Ch]
    unsigned int localClientNum; // [esp+C0h] [ebp-18h]
    GfxSceneDpvs *sceneDpvs; // [esp+C4h] [ebp-14h]
    DpvsDynamicCellCmd *dpvsCell; // [esp+C8h] [ebp-10h]
    //_UNKNOWN *v44; // [esp+CCh] [ebp-Ch]
    //void *dataa; // [esp+D0h] [ebp-8h]
    //int v46; // [esp+D4h] [ebp-4h] BYREF
    //int vars0; // [esp+D8h] [ebp+0h]
    //
    //v44 = a1;
    //dataa = (void *)vars0;
    dpvsCell = (DpvsDynamicCellCmd *)data;
    sceneDpvs = &scene.dpvs;
    localClientNum = scene.dpvs.localClientNum;
    worldDpvsPlanes = &rgp.world->dpvsPlanes;
    wordCount = gfxCfg.entCount >> 5;
    if (scene.dpvs.localClientNum >= gfxCfg.maxClientViews
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_sceneent.cpp",
            213,
            0,
            "localClientNum doesn't index gfxCfg.maxClientViews\n\t%i not in [0, %i)",
            localClientNum,
            gfxCfg.maxClientViews))
    {
        __debugbreak();
    }
    entInfo = scene.dpvs.entInfo[localClientNum];
    sceneXModelIndex = scene.dpvs.sceneXModelIndex;
    sceneDObjIndex = scene.dpvs.sceneDObjIndex;
    viewIndex = dpvsCell->viewIndex;
    cellIndex = dpvsCell->cellIndex;
    planesEA = dpvsCell->planes;
    planeCount = dpvsCell->planeCount;
    frustumPlaneCount = dpvsCell->frustumPlaneCount;
    if (cellIndex >= worldDpvsPlanes->cellCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_sceneent.cpp",
            225,
            0,
            "cellIndex doesn't index worldDpvsPlanes->cellCount\n\t%i not in [0, %i)",
            cellIndex,
            worldDpvsPlanes->cellCount))
    {
        __debugbreak();
    }
    dpvsEntity.entVisData = sceneDpvs->entVisData[viewIndex];
    dpvsEntity.planes = planesEA;
    dpvsEntity.planeCount = planeCount;
    dpvsEntity.cellIndex = cellIndex;
    offset = wordCount * localClientNum;
    if (wordCount * localClientNum >= 0x100
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_sceneent.cpp",
            234,
            0,
            "offset doesn't index MAX_TOTAL_ENT_COUNT >> 5\n\t%i not in [0, %i)",
            offset,
            256))
    {
        __debugbreak();
    }
    offseta = offset + (cellIndex << 8);
    planes = planesEA;
    entCellBits = &worldDpvsPlanes->sceneEntCellBits[offseta];
    if (frustumPlaneCount > planeCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_sceneent.cpp",
            261,
            0,
            "%s",
            "frustumPlaneCount <= planeCount"))
    {
        __debugbreak();
    }
    innerPlanes = &planesEA[frustumPlaneCount];
    innerPlaneCount = planeCount - frustumPlaneCount;
    for (wordIndex = 0; wordIndex < wordCount; ++wordIndex)
    {
        bits = entCellBits[wordIndex];
        while (1)
        {
            v2 = !_BitScanReverse(&v3, bits);
            if (v2)
                v3 = 63;
            indexLow = v3 ^ 0x1F;
            if ((v3 ^ 0x1Fu) >= 0x20)
                break;
            entnum = indexLow + 32 * wordIndex;
            if (((0x80000000 >> indexLow) & bits) == 0
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_sceneent.cpp",
                    278,
                    0,
                    "%s",
                    "bits & bit"))
            {
                __debugbreak();
            }
            bits &= ~(0x80000000 >> indexLow);
            if (!dpvsEntity.entVisData[entnum])
            {
                sceneEntIndex = sceneDObjIndex[entnum];
                if (sceneEntIndex == 0xFFFF)
                {
                    sceneEntIndexa = sceneXModelIndex[entnum];
                    if (sceneEntIndexa != 0xFFFF
                        && !R_CullSphereDpvs(
                            scene.sceneModel[sceneEntIndexa].placement.base.origin,
                            entInfo[entnum].radius,
                            innerPlanes,
                            innerPlaneCount))
                    {
                        dpvsEntity.entVisData[entnum] = 1;
                    }
                }
                else
                {
                    sceneEnt = &scene.sceneDObj[sceneEntIndex];
                    if (!R_CullSphereDpvs(sceneEnt->placement.base.origin, entInfo[entnum].radius, innerPlanes, innerPlaneCount))
                    {
                        if (sceneEnt->cull.state < 2)
                            goto LABEL_36;
                        v11 = 0;
                        v10 = planes;
                        while (v11 < planeCount)
                        {
                            if (R_DpvsPlaneMaxSignedDistToBox(v10, sceneEnt->cull.mins) <= 0.0)
                            {
                                v9 = 1;
                                goto LABEL_35;
                            }
                            ++v11;
                            ++v10;
                        }
                        v9 = 0;
                    LABEL_35:
                        if (!v9)
                        {
                        LABEL_36:
                            dpvsEntity.sceneEnt = &scene.sceneDObj[sceneEntIndex];
                            if (sceneEnt->cull.state < 2)
                                Sys_AddWorkerCmdInternal(&r_dpvs_entityWorkerCmd, (unsigned __int8 *)&dpvsEntity, 0);
                            else
                                R_AddEntitySurfacesInFrustumCmd((unsigned __int16 *)&dpvsEntity);
                        }
                    }
                }
            }
        }
    }
    entCellBitsa = &worldDpvsPlanes->sceneEntCellBits[256 * worldDpvsPlanes->cellCount + offseta];
    for (wordIndexa = 0; wordIndexa < wordCount; ++wordIndexa)
    {
        bitsa = entCellBitsa[wordIndexa];
        while (1)
        {
            v2 = !_BitScanReverse(&v4, bitsa);
            if (v2)
                v4 = 63;
            indexLowa = v4 ^ 0x1F;
            if ((v4 ^ 0x1Fu) >= 0x20)
                break;
            entnuma = indexLowa + 32 * wordIndexa;
            if (((0x80000000 >> indexLowa) & bitsa) == 0
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_sceneent.cpp",
                    382,
                    0,
                    "%s",
                    "bits & bit"))
            {
                __debugbreak();
            }
            bitsa &= ~(0x80000000 >> indexLowa);
            if (!dpvsEntity.entVisData[entnuma])
            {
                mins = entInfo[entnuma].bmodel->writable.mins;
                v7 = 0;
                v6 = innerPlanes;
                while (v7 < innerPlaneCount)
                {
                    if (R_DpvsPlaneMaxSignedDistToBox(v6, mins) <= 0.0)
                    {
                        v5 = 1;
                        goto LABEL_56;
                    }
                    ++v7;
                    ++v6;
                }
                v5 = 0;
            LABEL_56:
                if (!v5)
                    dpvsEntity.entVisData[entnuma] = 1;
            }
        }
    }
}