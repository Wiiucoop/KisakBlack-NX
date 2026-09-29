#include "r_light.h"
#include <qcommon/common.h>
#include "r_light_load_obj.h"
#include <database/db_registry.h>
#include "r_scene.h"
#include "r_dvars.h"
#include "r_add_bsp.h"
#include "r_marks.h"

#include <algorithm>
#include "r_add_cmdbuf.h"
#include "r_add_staticmodel.h"
#include <xanim/xmodel_utils.h>
#include "r_foliage.h"
#include "r_warn.h"
#include <physics/rope.h>
#include <DynEntity/DynEntity_load_obj.h>
#include <EffectsCore/fx_system.h>
#include <qcommon/dobj_management.h>
#include "r_drawsurf.h"
#include "r_pretess.h"
#include <cgame_mp/cg_ents_mp.h>

int(__cdecl *allowSurf_0[2])(int, void *) = { R_AllowBspSpotLight, R_AllowBspSpotLightShadows };

StaticModelLightCallback g_staticModelLightCallback;
LightGlobals lightGlob;

GfxLightDef *__cdecl R_RegisterLightDef(const char *name)
{
    if ( useFastFile->current.enabled )
        return R_RegisterLightDef_FastFile(name);
    else
        return R_RegisterLightDef_LoadObj(name);
}

GfxLightDef *__cdecl R_RegisterLightDef_LoadObj(const char *name)
{
    GfxLightDef *def; // [esp+0h] [ebp-8h]
    GfxLightDef *defa; // [esp+0h] [ebp-8h]
    int defIndex; // [esp+4h] [ebp-4h]
    int defIndexa; // [esp+4h] [ebp-4h]

    if ( !name && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp", 46, 0, "%s", "name") )
        __debugbreak();
    for ( defIndex = 0; defIndex < lightGlob.defCount; ++defIndex )
    {
        if ( !lightGlob.defs[defIndex]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                        50,
                        0,
                        "%s",
                        "lightGlob.defs[defIndex]") )
        {
            __debugbreak();
        }
        if ( !lightGlob.defs[defIndex]->name
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                        51,
                        0,
                        "%s",
                        "lightGlob.defs[defIndex]->name") )
        {
            __debugbreak();
        }
        if ( !I_stricmp(name, lightGlob.defs[defIndex]->name) )
            return lightGlob.defs[defIndex];
    }
    if ( lightGlob.defCount == 64 )
    {
        Com_Printf(8, "Loaded light defs:\n");
        for ( defIndexa = 0; defIndexa < lightGlob.defCount; ++defIndexa )
            Com_Printf(8, "    %s\n", lightGlob.defs[defIndexa]->name);
        Com_Error(
            ERR_DROP,
            "Can't load light def %s; %i unique light defs already loaded",
            lightGlob.defs[defIndex]->name,
            lightGlob.defCount);
    }
    defa = R_LoadLightDef(name);
    if ( defa )
    {
        lightGlob.defs[lightGlob.defCount++] = defa;
        return defa;
    }
    else
    {
        if ( !I_stricmp(name, "light_dynamic") )
            Com_Error(ERR_DROP, "Can't load the default light def '%s'", name);
        return R_RegisterLightDef("light_dynamic");
    }
}

GfxLightDef *__cdecl R_RegisterLightDef_FastFile(const char *name)
{
    return DB_FindXAssetHeader(ASSET_TYPE_LIGHT_DEF, (char*)name, 1, -1).lightDef;
}

void __cdecl R_InitLightDefs()
{
    if ( lightGlob.defCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                    103,
                    0,
                    "%s",
                    "lightGlob.defCount == 0") )
    {
        __debugbreak();
    }
    rgp.dlightDef = R_RegisterLightDef("light_dynamic");
}

void __cdecl R_ShutdownLightDefs()
{
    lightGlob.defCount = 0;
}

int __cdecl R_GetPointLightPartitions(GfxLight *visibleLights)
{
    const GfxLight *addedLights[32]; // [esp+8h] [ebp-90h] BYREF
    int visibleCount; // [esp+88h] [ebp-10h]
    int visibleLimit; // [esp+8Ch] [ebp-Ch]
    int lightIndex; // [esp+90h] [ebp-8h]
    int lightShadowIndex; // [esp+94h] [ebp-4h]

    if ( scene.addedLightCount > 32
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                    239,
                    1,
                    "%s\n\t(scene.addedLightCount) = %i",
                    "(scene.addedLightCount <= 32)",
                    scene.addedLightCount) )
    {
        __debugbreak();
    }
    visibleCount = 0;
    for ( lightIndex = 0; lightIndex < scene.addedLightCount; ++lightIndex )
    {
        if ( !scene.isAddedLightCulled[lightIndex] )
        {
            if ( visibleCount > 32
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                            246,
                            1,
                            "%s\n\t(visibleCount) = %i",
                            "(visibleCount <= 32)",
                            visibleCount) )
            {
                __debugbreak();
            }
            addedLights[visibleCount++] = &scene.addedLight[lightIndex];
        }
    }
    visibleLimit = r_dlightLimit->current.integer;
    if ( visibleLimit > 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                    253,
                    0,
                    "%s\n\t(visibleLimit) = %i",
                    "(visibleLimit <= 4)",
                    visibleLimit) )
    {
        __debugbreak();
    }
    if ( visibleCount > visibleLimit )
    {
        R_MostImportantLights(addedLights, visibleCount, visibleLimit);
        visibleCount = visibleLimit;
    }
    for ( lightIndex = 0; lightIndex < visibleCount; ++lightIndex )
    {
        memcpy(&visibleLights[lightIndex], addedLights[lightIndex], sizeof(GfxLight));
        scene.visLightShadow[lightIndex - 4].drawSurfCount = 0;
    }
    for ( lightShadowIndex = 0; (unsigned int)lightShadowIndex < 2; ++lightShadowIndex )
        scene.visLightShadow[lightShadowIndex].drawSurfCount = 0;
    return visibleCount;
}

void __cdecl R_MostImportantLights(const GfxLight **lights, int lightCount, int keepCount)
{
    int top; // [esp+54h] [ebp-10h]
    int bot; // [esp+58h] [ebp-Ch]
    const GfxLight *pivot; // [esp+5Ch] [ebp-8h]
    const GfxLight *swapCache; // [esp+60h] [ebp-4h]
    const GfxLight *swapCachea; // [esp+60h] [ebp-4h]

    if ( lightCount <= keepCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                    164,
                    0,
                    "%s",
                    "lightCount > keepCount") )
    {
        __debugbreak();
    }
    if ( keepCount < 1
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp", 165, 0, "%s", "keepCount >= 1") )
    {
        __debugbreak();
    }
    while ( 1 )
    {
        bot = 0;
        top = lightCount;
        pivot = *lights;
        while ( 1 )
        {
            if ( top < bot
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp", 178, 1, "%s", "top >= bot") )
            {
                __debugbreak();
            }
            do
                ++bot;
            while ( bot < top && R_LightImportanceGreaterEqual(lights[bot], pivot) );
            if ( bot > top )
                break;
            do
                --top;
            while ( top >= bot && R_LightImportanceGreaterEqual(pivot, lights[top]) );
            if ( bot > top )
                break;
            swapCache = lights[bot];
            lights[bot] = lights[top];
            lights[top] = swapCache;
        }
        if ( bot != top + 1
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp", 200, 1, "%s", "bot == top + 1") )
        {
            __debugbreak();
        }
        if ( bot == lightCount )
        {
            swapCachea = *lights;
            *lights = lights[top];
            lights[top] = swapCachea;
            --bot;
        }
        if ( bot == keepCount )
            break;
        if ( lightCount <= bot
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp", 216, 1, "%s", "lightCount > bot") )
        {
            __debugbreak();
        }
        if ( bot >= keepCount )
        {
            lightCount = bot;
        }
        else
        {
            lights += bot;
            lightCount -= bot;
            keepCount -= bot;
        }
    }
}

bool __cdecl R_LightImportanceGreaterEqual(const GfxLight *light0, const GfxLight *light1)
{
    float radiusSq; // [esp+1Ch] [ebp-10h]
    float radiusSq_4; // [esp+20h] [ebp-Ch]
    float distSq; // [esp+24h] [ebp-8h]
    float distSq_4; // [esp+28h] [ebp-4h]

    if ( light0->type != 3
        && light0->type != 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                    143,
                    1,
                    "%s",
                    "light0->type == GFX_LIGHT_TYPE_OMNI || light0->type == GFX_LIGHT_TYPE_SPOT") )
    {
        __debugbreak();
    }
    if ( light1->type != 3
        && light1->type != 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                    144,
                    1,
                    "%s",
                    "light1->type == GFX_LIGHT_TYPE_OMNI || light1->type == GFX_LIGHT_TYPE_SPOT") )
    {
        __debugbreak();
    }
    if ( light0->type != light1->type )
        return light0->type == 2;
    radiusSq = light0->radius * light0->radius;
    radiusSq_4 = light1->radius * light1->radius;
    distSq = Vec3DistanceSq(light0->origin, rg.viewOrg);
    distSq_4 = Vec3DistanceSq(light1->origin, rg.viewOrg);
    return (float)(radiusSq * distSq_4) >= (float)(radiusSq_4 * distSq);
}

void __cdecl R_CalcSpotLightPlanes(const GfxLight *light, float dynamicSpotLightNearPlaneOffset, float (*planes)[4])
{
    float radius; // [esp+14h] [ebp-48h]
    float origin[3]; // [esp+28h] [ebp-34h] BYREF
    float crossDirs[2][3]; // [esp+34h] [ebp-28h] BYREF
    float lightDirection[3]; // [esp+4Ch] [ebp-10h] BYREF
    float fSin; // [esp+58h] [ebp-4h]

    lightDirection[0] = -1.0 * light->dir[0];
    lightDirection[1] = -1.0 * light->dir[1];
    lightDirection[2] = -1.0 * light->dir[2];
    (*planes)[0] = -1.0 * lightDirection[0];
    (*planes)[1] = -1.0 * lightDirection[1];
    (*planes)[2] = -1.0 * lightDirection[2];
    origin[0] = (float)(dynamicSpotLightNearPlaneOffset * lightDirection[0]) + light->origin[0];
    origin[1] = (float)(dynamicSpotLightNearPlaneOffset * lightDirection[1]) + light->origin[1];
    origin[2] = (float)(dynamicSpotLightNearPlaneOffset * lightDirection[2]) + light->origin[2];
    ((*planes)[3]) = -((float)((float)((*planes)[0] * origin[0]) + (float)((*planes)[1] * origin[1])) + (float)((*planes)[2] * origin[2]));
    R_ComputeSpotLightCrossDirs(light, crossDirs);
    fSin = sqrtf(1.0 - (float)(light->cosHalfFovOuter * light->cosHalfFovOuter));
    R_CalcPlaneFromCosSinPointDirs(
        &(*planes)[4],
        light->cosHalfFovOuter,
        -fSin,
        light->origin,
        lightDirection,
        crossDirs[0]);
    R_CalcPlaneFromCosSinPointDirs(
        &(*planes)[8],
        light->cosHalfFovOuter,
        -fSin,
        light->origin,
        lightDirection,
        crossDirs[1]);
    R_CalcPlaneFromCosSinPointDirs(
        &(*planes)[12],
        -light->cosHalfFovOuter,
        -fSin,
        light->origin,
        lightDirection,
        crossDirs[0]);
    R_CalcPlaneFromCosSinPointDirs(
        &(*planes)[16],
        light->cosHalfFovOuter,
        -fSin,
        light->origin,
        lightDirection,
        crossDirs[1]);
    radius = light->radius;
    origin[0] = (float)(radius * lightDirection[0]) + light->origin[0];
    origin[1] = (float)(radius * lightDirection[1]) + light->origin[1];
    origin[2] = (float)(radius * lightDirection[2]) + light->origin[2];
    R_CalcPlaneFromPointDir(&(*planes)[20], origin, lightDirection);
}

void __cdecl R_CalcPlaneFromPointDir(float *plane, const float *origin, const float *dir)
{
    *plane = *dir;
    plane[1] = dir[1];
    plane[2] = dir[2];
    plane[3] = -((float)((float)(*origin * *dir) + (float)(origin[1] * dir[1])) + (float)(origin[2] * dir[2]));
}

void __cdecl R_ComputeSpotLightCrossDirs(const GfxLight *light, float (*crossDirs)[3])
{
    int axisIndex; // [esp+24h] [ebp-8h]
    int bestCrossAxis; // [esp+28h] [ebp-4h]

    bestCrossAxis = 0;
    for ( axisIndex = 1; axisIndex < 3; ++axisIndex )
    {
        if ( fabs(light->dir[bestCrossAxis]) > fabs(light->dir[axisIndex]) )
            bestCrossAxis = axisIndex;
    }
    (*crossDirs)[0] = 0.0f;
    (*crossDirs)[1] = 0.0f;
    (*crossDirs)[2] = 0.0f;
    (*crossDirs)[bestCrossAxis] = 1.0f;
    Vec3Cross(light->dir, (const float *)crossDirs, &(*crossDirs)[3]);
    Vec3Normalize(&(*crossDirs)[3]);
    Vec3Cross(&(*crossDirs)[3], light->dir, (float *)crossDirs);
    Vec3Normalize((float *)crossDirs);
}

void __cdecl R_CalcPlaneFromCosSinPointDirs(
                float *plane,
                float fCos,
                float fSin,
                const float *origin,
                const float *forward,
                const float *lateral)
{
    *plane = (float)(fCos * *lateral) + (float)(fSin * *forward);
    plane[1] = (float)(fCos * lateral[1]) + (float)(fSin * forward[1]);
    plane[2] = (float)(fCos * lateral[2]) + (float)(fSin * forward[2]);
    plane[3] = -((float)((float)(*plane * *origin) + (float)(plane[1] * origin[1])) + (float)(plane[2] * origin[2]));
}

void __cdecl R_GetBspLightSurfs(const GfxLight *visibleLights, int visibleCount)
{
    const GfxLight *light; // [esp+0h] [ebp-3Ch]
    GfxBspDrawSurfData surfData[2]; // [esp+4h] [ebp-38h] BYREF
    int lightIndex; // [esp+38h] [ebp-4h]

    if ( !visibleCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp", 586, 0, "%s", "visibleCount") )
    {
        __debugbreak();
    }
    if ( !rgp.world
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp", 587, 0, "%s", "rgp.world") )
    {
        __debugbreak();
    }
    R_InitBspDrawSurf((GfxSModelDrawSurfLightingData *)surfData);
    R_InitBspDrawSurf((GfxSModelDrawSurfLightingData *)&surfData[1]);
    for ( lightIndex = 0; lightIndex < visibleCount; ++lightIndex )
    {
        light = &visibleLights[lightIndex];
        if ( light->type != 3
            && light->type != 2
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                        596,
                        0,
                        "%s",
                        "light->type == GFX_LIGHT_TYPE_OMNI || light->type == GFX_LIGHT_TYPE_SPOT") )
        {
            __debugbreak();
        }
        if ( light->type != 3 )
            R_GetBspSpotLightSurfs(light, lightIndex, surfData);
    }
}

void __cdecl R_GetBspSpotLightSurfs(const GfxLight *light, int lightIndex, GfxBspDrawSurfData *surfData)
{
    unsigned __int16 triSurfList[2]; // [esp+90h] [ebp-48h] BYREF
    unsigned int surfIndex; // [esp+94h] [ebp-44h]
    float mins[3]; // [esp+98h] [ebp-40h] BYREF
    BspSpotLightCallback bspLightCallback; // [esp+A4h] [ebp-34h] BYREF
    unsigned __int8 *surfaceVisData; // [esp+A8h] [ebp-30h]
    GfxSurface **surfaces[2]; // [esp+ACh] [ebp-2Ch] BYREF
    float maxs[3]; // [esp+B4h] [ebp-24h] BYREF
    GfxDrawSurf *drawSurfs[2]; // [esp+C0h] [ebp-18h]
    GfxDrawSurf *surfaceMaterials; // [esp+C8h] [ebp-10h]
    unsigned int listSurfIndex; // [esp+CCh] [ebp-Ch]
    unsigned int surfCounts[2]; // [esp+D0h] [ebp-8h] BYREF

    if ( lightIndex >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                    484,
                    0,
                    "%s",
                    "lightIndex < MAX_VISIBLE_SHADOWABLE_DLIGHTS") )
    {
        __debugbreak();
    }
    surfaceVisData = rgp.world->dpvs.surfaceVisData[0];
    surfaceMaterials = rgp.world->dpvs.surfaceMaterials;
    mins[0] = light->origin[0] - light->radius;
    mins[1] = light->origin[1] - light->radius;
    mins[2] = light->origin[2] - light->radius;
    maxs[0] = light->origin[0] + light->radius;
    maxs[1] = light->origin[1] + light->radius;
    maxs[2] = light->origin[2] + light->radius;
    drawSurfs[1] = scene.visLightShadow[lightIndex].drawSurfs;
    surfaces[1] = (GfxSurface **)&drawSurfs[1][512];
    surfCounts[1] = 0;
    bspLightCallback.surfaceVisData = surfaceVisData;
    R_BoxSurfaces(mins, maxs, &allowSurf_0[1], &bspLightCallback, &surfaces[1], 0x400u, &surfCounts[1], 1u);
    scene.visLightShadow[lightIndex - 4].drawSurfCount = 0;
    if ( surfCounts[1] )
    {
        scene.visLightShadow[lightIndex].drawSurfCount = surfCounts[1];
        surfData[1].drawSurfList.current = drawSurfs[1];
        surfData[1].drawSurfList.end = (GfxDrawSurf *)&scene.visLightShadow[lightIndex + 1];

        //std::_Sort<Material * *,int,bool (__cdecl *)(Material const *,Material const *)>(
        //    (const GfxStaticModelDrawInst **)surfaces[1],
        //    (const GfxStaticModelDrawInst **)&surfaces[1][surfCounts[1]],
        //    (signed int)(4 * surfCounts[1]) >> 2,
        //    (bool (__cdecl *)(const GfxStaticModelDrawInst *, const GfxStaticModelDrawInst *))R_SortBspLightSurfaces);

        std::sort(&surfaces[1][0], &surfaces[1][surfCounts[1]], R_SortBspLightSurfaces);

        for ( listSurfIndex = 0; listSurfIndex < surfCounts[1]; ++listSurfIndex )
        {
            if ( listSurfIndex >= rgp.world->surfaceCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                            560,
                            0,
                            "listSurfIndex doesn't index rgp.world->surfaceCount\n\t%i not in [0, %i)",
                            listSurfIndex,
                            rgp.world->surfaceCount) )
            {
                __debugbreak();
            }
            surfIndex = surfaces[1][listSurfIndex] - rgp.world->dpvs.surfaces;
            triSurfList[0] = surfIndex;
            R_AddBspDrawSurfs(surfaceMaterials[surfIndex], (unsigned char*)triSurfList, 1u, surfData + 1);
        }
        R_EndCmdBuf(&surfData[1].delayedCmdBuf);
        scene.visLightShadow[lightIndex].drawSurfCount = surfData[1].drawSurfList.current
                                                                                                     - scene.visLightShadow[lightIndex].drawSurfs;
    }
    else
    {
        scene.visLightShadow[lightIndex].drawSurfCount = 0;
    }
}

bool __cdecl R_SortBspLightSurfaces(GfxSurface *surface0, GfxSurface *surface1)
{
    return surface0 < surface1;
}

int __cdecl R_AllowBspSpotLightShadows(int surfIndex, void *__formal)
{
    if ( r_spotLightShadows->current.enabled )
        return BoxInPlanes(
                         scene.dynamicSpotLightPlanes,
                         6u,
                         rgp.world->dpvs.surfaces[surfIndex].bounds[0],
                         rgp.world->dpvs.surfaces[surfIndex].bounds[1]);
    else
        return 0;
}

int __cdecl R_AllowBspSpotLight(int surfIndex, void *bspLightCallbackAsVoid)
{
    if (((const BspSpotLightCallback *)bspLightCallbackAsVoid)->surfaceVisData[surfIndex])
        return BoxInPlanes(
            scene.dynamicSpotLightPlanes,
            6u,
            rgp.world->dpvs.surfaces[surfIndex].bounds[0],
            rgp.world->dpvs.surfaces[surfIndex].bounds[1]);
    else
        return 0;
}

void __cdecl R_GetStaticModelLightSurfs(const GfxLight *visibleLights, int visibleCount)
{
    __int64 v2; // rax
    bool needsCharredTech; // [esp+1Fh] [ebp-889h]
    const GfxStaticModelDrawInst *smodelDrawInst; // [esp+24h] [ebp-884h]
    GfxDrawSurf drawSurf; // [esp+28h] [ebp-880h]
    float mins[3]; // [esp+34h] [ebp-874h] BYREF
    unsigned int surfaceIndex; // [esp+40h] [ebp-868h]
    const Material *material; // [esp+44h] [ebp-864h]
    GfxSModelDrawSurfData shadowSurfData; // [esp+48h] [ebp-860h] BYREF
    unsigned int surfaceCount; // [esp+64h] [ebp-844h]
    const GfxLight *light; // [esp+68h] [ebp-840h]
    GfxStaticModelId staticModelId; // [esp+6Ch] [ebp-83Ch]
    Material *const *pMaterial; // [esp+70h] [ebp-838h]
    unsigned __int16 list[2]; // [esp+74h] [ebp-834h] BYREF
    unsigned int *lodData; // [esp+78h] [ebp-830h]
    int lod; // [esp+7Ch] [ebp-82Ch]
    unsigned __int16 smodels[1026]; // [esp+80h] [ebp-828h] BYREF
    float maxs[3]; // [esp+888h] [ebp-20h] BYREF
    int smodelCount; // [esp+894h] [ebp-14h]
    XSurface *surfaces; // [esp+898h] [ebp-10h] BYREF
    int index; // [esp+89Ch] [ebp-Ch]
    int lightIndex; // [esp+8A0h] [ebp-8h]
    int smodelIndex; // [esp+8A4h] [ebp-4h]
    int savedregs; // [esp+8A8h] [ebp+0h] BYREF

    if ( !visibleCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp", 686, 0, "%s", "visibleCount") )
    {
        __debugbreak();
    }
    R_InitBspDrawSurf((GfxSModelDrawSurfLightingData *)&shadowSurfData);
    g_staticModelLightCallback.smodelVisData = rgp.world->dpvs.smodelVisData[0];
    lodData = rgp.world->dpvs.lodData;
    for ( lightIndex = 0; lightIndex < visibleCount; ++lightIndex )
    {
        light = &visibleLights[lightIndex];
        if ( light->type != 3
            && light->type != 2
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                        700,
                        1,
                        "%s",
                        "light->type == GFX_LIGHT_TYPE_OMNI || light->type == GFX_LIGHT_TYPE_SPOT") )
        {
            __debugbreak();
        }
        if ( light->type == 2 && r_spotLightShadows->current.enabled && r_spotLightSModelShadows->current.enabled )
        {
            mins[0] = light->origin[0] - light->radius;
            mins[1] = light->origin[1] - light->radius;
            mins[2] = light->origin[2] - light->radius;
            maxs[0] = light->origin[0] + light->radius;
            maxs[1] = light->origin[1] + light->radius;
            maxs[2] = light->origin[2] + light->radius;
            if ( light->type == 3 )
            {
                g_staticModelLightCallback.position[0] = light->origin[0];
                g_staticModelLightCallback.position[1] = light->origin[1];
                g_staticModelLightCallback.position[2] = light->origin[2];
                g_staticModelLightCallback.radiusSq = light->radius * light->radius;
                smodelCount = R_BoxStaticModels(mins, maxs, R_AllowStaticModelOmniLight, smodels, 1024);
            }
            else if ( (unsigned int)lightIndex >= 2 )
            {
                smodelCount = 0;
            }
            else
            {
                shadowSurfData.drawSurfList.current = &scene.visLightShadow[lightIndex].drawSurfs[scene.visLightShadow[lightIndex].drawSurfCount];
                shadowSurfData.drawSurfList.end = (GfxDrawSurf *)&scene.visLightShadow[lightIndex + 1];
                smodelCount = R_BoxStaticModels(mins, maxs, R_AllowStaticModelSpotLight, smodels, 1024);
            }
            for ( index = 0; index < smodelCount; ++index )
            {
                smodelIndex = smodels[index];
                smodelDrawInst = &rgp.world->dpvs.smodelDrawInsts[smodelIndex];
                lod = (lodData[smodelIndex >> 4] >> (2 * (smodelIndex & 0xF))) & 3;
                surfaceCount = XModelGetSurfaces(smodelDrawInst->model, &surfaces, lod);
                if ( !surfaceCount
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp", 749, 0, "%s", "surfaceCount") )
                {
                    __debugbreak();
                }
                staticModelId = R_GetStaticModelId(smodelIndex, lod);
                pMaterial = XModelGetSkins(smodelDrawInst->model, lod);
                if ( !pMaterial
                    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp", 756, 0, "%s", "pMaterial") )
                {
                    __debugbreak();
                }
                list[0] = staticModelId.objectId;
                needsCharredTech = R_StaticModelNeedsCharredTech(smodelIndex);
                surfaceIndex = 0;
                while ( surfaceIndex < surfaceCount )
                {
                    material = *pMaterial;
                    if ( !material
                        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp", 771, 0, "%s", "material") )
                    {
                        __debugbreak();
                    }
                    iassert(rgp.sortedMaterials[material->info.drawSurf.fields.materialSortedIndex] == material);

                    if ( (material->info.gameFlags & 1) == 0 && Material_GetTechnique(material, 0x74u) )
                    {
                        //drawSurf.fields = (GfxDrawSurfFields)material->info.drawSurf;
                        drawSurf.packed = material->info.drawSurf.packed;
                        if ( needsCharredTech )
                        {
                            HIDWORD(drawSurf.packed) = HIDWORD(drawSurf.packed) & 0xFE7FFFFF | 0x1000000;
                            v2 = ((drawSurf.packed >> 20) & 0x1D | 2) << 20;
                        }
                        else
                        {
                            v2 = ((drawSurf.packed >> 20) & 0x1D) << 20;
                        }
                        *(unsigned int *)&drawSurf.fields = v2 | *(unsigned int *)&drawSurf.fields & 0xFE0FFFFF;
                        HIDWORD(drawSurf.packed) = ((staticModelId.surfType & 0xF) << 19)
                                                                         | (HIDWORD(v2) | HIDWORD(drawSurf.packed)) & 0xFF87FFFF;
                        if ( light->type == 2 && r_spotLightShadows->current.enabled && r_spotLightSModelShadows->current.enabled )
                        {
                            if ( !R_AllocDrawSurf(&shadowSurfData.delayedCmdBuf, drawSurf, &shadowSurfData.drawSurfList, 2u + R_PRIM_PTR_WORDS) )
                                break;
                            R_AddDelayedStaticModelDrawSurf(
                                &shadowSurfData.delayedCmdBuf,
                                &surfaces[surfaceIndex],
                                (unsigned __int8 *)list,
                                1u,
                                lod);
                        }
                    }
                    ++surfaceIndex;
                    ++pMaterial;
                }
            }
            if ( (unsigned int)lightIndex >= 4
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                            818,
                            0,
                            "lightIndex doesn't index ARRAY_COUNT(scene.visLight)\n\t%i not in [0, %i)",
                            lightIndex,
                            4) )
            {
                __debugbreak();
            }
            if ( (unsigned int)lightIndex < 2 && light->type == 2 )
            {
                R_EndCmdBuf(&shadowSurfData.delayedCmdBuf);
                scene.visLightShadow[lightIndex].drawSurfCount = shadowSurfData.drawSurfList.current
                                                                                                             - scene.visLightShadow[lightIndex].drawSurfs;
            }
        }
    }
}

int __cdecl R_AllocDrawSurf(
                GfxDelayedCmdBuf *delayedCmdBuf,
                GfxDrawSurf drawSurf,
                GfxDrawSurfList *drawSurfList,
                unsigned int size)
{
    unsigned int primDrawSurfPos; // [esp+10h] [ebp-4h]

    if ( size >= 0x10000
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_cmdbuf.h",
                    80,
                    0,
                    "%s\n\t(size) = %i",
                    "(size < (128 * 512))",
                    size) )
    {
        __debugbreak();
    }
    if ( delayedCmdBuf->drawSurfKey.packed != drawSurf.packed )
        R_EndCmdBuf(delayedCmdBuf);
    if ( delayedCmdBuf->primDrawSurfSize > size )
    {
        primDrawSurfPos = delayedCmdBuf->primDrawSurfPos;
    }
    else
    {
        R_EndCmdBuf(delayedCmdBuf);
        primDrawSurfPos = _InterlockedExchangeAdd(&frontEndDataOut->primDrawSurfPos, 0x200u);
        if ( primDrawSurfPos >= 0x10000 )
        {
            delayedCmdBuf->primDrawSurfSize = 0;
            R_WarnOncePerFrame(R_WARN_PRIM_DRAW_SURF_BUFFER_SIZE);
            return 0;
        }
        delayedCmdBuf->primDrawSurfPos = primDrawSurfPos;
        delayedCmdBuf->primDrawSurfSize = 512;
    }
    if ( delayedCmdBuf->drawSurfKey.packed == drawSurf.packed )
        return 1;
    if ( drawSurfList->current < drawSurfList->end )
    {
        delayedCmdBuf->drawSurfKey = drawSurf;
        if ( primDrawSurfPos >= 0x10000
            && !Assert_MyHandler(
                        "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_cmdbuf.h",
                        146,
                        0,
                        "primDrawSurfPos doesn't index 1 << MTL_SORT_OBJECT_ID_BITS\n\t%i not in [0, %i)",
                        primDrawSurfPos,
                        0x10000) )
        {
            __debugbreak();
        }
        *(unsigned int *)&drawSurf.fields = (unsigned __int16)primDrawSurfPos | *(unsigned int *)&drawSurf.fields & 0xFFFF0000;
        drawSurfList->current->fields = drawSurf.fields;
        ++drawSurfList->current;
        return 1;
    }
    else
    {
        R_WarnOncePerFrame(R_WARN_MAX_SCENE_DRAWSURFS, "R_AllocDrawSurf");
        return 0;
    }
}

int __cdecl R_AllowStaticModelOmniLight(int smodelIndex)
{
    return g_staticModelLightCallback.smodelVisData[smodelIndex]
            && g_staticModelLightCallback.radiusSq >= PointToBoxDistSq(
                                                                                                    g_staticModelLightCallback.position,
                                                                                                    rgp.world->dpvs.smodelInsts[smodelIndex].mins,
                                                                                                    rgp.world->dpvs.smodelInsts[smodelIndex].maxs);
}

int __cdecl R_AllowStaticModelSpotLight(int smodelIndex)
{
    if ( g_staticModelLightCallback.smodelVisData[smodelIndex] )
        return BoxInPlanes(
                         scene.dynamicSpotLightPlanes,
                         6u,
                         rgp.world->dpvs.smodelInsts[smodelIndex].mins,
                         rgp.world->dpvs.smodelInsts[smodelIndex].maxs);
    else
        return 0;
}

void __cdecl R_GetSceneEntLightSurfs(const GfxLight *visibleLights, int visibleCount)
{
    XModel *dynEntXModel; // [esp+64h] [ebp-64h]
    GfxSceneDynBrush *sceneDynBrush; // [esp+6Ch] [ebp-5Ch]
    DynEntityPose *dynEntPose; // [esp+70h] [ebp-58h]
    GfxSceneModel *sceneModel; // [esp+78h] [ebp-50h]
    volatile unsigned int sceneEntCount; // [esp+84h] [ebp-44h]
    volatile unsigned int sceneEntCounta; // [esp+84h] [ebp-44h]
    unsigned int sceneEntCountb; // [esp+84h] [ebp-44h]
    volatile unsigned int sceneEntCountc; // [esp+84h] [ebp-44h]
    unsigned int sceneEntCountd; // [esp+84h] [ebp-44h]
    const GfxLight *light; // [esp+88h] [ebp-40h]
    const GfxLight *lighta; // [esp+88h] [ebp-40h]
    const GfxLight *lightb; // [esp+88h] [ebp-40h]
    const GfxLight *lightc; // [esp+88h] [ebp-40h]
    const GfxLight *lightd; // [esp+88h] [ebp-40h]
    float radius; // [esp+8Ch] [ebp-3Ch]
    float radiusa; // [esp+8Ch] [ebp-3Ch]
    DynEntityClient *dynEntClient; // [esp+90h] [ebp-38h]
    const DynEntityDef *dynEntDef; // [esp+94h] [ebp-34h]
    const DynEntityDef *dynEntDefa; // [esp+94h] [ebp-34h]
    unsigned int visLightShadowDrawSurfCount; // [esp+9Ch] [ebp-2Ch]
    unsigned int visLightShadowDrawSurfCounta; // [esp+9Ch] [ebp-2Ch]
    unsigned int visLightShadowDrawSurfCountb; // [esp+9Ch] [ebp-2Ch]
    unsigned int visLightShadowDrawSurfCountc; // [esp+9Ch] [ebp-2Ch]
    unsigned int visLightShadowDrawSurfCountd; // [esp+9Ch] [ebp-2Ch]
    unsigned int sceneEntIndex; // [esp+A0h] [ebp-28h]
    unsigned int sceneEntIndexa; // [esp+A0h] [ebp-28h]
    unsigned int sceneEntIndexb; // [esp+A0h] [ebp-28h]
    unsigned int sceneEntIndexc; // [esp+A0h] [ebp-28h]
    unsigned int sceneEntIndexd; // [esp+A0h] [ebp-28h]
    GfxSceneEntity *sceneEnt; // [esp+A4h] [ebp-24h]
    unsigned __int16 dynEntId; // [esp+ACh] [ebp-1Ch]
    unsigned __int16 dynEntIda; // [esp+ACh] [ebp-1Ch]
    unsigned __int8 *sceneEntVisData; // [esp+B0h] [ebp-18h]
    unsigned __int8 *sceneEntVisDataa; // [esp+B0h] [ebp-18h]
    int lightIndex; // [esp+B4h] [ebp-14h]
    int lightIndexa; // [esp+B4h] [ebp-14h]
    int lightIndexb; // [esp+B4h] [ebp-14h]
    int lightIndexc; // [esp+B4h] [ebp-14h]
    int lightIndexd; // [esp+B4h] [ebp-14h]
    GfxSceneDynModel *sceneDynModel; // [esp+B8h] [ebp-10h]
    float distSq; // [esp+BCh] [ebp-Ch]
    float distSqa; // [esp+BCh] [ebp-Ch]
    float distSqb; // [esp+BCh] [ebp-Ch]
    float distSqc; // [esp+BCh] [ebp-Ch]
    float distSqd; // [esp+BCh] [ebp-Ch]
    const GfxBrushModel *bmodel; // [esp+C0h] [ebp-8h]
    const GfxBrushModel *bmodela; // [esp+C0h] [ebp-8h]

    if ( visibleCount > 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                    893,
                    0,
                    "%s\n\t(visibleCount) = %i",
                    "(visibleCount <= 4)",
                    visibleCount) )
    {
        __debugbreak();
    }
    sceneEntCount = scene.sceneDObjCount;
    for ( sceneEntIndex = 0; sceneEntIndex < sceneEntCount; ++sceneEntIndex )
    {
        if ( scene.sceneDObjVisData[0][sceneEntIndex] == 1 )
        {
            sceneEnt = &scene.sceneDObj[sceneEntIndex];
            if ( sceneEnt->cull.state < 2
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                            904,
                            0,
                            "sceneEnt->cull.state >= CULL_STATE_BOUNDED\n\t%i, %i",
                            sceneEnt->cull.state,
                            2) )
            {
                __debugbreak();
            }
            lightIndex = 0;
LABEL_13:
            if ( lightIndex >= visibleCount )
                continue;
            light = &visibleLights[lightIndex];
            if ( light->type != 3
                && light->type != 2
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                            911,
                            1,
                            "%s",
                            "light->type == GFX_LIGHT_TYPE_OMNI || light->type == GFX_LIGHT_TYPE_SPOT") )
            {
                __debugbreak();
            }
            if ( light->type != 2 || !r_spotLightShadows->current.enabled || !r_spotLightEntityShadows->current.enabled )
            {
LABEL_12:
                ++lightIndex;
                goto LABEL_13;
            }
            if ( light->type == 3 )
            {
                distSq = PointToBoxDistSq(light->origin, sceneEnt->cull.mins, sceneEnt->cull.maxs);
                if ( distSq > (float)(light->radius * light->radius) )
                    goto LABEL_12;
            }
            else
            {
                if ( R_SpotLightIsAttachedToDobj(sceneEnt->obj) )
                    goto LABEL_12;
                if ( light->type != 2
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                                932,
                                0,
                                "%s",
                                "light->type == GFX_LIGHT_TYPE_SPOT") )
                {
                    __debugbreak();
                }
                if ( !BoxInPlanes(scene.dynamicSpotLightPlanes, 6u, sceneEnt->cull.mins, sceneEnt->cull.maxs)
                    || (frontEndDataOut->gfxEnts[sceneEnt->gfxEntIndex].renderFxFlags & 0x10) != 0 )
                {
                    goto LABEL_12;
                }
            }
            if ( (!DObjIsPlayerShadow(sceneEnt->obj) || !rg.enablePlayerShadowFlag)
                && light->type == 2
                && r_spotLightShadows->current.enabled
                && r_spotLightEntityShadows->current.enabled
                && (frontEndDataOut->gfxEnts[sceneEnt->gfxEntIndex].renderFxFlags & 1) == 0 )
            {
                if ( (unsigned int)lightIndex >= 2
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                                958,
                                0,
                                "lightIndex doesn't index MAX_VISIBLE_SHADOWABLE_DLIGHTS\n\t%i not in [0, %i)",
                                lightIndex,
                                2) )
                {
                    __debugbreak();
                }
                visLightShadowDrawSurfCount = scene.visLightShadow[lightIndex].drawSurfCount;
                scene.visLightShadow[lightIndex].drawSurfCount = visLightShadowDrawSurfCount
                                                                                                             + R_AddDObjSurfaces(
                                                                                                                     sceneEnt,
                                                                                                                     0x74u,
                                                                                                                     &scene.visLightShadow[lightIndex].drawSurfs[visLightShadowDrawSurfCount],
                                                                                                                     (GfxDrawSurf *)&scene.visLightShadow[lightIndex + 1])
                                                                                                             - &scene.visLightShadow[lightIndex].drawSurfs[visLightShadowDrawSurfCount];
            }
            goto LABEL_12;
        }
    }
    sceneEntCounta = scene.sceneModelCount;
    for ( sceneEntIndexa = 0; sceneEntIndexa < sceneEntCounta; ++sceneEntIndexa )
    {
        if ( scene.sceneModelVisData[0][sceneEntIndexa] == 1 )
        {
            sceneModel = &scene.sceneModel[sceneEntIndexa];
            for ( lightIndexa = 0; lightIndexa < visibleCount; ++lightIndexa )
            {
                lighta = &visibleLights[lightIndexa];
                if ( lighta->type != 3
                    && lighta->type != 2
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                                983,
                                1,
                                "%s",
                                "light->type == GFX_LIGHT_TYPE_OMNI || light->type == GFX_LIGHT_TYPE_SPOT") )
                {
                    __debugbreak();
                }
                if ( lighta->type == 2 && r_spotLightShadows->current.enabled && r_spotLightEntityShadows->current.enabled )
                {
                    if ( lighta->type == 3 )
                    {
                        distSqa = Vec3DistanceSq(lighta->origin, sceneModel->placement.base.origin);
                        radius = lighta->radius + sceneModel->radius;
                        if ( distSqa > (float)(radius * radius) )
                            continue;
LABEL_65:
                        if ( lighta->type == 2 && r_spotLightShadows->current.enabled && r_spotLightEntityShadows->current.enabled )
                        {
                            if ( (unsigned int)lightIndexa >= 2
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                                            1017,
                                            0,
                                            "lightIndex doesn't index MAX_VISIBLE_SHADOWABLE_DLIGHTS\n\t%i not in [0, %i)",
                                            lightIndexa,
                                            2) )
                            {
                                __debugbreak();
                            }
                            visLightShadowDrawSurfCounta = scene.visLightShadow[lightIndexa].drawSurfCount;
                            scene.visLightShadow[lightIndexa].drawSurfCount = visLightShadowDrawSurfCounta
                                                                                                                            + R_AddXModelSurfaces(
                                                                                                                                    &sceneModel->info,
                                                                                                                                    sceneModel->model,
                                                                                                                                    0x74u,
                                                                                                                                    &scene.visLightShadow[lightIndexa].drawSurfs[visLightShadowDrawSurfCounta],
                                                                                                                                    (GfxDrawSurf *)&scene.visLightShadow[lightIndexa + 1])
                                                                                                                            - &scene.visLightShadow[lightIndexa].drawSurfs[visLightShadowDrawSurfCounta];
                        }
                        continue;
                    }
                    if ( lighta->type != 2
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                                    1000,
                                    0,
                                    "%s",
                                    "light->type == GFX_LIGHT_TYPE_SPOT") )
                    {
                        __debugbreak();
                    }
                    if ( SphereInPlanes(scene.dynamicSpotLightPlanes, 6u, sceneModel->placement.base.origin, sceneModel->radius) )
                        goto LABEL_65;
                }
            }
        }
    }
    sceneEntCountb = scene.sceneDynModelCount;
    sceneEntVisData = rgp.world->dpvsDyn.dynEntVisData[0][0];
    for ( sceneEntIndexb = 0; sceneEntIndexb < sceneEntCountb; ++sceneEntIndexb )
    {
        sceneDynModel = &rgp.world->sceneDynModel[sceneEntIndexb];
        dynEntId = sceneDynModel->dynEntId;
        if ( sceneEntVisData[dynEntId] == 1 )
        {
            dynEntPose = DynEnt_GetClientPose(dynEntId, DYNENT_DRAW_MODEL);
            dynEntDef = DynEnt_GetEntityDef(dynEntId, DYNENT_DRAW_MODEL);
            dynEntClient = DynEnt_GetClientEntity(dynEntId, DYNENT_DRAW_MODEL);
            dynEntXModel = DynEntCl_GetCurrentXModel(dynEntDef, dynEntClient);
            for ( lightIndexb = 0; lightIndexb < visibleCount; ++lightIndexb )
            {
                lightb = &visibleLights[lightIndexb];
                if ( lightb->type != 3
                    && lightb->type != 2
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                                1049,
                                1,
                                "%s",
                                "light->type == GFX_LIGHT_TYPE_OMNI || light->type == GFX_LIGHT_TYPE_SPOT") )
                {
                    __debugbreak();
                }
                if ( lightb->type == 2 && r_spotLightShadows->current.enabled && r_spotLightEntityShadows->current.enabled )
                {
                    if ( lightb->type == 3 )
                    {
                        distSqb = Vec3DistanceSq(lightb->origin, dynEntPose->pose.origin);
                        radiusa = lightb->radius + dynEntPose->radius;
                        if ( distSqb > (float)(radiusa * radiusa) )
                            continue;
LABEL_96:
                        if ( lightb->type == 2 && r_spotLightShadows->current.enabled && r_spotLightEntityShadows->current.enabled )
                        {
                            if ( (unsigned int)lightIndexb >= 2
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                                            1083,
                                            0,
                                            "lightIndex doesn't index MAX_VISIBLE_SHADOWABLE_DLIGHTS\n\t%i not in [0, %i)",
                                            lightIndexb,
                                            2) )
                            {
                                __debugbreak();
                            }
                            visLightShadowDrawSurfCountb = scene.visLightShadow[lightIndexb].drawSurfCount;
                            scene.visLightShadow[lightIndexb].drawSurfCount = visLightShadowDrawSurfCountb
                                                                                                                            + R_AddXModelSurfaces(
                                                                                                                                    &sceneDynModel->info,
                                                                                                                                    dynEntXModel,
                                                                                                                                    0x74u,
                                                                                                                                    &scene.visLightShadow[lightIndexb].drawSurfs[visLightShadowDrawSurfCountb],
                                                                                                                                    (GfxDrawSurf *)&scene.visLightShadow[lightIndexb + 1])
                                                                                                                            - &scene.visLightShadow[lightIndexb].drawSurfs[visLightShadowDrawSurfCountb];
                        }
                        continue;
                    }
                    if ( lightb->type != 2
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                                    1066,
                                    0,
                                    "%s",
                                    "light->type == GFX_LIGHT_TYPE_SPOT") )
                    {
                        __debugbreak();
                    }
                    if ( SphereInPlanes(scene.dynamicSpotLightPlanes, 6u, dynEntPose->pose.origin, dynEntPose->radius) )
                        goto LABEL_96;
                }
            }
        }
    }
    sceneEntCountc = scene.sceneBrushCount;
    for ( sceneEntIndexc = 0; sceneEntIndexc < sceneEntCountc; ++sceneEntIndexc )
    {
        if ( scene.sceneBrushVisData[0][sceneEntIndexc] == 1 )
        {
            bmodel = scene.sceneBrush[sceneEntIndexc].bmodel;
            for ( lightIndexc = 0; lightIndexc < visibleCount; ++lightIndexc )
            {
                lightc = &visibleLights[lightIndexc];
                if ( lightc->type != 3
                    && lightc->type != 2
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                                1111,
                                1,
                                "%s",
                                "light->type == GFX_LIGHT_TYPE_OMNI || light->type == GFX_LIGHT_TYPE_SPOT") )
                {
                    __debugbreak();
                }
                if ( lightc->type == 2 && r_spotLightShadows->current.enabled && r_spotLightEntityShadows->current.enabled )
                {
                    if ( lightc->type == 3 )
                    {
                        distSqc = PointToBoxDistSq(lightc->origin, bmodel->writable.mins, bmodel->writable.maxs);
                        if ( distSqc > (float)(lightc->radius * lightc->radius) )
                            continue;
LABEL_127:
                        if ( lightc->type == 2 && r_spotLightShadows->current.enabled && r_spotLightEntityShadows->current.enabled )
                        {
                            if ( (unsigned int)lightIndexc >= 2
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                                            1144,
                                            0,
                                            "lightIndex doesn't index MAX_VISIBLE_SHADOWABLE_DLIGHTS\n\t%i not in [0, %i)",
                                            lightIndexc,
                                            2) )
                            {
                                __debugbreak();
                            }
                            visLightShadowDrawSurfCountc = scene.visLightShadow[lightIndexc].drawSurfCount;
                            scene.visLightShadow[lightIndexc].drawSurfCount = visLightShadowDrawSurfCountc
                                                                                                                            + R_AddBModelSurfaces(
                                                                                                                                    &scene.sceneBrush[sceneEntIndexc].info,
                                                                                                                                    scene.sceneBrush[sceneEntIndexc].bmodel,
                                                                                                                                    0x74u,
                                                                                                                                    &scene.visLightShadow[lightIndexc].drawSurfs[visLightShadowDrawSurfCountc],
                                                                                                                                    (GfxDrawSurf *)&scene.visLightShadow[lightIndexc + 1])
                                                                                                                            - &scene.visLightShadow[lightIndexc].drawSurfs[visLightShadowDrawSurfCountc];
                        }
                        continue;
                    }
                    if ( lightc->type != 2
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                                    1127,
                                    0,
                                    "%s",
                                    "light->type == GFX_LIGHT_TYPE_SPOT") )
                    {
                        __debugbreak();
                    }
                    if ( BoxInPlanes(scene.dynamicSpotLightPlanes, 6u, bmodel->writable.mins, bmodel->writable.maxs) )
                        goto LABEL_127;
                }
            }
        }
    }
    sceneEntCountd = scene.sceneDynBrushCount;
    sceneEntVisDataa = rgp.world->dpvsDyn.dynEntVisData[1][0];
    for ( sceneEntIndexd = 0; sceneEntIndexd < sceneEntCountd; ++sceneEntIndexd )
    {
        sceneDynBrush = &rgp.world->sceneDynBrush[sceneEntIndexd];
        dynEntIda = sceneDynBrush->dynEntId;
        if ( sceneEntVisDataa[dynEntIda] == 1 )
        {
            dynEntDefa = DynEnt_GetEntityDef(dynEntIda, DYNENT_DRAW_BRUSH);
            bmodela = R_GetBrushModel(dynEntDefa->brushModel);
            for ( lightIndexd = 0; lightIndexd < visibleCount; ++lightIndexd )
            {
                lightd = &visibleLights[lightIndexd];
                if ( lightd->type != 3
                    && lightd->type != 2
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                                1174,
                                1,
                                "%s",
                                "light->type == GFX_LIGHT_TYPE_OMNI || light->type == GFX_LIGHT_TYPE_SPOT") )
                {
                    __debugbreak();
                }
                if ( lightd->type == 2 && r_spotLightShadows->current.enabled && r_spotLightEntityShadows->current.enabled )
                {
                    if ( lightd->type == 3 )
                    {
                        distSqd = PointToBoxDistSq(lightd->origin, bmodela->writable.mins, bmodela->writable.maxs);
                        if ( distSqd > (float)(lightd->radius * lightd->radius) )
                            continue;
LABEL_158:
                        if ( lightd->type == 2 && r_spotLightShadows->current.enabled && r_spotLightEntityShadows->current.enabled )
                        {
                            if ( (unsigned int)lightIndexd >= 2
                                && !Assert_MyHandler(
                                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                                            1208,
                                            0,
                                            "lightIndex doesn't index MAX_VISIBLE_SHADOWABLE_DLIGHTS\n\t%i not in [0, %i)",
                                            lightIndexd,
                                            2) )
                            {
                                __debugbreak();
                            }
                            visLightShadowDrawSurfCountd = scene.visLightShadow[lightIndexd].drawSurfCount;
                            scene.visLightShadow[lightIndexd].drawSurfCount = visLightShadowDrawSurfCountd
                                                                                                                            + R_AddBModelSurfaces(
                                                                                                                                    (BModelDrawInfo *)sceneDynBrush,
                                                                                                                                    bmodela,
                                                                                                                                    0x74u,
                                                                                                                                    &scene.visLightShadow[lightIndexd].drawSurfs[visLightShadowDrawSurfCountd],
                                                                                                                                    (GfxDrawSurf *)&scene.visLightShadow[lightIndexd + 1])
                                                                                                                            - &scene.visLightShadow[lightIndexd].drawSurfs[visLightShadowDrawSurfCountd];
                        }
                        continue;
                    }
                    if ( lightd->type != 2
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                                    1190,
                                    0,
                                    "%s",
                                    "light->type == GFX_LIGHT_TYPE_SPOT") )
                    {
                        __debugbreak();
                    }
                    if ( BoxInPlanes(scene.dynamicSpotLightPlanes, 6u, bmodela->writable.mins, bmodela->writable.maxs) )
                        goto LABEL_158;
                }
            }
        }
    }
}

bool __cdecl R_SpotLightIsAttachedToDobj(const DObj *obj)
{
    DObj *attachedDobj; // [esp+4h] [ebp-8h]
    int spotlightDobjHandle; // [esp+8h] [ebp-4h] BYREF

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp", 840, 0, "%s", "obj") )
        __debugbreak();
    spotlightDobjHandle = -1;
    if ( !FX_GetActiveSpotLightBoltDobj(0, &spotlightDobjHandle) )
        return 0;
    attachedDobj = Com_GetClientDObj(spotlightDobjHandle, 0);
    return attachedDobj && attachedDobj == obj;
}

int __cdecl R_EmitPointLightPartitionSurfs(
                GfxViewInfo *viewInfo,
                const GfxLight *visibleLights,
                int visibleCount,
                const float *viewOrigin)
{
    int firstDrawSurf; // [esp+Ch] [ebp-28h]
    PointLightPartition *partitions; // [esp+10h] [ebp-24h]
    const GfxLight *light; // [esp+18h] [ebp-1Ch]
    int partitionCount; // [esp+20h] [ebp-14h]
    unsigned int lightDrawSurfCount; // [esp+24h] [ebp-10h]
    int lightIndex; // [esp+28h] [ebp-Ch]
    PointLightPartition *partition; // [esp+2Ch] [ebp-8h]
    int drawSurfCount; // [esp+30h] [ebp-4h]

    partitions = viewInfo->pointLightPartitions;
    partitionCount = 0;
    for ( lightIndex = 0; lightIndex < visibleCount; ++lightIndex )
    {
        light = &visibleLights[lightIndex];
        lightDrawSurfCount = scene.visLightShadow[lightIndex - 4].drawSurfCount;
        R_ReverseSortDrawSurfs(scene.visLight[lightIndex].drawSurfs, lightDrawSurfCount);
        partition = &partitions[partitionCount];
        R_InitDrawSurfListInfo(&partition->info);
        partition->info.baseTechType = R_GetTechniqueForLightType(light, viewInfo);
        partition->info.viewInfo = viewInfo;
        partition->info.viewOrigin[0] = *viewOrigin;
        partition->info.viewOrigin[1] = viewOrigin[1];
        partition->info.viewOrigin[2] = viewOrigin[2];
        partition->info.viewOrigin[3] = viewOrigin[3];
        partition->info.light = &partition->light;
        partition->info.cameraView = 1;
        firstDrawSurf = frontEndDataOut->drawSurfCount;
        R_EmitDrawSurfList(scene.visLight[lightIndex].drawSurfs, lightDrawSurfCount, frontEndDataOut);
        drawSurfCount = frontEndDataOut->drawSurfCount - firstDrawSurf;
        if ( drawSurfCount )
        {
            memcpy(partition, light, sizeof(*light));   // nx-port: was 0x170 (x86)
            partition->info.drawSurfs = &frontEndDataOut->drawSurfs[firstDrawSurf];
            partitions[partitionCount++].info.drawSurfCount = drawSurfCount;
        }
    }
    return partitionCount;
}

unsigned __int8 __cdecl R_GetTechniqueForLightType(const GfxLight *light, const GfxViewInfo *viewInfo)
{
    int PrimaryLightCount; // eax
    const char *v4; // eax

    if ( !viewInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp", 1224, 0, "%s", "viewInfo") )
    {
        __debugbreak();
    }
    if ( light->type == 2 )
    {
        if ( r_spotLightShadows->current.enabled
            && (PrimaryLightCount = Com_GetPrimaryLightCount(),
                    Com_BitCheckAssert(frontEndDataOut->shadowableLightHasShadowMap, PrimaryLightCount, 32)) )
        {
            if ( frontEndDataOut->emissiveSpotLightCount != 1
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                            1236,
                            0,
                            "%s",
                            "frontEndDataOut->emissiveSpotLightCount == 1") )
            {
                __debugbreak();
            }
            if ( Com_GetPrimaryLightCount() + 1 != frontEndDataOut->shadowableLightCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp",
                            1237,
                            0,
                            "%s",
                            "Com_GetPrimaryLightCount() + GFX_MAX_EMISSIVE_SPOT_LIGHTS == frontEndDataOut->shadowableLightCount") )
            {
                __debugbreak();
            }
            return 117;
        }
        else
        {
            return 115;
        }
    }
    else if ( light->type == 3 )
    {
        return 116;
    }
    else
    {
        v4 = va("Dynamic light type %d isn't supported.", light->type);
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_light.cpp", 1250, 0, v4) )
            __debugbreak();
        return 116;
    }
}

void __cdecl R_EmitShadowedLightPartitionSurfs(
                GfxViewInfo *viewInfo,
                unsigned int lightDrawSurfCount,
                const GfxDrawSurf *lightDrawSurfs,
                GfxDrawSurfListInfo *info)
{
    int firstDrawSurf; // [esp+0h] [ebp-8h]
    unsigned int drawSurfCount; // [esp+4h] [ebp-4h]

    firstDrawSurf = frontEndDataOut->drawSurfCount;
    R_EmitDrawSurfList(lightDrawSurfs, lightDrawSurfCount, frontEndDataOut);
    drawSurfCount = frontEndDataOut->drawSurfCount - firstDrawSurf;
    info->drawSurfs = &frontEndDataOut->drawSurfs[firstDrawSurf];
    info->drawSurfCount = drawSurfCount;
}

