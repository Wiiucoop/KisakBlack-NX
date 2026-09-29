#include "r_dpvs_dynmodel.h"
#include "r_dvars.h"
#include "r_init.h"

#include <DynEntity/DynEntity_load_obj.h>
#include "r_dpvs.h"

void __cdecl R_AddCellDynModelSurfacesInFrustumCmd(const DpvsPlane **data)
{
    unsigned int planeCount; // [esp+0h] [ebp-28h]
    const DpvsPlane *planes; // [esp+4h] [ebp-24h]
    unsigned int dynEntClientWordCount; // [esp+8h] [ebp-20h]
    unsigned __int8 *dynEntVisData; // [esp+10h] [ebp-18h]
    unsigned int *dynEntCellBits; // [esp+14h] [ebp-14h]
    DynEntityPose *dynModelList; // [esp+18h] [ebp-10h]
    GfxWorldDpvsDynamic *worldDpvsDyn; // [esp+24h] [ebp-4h]

    if ( r_drawDynEnts->current.enabled )
    {
        worldDpvsDyn = &rgp.world->dpvsDyn;
        const DpvsDynamicCellCmd *cmd = (const DpvsDynamicCellCmd *)data;   // nx-port: was read at x86 offsets
        if ( cmd->cellIndex >= rgp.world->dpvsPlanes.cellCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_dynmodel.cpp",
                        183,
                        0,
                        "dpvsCell->cellIndex doesn't index rgp.world->dpvsPlanes.cellCount\n\t%i not in [0, %i)",
                        cmd->cellIndex,
                        rgp.world->dpvsPlanes.cellCount) )
        {
            __debugbreak();
        }
        dynEntClientWordCount = worldDpvsDyn->dynEntClientWordCount[0];
        planeCount = cmd->planeCount;
        dynEntCellBits = &worldDpvsDyn->dynEntCellBits[0][worldDpvsDyn->dynEntClientWordCount[0] * cmd->cellIndex];
        dynEntVisData = worldDpvsDyn->dynEntVisData[0][cmd->viewIndex];
        planes = cmd->planes;
        dynModelList = DynEnt_GetClientModelPoseList();
        R_CullDynModelInCell(dynEntCellBits, dynEntClientWordCount, dynModelList, planes, planeCount, dynEntVisData);
    }
}

void __cdecl R_CullDynModelInCell(
                const unsigned int *dynEntCellBits,
                unsigned int dynEntClientWordCount,
                DynEntityPose *dynModelList,
                const DpvsPlane *planes,
                int planeCount,
                unsigned __int8 *dynEntVisData)
{
    DWORD v7; // eax
    unsigned int dynEntIndex; // [esp+1Ch] [ebp-10h]
    unsigned int bits; // [esp+20h] [ebp-Ch]
    unsigned int indexLow; // [esp+24h] [ebp-8h]
    unsigned int wordIndex; // [esp+28h] [ebp-4h]

    for ( wordIndex = 0; wordIndex < dynEntClientWordCount; ++wordIndex )
    {
        bits = dynEntCellBits[wordIndex];
        while ( 1 )
        {
            if (!_BitScanReverse(&v7, bits))
                v7 = 63;// `CountLeadingZeros'::`2': : notFound;
            indexLow = v7 ^ 0x1F;
            if ((v7 ^ 0x1Fu) >= 0x20)
                break;
            dynEntIndex = indexLow + 32 * wordIndex;
            if ( ((0x80000000 >> indexLow) & bits) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dpvs_dynmodel.cpp",
                            101,
                            0,
                            "%s",
                            "bits & bit") )
            {
                __debugbreak();
            }
            bits &= ~(0x80000000 >> indexLow);
            if ( !dynEntVisData[dynEntIndex]
                && !R_CullSphereDpvs(
                            dynModelList[dynEntIndex].pose.origin,
                            dynModelList[dynEntIndex].radius,
                            planes,
                            planeCount) )
            {
                dynEntVisData[dynEntIndex] = 1;
            }
        }
    }
}

