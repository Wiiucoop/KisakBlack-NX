#include "r_add_staticmodel.h"
#include "r_gfx.h"
#include <xanim/xanim.h>
#include "r_rendercmds.h"
#include <xanim/xmodel.h>
#include "r_model_lighting.h"
#include "r_dvars.h"
#include <xanim/xmodel_utils.h>
#include "r_staticmodelcache.h"
#include "r_add_bsp.h"
#include "r_primarylights.h"
#include <physics/rope.h>
#include <EffectsCore/fx_beam.h>
#include "r_debug.h"
#include "r_warn.h"
#include "r_add_cmdbuf.h"
#include "r_foliage.h"
#include "r_draw_method.h"
#include <qcommon/threads.h>
#include "r_light.h"
#include "r_pretess.h"
#include "r_xsurface.h"
#include <universal/com_files.h>
#include "r_drawsurf.h"
#include "rb_logfile.h"

GfxLodParms *g_lodParms;

int g_dumpStaticModelFileHandle;
int g_dumpStaticModelCount;
char g_dumpStaticModelFilePath[20] = "staticModelInfo.csv";



void __cdecl R_AddDelayedStaticModelDrawSurf(
                GfxDelayedCmdBuf *delayedCmdBuf,
                XSurface *xsurf,
                unsigned __int8 *list,
                unsigned int count,
                unsigned int which_lod)
{
    if ( count >= 0x10000000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                    278,
                    0,
                    "%s",
                    "count<(uint)1<<PACKED_LOD_DATA_SHIFT") )
    {
        __debugbreak();
    }
    R_WritePrimDrawSurfInt(delayedCmdBuf, count | (which_lod << 28));
    R_WritePrimDrawSurfPtr(delayedCmdBuf, xsurf);
    R_WritePrimDrawSurfData(delayedCmdBuf, list, (count + 1) >> 1);
}

void __cdecl R_WritePrimDrawSurfInt(GfxDelayedCmdBuf *delayedCmdBuf, unsigned int value)
{
    if ( !delayedCmdBuf->primDrawSurfSize
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_cmdbuf.h",
                    172,
                    0,
                    "%s",
                    "delayedCmdBuf->primDrawSurfSize") )
    {
        __debugbreak();
    }
    if ( delayedCmdBuf->primDrawSurfPos < 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_cmdbuf.h",
                    173,
                    0,
                    "%s\n\t(delayedCmdBuf->primDrawSurfPos) = %i",
                    "(delayedCmdBuf->primDrawSurfPos >= 0)",
                    delayedCmdBuf->primDrawSurfPos) )
    {
        __debugbreak();
    }
    --delayedCmdBuf->primDrawSurfSize;
    frontEndDataOut->primDrawSurfsBuf[delayedCmdBuf->primDrawSurfPos++] = value;
}

void __cdecl R_WritePrimDrawSurfData(GfxDelayedCmdBuf *delayedCmdBuf, unsigned __int8 *data, unsigned int count)
{
    if ( delayedCmdBuf->primDrawSurfSize < count
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_cmdbuf.h",
                    189,
                    0,
                    "%s\n\t(delayedCmdBuf->primDrawSurfSize) = %i",
                    "(delayedCmdBuf->primDrawSurfSize >= count)",
                    delayedCmdBuf->primDrawSurfSize) )
    {
        __debugbreak();
    }
    if ( delayedCmdBuf->primDrawSurfPos < 0
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_cmdbuf.h",
                    190,
                    0,
                    "%s",
                    "delayedCmdBuf->primDrawSurfPos >= 0") )
    {
        __debugbreak();
    }
    delayedCmdBuf->primDrawSurfSize -= count;
    memcpy((unsigned __int8 *)&frontEndDataOut->primDrawSurfsBuf[delayedCmdBuf->primDrawSurfPos], data, 4 * count);
    delayedCmdBuf->primDrawSurfPos += count;
}

void __cdecl R_WritePrimDrawSurfPtr(GfxDelayedCmdBuf *delayedCmdBuf, const void *ptr)
{
    unsigned int words[R_PRIM_PTR_WORDS];
    memcpy(words, &ptr, sizeof(ptr));
    R_WritePrimDrawSurfData(delayedCmdBuf, (unsigned __int8 *)words, R_PRIM_PTR_WORDS);
}

GfxStaticModelId __cdecl R_GetStaticModelId(unsigned int smodelIndex, int lod)
{
    Material **Skins; // eax
    const MaterialTechnique *technique; // [esp+10h] [ebp-24h]
    const XModelLodInfo *lodInfo; // [esp+14h] [ebp-20h]
    XModel *model; // [esp+1Ch] [ebp-18h]
    int surfaceIndex; // [esp+20h] [ebp-14h]
    int surfaceCount; // [esp+24h] [ebp-10h]
    GfxStaticModelId staticModelId; // [esp+28h] [ebp-Ch]
    GfxStaticModelId staticModelIda; // [esp+28h] [ebp-Ch]
    XSurface *surfaces; // [esp+2Ch] [ebp-8h] BYREF
    XSurface *xsurf; // [esp+30h] [ebp-4h]

    model = rgp.world->dpvs.smodelDrawInsts[smodelIndex].model;
    if ( !R_StaticModelHasLighting(smodelIndex)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                    740,
                    0,
                    "%s",
                    "R_StaticModelHasLighting( smodelIndex )") )
    {
        __debugbreak();
    }
    if ( !r_smc_enable->current.enabled )
        goto LABEL_15;
    if ( r_gfxopt_dynamic_foliage->current.enabled )
        goto LABEL_15;
    Skins = XModelGetSkins(model, lod);
    technique = Material_GetTechnique(*Skins, 0xAu);
    if ( r_gfxopt_dynamic_foliage->current.enabled && (!technique || (technique->flags & 0x20) != 0) )
        goto LABEL_15;
    lodInfo = XModelGetLodInfo(model, lod);
    if ( !lodInfo->smcIndexPlusOne )
        goto LABEL_15;
    if ( smodelIndex >= rgp.world->dpvs.smodelCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                    760,
                    0,
                    "smodelIndex doesn't index rgp.world->dpvs.smodelCount\n\t%i not in [0, %i)",
                    smodelIndex,
                    rgp.world->dpvs.smodelCount) )
    {
        __debugbreak();
    }
    staticModelId.objectId = R_CacheStaticModelSurface(lodInfo->smcIndexPlusOne - 1, smodelIndex, lodInfo);
    if ( staticModelId.objectId )
    {
        staticModelId.surfType = 4;
        return staticModelId;
    }
    else
    {
LABEL_15:
        staticModelIda.surfType = 2;
        staticModelIda.objectId = smodelIndex;
        surfaceCount = XModelGetSurfaces(model, &surfaces, lod);
        if ( !surfaceCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                        777,
                        0,
                        "%s",
                        "surfaceCount") )
        {
            __debugbreak();
        }
        for ( surfaceIndex = 0; surfaceIndex < surfaceCount; ++surfaceIndex )
        {
            xsurf = &surfaces[surfaceIndex];
            if ( (xsurf->flags & 0x80) != 0 || !useFastFile->current.enabled )
            {
                staticModelIda.surfType = 5;
                return staticModelIda;
            }
        }
        return staticModelIda;
    }
}

void __cdecl R_AddAllStaticModelSurfacesCamera(
                int viewIndex,
                int teleported,
                const GfxLight *visibleLights,
                int visibleLightCount,
                bool noLodCullOut)
{
    double v5; // st7
    double v6; // st7
    double v7; // st7
    char *Name; // eax
    double v9; // st7
    float adjustedDist; // [esp+0h] [ebp-1184h]
    float adjustedDista; // [esp+0h] [ebp-1184h]
    float val; // [esp+4h] [ebp-1180h]
    float vala; // [esp+4h] [ebp-1180h]
    bool dist; // [esp+8h] [ebp-117Ch]
    float v15; // [esp+Ch] [ebp-1178h]
    unsigned int dynamicSModelCount; // [esp+28h] [ebp-115Ch]
    const float *v17[2]; // [esp+64h] [ebp-1120h]  nx-port: the box corners, held as pointers
    float v18; // [esp+6Ch] [ebp-1118h]
    float v19; // [esp+70h] [ebp-1114h]
    float v20; // [esp+74h] [ebp-1110h]
    const GfxLight *v21; // [esp+78h] [ebp-110Ch]
    int i; // [esp+7Ch] [ebp-1108h]
    float v23; // [esp+80h] [ebp-1104h]
    char v24; // [esp+87h] [ebp-10FDh]
    GfxStaticModelId StaticModelId; // [esp+88h] [ebp-10FCh]
    char v26; // [esp+8Fh] [ebp-10F5h]
    unsigned int v27; // [esp+90h] [ebp-10F4h]
    unsigned int v28; // [esp+94h] [ebp-10F0h]
    GfxStaticModelDrawInst *inst; // [esp+98h] [ebp-10ECh]
    float scale; // [esp+9Ch] [ebp-10E8h]
    float bias; // [esp+A0h] [ebp-10E4h]
    unsigned __int16 staticModelLodList[4][4][128]; // [esp+A4h] [ebp-10E0h] BYREF
    bool prepassSizeCull; // [esp+10ABh] [ebp-D9h]
    GfxStaticModelDrawInst *smodelDrawInsts; // [esp+10ACh] [ebp-D8h]
    float v35; // [esp+10B0h] [ebp-D4h]
    int currentProbeIndex; // [esp+10B4h] [ebp-D0h]
    XModel *model_; // [esp+10B8h] [ebp-CCh]
    unsigned __int8 primaryLightIndex; // [esp+10BFh] [ebp-C5h]
    int v39; // [esp+10C0h] [ebp-C4h]
    XModel *model; // [esp+10C4h] [ebp-C0h]
    float *mins; // [esp+10C8h] [ebp-BCh]
    int reflectionProbeIndex; // [esp+10CCh] [ebp-B8h]
    int modelFade; // [esp+10D0h] [ebp-B4h]
    bool noDynamicShadow; // [esp+10D7h] [ebp-ADh]
    unsigned __int16 *list; // [esp+10D8h] [ebp-ACh]
    void *dest; // [esp+10DCh] [ebp-A8h]
    unsigned int visLightsMask; // [esp+10E0h] [ebp-A4h]
    int lod; // [esp+10E4h] [ebp-A0h]
    unsigned int smodelCount; // [esp+10E8h] [ebp-9Ch]
    _WORD *count; // [esp+10ECh] [ebp-98h]
    bool v51; // [esp+10F0h] [ebp-94h]
    bool v52; // [esp+10F4h] [ebp-90h]
    unsigned __int8 *visData; // [esp+10F8h] [ebp-8Ch]
    GfxStaticModelInst *smodelInsts; // [esp+10FCh] [ebp-88h]
    float (*planes)[4]; // [esp+1100h] [ebp-84h]
    bool v56; // [esp+1104h] [ebp-80h]
    _BYTE v57[3]; // [esp+1108h] [ebp-7Ch] BYREF
    unsigned __int8 currentLightIndex; // [esp+110Bh] [ebp-79h]
    float origin[3]; // [esp+110Ch] [ebp-78h] BYREF
    unsigned __int16 staticModelLodCount[4][4]; // [esp+1118h] [ebp-6Ch] BYREF
    GfxSModelDrawSurfLightingData surfData; // [esp+113Ch] [ebp-48h] BYREF
    bool v62; // [esp+116Bh] [ebp-19h]
    int allocatedLighting; // [esp+116Ch] [ebp-18h]
    float v64; // [esp+1170h] [ebp-14h]
    bool v65; // [esp+1177h] [ebp-Dh]
    unsigned int entryCount; // [esp+1178h] [ebp-Ch]
    DpvsGlob_sunShadow *p_sunShadow; // [esp+117Ch] [ebp-8h]
    int smodelIndex; // [esp+1180h] [ebp-4h]

    smodelCount = g_worldDpvs->smodelCount;
    dest = rgp.world->dpvs.lodData;
    v62 = 0;
    prepassSizeCull = 0;
    Com_Memset((unsigned int *)dest, 0, 32 * ((smodelCount + 127) / 128));
    visData = rgp.world->dpvs.smodelVisData[0];
    smodelDrawInsts = rgp.world->dpvs.smodelDrawInsts;
    smodelInsts = rgp.world->dpvs.smodelInsts;
    planes = scene.dynamicSpotLightPlanes;
    g_lodParms = &rg.correctedLodParms[viewIndex];
    iassert(g_lodParms->valid);
    scale = g_lodParms->ramp[0].scale;
    bias = g_lodParms->ramp[0].bias;
    origin[0] = g_lodParms->origin[0];
    origin[1] = g_lodParms->origin[1];
    origin[2] = g_lodParms->origin[2];

    R_InitBspDrawSurf(&surfData);

    surfData.drawSurf[0].current = scene.drawSurfs[DRAW_SURF_SMODEL_CAMERA_LIT];
    surfData.drawSurf[1].current = scene.drawSurfs[DRAW_SURF_SMODEL_CAMERA_DECAL];
    surfData.drawSurf[2].current = scene.drawSurfs[DRAW_SURF_SMODEL_CAMERA_EMISSIVE];

    surfData.drawSurf[0].end = scene.drawSurfs[DRAW_SURF_SMODEL_CAMERA_LIT] + 0x2000;
    surfData.drawSurf[1].end = scene.drawSurfs[DRAW_SURF_SMODEL_CAMERA_DECAL] + 0x200;
    surfData.drawSurf[2].end = scene.drawSurfs[DRAW_SURF_SMODEL_CAMERA_EMISSIVE] + 0x2000;

    memset(staticModelLodCount, 0, sizeof(staticModelLodCount));

    allocatedLighting = 0;
    model = 0;
    currentProbeIndex = 255;
    currentLightIndex = 0;
    modelFade = 0;
    visLightsMask = 0;
    noDynamicShadow = 0;
    if ( (int)g_worldDpvs->dynamicSModelCount > 2048 )
        dynamicSModelCount = 2048;
    else
        dynamicSModelCount = g_worldDpvs->dynamicSModelCount;
    v27 = dynamicSModelCount;
    p_sunShadow = &dpvsGlob.sunShadow;
    v51 = dpvsGlob.sunShadow.viewDir[0] < 0.0;
    v52 = dpvsGlob.sunShadow.viewDir[1] < 0.0;
    v56 = dpvsGlob.sunShadow.viewDir[2] < 0.0;
    for ( smodelIndex = 0; smodelIndex < smodelCount; ++smodelIndex )
    {
        if ( visData[smodelIndex] )
        {
            inst = &smodelDrawInsts[smodelIndex];
            mins = smodelInsts[smodelIndex].mins;
            //_mm_prefetch((const char *)&inst[1], 1);
            //_mm_prefetch((const char *)mins + 40, 1);
            v24 = 0;
            v35 = Vec3Distance(inst->placement.origin, origin);
            v64 = v35;
            v35 = (float)(v35 * scale) + bias;
            lod = -2;
            model_ = inst->model;
            v39 = 0;
            primaryLightIndex = inst->primaryLightIndex;
            reflectionProbeIndex = inst->reflectionProbeIndex;
            R_ShowPrimaryLightDebugLine(inst->placement.origin, primaryLightIndex, 1);
            v28 = 0;
            for ( i = 0; i < visibleLightCount; ++i )
            {
                v21 = &visibleLights[i];
                v5 = PointToBoxDistSq(v21->origin, mins, mins + 3);
                if ( v21->radius * v21->radius >= v5 )
                {
                    if ( v21->type == 3 )
                    {
                        v28 |= 1u;
                    }
                    else if ( BoxInPlanes(planes, 6u, mins, mins + 3) )
                    {
                        v28 |= 2u;
                    }
                }
            }
            v17[0] = mins;
            v17[1] = mins + 3;
            v18 = v17[v51][0];
            v19 = v17[v52][1];
            v20 = v17[v56][2];
            v65 = (float)((float)((float)((float)(v18 * p_sunShadow->viewDir[0]) + (float)(v19 * p_sunShadow->viewDir[1]))
                                                    + (float)(v20 * p_sunShadow->viewDir[2]))
                                    - p_sunShadow->viewDirDist) > p_sunShadow->sunShadowDrawDist;
            v26 = 0;
            if ( model_ != model
                || reflectionProbeIndex != currentProbeIndex
                || primaryLightIndex != currentLightIndex
                || v28 != visLightsMask
                || v65 != noDynamicShadow
                || v39 != modelFade )
            {
                if ( allocatedLighting )
                {
                    R_SkinStaticModelsCamera(
                        model,
                        currentLightIndex,
                        modelFade,
                        staticModelLodList,
                        staticModelLodCount,
                        prepassSizeCull,
                        &surfData,
                        visLightsMask,
                        noDynamicShadow);
                    memset(staticModelLodCount, 0, sizeof(staticModelLodCount));
                    allocatedLighting = 0;
                }
                if ( model_ != model )
                    model = model_;
                currentProbeIndex = reflectionProbeIndex;
                currentLightIndex = primaryLightIndex;
                modelFade = v39;
                visLightsMask = v28;
                noDynamicShadow = v65;
            }
            if ( inst->placement.scale >= 1.0 )
                v15 = inst->placement.scale;
            else
                v15 = 1.0f;
            v23 = v15;
            v6 = I_fres(v15);
            val = v6 * v64;
            v7 = I_fres(v23);
            adjustedDist = v7 * v35;
            lod = XModelGetLodForDist(model_, adjustedDist, val, noLodCullOut);
            if ( lod < 0 || v26 )
            {
                visData[smodelIndex] = 0;
            }
            else
            {
                if ( smodelIndex < v27 )
                    scene.dynSModelVisBitsCamera[(unsigned int)smodelIndex >> 5] |= 1 << (smodelIndex & 0x1F);
                if ( lod < 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                                1354,
                                0,
                                "%s",
                                "lod >= 0") )
                {
                    __debugbreak();
                }
                if ( r_showCullSModels->current.enabled )
                {
                    R_AddDebugBox(
                        &frontEndDataOut->debugGlobals,
                        rgp.world->dpvs.smodelInsts[smodelIndex].mins,
                        rgp.world->dpvs.smodelInsts[smodelIndex].maxs,
                        colorLtGrey);
                    R_AddDebugLine(
                        &frontEndDataOut->debugGlobals,
                        rgp.world->dpvs.smodelInsts[smodelIndex].mins,
                        rgp.world->dpvs.smodelInsts[smodelIndex].lightingOrigin,
                        colorLtOrange,
                        0);
                    R_AddDebugLine(
                        &frontEndDataOut->debugGlobals,
                        rgp.world->dpvs.smodelInsts[smodelIndex].lightingOrigin,
                        rgp.world->dpvs.smodelInsts[smodelIndex].maxs,
                        colorDkRed,
                        0);
                }
                if ( r_showSModelNames->current.enabled )
                {
                    Name = (char *)XModelGetName(model_);
                    R_AddDebugString(&frontEndDataOut->debugGlobals, inst->placement.origin, colorWhite, 0.30000001, Name);
                }
                dist = (model_->flags & 0x80000) != 0;
                vala = model_->lodInfo[lod].dist;
                v9 = I_fres(v23);
                adjustedDista = v9 * v35;
                ShodLodInfo(inst->placement.origin, lod, model_->numLods, adjustedDista, vala, dist);
                R_ShowCountsStaticModel(smodelIndex, lod);
                if ( r_staticModelDumpLodInfo->current.enabled )
                    R_DumpStaticModelLodInfo(inst, v35);
                if ( R_AllocStaticModelLighting(inst, smodelIndex) )
                {
                    allocatedLighting = 1;
                    *((unsigned int *)dest + ((unsigned int)smodelIndex >> 4)) |= lod << (2 * (smodelIndex & 0xF));
                    StaticModelId = R_GetStaticModelId(smodelIndex, lod);
                    //count = (uint16*) & v57[8 * StaticModelId.surfType + 2 * lod];
                    count = &staticModelLodCount[StaticModelId.surfType - 2][lod];
                    list = staticModelLodList[StaticModelId.surfType - 2][lod];
                    entryCount = *count;
                    list[entryCount++] = StaticModelId.objectId;
                    if ( entryCount < 0x80 )
                    {
                        *count = entryCount;
                    }
                    else
                    {
                        R_SkinStaticModelsCameraForLod(
                            model_,
                            primaryLightIndex,
                            v39,
                            (unsigned __int8 *)list,
                            entryCount,
                            StaticModelId.surfType,
                            lod,
                            v62,
                            &surfData,
                            v28,
                            v65);
                        *count = 0;
                    }
                }
                else
                {
                    R_WarnOncePerFrame(R_WARN_SMODEL_LIGHTING);
                    visData[smodelIndex] = 0;
                }
            }
        }
    }
    if ( r_staticModelDumpLodInfo->current.enabled )
        R_EndDumpStaticModelLodInfo();
    if ( allocatedLighting )
        R_SkinStaticModelsCamera(
            model,
            currentLightIndex,
            modelFade,
            staticModelLodList,
            staticModelLodCount,
            prepassSizeCull,
            &surfData,
            visLightsMask,
            noDynamicShadow);
    R_EndCmdBuf(&surfData.delayedCmdBuf);
    scene.drawSurfCount[DRAW_SURF_SMODEL_CAMERA_LIT] = surfData.drawSurf[0].current - scene.drawSurfs[DRAW_SURF_SMODEL_CAMERA_LIT];
    scene.drawSurfCount[DRAW_SURF_SMODEL_CAMERA_DECAL] = surfData.drawSurf[1].current - scene.drawSurfs[DRAW_SURF_SMODEL_CAMERA_DECAL];
    scene.drawSurfCount[DRAW_SURF_SMODEL_CAMERA_EMISSIVE] = surfData.drawSurf[2].current - scene.drawSurfs[DRAW_SURF_SMODEL_CAMERA_EMISSIVE];
}

void __cdecl R_SkinStaticModelsCameraForLod(
                const XModel *model,
                unsigned __int8 primaryLightIndex,
                int modelFade,
                unsigned __int8 *list,
                unsigned int count,
                unsigned int surfType,
                unsigned int lod,
                bool prepassSizeCull,
                GfxSModelDrawSurfLightingData *surfData,
                char visLightsMask,
                bool noDynamicShadow)
{
    unsigned __int16 listVal; // [esp+0h] [ebp-214h]
    unsigned int listIndex; // [esp+4h] [ebp-210h]
    unsigned int uncharredCount; // [esp+8h] [ebp-20Ch]
    unsigned __int16 uncharred[128]; // [esp+Ch] [ebp-208h] BYREF
    unsigned __int16 charred[128]; // [esp+10Ch] [ebp-108h] BYREF
    unsigned int charredCount; // [esp+210h] [ebp-4h]

    if ( r_use_separate_char_tech->current.enabled )
    {
        if ( count > 0x80
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                        462,
                        0,
                        "%s",
                        "count <= MAX_STATICMODEL_LIST") )
        {
            __debugbreak();
        }
        charredCount = 0;
        uncharredCount = 0;
        for ( listIndex = 0; listIndex < count; ++listIndex )
        {
            listVal = *(_WORD *)&list[2 * listIndex];
            if ( R_StaticModelNeedsCharredTech(listVal) )
                charred[charredCount++] = listVal;
            else
                uncharred[uncharredCount++] = listVal;
        }
        if ( charredCount )
            R_SkinStaticModelsCameraForLod_Internal(
                model,
                primaryLightIndex,
                modelFade,
                (unsigned __int8 *)charred,
                charredCount,
                surfType,
                lod,
                prepassSizeCull,
                surfData,
                1,
                visLightsMask,
                noDynamicShadow);
        if ( uncharredCount )
            R_SkinStaticModelsCameraForLod_Internal(
                model,
                primaryLightIndex,
                modelFade,
                (unsigned __int8 *)uncharred,
                uncharredCount,
                surfType,
                lod,
                prepassSizeCull,
                surfData,
                0,
                visLightsMask,
                noDynamicShadow);
    }
    else
    {
        R_SkinStaticModelsCameraForLod_Internal(
            model,
            primaryLightIndex,
            modelFade,
            list,
            count,
            surfType,
            lod,
            prepassSizeCull,
            surfData,
            0,
            visLightsMask,
            noDynamicShadow);
    }
}

void __cdecl R_SkinStaticModelsCameraForLod_Internal(
                const XModel *model,
                unsigned __int8 primaryLightIndex,
                int modelFade,
                unsigned __int8 *list,
                unsigned int count,
                unsigned int surfType,
                unsigned int lod,
                bool prepassSizeCull,
                GfxSModelDrawSurfLightingData *surfData,
                bool needsCharredTech,
                char visLightsMask,
                bool noDynamicShadow)
{
    __int64 v12; // rax
    bool enabled; // [esp+Eh] [ebp-2Eh]
    Material **materialForSurf; // [esp+10h] [ebp-2Ch]
    GfxDrawSurf drawSurf; // [esp+14h] [ebp-28h]
    const Material *material; // [esp+24h] [ebp-18h]
    unsigned int surfaceIndex; // [esp+28h] [ebp-14h]
    int surfaceCount; // [esp+2Ch] [ebp-10h]
    XSurface *surfaces; // [esp+30h] [ebp-Ch] BYREF
    unsigned int region; // [esp+34h] [ebp-8h]

    surfaceCount = XModelGetSurfaces(model, &surfaces, lod);
    if ( !surfaceCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                    357,
                    0,
                    "%s",
                    "surfaceCount") )
    {
        __debugbreak();
    }
    materialForSurf = XModelGetSkins(model, lod);
    if ( !materialForSurf
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                    360,
                    0,
                    "%s",
                    "materialForSurf") )
    {
        __debugbreak();
    }
    if ( (surfaceCount < 1 || surfaceCount > 80)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                    363,
                    0,
                    "surfaceCount not in [1, XMODEL_MAX_SURFS]\n\t%i not in [%i, %i]",
                    surfaceCount,
                    1,
                    80) )
    {
        __debugbreak();
    }
    if ( gfxDrawMethod.emissiveTechType >= 0x82u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                    366,
                    0,
                    "gfxDrawMethod.emissiveTechType doesn't index TECHNIQUE_COUNT\n\t%i not in [0, %i)",
                    gfxDrawMethod.emissiveTechType,
                    130) )
    {
        __debugbreak();
    }
    for ( surfaceIndex = 0; (int)surfaceIndex < surfaceCount; ++surfaceIndex )
    {
        if ( surfaceIndex >= 0xFF
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                        371,
                        0,
                        "surfaceIndex doesn't index UCHAR_MAX\n\t%i not in [0, %i)",
                        surfaceIndex,
                        255) )
        {
            __debugbreak();
        }
        material = *materialForSurf;
        if ( !*materialForSurf
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                        374,
                        0,
                        "%s",
                        "material") )
        {
            __debugbreak();
        }
        iassert(rgp.sortedMaterials[material->info.drawSurf.fields.materialSortedIndex] == material);
        region = material->cameraRegion;
        if ( region != 3 )
        {
            //drawSurf.fields = (GfxDrawSurfFields)material->info.drawSurf;
            drawSurf.packed = material->info.drawSurf.packed;
            HIDWORD(drawSurf.packed) = (primaryLightIndex << 11)
                                                             | ((surfType & 0xF) << 19)
                                                             | HIDWORD(drawSurf.packed) & 0xFF8007FF;
            if ( needsCharredTech )
            {
                HIDWORD(drawSurf.packed) = HIDWORD(drawSurf.packed) & 0xFE7FFFFF | 0x1000000;
                v12 = ((drawSurf.packed >> 20) & 0x1D | 2) << 20;
            }
            else
            {
                v12 = ((drawSurf.packed >> 20) & 0x1D) << 20;
            }
            *(unsigned int *)&drawSurf.fields = (((visLightsMask & 2) != 0) << 30)
                                                                    | ((visLightsMask & 1) << 29) & 0xBFFFFFFF
                                                                    | v12 & 0x9FFFFFFF
                                                                    | *(unsigned int *)&drawSurf.fields & 0x9E0FFFFF;
            HIDWORD(drawSurf.packed) = (noDynamicShadow << 25) | (HIDWORD(v12) | HIDWORD(drawSurf.packed)) & 0xFDFFFFFF;

            iassert(rgp.sortedMaterials[material->info.drawSurf.fields.materialSortedIndex] == material);

            if ( (material->info.gameFlags & 1) == 0 )
            {
                if ( surfType != 4
                    || (Sys_QueryD3DDeviceOKEvent() ? (enabled = r_pretess->current.enabled) : (enabled = 0),
                            !enabled
                     || !R_PreTessStaticModelCachedList(
                                 model,
                                 (unsigned __int16 *)list,
                                 count,
                                 lod,
                                 surfaceIndex,
                                 drawSurf,
                                 &surfData->drawSurf[region],
                                 &surfData->delayedCmdBuf)) )
                {
                    if ( !R_AllocDrawSurf(&surfData->delayedCmdBuf, drawSurf, &surfData->drawSurf[region], ((count + 1) >> 1) + 1 + R_PRIM_PTR_WORDS) )
                        return;
                    R_AddDelayedStaticModelDrawSurf(&surfData->delayedCmdBuf, &surfaces[surfaceIndex], list, count, lod);
                }
            }
        }
        ++materialForSurf;
    }
}

char __cdecl R_PreTessStaticModelCachedList(
                const XModel *model,
                unsigned __int16 *list,
                unsigned int count,
                unsigned int lod,
                int surfaceIndex,
                GfxDrawSurf drawSurf,
                GfxDrawSurfList *drawSurfList,
                GfxDelayedCmdBuf *delayedCmdBuf)
{
    const char *v9; // eax
    unsigned int baseIndex; // [esp+8h] [ebp-28h]
    unsigned __int16 *preTessIndices; // [esp+10h] [ebp-20h]
    unsigned int listIter; // [esp+18h] [ebp-18h]
    unsigned int firstIndex; // [esp+1Ch] [ebp-14h]
    unsigned __int16 *dstIndices; // [esp+20h] [ebp-10h]
    unsigned int preTessSurf; // [esp+24h] [ebp-Ch]
    const XSurface *xsurf; // [esp+28h] [ebp-8h]
    unsigned int surfIndexCount; // [esp+2Ch] [ebp-4h]

    xsurf = XModelGetSurface(model, lod, surfaceIndex);
    surfIndexCount = 3 * xsurf->triCount;
    preTessIndices = R_AllocPreTessIndices(surfIndexCount * count);
    if ( !preTessIndices )
        return 0;
    dstIndices = preTessIndices;
    for ( listIter = 0; listIter < count; ++listIter )
    {
        baseIndex = 3 * xsurf->baseTriIndex + 4 * R_GetCachedSModelSurf(list[listIter])->cachedSurf.baseVertIndex;
        if ( baseIndex >= 0x100000 || surfIndexCount + baseIndex > 0x100000 )
        {
            v9 = va("%i, %i", baseIndex, surfIndexCount);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                            312,
                            0,
                            "%s\n\t%s",
                            "baseIndex < SMC_MAX_INDEX_IN_CACHE && baseIndex + surfIndexCount <= SMC_MAX_INDEX_IN_CACHE",
                            v9) )
                __debugbreak();
        }
        Com_Memcpy(dstIndices, &gfxBuf.smodelCache.indices[baseIndex], 2 * surfIndexCount);
        dstIndices += surfIndexCount;
    }
    HIDWORD(drawSurf.packed) = HIDWORD(drawSurf.packed) & 0xFF87FFFF | 0x180000;
    if ( R_AllocDrawSurf(delayedCmdBuf, drawSurf, drawSurfList, 3u) )
    {
        BYTE1(preTessSurf) = lod;
        LOBYTE(preTessSurf) = surfaceIndex;
        HIWORD(preTessSurf) = *list;
        firstIndex = preTessIndices - gfxBuf.preTessIndexBuffer->indices;
        if ( firstIndex >= 0x100000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                        327,
                        0,
                        "firstIndex doesn't index R_MAX_PRETESS_INDICES\n\t%i not in [0, %i)",
                        firstIndex,
                        0x100000) )
        {
            __debugbreak();
        }
        R_WritePrimDrawSurfInt(delayedCmdBuf, count);
        R_WritePrimDrawSurfInt(delayedCmdBuf, preTessSurf);
        R_WritePrimDrawSurfInt(delayedCmdBuf, firstIndex);
    }
    return 1;
}

void __cdecl R_SkinStaticModelsCamera(
                const XModel *model,
                unsigned __int8 primaryLightIndex,
                int modelFade,
                unsigned __int16 (*staticModelLodList)[4][128],
                unsigned __int16 (*staticModelLodCount)[4],
                bool prepassSizeCull,
                GfxSModelDrawSurfLightingData *surfData,
                char visLightsMask,
                bool noDynamicShadow)
{
    unsigned int surfTypeIndex; // [esp+0h] [ebp-4h]

    for ( surfTypeIndex = 0; surfTypeIndex < 4; ++surfTypeIndex )
        R_SkinStaticModelsCameraForSurface(
            model,
            primaryLightIndex,
            modelFade,
            (unsigned __int16 (*)[128])(*staticModelLodList)[4 * surfTypeIndex],
            &(*staticModelLodCount)[4 * surfTypeIndex],
            surfTypeIndex + 2,
            prepassSizeCull,
            surfData,
            visLightsMask,
            noDynamicShadow);
}

void __cdecl R_SkinStaticModelsCameraForSurface(
                const XModel *model,
                unsigned __int8 primaryLightIndex,
                int modelFade,
                unsigned __int16 (*staticModelLodList)[128],
                unsigned __int16 *staticModelLodCount,
                unsigned int surfType,
                bool prepassSizeCull,
                GfxSModelDrawSurfLightingData *surfData,
                char visLightsMask,
                bool noDynamicShadow)
{
    signed int lod; // [esp+0h] [ebp-8h]

    for ( lod = 0; lod < 4; ++lod )
    {
        if ( staticModelLodCount[lod] )
            R_SkinStaticModelsCameraForLod(
                model,
                primaryLightIndex,
                modelFade,
                (unsigned __int8 *)&(*staticModelLodList)[128 * lod],
                staticModelLodCount[lod],
                surfType,
                lod,
                prepassSizeCull,
                surfData,
                visLightsMask,
                noDynamicShadow);
    }
}

void __cdecl R_ShowCountsStaticModel(int smodelIndex, int lod)
{
    char *v2; // eax
    char *v3; // eax
    char *v4; // eax
    int totalVertCount; // [esp+4h] [ebp-20h]
    GfxStaticModelDrawInst *smodelDrawInst; // [esp+8h] [ebp-1Ch]
    int totalTriCount; // [esp+Ch] [ebp-18h]
    int surfaceIndex; // [esp+10h] [ebp-14h]
    const XModel *model; // [esp+14h] [ebp-10h]
    int surfaceCount; // [esp+18h] [ebp-Ch]
    XSurface *surfaces; // [esp+1Ch] [ebp-8h] BYREF
    const XSurface *xsurf; // [esp+20h] [ebp-4h]

    if ( r_showTriCounts->current.enabled || r_showVertCounts->current.enabled || r_showSurfCounts->current.enabled )
    {
        smodelDrawInst = &rgp.world->dpvs.smodelDrawInsts[smodelIndex];
        model = smodelDrawInst->model;
        XModelGetLodInfo(model, lod);
        surfaceCount = XModelGetSurfaces(model, &surfaces, lod);
        if ( !surfaceCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                        697,
                        0,
                        "%s",
                        "surfaceCount") )
        {
            __debugbreak();
        }
        totalTriCount = 0;
        totalVertCount = 0;
        for ( surfaceIndex = 0; surfaceIndex < surfaceCount; ++surfaceIndex )
        {
            xsurf = &surfaces[surfaceIndex];
            if ( r_showTriCounts->current.enabled )
            {
                totalTriCount += XSurfaceGetNumTris(xsurf);
            }
            else if ( r_showVertCounts->current.enabled )
            {
                totalVertCount += XSurfaceGetNumVerts(xsurf);
            }
        }
        if ( r_showTriCounts->current.enabled )
        {
            v2 = va("%i", totalTriCount);
            R_AddXModelDebugString(smodelDrawInst->placement.origin, v2);
        }
        else if ( r_showVertCounts->current.enabled )
        {
            v3 = va("%i", totalVertCount);
            R_AddXModelDebugString(smodelDrawInst->placement.origin, v3);
        }
        else if ( r_showSurfCounts->current.enabled )
        {
            v4 = va("%i", surfaceCount);
            R_AddXModelDebugString(smodelDrawInst->placement.origin, v4);
        }
    }
}

void __cdecl R_DumpStaticModelLodInfo(const GfxStaticModelDrawInst *smodelDrawInst, float dist)
{
    if ( g_dumpStaticModelFileHandle != -1 )
    {
        if ( !g_dumpStaticModelFileHandle )
        {
            g_dumpStaticModelFileHandle = FS_FOpenTextFileWrite(g_dumpStaticModelFilePath);
            if ( g_dumpStaticModelFileHandle )
                R_StaticModelWriteInfoHeader(g_dumpStaticModelFileHandle);
            g_dumpStaticModelCount = 0;
        }
        if ( g_dumpStaticModelFileHandle )
        {
            R_StaticModelWriteInfo(g_dumpStaticModelFileHandle, smodelDrawInst, dist);
        }
        else
        {
            g_dumpStaticModelFileHandle = -1;
            Com_PrintError(1, "Could not dump model info.\n");
        }
    }
}

void __cdecl R_StaticModelWriteInfoHeader(int fileHandle)
{
    char dest; // [esp+10h] [ebp-1008h] BYREF
    _BYTE v2[4103]; // [esp+11h] [ebp-1007h] BYREF

    *(unsigned int *)&v2[4099] = 4096;
    Com_sprintf(
        &dest,
        0x1000u,
        "index,name,radius,numLods,lodDist,lodPixels720p,1PixelDist720p,scaledDist,posx,posy,posz,pixels\n");
    FS_Write(&dest, &v2[strlen(&dest)] - v2, fileHandle);
}


void __cdecl R_StaticModelWriteInfo(int fileHandle, const GfxStaticModelDrawInst *smodelDrawInst, float dist)
{
    long double len; // [esp+4Ch] [ebp-1028h]
    char dest; // [esp+5Ch] [ebp-1018h] BYREF
    _BYTE v5[4103]; // [esp+5Dh] [ebp-1017h] BYREF
    float v6; // [esp+1064h] [ebp-10h]
    XModel *model; // [esp+1068h] [ebp-Ch]
    float v8; // [esp+106Ch] [ebp-8h]
    float v9; // [esp+1070h] [ebp-4h]


    //if ( (_S1_16 & 1) == 0 )
    //{
    //    _S1_16 |= 1u;
    //    __libm_sse2_tan(len);
    //    radius2pixels = 720.0 / (float)22.5;
    //}
    static float radius2pixels = 720.0f / tanf(22.5f); // KISAKTODO: should this be in radians?

    *(unsigned int *)&v5[4099] = 4096;
    model = smodelDrawInst->model;
    if ( !model
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp", 826, 0, "%s", "xmodel") )
    {
        __debugbreak();
    }
    if ( !model->name
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                    827,
                    0,
                    "%s",
                    "xmodel->name") )
    {
        __debugbreak();
    }
    if ( model->numLods <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                    828,
                    0,
                    "%s",
                    "xmodel->numLods > 0") )
    {
        __debugbreak();
    }
    v9 = *((float *)&model->localBoneNames + 8 * model->numLods);
    if ( v9 <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                    832,
                    0,
                    "%s",
                    "lodDist > 0.0f") )
    {
        __debugbreak();
    }
    v6 = (float)(radius2pixels * model->radius) / v9;
    v8 = radius2pixels * model->radius;
    ++g_dumpStaticModelCount;
    if ( smodelDrawInst->placement.scale > 0.0 && dist > 0.0 )
        Com_sprintf(
            &dest,
            0x1000u,
            "%d,%s,%.1f,%d,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f\n",
            g_dumpStaticModelCount,
            model->name,
            model->radius,
            model->numLods,
            v9,
            v6,
            v8,
            (float)(dist / smodelDrawInst->placement.scale),
            smodelDrawInst->placement.origin[0],
            smodelDrawInst->placement.origin[1],
            smodelDrawInst->placement.origin[2],
            (float)(v8 / (float)(dist / smodelDrawInst->placement.scale)));
    FS_Write(&dest, &v5[strlen(&dest)] - v5, fileHandle);
}

void R_EndDumpStaticModelLodInfo()
{
    char *basePath; // [esp+0h] [ebp-4h]

    Dvar_SetBool((dvar_s *)r_staticModelDumpLodInfo, 0);
    if ( g_dumpStaticModelFileHandle )
    {
        if ( g_dumpStaticModelFileHandle == -1 )
        {
            g_dumpStaticModelFileHandle = 0;
        }
        else
        {
            FS_FCloseFile(g_dumpStaticModelFileHandle);
            basePath = Sys_DefaultInstallPath();
            Com_Printf(18, "^7Successfully wrote static model info [%s\\%s].\n", basePath, g_dumpStaticModelFilePath);
        }
    }
}

void __cdecl R_SortAllStaticModelSurfacesCamera()
{
    PROF_SCOPED("R_SortAllStaticModelSurfacesCamera");

    {
        PROF_SCOPED("sort surfs SMODEL_CAMERA_LIT");
        R_SortDrawSurfs(scene.drawSurfs[1], scene.drawSurfCount[1]);
    }
    {
        PROF_SCOPED("sort surfs SMODEL_CAMERA_DECAL");
        R_SortDrawSurfs(scene.drawSurfs[4], scene.drawSurfCount[4]);
    }
    {
        PROF_SCOPED("sort surfs SMODEL_CAMERA_EMISSIVE");
        R_SortDrawSurfs(scene.drawSurfs[11], scene.drawSurfCount[11]);
    }
}

void __cdecl R_SortAllStaticModelSurfacesSunShadow()
{
    {
        PROF_SCOPED("sort surfs");
        R_SortDrawSurfs(scene.drawSurfs[20], scene.drawSurfCount[20]);
    }
    {
        PROF_SCOPED("sort surfs");
        R_SortDrawSurfs(scene.drawSurfs[24], scene.drawSurfCount[24]);
    }
}

void __cdecl R_AddAllStaticModelSurfacesSunShadow(int viewIndex)
{
    R_AddAllStaticModelSurfacesRangeSunShadow(viewIndex, 0, 0x1000u);
    R_AddAllStaticModelSurfacesRangeSunShadow(viewIndex, 1u, 0x2000u);
}

void __cdecl R_AddAllStaticModelSurfacesRangeSunShadow(
                int viewIndex,
                unsigned int partitionIndex,
                unsigned int maxDrawSurfCount)
{
    float adjustedDist; // [esp+0h] [ebp-10C8h]
    float baseDist; // [esp+4h] [ebp-10C4h]
    //float scale; // [esp+Ch] [ebp-10BCh]
    unsigned int dynamicSModelCount; // [esp+10h] [ebp-10B8h]
    GfxStaticModelId StaticModelId; // [esp+34h] [ebp-1094h]
    GfxStaticModelDrawInst *inst; // [esp+3Ch] [ebp-108Ch]
    float scale; // [esp+40h] [ebp-1088h]
    float bias; // [esp+44h] [ebp-1084h]
    unsigned __int16 staticModelLodList[4][4][128]; // [esp+48h] [ebp-1080h] BYREF
    GfxStaticModelDrawInst *smodelDrawInsts; // [esp+1048h] [ebp-80h]
    //float originDist; // [esp+104Ch] [ebp-7Ch]
    XModel *nextModel; // [esp+1050h] [ebp-78h]
    XModel *model; // [esp+1054h] [ebp-74h]
    unsigned int stage; // [esp+1058h] [ebp-70h]
    unsigned __int16 *list; // [esp+105Ch] [ebp-6Ch]
    int lod; // [esp+1060h] [ebp-68h]
    unsigned int smodelCount; // [esp+1064h] [ebp-64h]
    _WORD *count; // [esp+1068h] [ebp-60h]
    unsigned __int8 *visdata; // [esp+106Ch] [ebp-5Ch] BYREF
    float origin[3]; // [esp+1070h] [ebp-58h] BYREF
    unsigned __int16 staticModelLodCount[4][4]; // [esp+107Ch] [ebp-4Ch] BYREF
    GfxSModelDrawSurfData surfData; // [esp+10A0h] [ebp-28h] BYREF
    int allocedLighting; // [esp+10B8h] [ebp-10h]
    float originDist; // [esp+10BCh] [ebp-Ch]
    unsigned int entryCount; // [esp+10C0h] [ebp-8h]
    unsigned int i; // [esp+10C4h] [ebp-4h]

    smodelCount = g_worldDpvs->smodelCount;
    visdata = rgp.world->dpvs.smodelVisData[partitionIndex + 1];
    smodelDrawInsts = rgp.world->dpvs.smodelDrawInsts;
    g_lodParms = &rg.correctedLodParms[viewIndex];
    iassert(g_lodParms->valid);

    scale = g_lodParms->ramp[0].scale;
    bias = g_lodParms->ramp[0].bias;
    origin[0] = g_lodParms->origin[0];
    origin[1] = g_lodParms->origin[1];
    origin[2] = g_lodParms->origin[2];

    R_InitBspDrawSurf((GfxSModelDrawSurfLightingData *)&surfData);

    stage = DRAW_SURF_SMODEL_SUNSHADOW_0 + (4 * partitionIndex);

    surfData.drawSurfList.current = scene.drawSurfs[stage];

    iassert((int)maxDrawSurfCount == scene.maxDrawSurfCount[stage]);

    surfData.drawSurfList.end = &surfData.drawSurfList.current[maxDrawSurfCount];
    memset(staticModelLodCount, 0, sizeof(staticModelLodCount));
    allocedLighting = 0;
    model = 0;

    if ( (int)g_worldDpvs->dynamicSModelCount > 2048 )
        dynamicSModelCount = 2048;
    else
        dynamicSModelCount = g_worldDpvs->dynamicSModelCount;

    for ( i = 0; i < smodelCount; ++i )
    {
        if (!visdata[i])
        {
            continue;
        }

        inst = &smodelDrawInsts[i];

        if ((inst->flags & 1) != 0)
        {
            continue;
        }

        originDist = Vec3Distance(inst->placement.origin, origin) * scale + bias;

        if (originDist >= inst->cullDist)
        {
            visdata[i] = 0;
            continue;
        }

        nextModel = inst->model;
        if ( nextModel != model )
        {
            if ( allocedLighting )
            {
                R_SkinStaticModelsShadow(model, staticModelLodList, staticModelLodCount, &surfData);
                memset(staticModelLodCount, 0, sizeof(staticModelLodCount));
                allocedLighting = 0;
            }
            model = nextModel;
        }

        if ( inst->placement.scale >= 1.0 )
            scale = inst->placement.scale;
        else
            scale = 1.0f;

        baseDist = I_fres(scale) * originDist;
        adjustedDist = I_fres(scale) * originDist;
        lod = XModelGetLodForDist(nextModel, adjustedDist, baseDist, 0);

        if (lod < 0)
        {
            continue;
        }

        if ( R_AllocStaticModelLighting(inst, i) )
        {
            if ( i < dynamicSModelCount )
                scene.dynSModelVisBitsSunShadow[i >> 5] |= 1 << (i & 0x1F);
            allocedLighting = 1;
            StaticModelId = R_GetStaticModelId(i, lod);
            //count = (_WORD *)&origin[2 * StaticModelId.surfType - 1] + lod;
            count = &staticModelLodCount[StaticModelId.surfType - 2][lod];
            list = staticModelLodList[StaticModelId.surfType - 2][lod];
            entryCount = (unsigned __int16)*count;
            list[entryCount++] = StaticModelId.objectId;
            if ( entryCount >= 0x80 )
            {
                R_SkinStaticModelsShadowForLod(
                    nextModel,
                    (unsigned __int8 *)list,
                    entryCount,
                    StaticModelId.surfType,
                    lod,
                    &surfData);
                *count = 0;
            }
            else
            {
                *count = entryCount;
            }
        }
        else
        {
            R_WarnOncePerFrame(R_WARN_SMODEL_LIGHTING);
        }
    }
    if ( allocedLighting )
        R_SkinStaticModelsShadow(model, staticModelLodList, staticModelLodCount, &surfData);
    R_EndCmdBuf(&surfData.delayedCmdBuf);
    scene.drawSurfCount[stage] = surfData.drawSurfList.current - scene.drawSurfs[stage];
}

void __cdecl R_SkinStaticModelsShadowForLod(
                const XModel *model,
                unsigned __int8 *list,
                unsigned int count,
                unsigned int surfType,
                unsigned int lod,
                GfxSModelDrawSurfData *surfData)
{
    bool v6; // zf
    const char *v7; // eax
    bool enabled; // [esp+Fh] [ebp-2Dh]
    Material **materialForSurf; // [esp+18h] [ebp-24h]
    GfxDrawSurf drawSurf; // [esp+1Ch] [ebp-20h]
    unsigned __int8 shadowmapBuildTechType; // [esp+27h] [ebp-15h]
    const Material *material; // [esp+28h] [ebp-14h]
    int surfaceIndex; // [esp+2Ch] [ebp-10h]
    int surfaceCount; // [esp+30h] [ebp-Ch]
    XSurface *surfaces; // [esp+34h] [ebp-8h] BYREF
    int castsShadow; // [esp+38h] [ebp-4h]

    surfaceCount = XModelGetSurfaces(model, &surfaces, lod);
    if ( !surfaceCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                    519,
                    0,
                    "%s",
                    "surfaceCount") )
    {
        __debugbreak();
    }
    materialForSurf = XModelGetSkins(model, lod);
    if ( !materialForSurf
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                    522,
                    0,
                    "%s",
                    "materialForSurf") )
    {
        __debugbreak();
    }
    if ( (surfaceCount < 1 || surfaceCount > 80)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                    525,
                    0,
                    "surfaceCount not in [1, XMODEL_MAX_SURFS]\n\t%i not in [%i, %i]",
                    surfaceCount,
                    1,
                    80) )
    {
        __debugbreak();
    }
    shadowmapBuildTechType = gfxMetrics.shadowmapBuildTechType;
    for ( surfaceIndex = 0; surfaceIndex < surfaceCount; ++surfaceIndex )
    {
        material = *materialForSurf;
        if ( !*materialForSurf
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
                        538,
                        0,
                        "%s",
                        "material") )
        {
            __debugbreak();
        }
        iassert(rgp.sortedMaterials[material->info.drawSurf.fields.materialSortedIndex] == material);

        v6 = (material->info.gameFlags & 0x40) == 0;
        castsShadow = (material->info.gameFlags & 0x40) != 0;
        if ( !v6 )
        {
            if ( !Material_GetTechnique(material, shadowmapBuildTechType) )
            {
                v7 = RB_LogTechniqueType(shadowmapBuildTechType);
                Com_Error(ERR_DROP, "material \"%s\" lacks shadow tech %d %s", material->info.name, shadowmapBuildTechType, v7);
            }
            //drawSurf.fields = (GfxDrawSurfFields)material->info.drawSurf;
            drawSurf.packed = material->info.drawSurf.packed;
            HIDWORD(drawSurf.packed) = ((surfType & 0xF) << 19) | HIDWORD(drawSurf.packed) & 0xFF87FFFF;
            if ( surfType != 4
                || (Sys_QueryD3DDeviceOKEvent() ? (enabled = r_pretess->current.enabled) : (enabled = 0),
                        !enabled
                 || !R_PreTessStaticModelCachedList(
                             model,
                             (unsigned __int16 *)list,
                             count,
                             lod,
                             surfaceIndex,
                             drawSurf,
                             &surfData->drawSurfList,
                             &surfData->delayedCmdBuf)) )
            {
                if ( !R_AllocDrawSurf(&surfData->delayedCmdBuf, drawSurf, &surfData->drawSurfList, ((count + 1) >> 1) + 1 + R_PRIM_PTR_WORDS) )
                    return;
                R_AddDelayedStaticModelDrawSurf(&surfData->delayedCmdBuf, &surfaces[surfaceIndex], list, count, lod);
            }
        }
        ++materialForSurf;
    }
}

void __cdecl R_SkinStaticModelsShadow(
                const XModel *model,
                unsigned __int16 (*staticModelLodList)[4][128],
                unsigned __int16 (*staticModelLodCount)[4],
                GfxSModelDrawSurfData *surfData)
{
    unsigned int surfTypeIndex; // [esp+0h] [ebp-4h]

    for ( surfTypeIndex = 0; surfTypeIndex < 4; ++surfTypeIndex )
        R_SkinStaticModelsShadowForSurface(
            model,
            (unsigned __int16 (*)[128])(*staticModelLodList)[4 * surfTypeIndex],
            &(*staticModelLodCount)[4 * surfTypeIndex],
            surfTypeIndex + 2,
            surfData);
}

void __cdecl R_SkinStaticModelsShadowForSurface(
                const XModel *model,
                unsigned __int16 (*staticModelLodList)[128],
                unsigned __int16 *staticModelLodCount,
                unsigned int surfType,
                GfxSModelDrawSurfData *surfData)
{
    signed int lod; // [esp+0h] [ebp-8h]

    for ( lod = 0; lod < 4; ++lod )
    {
        if ( staticModelLodCount[lod] )
            R_SkinStaticModelsShadowForLod(
                model,
                (unsigned __int8 *)&(*staticModelLodList)[128 * lod],
                staticModelLodCount[lod],
                surfType,
                lod,
                surfData);
    }
}

void __cdecl R_AddAllStaticModelSurfacesSpotShadow(
                int viewIndex,
                unsigned int spotShadowIndex,
                unsigned int primaryLightIndex)
{
    double v3; // st7
    double v4; // st7
    float v5; // [esp+0h] [ebp-10CCh]
    float v6; // [esp+4h] [ebp-10C8h]
    float val; // [esp+Ch] [ebp-10C0h]
    GfxStaticModelId StaticModelId; // [esp+34h] [ebp-1098h]
    GfxStaticModelDrawInst *smodelDrawInst; // [esp+38h] [ebp-1094h]
    float scale; // [esp+3Ch] [ebp-1090h]
    float bias; // [esp+40h] [ebp-108Ch]
    unsigned __int16 staticModelLodList[4][4][128]; // [esp+44h] [ebp-1088h] BYREF
    GfxStaticModelDrawInst *smodelDrawInsts; // [esp+1048h] [ebp-84h]
    float v14; // [esp+104Ch] [ebp-80h]
    XModel *model; // [esp+1050h] [ebp-7Ch]
    GfxShadowGeometry *v16; // [esp+1054h] [ebp-78h]
    XModel *v17; // [esp+1058h] [ebp-74h]
    unsigned int v18; // [esp+105Ch] [ebp-70h]
    unsigned __int16 *list; // [esp+1060h] [ebp-6Ch]
    int lod; // [esp+1064h] [ebp-68h]
    unsigned int i; // [esp+1068h] [ebp-64h]
    unsigned int smodelCount; // [esp+106Ch] [ebp-60h]
    _WORD *v23; // [esp+1070h] [ebp-5Ch] BYREF
    float origin[3]; // [esp+1074h] [ebp-58h] BYREF
    unsigned __int16 staticModelLodCount[4][4]; // [esp+1080h] [ebp-4Ch] BYREF
    GfxSModelDrawSurfData surfData; // [esp+10A4h] [ebp-28h] BYREF
    float v14_; // [esp+10BCh] [ebp-10h]
    unsigned int v28; // [esp+10C0h] [ebp-Ch]
    unsigned int v29; // [esp+10C4h] [ebp-8h]
    int surfCount; // [esp+10C8h] [ebp-4h]

    smodelCount = rgp.world->dpvs.smodelCount;
    smodelDrawInsts = rgp.world->dpvs.smodelDrawInsts;
    if (!rg.lodParms.valid
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_staticmodel.cpp",
            1935,
            0,
            "%s",
            "rg.lodParms.valid"))
    {
        __debugbreak();
    }
    scale = rg.correctedLodParms[viewIndex].ramp[0].scale;
    bias = rg.correctedLodParms[viewIndex].ramp[0].bias;
    origin[0] = rg.correctedLodParms[viewIndex].origin[0];
    origin[1] = rg.correctedLodParms[viewIndex].origin[1];
    origin[2] = rg.correctedLodParms[viewIndex].origin[2];
    R_InitBspDrawSurf((GfxSModelDrawSurfLightingData *)&surfData);
    v18 = 3 * spotShadowIndex + 28;
    surfData.drawSurfList.current = scene.drawSurfs[v18];
    surfData.drawSurfList.end = &scene.drawSurfs[v18][scene.maxDrawSurfCount[v18]];
    v17 = 0;
    v16 = &rgp.world->shadowGeom[primaryLightIndex];
    for (i = 0; i < v16->smodelCount; ++i)
    {
        v29 = v16->smodelIndex[i];
        smodelDrawInst = &smodelDrawInsts[v29];
        model = smodelDrawInst->model;
        if ((smodelDrawInst->flags & 1) == 0)
        {
            v14 = Vec3Distance(smodelDrawInst->placement.origin, origin);
            v14_ = v14;
            v14 = (float)(v14 * scale) + bias;
            if (v14_ < smodelDrawInst->cullDist)
            {
                model = smodelDrawInst->model;
                val = smodelDrawInst->placement.scale >= 1.0 ? smodelDrawInst->placement.scale : 1.0f;
                v3 = I_fres(val);
                v6 = v3 * v14_;
                v4 = I_fres(val);
                v5 = v4 * v14;
                lod = XModelGetLodForDist(model, v5, v6, 0);
                if (lod >= 0)
                {
                    if (R_AllocStaticModelLighting(smodelDrawInst, v29))
                    {
                        if (model != v17)
                        {
                            if (v17)
                                R_SkinStaticModelsShadow(v17, staticModelLodList, staticModelLodCount, &surfData);
                            v17 = model;
                            memset(staticModelLodCount, 0, sizeof(staticModelLodCount));
                        }
                        StaticModelId = R_GetStaticModelId(v29, lod);
                        //v23 = (_WORD *)&origin[2 * StaticModelId.surfType - 1] + lod;
                        v23 = &staticModelLodCount[StaticModelId.surfType - 2][lod];
                        list = staticModelLodList[StaticModelId.surfType - 2][lod];
                        v28 = (unsigned __int16)*v23;
                        list[v28++] = StaticModelId.objectId;
                        if (v28 >= 0x80)
                        {
                            R_SkinStaticModelsShadowForLod(
                                model,
                                (unsigned __int8 *)list,
                                v28,
                                StaticModelId.surfType,
                                lod,
                                &surfData);
                            *v23 = 0;
                        }
                        else
                        {
                            *v23 = v28;
                        }
                    }
                    else
                    {
                        R_WarnOncePerFrame(R_WARN_SMODEL_LIGHTING);
                    }
                }
            }
        }
    }
    if (v17)
        R_SkinStaticModelsShadow(v17, staticModelLodList, staticModelLodCount, &surfData);
    R_EndCmdBuf(&surfData.delayedCmdBuf);
    surfCount = surfData.drawSurfList.current - scene.drawSurfs[v18];
    scene.drawSurfCount[v18] = surfCount;

    PROF_SCOPED("sort surfs");
    R_SortDrawSurfs(scene.drawSurfs[v18], surfCount);
}

