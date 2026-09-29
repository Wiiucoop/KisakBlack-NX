#include "rb_tess.h"
#include "r_dvars.h"
#include "r_debug.h"
#include "r_shade.h"
#include "r_state.h"
#include "rb_stats.h"
#include "r_foliage.h"
#include "r_model_lighting.h"
#include "rb_logfile.h"
#include "rb_shade.h"
#include "r_state_utils.h"
#include "r_xsurface.h"
#include "r_draw_material.h"
#include "r_draw_xmodel.h"
#include <ik/ik_math.h>
#include "r_water_sim.h"
#include "r_draw_staticmodel.h"
#include "r_draw_bsp.h"
#include "rb_draw3d.h"

GfxScaledPlacement s_manualObjectPlacement;

void __cdecl RB_ShowTess(GfxCmdBufContext context, const float *center, const char *tessName, const float *color)
{
    const char *v4; // eax
    char *v5; // eax
    const GfxBackEndData *data; // [esp+14h] [ebp-2Ch]
    const char *infoIdString; // [esp+1Ch] [ebp-24h]
    float offsetCenter[3]; // [esp+24h] [ebp-1Ch] BYREF
    const MaterialTechnique *tech; // [esp+30h] [ebp-10h]
    const MaterialTechniqueSet *techSet; // [esp+34h] [ebp-Ch]
    float TEXT_SIZE; // [esp+38h] [ebp-8h]
    const char *infoString; // [esp+3Ch] [ebp-4h]

    TEXT_SIZE = 0.6f;
    if ( !center && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp", 94, 0, "%s", "center") )
        __debugbreak();
    if ( !tessName
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp", 95, 0, "%s", "tessName") )
    {
        __debugbreak();
    }
    offsetCenter[0] = *center;
    offsetCenter[1] = center[1];
    offsetCenter[2] = center[2];
    if ( !context.state->material
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                    99,
                    0,
                    "%s",
                    "context.state->material") )
    {
        __debugbreak();
    }
    tech = Material_GetTechnique(context.state->material, context.state->techType);
    if ( !tech && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp", 102, 0, "%s", "tech") )
        __debugbreak();
    switch ( r_showTess->current.integer )
    {
        case 1:
            infoString = tech->name;
            offsetCenter[2] = (float)((float)((float)context.state->techType - 16.0) * 0.30000001) + offsetCenter[2];
            infoIdString = "T";
            break;
        case 2:
            techSet = Material_GetTechniqueSet(context.state->material);
            if ( !techSet
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp", 134, 0, "%s", "techSet") )
            {
                __debugbreak();
            }
            infoString = techSet->name;
            infoIdString = "TS";
            break;
        case 3:
            infoString = context.state->material->info.name;
            infoIdString = "M";
            break;
        case 4:
            if ( !tech->passCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                            113,
                            0,
                            "%s",
                            "tech->passCount > 0") )
            {
                __debugbreak();
            }
            if ( tech->passArray[0].vertexShader )
                infoString = tech->passArray[0].vertexShader->name;
            else
                infoString = "<NONE>";
            offsetCenter[2] = (float)((float)((float)context.state->techType - 16.0) * 0.30000001) + offsetCenter[2];
            infoIdString = "VS";
            break;
        case 5:
            if ( !tech->passCount
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                            123,
                            0,
                            "%s",
                            "tech->passCount > 0") )
            {
                __debugbreak();
            }
            if ( tech->passArray[0].pixelShader )
                infoString = tech->passArray[0].pixelShader->name;
            else
                infoString = "<NONE>";
            offsetCenter[2] = (float)((float)((float)context.state->techType - 16.0) * 0.30000001) + offsetCenter[2];
            infoIdString = "PS";
            break;
        default:
            v4 = va("Unknown value for r_showTess: %i", r_showTess);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp", 145, 0, v4) )
                __debugbreak();
            infoString = "?";
            infoIdString = "?";
            break;
    }
    data = context.source->input.data;
    v5 = va("%s:%s=%s", tessName, infoIdString, infoString);
    R_AddDebugString((DebugGlobals*)&data->debugGlobals, offsetCenter, color, TEXT_SIZE, v5);
}

void __cdecl R_SetVertexDeclTypeNormal(GfxCmdBufState *state, MaterialVertexDeclType vertDeclType)
{
    state->prim.vertDeclType = vertDeclType;
    R_UpdateVertexDecl(state);
}

void __cdecl R_SetVertexDeclTypeWorld(GfxCmdBufState *state)
{
    if ( (state->technique->flags & 8) != 0 )
        R_SetVertexDeclTypeNormal(state, (MaterialVertexDeclType)(state->material->localTechniqueSet->worldVertFormat + 2));
    else
        R_SetVertexDeclTypeNormal(state, VERTDECL_WORLD);
}

void __cdecl R_SetVertexDeclTypeModelSkinned(const XSurface *surf, GfxCmdBufState *state)
{
    R_SetVertexDeclTypeNormal(state, VERTDECL_PACKED);
}

void __cdecl R_SetObjectIdentityPlacement(GfxCmdBufSourceState *source)
{
    if (source->objectPlacement != &g_drawConsts.identityPlacement)
    {
        source->objectPlacement = &g_drawConsts.identityPlacement;
        R_ChangeObjectPlacementRemote(source, &g_drawConsts.identityPlacement);
    }
}
unsigned int __cdecl R_TessCodeMeshList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext)
{
    GfxDepthRangeType depthRangeType; // [esp+8h] [ebp-64h]
    GfxCmdBufContext context; // [esp+Ch] [ebp-60h]
    const GfxDrawSurfListInfo *info; // [esp+18h] [ebp-54h]
    GfxDrawSurf drawSurf; // [esp+1Ch] [ebp-50h]
    const GfxBackEndData *data; // [esp+28h] [ebp-44h]
    const FxCodeMeshData *codeMesh; // [esp+30h] [ebp-3Ch]
    unsigned int drawSurfIndex; // [esp+38h] [ebp-34h]
    unsigned __int8 *indices; // [esp+44h] [ebp-28h]
    unsigned int argCount; // [esp+48h] [ebp-24h]
    const GfxDrawSurf *drawSurfList; // [esp+4Ch] [ebp-20h]
    GfxDrawPrimArgs args; // [esp+50h] [ebp-1Ch] BYREF
    unsigned __int64 drawSurfKey; // [esp+5Ch] [ebp-10h]
    unsigned int drawSurfCount; // [esp+68h] [ebp-4h]

    context = listArgs->context;
    info = listArgs->info;
    drawSurfCount = info->drawSurfCount - listArgs->firstDrawSurfIndex;
    drawSurfList = &info->drawSurfs[listArgs->firstDrawSurfIndex];
    drawSurf.fields = drawSurfList->fields;
    if (!R_SetVertexDeclTypeNormal_Safe(context.state, VERTDECL_PACKED))
        return 1;
    R_SetupPassCriticalPixelShaderArgs(context);
    if (prepassContext.state
        && context.source != prepassContext.source
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            325,
            0,
            "%s",
            "prepassContext.local.state == NULL || context.local.source == prepassContext.local.source"))
    {
        __debugbreak();
    }
    R_SetObjectIdentityPlacement(context.source);
    R_ChangeDepthHackNearClip(context.source, 0);
    depthRangeType = (GfxDepthRangeType)((context.source->cameraView != 0) - 1);
    if (depthRangeType != context.state->depthRangeType)
        R_ChangeDepthRange(context.state, depthRangeType);
    data = context.source->input.data;
    R_SetMeshStream(context.state, data->codeMeshPtr);
    indices = 0;
    args.baseIndex = 0;
    args.vertexCount = 0x4000;
    args.triCount = 0;
    drawSurfKey = drawSurf.packed & 0xFFFFFFFFF00F0000uLL;
    R_TrackPrims(&context.state->prim, GFX_PRIM_STATS_FX);
    argCount = 0;
    drawSurfIndex = 0;
    do
    {
        codeMesh = &data->codeMeshes[LOWORD(drawSurf.packed)];
        if (!codeMesh->triCount
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                364,
                0,
                "%s\n\t(codeMesh->triCount) = %i",
                "(codeMesh->triCount > 0)",
                codeMesh->triCount))
        {
            __debugbreak();
        }
        if (argCount
            || data->codeMeshes[LOWORD(drawSurf.packed)].argCount
            || &indices[6 * args.triCount] != (unsigned __int8 *)data->codeMeshes[LOWORD(drawSurf.packed)].indices)
        {
            if (args.triCount)
            {
                args.baseIndex = R_SetIndexData(&context.state->prim, indices, args.triCount);
                R_DrawIndexedPrimitive(&context.state->prim, &args);
                args.triCount = 0;
            }
            indices = (unsigned __int8 *)data->codeMeshes[LOWORD(drawSurf.packed)].indices;
            R_TessCodeMeshList_AddCodeMeshArgs(context.source, data, codeMesh);
            R_SetupPassPerObjectArgs(context);
            R_SetupPassPerPrimArgs(context);
        }
        argCount = data->codeMeshes[LOWORD(drawSurf.packed)].argCount;
        args.triCount += codeMesh->triCount;
        context.state->prim.frameStats.fxIndexCount += 3 * codeMesh->triCount;
        if (++drawSurfIndex == drawSurfCount)
            break;
        drawSurf = drawSurfList[drawSurfIndex];
    } while ((drawSurf.packed & 0xFFFFFFFFF00F0000uLL) == drawSurfKey);
    if (args.triCount)
    {
        args.baseIndex = R_SetIndexData(&context.state->prim, indices, args.triCount);
        R_DrawIndexedPrimitive(&context.state->prim, &args);
    }
    context.state->prim.primStats = 0;
    return drawSurfIndex;
}

int __cdecl R_SetVertexDeclTypeNormal_Safe(GfxCmdBufState *state, MaterialVertexDeclType vertDeclType)
{
    state->prim.vertDeclType = vertDeclType;
    R_UpdateVertexDecl(state);
    return 1;
}

void __cdecl R_TessCodeMeshList_AddCodeMeshArgs(
                GfxCmdBufSourceState *source,
                const GfxBackEndData *data,
                const FxCodeMeshData *codeMesh)
{
    unsigned int argIndex; // [esp+14h] [ebp-18h]
    unsigned int argGlobalIndex; // [esp+18h] [ebp-14h]
    unsigned int argCount; // [esp+20h] [ebp-Ch]
    unsigned int argOffset; // [esp+24h] [ebp-8h]
    unsigned int constantId; // [esp+28h] [ebp-4h]

    argOffset = codeMesh->argOffset;
    argCount = codeMesh->argCount;

    for (argIndex = 0; argIndex != argCount; ++argIndex)
    {
        constantId = argIndex + 76;

        iassert(constantId <= CONST_SRC_CODE_CODE_MESH_ARG_LAST);

        argGlobalIndex = argOffset + argIndex;

        iassert(argGlobalIndex < static_cast<uint>(data->codeMeshArgsCount));

        R_SetCodeConstant(
            source,
            (CodeConstant)constantId,
            data->codeMeshArgs[argGlobalIndex][0],
            data->codeMeshArgs[argGlobalIndex][1],
            data->codeMeshArgs[argGlobalIndex][2],
            data->codeMeshArgs[argGlobalIndex][3]);
    }
}

unsigned int __cdecl R_TessRopeMeshList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext)
{
    char *v4; // eax
    GfxDepthRangeType depthRangeType; // [esp+8h] [ebp-6Ch]
    GfxCmdBufContext context; // [esp+14h] [ebp-60h]
    const GfxDrawSurfListInfo *info; // [esp+20h] [ebp-54h]
    GfxDrawSurf drawSurf; // [esp+24h] [ebp-50h]
    const GfxBackEndData *data; // [esp+30h] [ebp-44h]
    const FxCodeMeshData *codeMesh; // [esp+38h] [ebp-3Ch]
    unsigned int drawSurfIndex; // [esp+40h] [ebp-34h]
    unsigned __int8 *indices; // [esp+4Ch] [ebp-28h]
    unsigned int argCount; // [esp+50h] [ebp-24h]
    const GfxDrawSurf *drawSurfList; // [esp+54h] [ebp-20h]
    GfxDrawPrimArgs args; // [esp+58h] [ebp-1Ch] BYREF
    unsigned __int64 drawSurfKey; // [esp+64h] [ebp-10h]
    unsigned int drawSurfCount; // [esp+70h] [ebp-4h]

    context = listArgs->context;
    info = listArgs->info;
    drawSurfCount = info->drawSurfCount - listArgs->firstDrawSurfIndex;
    drawSurfList = &info->drawSurfs[listArgs->firstDrawSurfIndex];
    if (!R_SetVertexDeclTypeNormal_Safe(context.state, VERTDECL_PACKED))
        return 1;
    R_SetupPassCriticalPixelShaderArgs(context);
    if (prepassContext.state
        && context.source != prepassContext.source
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            482,
            0,
            "%s",
            "prepassContext.local.state == NULL || commonSource == prepassContext.local.source"))
    {
        __debugbreak();
    }
    drawSurf.fields = drawSurfList->fields;
    R_SetObjectIdentityPlacement(context.source);
    R_ChangeDepthHackNearClip(context.source, 0);
    depthRangeType = (GfxDepthRangeType)((context.source->cameraView != 0) - 1);
    if (depthRangeType != context.state->depthRangeType)
        R_ChangeDepthRange(context.state, depthRangeType);
    data = context.source->input.data;
    R_SetMeshStream(context.state, data->codeMeshPtr);
    indices = 0;
    args.baseIndex = 0;
    args.vertexCount = 0x4000;
    args.triCount = 0;
    drawSurfKey = drawSurf.packed & 0xFFFFFFFFF00F0000uLL;
    R_TrackPrims(&context.state->prim, GFX_PRIM_STATS_FX);
    argCount = 0;
    drawSurfIndex = 0;
    do
    {
        codeMesh = &data->codeMeshes[LOWORD(drawSurf.packed)];
        if (!codeMesh->triCount
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                527,
                0,
                "%s\n\t(codeMesh->triCount) = %i",
                "(codeMesh->triCount > 0)",
                codeMesh->triCount))
        {
            __debugbreak();
        }
        if (data->codeMeshes[LOWORD(drawSurf.packed)].indices < data->codeMeshPtr->indices
            || &data->codeMeshes[LOWORD(drawSurf.packed)].indices[3 * data->codeMeshes[LOWORD(drawSurf.packed)].triCount] > data->codeMeshPtr->indices + 61440)
        {
            v4 = va(
                "0x%08x 0x%08x %i %i %s",
                data->codeMeshes[LOWORD(drawSurf.packed)].indices,
                data->codeMeshPtr->indices,
                3 * data->codeMeshes[LOWORD(drawSurf.packed)].triCount,
                61440,
                context.state->material->info.name);
            if (!Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                531,
                0,
                "%s\n\t%s",
                "&codeMesh->indices[0] >= &data->codeMeshPtr->indices[0] && &codeMesh->indices[codeMesh->triCount * 3] <= &"
                "data->codeMeshPtr->indices[GFX_CODE_MESH_INDEX_LIMIT]",
                v4))
                __debugbreak();
        }
        if (argCount
            || data->codeMeshes[LOWORD(drawSurf.packed)].argCount
            || &indices[6 * args.triCount] != (unsigned __int8 *)data->codeMeshes[LOWORD(drawSurf.packed)].indices)
        {
            if (args.triCount)
            {
                args.baseIndex = R_SetIndexData(&context.state->prim, indices, args.triCount);
                R_DrawIndexedPrimitive(&context.state->prim, &args);
                args.triCount = 0;
            }
            indices = (unsigned __int8 *)data->codeMeshes[LOWORD(drawSurf.packed)].indices;
            R_SetModelLightingCoordsForSource(
                LODWORD(data->codeMeshArgs[data->codeMeshes[LOWORD(drawSurf.packed)].argOffset][0]),
                context.source);
            R_SetupPassPerObjectArgs(context);
            R_SetupPassPerPrimArgs(context);
        }
        argCount = data->codeMeshes[LOWORD(drawSurf.packed)].argCount;
        args.triCount += codeMesh->triCount;
        context.state->prim.frameStats.fxIndexCount += 3 * codeMesh->triCount;
        if (++drawSurfIndex == drawSurfCount)
            break;
        drawSurf = drawSurfList[drawSurfIndex];
    } while ((drawSurf.packed & 0xFFFFFFFFF00F0000uLL) == drawSurfKey);
    if (args.triCount)
    {
        args.baseIndex = R_SetIndexData(&context.state->prim, indices, args.triCount);
        R_DrawIndexedPrimitive(&context.state->prim, &args);
    }
    context.state->prim.primStats = 0;
    return drawSurfIndex;
}

unsigned int __cdecl R_TessGlassMeshList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext)
{
    GfxDepthRangeType depthRangeType; // [esp+8h] [ebp-60h]
    GfxCmdBufContext context; // [esp+Ch] [ebp-5Ch]
    const GfxDrawSurfListInfo *info; // [esp+14h] [ebp-54h]
    GfxDrawSurf drawSurf; // [esp+18h] [ebp-50h]
    const GfxBackEndData *data; // [esp+24h] [ebp-44h]
    const FxCodeMeshData *codeMesh; // [esp+2Ch] [ebp-3Ch]
    unsigned int drawSurfIndex; // [esp+34h] [ebp-34h]
    unsigned __int8 *indices; // [esp+40h] [ebp-28h]
    unsigned int lightHandle; // [esp+44h] [ebp-24h]
    const GfxDrawSurf *drawSurfList; // [esp+48h] [ebp-20h]
    GfxDrawPrimArgs args; // [esp+4Ch] [ebp-1Ch] BYREF
    unsigned __int64 drawSurfKey; // [esp+58h] [ebp-10h]
    unsigned int drawSurfCount; // [esp+64h] [ebp-4h]

    context = listArgs->context;
    info = listArgs->info;
    drawSurfCount = info->drawSurfCount - listArgs->firstDrawSurfIndex;
    drawSurfList = &info->drawSurfs[listArgs->firstDrawSurfIndex];
    if (!R_SetVertexDeclTypeNormal_Safe(context.state, VERTDECL_PACKED))
        return 1;
    R_SetupPassCriticalPixelShaderArgs(context);
    if (prepassContext.state
        && context.source != prepassContext.source
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            657,
            0,
            "%s",
            "prepassContext.local.state == NULL || commonSource == prepassContext.local.source"))
    {
        __debugbreak();
    }
    drawSurf.fields = drawSurfList->fields;
    R_SetObjectIdentityPlacement(context.source);
    R_ChangeDepthHackNearClip(context.source, 0);
    depthRangeType = (GfxDepthRangeType)((context.source->cameraView != 0) - 1);
    if (depthRangeType != context.state->depthRangeType)
        R_ChangeDepthRange(context.state, depthRangeType);
    data = context.source->input.data;
    R_SetMeshStream(context.state, data->codeMeshPtr);
    args.baseIndex = 0;
    args.vertexCount = 0x4000;
    args.triCount = 0;
    drawSurfKey = drawSurf.packed & 0xFFFFFFFFF00F0000uLL;
    R_TrackPrims(&context.state->prim, GFX_PRIM_STATS_FX);
    indices = 0;
    lightHandle = 0;
    drawSurfIndex = 0;
    do
    {
        codeMesh = &data->codeMeshes[LOWORD(drawSurf.packed)];
        if (!codeMesh->triCount
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                705,
                0,
                "%s\n\t(codeMesh->triCount) = %i",
                "(codeMesh->triCount > 0)",
                codeMesh->triCount))
        {
            __debugbreak();
        }
        if (lightHandle != data->codeMeshes[LOWORD(drawSurf.packed)].lightHandle
            || &indices[6 * args.triCount] != (unsigned __int8 *)data->codeMeshes[LOWORD(drawSurf.packed)].indices)
        {
            if (args.triCount)
            {
                args.baseIndex = R_SetIndexData(&context.state->prim, indices, args.triCount);
                R_DrawIndexedPrimitive(&context.state->prim, &args);
                args.triCount = 0;
            }
            indices = (unsigned __int8 *)data->codeMeshes[LOWORD(drawSurf.packed)].indices;
            lightHandle = data->codeMeshes[LOWORD(drawSurf.packed)].lightHandle;
            R_SetModelLightingCoordsForSource(data->codeMeshes[LOWORD(drawSurf.packed)].lightHandle, context.source);
            R_SetReflectionProbe(context, (drawSurf.packed >> 25) & 7);
            R_SetupPassPerObjectArgs(context);
            R_SetupPassPerPrimArgs(context);
        }
        args.triCount += codeMesh->triCount;
        context.state->prim.frameStats.fxIndexCount += 3 * codeMesh->triCount;
        if (++drawSurfIndex == drawSurfCount)
            break;
        drawSurf= drawSurfList[drawSurfIndex];
    } while ((drawSurf.packed & 0xFFFFFFFFF00F0000uLL) == drawSurfKey);
    if (args.triCount)
    {
        args.baseIndex = R_SetIndexData(&context.state->prim, indices, args.triCount);
        R_DrawIndexedPrimitive(&context.state->prim, &args);
    }
    context.state->prim.primStats = 0;
    return drawSurfIndex;
}

float g_ftDecalFadePower = 2.0f;
unsigned int __cdecl R_TessMarkMeshList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext)
{
    int packed_low; // eax
    int v4; // eax
    BOOL v6; // [esp+10h] [ebp-E4h]
    BOOL v7; // [esp+14h] [ebp-E0h]
    bool v8; // [esp+18h] [ebp-DCh]
    bool v9; // [esp+20h] [ebp-D4h]
    bool v10; // [esp+24h] [ebp-D0h]
    float v11; // [esp+34h] [ebp-C0h]
    float v12; // [esp+4Ch] [ebp-A8h]
    GfxDepthRangeType v13; // [esp+58h] [ebp-9Ch]
    GfxDepthRangeType depthRangeType; // [esp+5Ch] [ebp-98h]
    GfxCmdBufContext context; // [esp+64h] [ebp-90h]
    GfxMeshData *meshData; // [esp+6Ch] [ebp-88h]
    const GfxDrawSurfListInfo *info; // [esp+70h] [ebp-84h]
    GfxDrawSurf drawSurf; // [esp+74h] [ebp-80h]
    const GfxBackEndData *data; // [esp+80h] [ebp-74h]
    float curNormal; // [esp+84h] [ebp-70h]
    float curNormal_4; // [esp+88h] [ebp-6Ch]
    float curNormal_8; // [esp+8Ch] [ebp-68h]
    unsigned __int64 drawSurfSubKey; // [esp+94h] [ebp-60h]
    GfxDrawSurf drawSurfSubMask; // [esp+9Ch] [ebp-58h]
    unsigned __int8 baseTechType; // [esp+A7h] [ebp-4Dh]
    unsigned int drawSurfIndex; // [esp+A8h] [ebp-4Ch]
    unsigned __int8 *indices; // [esp+B8h] [ebp-3Ch]
    float newNormal; // [esp+BCh] [ebp-38h]
    float newNormal_4; // [esp+C0h] [ebp-34h]
    float newNormal_8; // [esp+C4h] [ebp-30h]
    MaterialVertexDeclType declType; // [esp+C8h] [ebp-2Ch]
    const FxMarkMeshData *markMesh; // [esp+CCh] [ebp-28h]
    const FxMarkMeshData *markMesha; // [esp+CCh] [ebp-28h]
    unsigned int markType; // [esp+D0h] [ebp-24h]
    unsigned int markTypea; // [esp+D0h] [ebp-24h]
    unsigned int markTypeb; // [esp+D0h] [ebp-24h]
    const GfxDrawSurf *drawSurfList; // [esp+D4h] [ebp-20h]
    GfxDrawPrimArgs args; // [esp+D8h] [ebp-1Ch] BYREF
    unsigned __int64 drawSurfKey; // [esp+E4h] [ebp-10h]
    int needsMarksHitNormal; // [esp+ECh] [ebp-8h]
    unsigned int drawSurfCount; // [esp+F0h] [ebp-4h]

    context = listArgs->context;
    info = listArgs->info;
    needsMarksHitNormal = listArgs->context.state->technique->flags & 0x100;
    curNormal = 0.0f;
    curNormal_4 = 0.0f;
    curNormal_8 = 0.0f;
    drawSurfCount = info->drawSurfCount - listArgs->firstDrawSurfIndex;
    drawSurfList = &info->drawSurfs[listArgs->firstDrawSurfIndex];
    R_SetupPassCriticalPixelShaderArgs(context);
    if (prepassContext.state
        && context.source != prepassContext.source
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            873,
            0,
            "%s",
            "prepassContext.local.state == NULL || commonSource == prepassContext.local.source"))
    {
        __debugbreak();
    }
    baseTechType = info->baseTechType;
    drawSurf.fields = drawSurfList->fields;
    R_SetObjectIdentityPlacement(context.source);
    R_ChangeDepthHackNearClip(context.source, 0);
    data = context.source->input.data;
    meshData = (GfxMeshData*)&data->markMesh;
    markType = data->markMeshes[LOWORD(drawSurf.packed)].modelTypeAndSurf & 0xC0;
    v10 = markType == 64 || markType == 192;
    declType = (MaterialVertexDeclType)(2 - v10);
    R_SetVertexDeclTypeNormal(context.state, declType);
    depthRangeType = (GfxDepthRangeType)((context.source->cameraView != 0) - 1);
    if (depthRangeType != context.state->depthRangeType)
        R_ChangeDepthRange(context.state, depthRangeType);
    R_SetMeshStream(context.state, meshData);
    if (prepassContext.state)
    {
        R_SetVertexDeclTypeNormal(prepassContext.state, declType);
        v13 = (GfxDepthRangeType)((context.source->cameraView != 0) - 1);
        if (v13 != prepassContext.state->depthRangeType)
            R_ChangeDepthRange(prepassContext.state, v13);
        R_SetMeshStream(prepassContext.state, meshData);
    }
    indices = 0;
    args.baseIndex = 0;
    args.vertexCount = 6144;
    args.triCount = 0;
    drawSurfSubMask.packed = -65536;
    if (baseTechType != 10)
    {
        *(_DWORD *)&drawSurfSubMask.fields = -267452416;
        R_SetupPassPerObjectArgs(context);
        if (prepassContext.state)
            R_SetupPassPerObjectArgs(prepassContext);
    }
    drawSurfKey = drawSurf.packed & 0xFFFFFFFFF00F0000uLL;
    R_TrackPrims(&context.state->prim, GFX_PRIM_STATS_FX);
    drawSurfIndex = 0;
    do
    {
        if (baseTechType == 10)
        {
            if (args.triCount)
            {
                R_SetupPassPerPrimArgs(context);
                args.baseIndex = R_SetIndexData(&context.state->prim, indices, args.triCount);
                R_DrawIndexedPrimitive(&context.state->prim, &args);
                if (prepassContext.state)
                {
                    R_SetupPassPerPrimArgs(prepassContext);
                    args.baseIndex = R_SetIndexData(&prepassContext.state->prim, indices, args.triCount);
                    R_DrawIndexedPrimitive(&prepassContext.state->prim, &args);
                }
                indices = 0;
                args.triCount = 0;
            }
            packed_low = LOWORD(drawSurf.packed);
            markMesh = &data->markMeshes[packed_low];
            markTypea = data->markMeshes[packed_low].modelTypeAndSurf & 0xC0;
            v9 = markTypea == 64 || markTypea == 192;
            if (declType != 2 - v9
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                    974,
                    0,
                    "%s\n\t(markType) = %i",
                    "(declType == R_Tess_DeclForMarkType( markType ))",
                    markTypea))
            {
                __debugbreak();
            }
            if (needsMarksHitNormal)
            {
                curNormal = markMesh->normal[0];
                curNormal_4 = markMesh->normal[1];
                curNormal_8 = markMesh->normal[2];
                v12 = g_ftDecalFadePower;
                context.source->input.consts[CONST_SRC_CODE_MARKS_HIT_NORMAL][0] = curNormal;
                context.source->input.consts[CONST_SRC_CODE_MARKS_HIT_NORMAL][1] = curNormal_4;
                context.source->input.consts[CONST_SRC_CODE_MARKS_HIT_NORMAL][2] = curNormal_8;
                context.source->input.consts[CONST_SRC_CODE_MARKS_HIT_NORMAL][3] = v12;
                R_DirtyCodeConstant(context.source, CONST_SRC_CODE_MARKS_HIT_NORMAL);
            }
            if (markTypea == 64)
            {
                R_SetStaticModelLightingCoordsForSource(markMesh->modelIndex, context.source);
            }
            else if (markTypea == 192)
            {
                R_SetModelLightingCoordsForSource(markMesh->modelIndex, context.source);
            }
            else
            {
                if (markTypea
                    && markTypea != 128
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                        997,
                        0,
                        "%s\n\t(markType) = %i",
                        "((markType == MARK_MODEL_TYPE_WORLD_BRUSH) || (markType == MARK_MODEL_TYPE_ENT_BRUSH))",
                        markTypea))
                {
                    __debugbreak();
                }
                R_SetLightmap(context, (drawSurf.packed >> 20) & 0x1F);
            }
            R_SetReflectionProbe(context, (drawSurf.packed >> 25) & 7);
            R_SetupPassPerObjectArgs(context);
            if (prepassContext.state)
                R_SetupPassPerObjectArgs(prepassContext);
        }
        drawSurfSubKey = drawSurfSubMask.packed & drawSurf.packed;
        do
        {
            v4 = LOWORD(drawSurf.packed);
            markMesha = &data->markMeshes[v4];
            markTypeb = data->markMeshes[v4].modelTypeAndSurf & 0xC0;
            v8 = markTypeb == 64 || markTypeb == 192;
            if (declType != 2 - v8
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                    1014,
                    0,
                    "%s\n\t(markType) = %i",
                    "(declType == R_Tess_DeclForMarkType( markType ))",
                    markTypeb))
            {
                __debugbreak();
            }
            if (!markMesha->triCount
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                    1015,
                    0,
                    "%s\n\t(markMesh->triCount) = %i",
                    "(markMesh->triCount > 0)",
                    markMesha->triCount))
            {
                __debugbreak();
            }
            newNormal = markMesha->normal[0];
            newNormal_4 = markMesha->normal[1];
            newNormal_8 = markMesha->normal[2];
            if ((markMesha->indices < data->markMesh.indices
                || &markMesha->indices[3 * markMesha->triCount] > data->markMesh.indices + 9216)
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                    1020,
                    0,
                    "%s",
                    "&markMesh->indices[0] >= &data->markMesh.indices[0] && &markMesh->indices[markMesh->triCount * 3] <= &data"
                    "->markMesh.indices[GFX_MARK_MESH_INDEX_LIMIT]"))
            {
                __debugbreak();
            }
            if (needsMarksHitNormal)
            {
                v7 = curNormal == newNormal && curNormal_4 == newNormal_4 && curNormal_8 == newNormal_8;
                v6 = !v7;
            }
            else
            {
                v6 = 0;
            }
            if (v6 || &indices[6 * args.triCount] != (unsigned __int8 *)markMesha->indices)
            {
                if (args.triCount)
                {
                    R_SetupPassPerPrimArgs(context);
                    args.baseIndex = R_SetIndexData(&context.state->prim, indices, args.triCount);
                    R_DrawIndexedPrimitive(&context.state->prim, &args);
                    if (prepassContext.state)
                    {
                        R_SetupPassPerPrimArgs(prepassContext);
                        R_DrawIndexedPrimitive(&prepassContext.state->prim, &args);
                    }
                    args.triCount = 0;
                }
                if (v6)
                {
                    curNormal = newNormal;
                    curNormal_4 = newNormal_4;
                    curNormal_8 = newNormal_8;
                    v11 = g_ftDecalFadePower;
                    context.source->input.consts[CONST_SRC_CODE_MARKS_HIT_NORMAL][0] = newNormal;
                    context.source->input.consts[CONST_SRC_CODE_MARKS_HIT_NORMAL][1] = newNormal_4;
                    context.source->input.consts[CONST_SRC_CODE_MARKS_HIT_NORMAL][2] = newNormal_8;
                    context.source->input.consts[CONST_SRC_CODE_MARKS_HIT_NORMAL][3] = v11;
                    R_DirtyCodeConstant(context.source, CONST_SRC_CODE_MARKS_HIT_NORMAL);
                }
                indices = (unsigned __int8 *)markMesha->indices;
            }
            args.triCount += markMesha->triCount;
            context.state->prim.frameStats.fxIndexCount += 3 * markMesha->triCount;
            if (++drawSurfIndex == drawSurfCount)
                break;
            drawSurf = drawSurfList[drawSurfIndex];
        } while (__PAIR64__(HIDWORD(drawSurf.packed), *(_DWORD *)&drawSurfSubMask.fields & *(_DWORD *)&drawSurf.fields) == drawSurfSubKey);
    } while (drawSurfIndex != drawSurfCount && (drawSurf.packed & 0xFFFFFFFFF00F0000uLL) == drawSurfKey);
    if (args.triCount)
    {
        R_SetupPassPerPrimArgs(context);
        args.baseIndex = R_SetIndexData(&context.state->prim, indices, args.triCount);
        R_DrawIndexedPrimitive(&context.state->prim, &args);
        if (prepassContext.state)
        {
            R_SetupPassPerPrimArgs(prepassContext);
            args.baseIndex = R_SetIndexData(&prepassContext.state->prim, indices, args.triCount);
            R_DrawIndexedPrimitive(&prepassContext.state->prim, &args);
        }
    }
    context.state->prim.primStats = 0;
    return drawSurfIndex;
}

unsigned int __cdecl R_TessParticleCloudList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext)
{
    const char *v2; // eax
    GfxDepthRangeType depthRangeType; // [esp+8h] [ebp-3Ch]
    const GfxParticleCloud *cloud; // [esp+Ch] [ebp-38h]
    GfxCmdBufContext context; // [esp+10h] [ebp-34h]
    const GfxDrawSurfListInfo *info; // [esp+18h] [ebp-2Ch]
    GfxDrawSurf drawSurf; // [esp+1Ch] [ebp-28h]
    const GfxBackEndData *data; // [esp+24h] [ebp-20h]
    GfxCmdBufSourceState *commonSource; // [esp+28h] [ebp-1Ch]
    unsigned int offset; // [esp+2Ch] [ebp-18h]
    GfxDrawPrimArgs args; // [esp+34h] [ebp-10h] BYREF
    unsigned int count; // [esp+40h] [ebp-4h]

    context = listArgs->context;
    commonSource = listArgs->context.source;
    if (prepassContext.state
        && commonSource != prepassContext.source
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            1230,
            0,
            "%s",
            "prepassContext.local.state == NULL || commonSource == prepassContext.local.source"))
    {
        __debugbreak();
    }
    info = listArgs->info;
    if (r_logFile->current.integer)
    {
        v2 = va("--- RB_TessParticleCloud( %s ) ---\n", context.state->material->info.name);
        RB_LogPrint(v2);
    }
    drawSurf.fields = info->drawSurfs[listArgs->firstDrawSurfIndex].fields;
    data = commonSource->input.data;
    if ((unsigned int)LOWORD(drawSurf.packed) >= data->cloudCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            1247,
            0,
            "drawSurf.fields.objectId doesn't index data->cloudCount\n\t%i not in [0, %i)",
            LOWORD(drawSurf.packed),
            data->cloudCount))
    {
        __debugbreak();
    }
    cloud = &data->clouds[LOWORD(drawSurf.packed)];
    R_SetParticleCloudConstants(commonSource, cloud);
    R_SetupPassCriticalPixelShaderArgs(context);
    R_ChangeDepthHackNearClip(commonSource, 0);
    R_SetVertexDeclTypeNormal(context.state, VERTDECL_PARTICLECLOUD);
    depthRangeType = (GfxDepthRangeType)((commonSource->cameraView != 0) - 1);
    if (depthRangeType != context.state->depthRangeType)
        R_ChangeDepthRange(context.state, depthRangeType);
    offset = data->clouds[LOWORD(drawSurf.packed)].offset;
    count = data->clouds[LOWORD(drawSurf.packed)].count;
    if ((!count || count + offset > 0x400)
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            1267,
            0,
            "%s",
            "count && ((offset+count) <= PARTICLE_CLOUD_QUADS)"))
    {
        __debugbreak();
    }
    if (data->clouds[LOWORD(drawSurf.packed)].gaussian)
        offset += 1024;
    if (context.state->prim.indexBuffer != gfxBuf.particleCloudIndexBuffer)
        R_ChangeIndices(&context.state->prim, gfxBuf.particleCloudIndexBuffer);
    R_SetStreamSource(&context.state->prim, gfxBuf.particleCloudVertexBuffer, 0, 8u);
    if ((const GfxParticleCloud *)commonSource->objectPlacement != cloud)
    {
        commonSource->objectPlacement = &cloud->placement;
        R_ChangeObjectPlacementRemote(commonSource, &cloud->placement);
    }
    R_SetupPassPerObjectArgs(context);
    R_SetupPassPerPrimArgs(context);
    args.baseIndex = 6 * offset;
    args.vertexCount = 4 * count;
    if (data->clouds[LOWORD(drawSurf.packed)].gaussian)
        args.vertexCount += 4096;
    else
        args.vertexCount = 4096;
    args.triCount = 2 * count;
    context.state->prim.frameStats.fxIndexCount += 6 * count;
    RB_TrackImmediatePrims(&context.state->prim, GFX_PRIM_STATS_FX);
    if (!context.state->prim.primStats
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            1332,
            0,
            "%s",
            "context.state->prim.primStats"))
    {
        __debugbreak();
    }
    R_DrawIndexedPrimitive(&context.state->prim, &args);
    if (!context.state->prim.primStats
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            1344,
            0,
            "%s",
            "context.state->prim.primStats"))
    {
        __debugbreak();
    }
    context.state->prim.primStats->staticVertexCount += args.vertexCount;
    context.state->prim.primStats->staticIndexCount += 3 * args.triCount;
    RB_EndTrackImmediatePrims(&context.state->prim);
    return 1;
}

void __cdecl R_SetParticleCloudConstants(GfxCmdBufSourceState *source, const GfxParticleCloud *cloud)
{
    float v4; // [esp+24h] [ebp-7Ch]
    float scaledWorldUp[3]; // [esp+4Ch] [ebp-54h] BYREF
    float viewUp[3]; // [esp+58h] [ebp-48h] BYREF
    float viewAxis[2][2]; // [esp+64h] [ebp-3Ch] BYREF
    float particleCloudMatrix[4]; // [esp+74h] [ebp-2Ch] BYREF
    float particleColor[4]; // [esp+84h] [ebp-1Ch] BYREF
    float worldUp[3]; // [esp+94h] [ebp-Ch] BYREF

    iassert(cloud);

    if (cloud->radius[0] == cloud->radius[1] || VecNCompareCustomEpsilon(cloud->placement.base.origin, cloud->endpos, 0.001, 3))
    {
        viewAxis[0][0] = cloud->radius[0];
        viewAxis[0][1] = 0.0f;
        viewAxis[1][0] = 0.0f;
        viewAxis[1][1] = cloud->radius[1];
    }
    else
    {
        worldUp[0] = cloud->endpos[0] - cloud->placement.base.origin[0];
        worldUp[1] = cloud->endpos[1] - cloud->placement.base.origin[1];
        worldUp[2] = cloud->endpos[2] - cloud->placement.base.origin[2];
        Vec3Normalize(worldUp);
        v4 = cloud->radius[1];
        scaledWorldUp[0] = v4 * worldUp[0];
        scaledWorldUp[1] = v4 * worldUp[1];
        scaledWorldUp[2] = v4 * worldUp[2];
        RB_Vec3DirWorldToView(source, scaledWorldUp, viewUp);
        RB_CreateParticleCloud2dAxis(cloud, viewUp, viewAxis);
    }
    particleCloudMatrix[0] = viewAxis[0][0];
    particleCloudMatrix[1] = viewAxis[0][1];
    particleCloudMatrix[2] = viewAxis[1][0];
    particleCloudMatrix[3] = viewAxis[1][1];

    R_SetCodeConstantFromVec4(source, CONST_SRC_CODE_PARTICLE_CLOUD_MATRIX, particleCloudMatrix);
    Byte4UnpackVertexColor((const unsigned __int8 *)&cloud->color, particleColor);
    R_SetCodeConstantFromVec4(source, CONST_SRC_CODE_PARTICLE_CLOUD_COLOR, particleColor);
}

void __cdecl Byte4UnpackVertexColor(const unsigned __int8 *from, float *to)
{
    *to = (float)from[2] * 0.0039215689;
    to[1] = (float)from[1] * 0.0039215689;
    to[2] = (float)*from * 0.0039215689;
    to[3] = (float)from[3] * 0.0039215689;
}

void __cdecl RB_Vec3DirWorldToView(const GfxCmdBufSourceState *source, const float *worldDir, float *viewDir)
{
    float viewAxis[3][3]; // [esp+30h] [ebp-24h] BYREF

    iassert(source->viewMode == VIEW_MODE_3D);

    viewAxis[0][0] = source->viewParms.viewMatrix.m[0][0];
    viewAxis[0][1] = source->viewParms.viewMatrix.m[0][1];
    viewAxis[0][2] = source->viewParms.viewMatrix.m[0][2];

    viewAxis[1][0] = source->viewParms.viewMatrix.m[1][0];
    viewAxis[1][1] = source->viewParms.viewMatrix.m[1][1];
    viewAxis[1][2] = source->viewParms.viewMatrix.m[1][2];

    viewAxis[2][0] = source->viewParms.viewMatrix.m[2][0];
    viewAxis[2][1] = source->viewParms.viewMatrix.m[2][1];
    viewAxis[2][2] = source->viewParms.viewMatrix.m[2][2];
    Vec3RotateTranspose(worldDir, viewAxis, viewDir);
}

void __cdecl RB_CreateParticleCloud2dAxis(const GfxParticleCloud *cloud, const float *viewUp, float (*viewAxis)[2])
{
    float v3; // [esp+Ch] [ebp-48h]
    float v4; // [esp+14h] [ebp-40h]
    float v5; // [esp+18h] [ebp-3Ch]
    float v6; // [esp+2Ch] [ebp-28h]
    float v7; // [esp+34h] [ebp-20h]
    float viewAxisLength; // [esp+3Ch] [ebp-18h]
    float viewRight_4; // [esp+44h] [ebp-10h]

    if ( !cloud && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp", 1146, 0, "%s", "cloud") )
        __debugbreak();
    if ( *viewUp >= 0.001 || viewUp[1] >= 0.001 )
    {
        viewRight_4 = -1.0 * *viewUp;
        //(*viewAxis)[0] = COERCE_FLOAT(LODWORD(-1.0f) ^ _mask__NegFloat_) * viewUp[1];
        (*viewAxis)[0] = (-(-1.0f) ) * viewUp[1];
        (*viewAxis)[1] = viewRight_4;
        v6 = viewUp[1];
        (*viewAxis)[2] = *viewUp;
        (*viewAxis)[3] = v6;
        viewAxisLength = Vec2Length((const float *)viewAxis);
        if ( viewAxisLength <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                        1165,
                        0,
                        "%s\n\t(viewAxisLength) = %g",
                        "(viewAxisLength > 0)",
                        viewAxisLength) )
        {
            __debugbreak();
        }
        v5 = viewAxisLength - Vec2Length(&(*viewAxis)[2]);
        if ( fabs(v5) >= 0.001
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                        1166,
                        0,
                        "%s",
                        "fabs( viewAxisLength - Vec2Length( viewAxis[1] ) ) < EQUAL_EPSILON") )
        {
            __debugbreak();
        }
        v4 = cloud->radius[0] / viewAxisLength;
        (*viewAxis)[0] = v4 * (*viewAxis)[0];
        (*viewAxis)[1] = v4 * (*viewAxis)[1];
        if ( cloud->radius[0] > viewAxisLength )
        {
            v3 = cloud->radius[0] / viewAxisLength;
            (*viewAxis)[2] = v3 * (*viewAxis)[2];
            (*viewAxis)[3] = v3 * (*viewAxis)[3];
        }
    }
    else
    {
        (*viewAxis)[0] = cloud->radius[0];
        (*viewAxis)[1] = 0.0f;
        v7 = cloud->radius[1];
        (*viewAxis)[2] = 0.0f;
        (*viewAxis)[3] = v7;
    }
}

void __cdecl R_DrawXModelSkinnedModelSurf(GfxCmdBufContext context, const GfxModelSkinnedSurface *modelSurf)
{
    if ( !modelSurf
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp", 1546, 0, "%s", "modelSurf") )
    {
        __debugbreak();
    }
    if ( modelSurf->skinnedCachedOffset < 0 )
        R_DrawXModelSkinnedUncached(context, modelSurf->xsurf, modelSurf->skinnedVert);
    else
        R_DrawXModelSkinnedCached(context, modelSurf);
}

void __cdecl R_DrawXModelSkinnedCached(GfxCmdBufContext context, const GfxModelSkinnedSurface *modelSurf)
{
    const char *v2; // eax
    GfxCmdBufSourceState *ActiveWorldMatrix; // eax
    const GfxBackEndData *data; // [esp+4h] [ebp-18h]
    XSurface *xsurf; // [esp+8h] [ebp-14h]
    IDirect3DIndexBuffer9 *ib; // [esp+Ch] [ebp-10h]
    GfxDrawPrimArgs args; // [esp+10h] [ebp-Ch] BYREF

    if ( r_logFile->current.integer )
    {
        v2 = va("--- R_DrawXModelSkinnedCached( %s ) ---\n", context.state->material->info.name);
        RB_LogPrint(v2);
    }
    if ( !modelSurf
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp", 1396, 0, "%s", "modelSurf") )
    {
        __debugbreak();
    }
    xsurf = modelSurf->xsurf;
    if ( !xsurf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp", 1399, 0, "%s", "xsurf") )
        __debugbreak();
    if ( !xsurf->vertCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp", 1400, 0, "%s", "xsurf->vertCount") )
    {
        __debugbreak();
    }
    if ( modelSurf->skinnedCachedOffset < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                    1402,
                    0,
                    "%s",
                    "modelSurf->skinnedCachedOffset >= 0") )
    {
        __debugbreak();
    }
    args.baseIndex = 0;
    args.vertexCount = xsurf->vertCount;
    args.triCount = xsurf->triCount;
    context.state->prim.frameStats.geoIndexCount += 3 * args.triCount;
    ib = xsurf->indexBuffer;
    if ( !ib && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp", 1415, 0, "%s", "ib") )
        __debugbreak();
    data = context.source->input.data;
    if ( context.state->prim.indexBuffer != ib )
        R_ChangeIndices(&context.state->prim, ib);
    R_SetStreamSource(&context.state->prim, data->skinnedCacheVb->buffer, modelSurf->skinnedCachedOffset, 0x20u);
    R_DrawIndexedPrimitive(&context.state->prim, &args);
    if ( context.state->prim.primStats )
    {
        context.state->prim.primStats->staticIndexCount += 3 * args.triCount;
        context.state->prim.primStats->staticVertexCount += args.vertexCount;
    }
    if ( r_showTess->current.integer )
    {
        ActiveWorldMatrix = R_GetActiveWorldMatrix(context.source);
        RB_ShowTess(context, ActiveWorldMatrix->matrices.matrix[0].m[3], "XMSkin$", colorWhite);
    }
}

void __cdecl R_DrawXModelSkinnedUncached(GfxCmdBufContext context, XSurface *xsurf, GfxPackedVertex *skinnedVert)
{
    GfxCmdBufSourceState *ActiveWorldMatrix; // eax
    IDirect3DVertexBuffer9 *vb; // [esp+0h] [ebp-14h]
    unsigned int vertexOffset; // [esp+4h] [ebp-10h]
    GfxDrawPrimArgs args; // [esp+8h] [ebp-Ch] BYREF

    if ( r_logFile->current.integer )
    {
        RB_LogPrint(va("--- R_DrawXModelSkinnedUncached( %s ) ---\n", context.state->material->info.name));
    }

    iassert(xsurf);
    iassert(skinnedVert);

    args.triCount = XSurfaceGetNumTris(xsurf);
    args.vertexCount = XSurfaceGetNumVerts(xsurf);
    context.state->prim.frameStats.geoIndexCount += 3 * args.triCount;
    args.baseIndex = R_SetIndexData(&context.state->prim, (unsigned __int8 *)xsurf->triIndices, args.triCount);
    R_CheckVertexDataOverflow(32 * args.vertexCount);
    vertexOffset = R_SetVertexData(context.state, skinnedVert, args.vertexCount, 32);
    vb = gfxBuf.dynamicVertexBuffer->buffer;
    iassert(vb);
    R_SetStreamSource(&context.state->prim, vb, vertexOffset, 0x20u);
    R_DrawIndexedPrimitive(&context.state->prim, &args);

    iassert(context.state->prim.primStats);

    context.state->prim.primStats->staticIndexCount += 3 * args.triCount;
    context.state->prim.primStats->staticVertexCount += args.vertexCount;
    if ( r_showTess->current.integer )
    {
        ActiveWorldMatrix = R_GetActiveWorldMatrix(context.source);
        RB_ShowTess(context, ActiveWorldMatrix->matrices.matrix[0].m[3], "XMSkinUn$", colorWhite);
    }
}

unsigned int __cdecl R_TessXModelSkinnedDrawSurfList(
    const GfxDrawSurfListArgs *listArgs,
    GfxCmdBufContext prepassContext)
{
    int updated; // eax
    int v3; // eax
    int v4; // eax
    unsigned int packed_high; // ecx
    int v6; // eax
    float z; // [esp+34h] [ebp-ACh]
    float y; // [esp+38h] [ebp-A8h]
    const GfxScaledPlacement *remotePlacement; // [esp+54h] [ebp-8Ch]
    GfxCmdBufContext context; // [esp+68h] [ebp-78h]
    unsigned int baseGfxEntIndex; // [esp+70h] [ebp-70h]
    const GfxDrawSurfListInfo *info; // [esp+74h] [ebp-6Ch]
    GfxDrawSurf drawSurf; // [esp+78h] [ebp-68h] BYREF
    const GfxBackEndData *data; // [esp+84h] [ebp-5Ch]
    GfxCmdBufSourceState *commonSource; // [esp+88h] [ebp-58h]
    float curWetness; // [esp+8Ch] [ebp-54h]
    unsigned __int8 baseTechType; // [esp+93h] [ebp-4Dh]
    int setupVertexShader; // [esp+94h] [ebp-4Ch]
    unsigned int drawSurfIndex; // [esp+98h] [ebp-48h]
    int updatedFade; // [esp+9Ch] [ebp-44h]
    GfxDrawSurf drawSurfMask; // [esp+A0h] [ebp-40h]
    const GfxModelSkinnedSurface *modelSurf; // [esp+ACh] [ebp-34h]
    float curDestructibleBurnAmount; // [esp+B0h] [ebp-30h]
    unsigned int depthHackFlags; // [esp+B4h] [ebp-2Ch]
    float curDestructibleFadeAmount; // [esp+B8h] [ebp-28h]
    float materialTime; // [esp+BCh] [ebp-24h]
    int currTextureOverride; // [esp+C0h] [ebp-20h]
    const GfxDrawSurf *drawSurfList; // [esp+C4h] [ebp-1Ch]
    unsigned __int64 drawSurfKey; // [esp+C8h] [ebp-18h]
    unsigned int drawSurfCount; // [esp+D4h] [ebp-Ch]
    unsigned int drawOutlineFlags; // [esp+D8h] [ebp-8h]
    int updatedBurnFadeConstant; // [esp+DCh] [ebp-4h]

    drawOutlineFlags = 0;
    updatedBurnFadeConstant = 0;
    context = listArgs->context;
    commonSource = listArgs->context.source;
    if (prepassContext.state
        && commonSource != prepassContext.source
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            1610,
            0,
            "%s",
            "prepassContext.local.state == NULL || commonSource == prepassContext.local.source"))
    {
        __debugbreak();
    }
    data = commonSource->input.data;
    info = listArgs->info;
    baseTechType = info->baseTechType;
    drawSurfCount = info->drawSurfCount - listArgs->firstDrawSurfIndex;
    drawSurfList = &info->drawSurfs[listArgs->firstDrawSurfIndex];
    drawSurf.fields = drawSurfList->fields;
    remotePlacement = &commonSource->skinnedPlacement;
    if (commonSource->objectPlacement != &commonSource->skinnedPlacement)
    {
        commonSource->objectPlacement = remotePlacement;
        R_ChangeObjectPlacementRemote(commonSource, remotePlacement);
    }
    updatedFade = R_UpdateCodeConstant(commonSource, CONST_SRC_CODE_CROSSFADE_PARMS, 1.0, 1.0, 1.0, 1.0);
    RB_TrackImmediatePrims(&context.state->prim, GFX_PRIM_STATS_XMODELSKINNED);
    drawSurfIndex = 0;
    if (info->cameraView)
    {
        modelSurf = (const GfxModelSkinnedSurface *)((char *)data + 4 * LOWORD(drawSurf.packed));
        baseGfxEntIndex = modelSurf->info.gfxEntIndex;
        if (modelSurf->info.gfxEntIndex)
        {
            depthHackFlags = data->gfxEnts[baseGfxEntIndex].renderFxFlags & 2;
            materialTime = data->gfxEnts[baseGfxEntIndex].materialTime;
            curDestructibleBurnAmount = data->gfxEnts[baseGfxEntIndex].destructibleBurnAmount;
            curDestructibleFadeAmount = data->gfxEnts[baseGfxEntIndex].destructibleFadeAmount;
            curWetness = data->gfxEnts[baseGfxEntIndex].wetness;
            currTextureOverride = data->gfxEnts[baseGfxEntIndex].textureOverrideIndex;
        }
        else
        {
            curDestructibleBurnAmount = 0.0f;
            curDestructibleFadeAmount = 0.0f;
            curWetness = 0.0f;
            depthHackFlags = 0;
            materialTime = 0.0f;
            currTextureOverride = -1;
        }
        if (RB_DrawSurf_Uses_ShaderConstantSet(&drawSurf))
        {
            updatedBurnFadeConstant = 0;
        }
        else
        {
            updatedBurnFadeConstant = R_UpdateCodeConstant(
                commonSource,
                CONST_SRC_CODE_DESTRUCTIBLE_PARMS,
                curDestructibleBurnAmount,
                r_burnedDestructibleColor->current.value,
                curDestructibleFadeAmount,
                0.0);
            if (r_swrk_override_enable->current.enabled)
            {
                y = r_swrk_override_wetness->current.value;
            }
            else
            {
                //LODWORD(y) = LODWORD(curWetness) & _mask__AbsFloat_;
                y = fabs(curWetness);
            }
            if (curWetness >= 0.0)
                z = 0.0f;
            else
                z = 1.0f;
            updated = R_UpdateCodeConstant(commonSource, CONST_SRC_CODE_CHARACTER_CHARRED_AMOUNT, curDestructibleBurnAmount, y, z, 0.0);
            updatedBurnFadeConstant |= updated;
        }
        v3 = R_UpdateMaterialTime(
            commonSource,
            materialTime,
            curDestructibleBurnAmount,
            curDestructibleFadeAmount,
            curWetness);
        setupVertexShader = updatedFade | updatedBurnFadeConstant | v3;
        R_SetupPassCriticalPixelShaderArgs(context);
        if (setupVertexShader)
            R_SetupPassVertexShaderArgs(context);
        R_ChangeDepthHackNearClip(commonSource, depthHackFlags);
        R_SetVertexDeclTypeModelSkinned(modelSurf->xsurf, context.state);
        if (depthHackFlags != context.state->depthRangeType)
            R_ChangeDepthRange(context.state, (GfxDepthRangeType)depthHackFlags);
        if (baseTechType == 10)
        {
            R_SetupPassPerObjectArgs(context);
            if (prepassContext.state)
            {
                R_SetupPassCriticalPixelShaderArgs(prepassContext);
                if (setupVertexShader)
                    R_SetupPassVertexShaderArgs(prepassContext);
                R_SetVertexDeclTypeModelSkinned(modelSurf->xsurf, prepassContext.state);
                if (depthHackFlags != prepassContext.state->depthRangeType)
                    R_ChangeDepthRange(prepassContext.state, (GfxDepthRangeType)depthHackFlags);
                R_SetupPassPerObjectArgs(prepassContext);
                R_SetupPassPerPrimArgs(prepassContext);
            }
            if (currTextureOverride >= 0)
                R_TextureOverride(data, context, modelSurf->info.dobjModelIndex, currTextureOverride);
            drawSurfMask.packed = -234946560;
            drawSurfKey = drawSurf.packed & 0xFFFFFFFFF1FF0000uLL;
            do
            {
                if (prepassContext.state)
                    R_DrawXModelSkinnedModelSurf(prepassContext, modelSurf);
                R_SetModelLightingCoordsForSource(modelSurf->info.lightingHandle, commonSource);
                R_SetReflectionProbe(context, (drawSurf.packed >> 25) & 7);
                R_SetupPassPerPrimArgs(context);
                R_DrawXModelSkinnedModelSurf(context, modelSurf);
                if (++drawSurfIndex == drawSurfCount)
                    break;
                //drawSurf.fields = (GfxDrawSurfFields)drawSurfList[drawSurfIndex];
                drawSurf.packed = drawSurfList[drawSurfIndex].packed;
                if ((drawSurfMask.packed & drawSurf.packed) != drawSurfKey)
                    break;
                v4 = 4 * LOWORD(drawSurf.packed);
                modelSurf = (const GfxModelSkinnedSurface *)&data->surfsBuffer[v4];
            } while (modelSurf->info.gfxEntIndex == baseGfxEntIndex
                || !R_DrawXModelSurfCheckBreak(
                    modelSurf->info.gfxEntIndex,
                    data,
                    depthHackFlags,
                    materialTime,
                    curDestructibleBurnAmount,
                    curDestructibleFadeAmount,
                    curWetness,
                    1,
                    currTextureOverride,
                    drawOutlineFlags));
        }
        else
        {
            if (prepassContext.state
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                    1798,
                    0,
                    "%s",
                    "prepassContext.local.state == NULL"))
            {
                __debugbreak();
            }
            if (baseTechType == 10
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                    1799,
                    0,
                    "%s",
                    "baseTechType != TECHNIQUE_LIT"))
            {
                __debugbreak();
            }
            R_SetupPassPerObjectArgs(context);
            R_SetupPassPerPrimArgs(context);
            if (currTextureOverride >= 0)
                R_TextureOverride(data, context, modelSurf->info.dobjModelIndex, currTextureOverride);
            drawSurfMask.packed = -267452416;
            drawSurfKey = drawSurf.packed & 0xFFFFFFFFF00F0000uLL;
            do
            {
                R_DrawXModelSkinnedModelSurf(context, modelSurf);
                if (++drawSurfIndex == drawSurfCount)
                    break;
                packed_high = HIDWORD(drawSurfList[drawSurfIndex].packed);
                //*(_DWORD *)&drawSurf.fields = drawSurfList[drawSurfIndex].fields;
                drawSurf.packed = drawSurfList[drawSurfIndex].packed;
                HIDWORD(drawSurf.packed) = packed_high;
                if (__PAIR64__(
                    HIDWORD(drawSurfMask.packed) & packed_high,
                    *(_DWORD *)&drawSurfMask.fields & *(_DWORD *)&drawSurf.fields) != drawSurfKey)
                    break;
                v6 = 4 * LOWORD(drawSurf.packed);
                modelSurf = (const GfxModelSkinnedSurface *)&data->surfsBuffer[v6];
            } while (modelSurf->info.gfxEntIndex == baseGfxEntIndex
                || !R_DrawXModelSurfCheckBreak(
                    modelSurf->info.gfxEntIndex,
                    data,
                    depthHackFlags,
                    materialTime,
                    curDestructibleBurnAmount,
                    curDestructibleFadeAmount,
                    curWetness,
                    1,
                    currTextureOverride,
                    drawOutlineFlags));
        }
    }
    else
    {
        if (prepassContext.state
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                1852,
                0,
                "%s",
                "prepassContext.local.state == NULL"))
        {
            __debugbreak();
        }
        if (baseTechType == 10
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                1853,
                0,
                "%s",
                "baseTechType != TECHNIQUE_LIT"))
        {
            __debugbreak();
        }
        modelSurf = (const GfxModelSkinnedSurface *)((char *)data + 4 * LOWORD(drawSurf.packed));
        if (modelSurf->info.gfxEntIndex)
            materialTime = data->gfxEnts[modelSurf->info.gfxEntIndex].materialTime;
        else
            materialTime = 0.0f;
        setupVertexShader = R_UpdateMaterialTime(commonSource, materialTime, 0.0, 0.0, 0.0);
        R_SetupPassCriticalPixelShaderArgs(context);
        if (setupVertexShader)
            R_SetupPassVertexShaderArgs(context);
        R_ChangeDepthHackNearClip(commonSource, 0);
        R_SetVertexDeclTypeModelSkinned(modelSurf->xsurf, context.state);
        if (context.state->depthRangeType != GFX_DEPTH_RANGE_FULL)
            R_ChangeDepthRange(context.state, GFX_DEPTH_RANGE_FULL);
        R_SetupPassPerObjectArgs(context);
        R_SetupPassPerPrimArgs(context);
        drawSurfMask.packed = -267452416;
        drawSurfKey = drawSurf.packed & 0xFFFFFFFFF00F0000uLL;
        while (1)
        {
            R_DrawXModelSkinnedModelSurf(context, modelSurf);
            if (++drawSurfIndex == drawSurfCount)
                break;
            //drawSurf.fields = (GfxDrawSurfFields)drawSurfList[drawSurfIndex];
            drawSurf.packed = drawSurfList[drawSurfIndex].packed;
            if ((drawSurfMask.packed & drawSurf.packed) != drawSurfKey)
                break;
            modelSurf = (const GfxModelSkinnedSurface *)((char *)data + 4 * LOWORD(drawSurf.packed));
        }
    }
    RB_EndTrackImmediatePrims(&context.state->prim);
    if (!drawSurfIndex
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp", 1937, 0, "%s", "drawSurfIndex"))
    {
        __debugbreak();
    }
    return drawSurfIndex;
}

unsigned int __cdecl R_TessXModelRigidDrawSurfList(
    const GfxDrawSurfListArgs *listArgs,
    GfxCmdBufContext prepassContext)
{
    const char *v2; // eax
    int updated; // eax
    int v4; // eax
    unsigned int gfxEntIndex; // [esp+2Ch] [ebp-58h]
    GfxCmdBufContext context; // [esp+30h] [ebp-54h]
    const GfxDrawSurfListInfo *info; // [esp+38h] [ebp-4Ch]
    GfxDrawSurf drawSurf; // [esp+3Ch] [ebp-48h] BYREF
    const GfxBackEndData *data; // [esp+48h] [ebp-3Ch]
    GfxCmdBufSourceState *commonSource; // [esp+4Ch] [ebp-38h]
    float curWetness; // [esp+50h] [ebp-34h]
    unsigned __int8 baseTechType; // [esp+57h] [ebp-2Dh]
    int setupVertexShader; // [esp+58h] [ebp-2Ch]
    unsigned int drawSurfIndex; // [esp+5Ch] [ebp-28h]
    int updatedFade; // [esp+60h] [ebp-24h]
    const GfxModelRigidSurface *modelSurf; // [esp+64h] [ebp-20h]
    float curDestructibleBurnAmount; // [esp+68h] [ebp-1Ch]
    unsigned int depthHackFlags; // [esp+6Ch] [ebp-18h]
    float curDestructibleFadeAmount; // [esp+70h] [ebp-14h]
    float materialTime; // [esp+74h] [ebp-10h]
    const GfxDrawSurf *drawSurfList; // [esp+78h] [ebp-Ch]
    unsigned int drawSurfCount; // [esp+7Ch] [ebp-8h]
    int updatedBurnFadeConstant; // [esp+80h] [ebp-4h]

    updatedBurnFadeConstant = 0;
    context = listArgs->context;
    commonSource = listArgs->context.source;
    if (prepassContext.state
        && commonSource != prepassContext.source
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            1966,
            0,
            "%s",
            "prepassContext.local.state == NULL || commonSource == prepassContext.local.source"))
    {
        __debugbreak();
    }
    data = commonSource->input.data;
    info = listArgs->info;
    baseTechType = info->baseTechType;
    if (r_logFile->current.integer)
    {
        v2 = va("--- RB_TessXModelRigid( %s ) ---\n", context.state->material->info.name);
        RB_LogPrint(v2);
    }
    drawSurfCount = info->drawSurfCount - listArgs->firstDrawSurfIndex;
    drawSurfList = &info->drawSurfs[listArgs->firstDrawSurfIndex];
    drawSurf.fields = drawSurfList->fields;
    commonSource->objectPlacement = &s_manualObjectPlacement;
    updatedFade = R_UpdateCodeConstant(commonSource, CONST_SRC_CODE_CROSSFADE_PARMS, 1.0, 1.0, 1.0, 1.0);
    RB_TrackImmediatePrims(&context.state->prim, GFX_PRIM_STATS_XMODELRIGID);
    if (!context.state->prim.primStats
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            1991,
            0,
            "%s",
            "context.state->prim.primStats"))
    {
        __debugbreak();
    }
    if (info->cameraView)
    {
        modelSurf = (const GfxModelRigidSurface *)((char *)data + 4 * LOWORD(drawSurf.packed));
        gfxEntIndex = modelSurf->surf.info.gfxEntIndex;
        if (modelSurf->surf.info.gfxEntIndex)
        {
            depthHackFlags = data->gfxEnts[gfxEntIndex].renderFxFlags & 2;
            materialTime = data->gfxEnts[gfxEntIndex].materialTime;
            curDestructibleFadeAmount = data->gfxEnts[gfxEntIndex].destructibleFadeAmount;
            curDestructibleBurnAmount = data->gfxEnts[gfxEntIndex].destructibleBurnAmount;
            curWetness = data->gfxEnts[gfxEntIndex].wetness;
            if (data->gfxEnts[gfxEntIndex].textureOverrideIndex != -1)
                R_TextureOverride(
                    data,
                    context,
                    modelSurf->surf.info.dobjModelIndex,
                    data->gfxEnts[gfxEntIndex].textureOverrideIndex);
        }
        else
        {
            depthHackFlags = 0;
            materialTime = 0.0f;
            curDestructibleBurnAmount = 0.0f;
            curDestructibleFadeAmount = 0.0f;
            curWetness = 0.0f;
        }
        if (RB_DrawSurf_Uses_ShaderConstantSet(&drawSurf))
        {
            updatedBurnFadeConstant = 0;
        }
        else
        {
            updatedBurnFadeConstant = R_UpdateCodeConstant(
                commonSource,
                CONST_SRC_CODE_DESTRUCTIBLE_PARMS,
                curDestructibleBurnAmount,
                r_burnedDestructibleColor->current.value,
                curDestructibleFadeAmount,
                0.0);
            updated = R_UpdateCodeConstant(commonSource, CONST_SRC_CODE_CHARACTER_CHARRED_AMOUNT, curDestructibleBurnAmount, 0.0, 0.0, 0.0);
            updatedBurnFadeConstant |= updated;
        }
        v4 = R_UpdateMaterialTime(
            commonSource,
            materialTime,
            curDestructibleBurnAmount,
            curDestructibleFadeAmount,
            curWetness);
        setupVertexShader = updatedFade | updatedBurnFadeConstant | v4;
        R_SetupPassCriticalPixelShaderArgs(context);
        if (setupVertexShader)
            R_SetupPassVertexShaderArgs(context);
        R_ChangeDepthHackNearClip(commonSource, depthHackFlags);
        R_SetVertexDeclTypeModelSkinned(modelSurf->surf.xsurf, context.state);
        if (depthHackFlags != context.state->depthRangeType)
            R_ChangeDepthRange(context.state, (GfxDepthRangeType)depthHackFlags);
        if (baseTechType == 10)
        {
            if (prepassContext.state)
            {
                R_SetupPassCriticalPixelShaderArgs(prepassContext);
                if (setupVertexShader)
                    R_SetupPassVertexShaderArgs(prepassContext);
                R_SetVertexDeclTypeModelSkinned(modelSurf->surf.xsurf, prepassContext.state);
                if (depthHackFlags != prepassContext.state->depthRangeType)
                    R_ChangeDepthRange(prepassContext.state, (GfxDepthRangeType)depthHackFlags);
            }
            if (prepassContext.state)
                R_SetupPassPerObjectArgs(prepassContext);
            R_SetupPassPerObjectArgs(context);
            drawSurfIndex = R_DrawXModelSurfLit(drawSurfList, drawSurfCount, context);
        }
        else
        {
            if (prepassContext.state
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                    2108,
                    0,
                    "%s",
                    "prepassContext.local.state == NULL"))
            {
                __debugbreak();
            }
            if (baseTechType == 10
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                    2109,
                    0,
                    "%s",
                    "baseTechType != TECHNIQUE_LIT"))
            {
                __debugbreak();
            }
            R_SetupPassPerObjectArgs(context);
            drawSurfIndex = R_DrawXModelSurfCamera(drawSurfList, drawSurfCount, context);
        }
    }
    else
    {
        if (prepassContext.state
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                2118,
                0,
                "%s",
                "prepassContext.local.state == NULL"))
        {
            __debugbreak();
        }
        if (baseTechType == 10
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                2119,
                0,
                "%s",
                "baseTechType != TECHNIQUE_LIT"))
        {
            __debugbreak();
        }
        R_SetupPassCriticalPixelShaderArgs(context);
        R_ChangeDepthHackNearClip(commonSource, 0);
        R_SetVertexDeclTypeNormal(context.state, VERTDECL_PACKED);
        if (context.state->depthRangeType != GFX_DEPTH_RANGE_FULL)
            R_ChangeDepthRange(context.state, GFX_DEPTH_RANGE_FULL);
        R_SetupPassPerObjectArgs(context);
        drawSurfIndex = R_DrawXModelSurf(drawSurfList, drawSurfCount, context);
    }
    RB_EndTrackImmediatePrims(&context.state->prim);
    if (!drawSurfIndex
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp", 2146, 0, "%s", "drawSurfIndex"))
    {
        __debugbreak();
    }
    return drawSurfIndex;
}

unsigned int __cdecl R_TessXModelWaterList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext)
{
    GfxCmdBufState *state; // edx
    const char *v3; // eax
    GfxMatrix *ActiveWorldMatrix; // eax
    GfxMatrix *matrix; // [esp+28h] [ebp-90h]
    float origin[3]; // [esp+2Ch] [ebp-8Ch] BYREF
    unsigned int offset1; // [esp+38h] [ebp-80h]
    unsigned int offset0; // [esp+3Ch] [ebp-7Ch]
    const GfxModelWaterSurface *modelSurf; // [esp+40h] [ebp-78h]
    float axis[3][3]; // [esp+44h] [ebp-74h] BYREF
    GfxDrawPrimArgs args; // [esp+68h] [ebp-50h] BYREF
    GfxCmdBufContext context; // [esp+74h] [ebp-44h]
    const GfxDrawSurfListInfo *info; // [esp+7Ch] [ebp-3Ch]
    GfxDrawSurf drawSurf; // [esp+80h] [ebp-38h]
    const GfxBackEndData *data; // [esp+8Ch] [ebp-2Ch]
    GfxCmdBufSourceState *commonSource; // [esp+90h] [ebp-28h]
    unsigned int drawSurfIndex; // [esp+94h] [ebp-24h]
    GfxDrawSurf drawSurfMask; // [esp+98h] [ebp-20h]
    const GfxDrawSurf *drawSurfList; // [esp+A4h] [ebp-14h]
    unsigned __int64 drawSurfKey; // [esp+A8h] [ebp-10h]
    unsigned int drawSurfCount; // [esp+B4h] [ebp-4h]

    state = listArgs->context.state;
    context.source = listArgs->context.source;
    context.state = state;
    commonSource = context.source;
    if (prepassContext.state
        && commonSource != prepassContext.source
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            2162,
            0,
            "%s",
            "prepassContext.local.state == NULL || commonSource == prepassContext.local.source"))
    {
        __debugbreak();
    }
    info = listArgs->info;
    if (r_logFile->current.integer)
    {
        v3 = va("--- R_TessXModelWaterList( %s ) ---\n", context.state->material->info.name);
        RB_LogPrint(v3);
    }
    drawSurfCount = info->drawSurfCount - listArgs->firstDrawSurfIndex;
    drawSurfList = &info->drawSurfs[listArgs->firstDrawSurfIndex];
    drawSurf.fields = drawSurfList->fields;
    data = commonSource->input.data;
    if (!commonSource->cameraView
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            2180,
            0,
            "%s",
            "commonSource->cameraView"))
    {
        __debugbreak();
    }
    drawSurfMask.packed = 0xFFF807FFF00F0000uLL;
    drawSurfKey = drawSurf.packed & 0xFFF807FFF00F0000uLL;
    commonSource->objectPlacement = &s_manualObjectPlacement;
    R_UpdateMaterialTime(commonSource, 0.0, 0.0, 0.0, 0.0);
    if (context.state->depthRangeType)
        R_ChangeDepthRange(context.state, GFX_DEPTH_RANGE_SCENE);
    R_SetupPassVertexShaderArgs(context);
    R_SetupPassCriticalPixelShaderArgs(context);
    R_SetVertexDeclTypeNormal(context.state, VERTDECL_WATER);
    RB_TrackImmediatePrims(&context.state->prim, GFX_PRIM_STATS_CODE);
    if (!context.state->prim.primStats
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            2210,
            0,
            "%s",
            "context.state->prim.primStats"))
    {
        __debugbreak();
    }
    drawSurfIndex = 0;
    do
    {
        modelSurf = (const GfxModelWaterSurface *)((char *)data + 4 * LOWORD(drawSurf.packed));
        matrix = R_GetActiveWorldMatrix(commonSource)->matrices.matrix;
        MatrixIdentity33(axis);
        origin[0] = modelSurf->origin[0] - commonSource->eyeOffset[0];
        origin[1] = modelSurf->origin[1] - commonSource->eyeOffset[1];
        origin[2] = modelSurf->origin[2] - commonSource->eyeOffset[2];
        ikMatrixSet44(matrix->m, origin, axis, 1.0);
        args.baseIndex = 0;
        args.triCount = modelSurf->triCount;
        args.vertexCount = modelSurf->vertCount;
        offset0 = 16 * modelSurf->baseVertex0;
        offset1 = 16 * modelSurf->baseVertex1;
        if (context.state->prim.indexBuffer != modelSurf->ib)
            R_ChangeIndices(&context.state->prim, modelSurf->ib);
        R_SetDoubleStreamSource(&context.state->prim, modelSurf->vb0, offset0, 0x10u, modelSurf->vb1, offset1, 0x10u);
        R_SetWaterSimulationConstants(commonSource, modelSurf->interpolate);
        R_SetupPassPerObjectArgs(context);
        R_SetupPassPerPrimArgs(context);
        R_DrawIndexedPrimitive(&context.state->prim, &args);
        if (r_showTess->current.integer)
        {
            ActiveWorldMatrix = R_GetActiveWorldMatrix(commonSource)->matrices.matrix;
            RB_ShowTess(context, ActiveWorldMatrix->m[3], "XMRigid", colorWhite);
        }
        if (++drawSurfIndex == drawSurfCount)
            break;
        //drawSurf.fields = (GfxDrawSurfFields)drawSurfList[drawSurfIndex];
        drawSurf.packed = drawSurfList[drawSurfIndex].packed;
    } while ((drawSurfMask.packed & drawSurf.packed) == drawSurfKey);
    RB_EndTrackImmediatePrims(&context.state->prim);
    return drawSurfIndex;
}

void __cdecl R_SetDoubleStreamSource(
                GfxCmdBufPrimState *primState,
                IDirect3DVertexBuffer9 *vb0,
                unsigned int vertexOffset0,
                unsigned int vertexStride0,
                IDirect3DVertexBuffer9 *vb1,
                unsigned int vertexOffset1,
                unsigned int vertexStride1)
{
    if ( primState->streams[0].vb != vb0
        || primState->streams[0].offset != vertexOffset0
        || primState->streams[0].stride != vertexStride0 )
    {
        R_ChangeStreamSource(primState, 0, vb0, vertexOffset0, vertexStride0);
    }
    if ( primState->streams[1].vb != vb1
        || primState->streams[1].offset != vertexOffset1
        || primState->streams[1].stride != vertexStride1 )
    {
        R_ChangeStreamSource(primState, 1u, vb1, vertexOffset1, vertexStride1);
    }
    if ( primState->streams[2].vb || primState->streams[2].offset || primState->streams[2].stride )
        R_ChangeStreamSource(primState, 2u, 0, 0, 0);
}

unsigned int __cdecl R_TessXModelRigidSkinnedDrawSurfList(
    const GfxDrawSurfListArgs *listArgs,
    GfxCmdBufContext prepassContext)
{
    const char *v2; // eax
    GfxScaledPlacement *remotePlacement; // [esp+20h] [ebp-78h]
    unsigned int gfxEntIndex; // [esp+28h] [ebp-70h]
    GfxCmdBufContext context; // [esp+2Ch] [ebp-6Ch]
    const GfxDrawSurfListInfo *info; // [esp+34h] [ebp-64h]
    GfxDrawSurf drawSurf; // [esp+38h] [ebp-60h]
    const GfxBackEndData *data; // [esp+44h] [ebp-54h]
    GfxCmdBufSourceState *commonSource; // [esp+48h] [ebp-50h]
    int setupPixelShader; // [esp+4Ch] [ebp-4Ch]
    unsigned __int64 drawSurfSubKey; // [esp+50h] [ebp-48h]
    GfxDrawSurf drawSurfSubMask; // [esp+58h] [ebp-40h]
    unsigned __int8 baseTechType; // [esp+67h] [ebp-31h]
    int setupVertexShader; // [esp+68h] [ebp-30h]
    unsigned int drawSurfIndex; // [esp+6Ch] [ebp-2Ch]
    const GfxModelRigidSurface *modelSurf; // [esp+78h] [ebp-20h]
    GfxDepthRangeType depthHackFlags; // [esp+7Ch] [ebp-1Ch]
    float materialTime; // [esp+80h] [ebp-18h]
    const GfxDrawSurf *drawSurfList; // [esp+84h] [ebp-14h]
    unsigned __int64 drawSurfKey; // [esp+88h] [ebp-10h]
    unsigned int drawSurfCount; // [esp+94h] [ebp-4h]

    context = listArgs->context;
    commonSource = listArgs->context.source;
    if (prepassContext.state
        && commonSource != prepassContext.source
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            2306,
            0,
            "%s",
            "prepassContext.state == NULL || commonSource == prepassContext.source"))
    {
        __debugbreak();
    }
    data = commonSource->input.data;
    info = listArgs->info;
    baseTechType = info->baseTechType;
    if (r_logFile->current.integer)
    {
        v2 = va("--- R_TessXModelRigidSkinnedDrawSurfList( %s ) ---\n", context.state->material->info.name);
        RB_LogPrint(v2);
    }
    drawSurfCount = info->drawSurfCount - listArgs->firstDrawSurfIndex;
    drawSurfList = &info->drawSurfs[listArgs->firstDrawSurfIndex];
    if (!info->cameraView)
    {
        R_SetupPassCriticalPixelShaderArgs(context);
        R_ChangeDepthHackNearClip(commonSource, 0);
        if (context.state->depthRangeType != GFX_DEPTH_RANGE_FULL)
            R_ChangeDepthRange(context.state, GFX_DEPTH_RANGE_FULL);
    }
    R_SetVertexDeclTypeNormal(context.state, VERTDECL_PACKED);
    setupPixelShader = 1;
    drawSurfSubMask.packed = -65536;
    if (baseTechType != 10)
        *(_DWORD *)&drawSurfSubMask.fields = -267452416;
    drawSurf.fields = drawSurfList->fields;
    drawSurfKey = drawSurfList->packed & 0xFFFFFFFFF00F0000uLL;
    RB_TrackImmediatePrims(&context.state->prim, GFX_PRIM_STATS_XMODELRIGID);
    if (!context.state->prim.primStats
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            2347,
            0,
            "%s",
            "context.state->prim.primStats"))
    {
        __debugbreak();
    }
    drawSurfIndex = 0;
    do
    {
        drawSurfSubKey = drawSurfSubMask.packed & drawSurf.packed;
        do
        {
            modelSurf = (const GfxModelRigidSurface *)((char *)data + 4 * LOWORD(drawSurf.packed));
            if (info->cameraView)
            {
                gfxEntIndex = modelSurf->surf.info.gfxEntIndex;
                if (modelSurf->surf.info.gfxEntIndex)
                {
                    depthHackFlags = (GfxDepthRangeType)(data->gfxEnts[gfxEntIndex].renderFxFlags & 2);
                    materialTime = data->gfxEnts[gfxEntIndex].materialTime;
                }
                else
                {
                    depthHackFlags = GFX_DEPTH_RANGE_SCENE;
                    materialTime = 0.0f;
                }
                setupVertexShader = R_UpdateMaterialTime(commonSource, materialTime, 0.0, 0.0, 0.0);
                if (setupVertexShader | setupPixelShader)
                    R_SetupPassCriticalPixelShaderArgs(context);
                if (setupVertexShader)
                    R_SetupPassVertexShaderArgs(context);
                R_ChangeDepthHackNearClip(commonSource, depthHackFlags);
                if (baseTechType == 10)
                {
                    R_SetModelLightingCoordsForSource(modelSurf->surf.info.lightingHandle, commonSource);
                    R_SetReflectionProbe(context, (drawSurf.packed >> 25) & 7);
                }
                if (depthHackFlags != context.state->depthRangeType)
                    R_ChangeDepthRange(context.state, depthHackFlags);
                setupPixelShader = 0;
            }
            remotePlacement = (GfxScaledPlacement *)&modelSurf->placement;
            if (commonSource->objectPlacement != &modelSurf->placement)
            {
                commonSource->objectPlacement = remotePlacement;
                R_ChangeObjectPlacementRemote(commonSource, remotePlacement);
            }
            R_SetupPassPerObjectArgs(context);
            R_SetupPassPerPrimArgs(context);
            R_DrawXModelSkinnedUncached(context, modelSurf->surf.xsurf, modelSurf->surf.xsurf->verts0);
            if (++drawSurfIndex == drawSurfCount)
                break;
            //drawSurf.fields = (GfxDrawSurfFields)drawSurfList[drawSurfIndex];
            drawSurf.packed = drawSurfList[drawSurfIndex].packed;
        } while ((drawSurfSubMask.packed & drawSurf.packed) == drawSurfSubKey);
    } while (drawSurfIndex != drawSurfCount
        && __PAIR64__(HIDWORD(drawSurf.packed), *(_DWORD *)&drawSurf.fields & 0xF00F0000) == drawSurfKey);
    RB_EndTrackImmediatePrims(&context.state->prim);
    return drawSurfIndex;
}

unsigned int __cdecl R_TessStaticModelRigidDrawSurfList(
    const GfxDrawSurfListArgs *listArgs,
    GfxCmdBufContext prepassContext)
{
    GfxDepthRangeType v4; // [esp+4h] [ebp-28h]
    GfxDepthRangeType depthRangeType; // [esp+8h] [ebp-24h]
    GfxCmdBufContext context; // [esp+Ch] [ebp-20h]
    const GfxDrawSurfListInfo *info; // [esp+14h] [ebp-18h]
    GfxCmdBufSourceState *commonSource; // [esp+1Ch] [ebp-10h]
    unsigned __int8 baseTechType; // [esp+23h] [ebp-9h]
    const unsigned int *primDrawSurfPos; // [esp+24h] [ebp-8h]

    context = listArgs->context;
    commonSource = listArgs->context.source;
    if (prepassContext.state
        && commonSource != prepassContext.source
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            2450,
            0,
            "%s",
            "prepassContext.local.state == NULL || commonSource == prepassContext.local.source"))
    {
        __debugbreak();
    }
    info = listArgs->info;
    if (r_logFile->current.integer)
    {
        RB_LogPrint(va("--- R_TessStaticModelRigidDrawSurfList( %s ) ---\n", context.state->material->info.name));
    }
    baseTechType = info->baseTechType;
    R_SetupPassCriticalPixelShaderArgs(context);
    R_SetGameTime(context.source, context.source->sceneDef.floatTime);
    R_SetupPassVertexShaderArgs(context);
    if (prepassContext.state)
        R_SetupPassVertexShaderArgs(prepassContext);
    commonSource->objectPlacement = &s_manualObjectPlacement;
    R_ChangeDepthHackNearClip(commonSource, 0);
    R_SetVertexDeclTypeNormal(context.state, VERTDECL_PACKED);
    depthRangeType = (GfxDepthRangeType)((commonSource->cameraView != 0) - 1);
    if (depthRangeType != context.state->depthRangeType)
        R_ChangeDepthRange(context.state, depthRangeType);
    if (prepassContext.state)
    {
        R_SetupPassCriticalPixelShaderArgs(prepassContext);
        R_SetVertexDeclTypeNormal(prepassContext.state, VERTDECL_PACKED);
        v4 = (GfxDepthRangeType)((commonSource->cameraView != 0) - 1);
        if (v4 != prepassContext.state->depthRangeType)
            R_ChangeDepthRange(prepassContext.state, v4);
    }
    primDrawSurfPos = &commonSource->input.data->primDrawSurfsBuf[(unsigned __int16)*(_DWORD *)&info->drawSurfs[listArgs->firstDrawSurfIndex].fields];
    RB_TrackImmediatePrims(&context.state->prim, GFX_PRIM_STATS_SMODELRIGID);
    if (!context.state->prim.primStats
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            2499,
            0,
            "%s",
            "context.state->prim.primStats"))
    {
        __debugbreak();
    }
    if (baseTechType == 10)
    {
        if (prepassContext.state)
            R_SetupPassPerObjectArgs(prepassContext);
        R_SetupPassPerObjectArgs(context);
        R_DrawStaticModelSurfLit(primDrawSurfPos, context, prepassContext, info);
    }
    else
    {
        if (prepassContext.state
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                2516,
                0,
                "%s",
                "prepassContext.local.state == NULL"))
        {
            __debugbreak();
        }
        R_SetupPassPerObjectArgs(context);
        R_DrawStaticModelSurf(primDrawSurfPos, context, info);
    }
    RB_EndTrackImmediatePrims(&context.state->prim);
    return 1;
}

unsigned int __cdecl R_TessStaticModelSkinnedDrawSurfList(
    const GfxDrawSurfListArgs *listArgs,
    GfxCmdBufContext prepassContext)
{
    const char *v2; // eax
    GfxDepthRangeType depthRangeType; // [esp+0h] [ebp-20h]
    GfxCmdBufContext context; // [esp+4h] [ebp-1Ch]
    const GfxDrawSurfListInfo *info; // [esp+Ch] [ebp-14h]
    GfxCmdBufSourceState *commonSource; // [esp+14h] [ebp-Ch]
    unsigned __int8 baseTechType; // [esp+1Bh] [ebp-5h]
    const unsigned int *primDrawSurfPos; // [esp+1Ch] [ebp-4h]

    context = listArgs->context;
    commonSource = listArgs->context.source;
    if (prepassContext.state
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            2546,
            0,
            "%s",
            "prepassContext.state == NULL"))
    {
        __debugbreak();
    }
    info = listArgs->info;
    if (r_logFile->current.integer)
    {
        v2 = va("--- R_TessStaticModelSkinnedDrawSurfList( %s ) ---\n", context.state->material->info.name);
        RB_LogPrint(v2);
    }
    R_SetupPassCriticalPixelShaderArgs(context);
    baseTechType = info->baseTechType;
    commonSource->objectPlacement = &s_manualObjectPlacement;
    R_ChangeDepthHackNearClip(commonSource, 0);
    R_SetVertexDeclTypeNormal(context.state, VERTDECL_PACKED);
    depthRangeType = (GfxDepthRangeType)((commonSource->cameraView != 0) - 1);
    if (depthRangeType != context.state->depthRangeType)
        R_ChangeDepthRange(context.state, depthRangeType);
    primDrawSurfPos = &commonSource->input.data->primDrawSurfsBuf[(unsigned __int16)*(_DWORD *)&info->drawSurfs[listArgs->firstDrawSurfIndex].fields];
    RB_TrackImmediatePrims(&context.state->prim, GFX_PRIM_STATS_SMODELRIGID);
    if (!context.state->prim.primStats
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            2567,
            0,
            "%s",
            "context.state->prim.primStats"))
    {
        __debugbreak();
    }
    if (baseTechType == 10)
        R_DrawStaticModelSkinnedSurfLit(primDrawSurfPos, context, info);
    else
        R_DrawStaticModelSkinnedSurf(primDrawSurfPos, context, info);
    RB_EndTrackImmediatePrims(&context.state->prim);
    return 1;
}

unsigned int __cdecl R_TessStaticModelPreTessList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext)
{
    const char *v2; // eax
    GfxDepthRangeType depthRangeType; // [esp+0h] [ebp-20h]
    GfxCmdBufContext context; // [esp+4h] [ebp-1Ch]
    const GfxDrawSurfListInfo *info; // [esp+Ch] [ebp-14h]
    GfxCmdBufSourceState *commonSource; // [esp+14h] [ebp-Ch]
    unsigned __int8 baseTechType; // [esp+1Bh] [ebp-5h]
    const unsigned int *primDrawSurfPos; // [esp+1Ch] [ebp-4h]

    context = listArgs->context;
    commonSource = listArgs->context.source;
    if (prepassContext.state
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            2602,
            0,
            "%s",
            "prepassContext.state == NULL"))
    {
        __debugbreak();
    }
    info = listArgs->info;
    if (r_logFile->current.integer)
    {
        v2 = va("--- RB_TessStaticModelCached( %s ) ---\n", context.state->material->info.name);
        RB_LogPrint(v2);
    }
    R_SetupPassCriticalPixelShaderArgs(context);
    baseTechType = info->baseTechType;
    R_ChangeDepthHackNearClip(commonSource, 0);
    R_SetObjectIdentityPlacement(commonSource);
    R_SetVertexDeclTypeNormal(context.state, VERTDECL_STATICMODELCACHE);
    depthRangeType = (GfxDepthRangeType)((commonSource->cameraView != 0) - 1);
    if (depthRangeType != context.state->depthRangeType)
        R_ChangeDepthRange(context.state, depthRangeType);
    primDrawSurfPos = &commonSource->input.data->primDrawSurfsBuf[(unsigned __int16)*(_DWORD *)&info->drawSurfs[listArgs->firstDrawSurfIndex].fields];
    R_TrackPrims(&context.state->prim, GFX_PRIM_STATS_SMODELCACHED);
    if (baseTechType == 10)
        R_DrawStaticModelPreTessSurfLit(primDrawSurfPos, context);
    else
        R_DrawStaticModelPreTessSurf(primDrawSurfPos, context);
    context.state->prim.primStats = 0;
    return 1;
}

unsigned int __cdecl R_TessStaticModelCachedList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext)
{
    const char *v2; // eax
    GfxDepthRangeType depthRangeType; // [esp+0h] [ebp-20h]
    GfxCmdBufContext context; // [esp+4h] [ebp-1Ch]
    const GfxDrawSurfListInfo *info; // [esp+Ch] [ebp-14h]
    GfxCmdBufSourceState *commonSource; // [esp+14h] [ebp-Ch]
    unsigned __int8 baseTechType; // [esp+1Bh] [ebp-5h]
    const unsigned int *primDrawSurfPos; // [esp+1Ch] [ebp-4h]

    context = listArgs->context;
    commonSource = listArgs->context.source;
    if (prepassContext.state
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            2658,
            0,
            "%s",
            "prepassContext.state == NULL"))
    {
        __debugbreak();
    }
    info = listArgs->info;
    if (r_logFile->current.integer)
    {
        v2 = va("--- RB_TessStaticModelCached( %s ) ---\n", context.state->material->info.name);
        RB_LogPrint(v2);
    }
    R_SetupPassCriticalPixelShaderArgs(context);
    baseTechType = info->baseTechType;
    R_ChangeDepthHackNearClip(commonSource, 0);
    R_SetObjectIdentityPlacement(commonSource);
    R_SetVertexDeclTypeNormal(context.state, VERTDECL_STATICMODELCACHE);
    depthRangeType = (GfxDepthRangeType)((commonSource->cameraView != 0) - 1);
    if (depthRangeType != context.state->depthRangeType)
        R_ChangeDepthRange(context.state, depthRangeType);
    primDrawSurfPos = &commonSource->input.data->primDrawSurfsBuf[(unsigned __int16)*(_DWORD *)&info->drawSurfs[listArgs->firstDrawSurfIndex].fields];
    R_TrackPrims(&context.state->prim, GFX_PRIM_STATS_SMODELCACHED);
    if (baseTechType == 10)
        R_DrawStaticModelCachedSurfLit(primDrawSurfPos, context);
    else
        R_DrawStaticModelCachedSurf(primDrawSurfPos, context);
    context.state->prim.primStats = 0;
    return 1;
}

unsigned int __cdecl R_TessTrianglesList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext)
{
    GfxDepthRangeType v3; // [esp+0h] [ebp-28h]
    GfxDepthRangeType depthRangeType; // [esp+4h] [ebp-24h]
    GfxCmdBufContext context; // [esp+8h] [ebp-20h]
    const GfxDrawSurfListInfo *info; // [esp+10h] [ebp-18h]
    GfxCmdBufSourceState *commonSource; // [esp+18h] [ebp-10h]
    unsigned __int8 baseTechType; // [esp+1Fh] [ebp-9h]
    const unsigned int *primDrawSurfPos; // [esp+20h] [ebp-8h]

    context = listArgs->context;
    commonSource = listArgs->context.source;
    if (prepassContext.state
        && context.source != prepassContext.source
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            2740,
            0,
            "%s",
            "prepassContext.local.state == NULL || context.local.source == prepassContext.local.source"))
    {
        __debugbreak();
    }
    info = listArgs->info;
    baseTechType = info->baseTechType;
    R_SetupPassCriticalPixelShaderArgs(context);
    R_SetObjectIdentityPlacement(commonSource);
    R_ChangeDepthHackNearClip(commonSource, 0);
    R_SetVertexDeclTypeWorld(context.state);
    depthRangeType = (GfxDepthRangeType)((commonSource->cameraView != 0) - 1);
    if (depthRangeType != context.state->depthRangeType)
        R_ChangeDepthRange(context.state, depthRangeType);
    if (prepassContext.state)
    {
        R_SetupPassCriticalPixelShaderArgs(prepassContext);
        R_SetVertexDeclTypeWorld(prepassContext.state);
        v3 = (GfxDepthRangeType)((commonSource->cameraView != 0) - 1);
        if (v3 != prepassContext.state->depthRangeType)
            R_ChangeDepthRange(prepassContext.state, v3);
    }
    primDrawSurfPos = &commonSource->input.data->primDrawSurfsBuf[(unsigned __int16)*(_DWORD *)&info->drawSurfs[listArgs->firstDrawSurfIndex].fields];
    R_TrackPrims(&context.state->prim, GFX_PRIM_STATS_WORLD);
    if (baseTechType == 10)
    {
        R_SetTerrainScorchTextures(listArgs->context.source, listArgs->context.state->material);
        R_SetupPassPerObjectArgs(context);
        R_SetupPassPerPrimArgs(context);
        if (prepassContext.state)
        {
            R_SetupPassPerObjectArgs(prepassContext);
            R_SetupPassPerPrimArgs(prepassContext);
        }
        R_DrawBspDrawSurfsLit(primDrawSurfPos, context, prepassContext);
    }
    else
    {
        R_SetupPassPerObjectArgs(context);
        R_SetupPassPerPrimArgs(context);
        R_DrawBspDrawSurfs(primDrawSurfPos, context.state);
    }
    context.state->prim.primStats = 0;
    return 1;
}

void __cdecl R_SetTerrainScorchTextures(GfxCmdBufSourceState *source, const Material *material)
{
    GfxImage *whiteImage; // [esp+0h] [ebp-24h]
    GfxImage *scorchedImage; // [esp+8h] [ebp-1Ch]
    unsigned int layerSurfType; // [esp+Ch] [ebp-18h]
    unsigned int layer; // [esp+10h] [ebp-14h]
    unsigned int layeredSurfTypes; // [esp+20h] [ebp-4h]

    if (g_worldDraw->terrainScorchImages[0])
        whiteImage = g_worldDraw->terrainScorchImages[0];
    else
        whiteImage = rgp.whiteImage;
    layeredSurfTypes = material->info.layeredSurfaceTypes;
    for (layer = 0; layer < layeredSurfTypes >> 29; ++layer)
    {
        layerSurfType = (layeredSurfTypes >> (6 * layer)) & 0x3F;
        if (layerSurfType >= 0x1F
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                2717,
                0,
                "layerSurfType doesn't index ARRAY_COUNT(g_worldDraw->terrainScorchImages)\n\t%i not in [0, %i)",
                layerSurfType,
                31))
        {
            __debugbreak();
        }
        scorchedImage = g_worldDraw->terrainScorchImages[layerSurfType];
        if (!scorchedImage)
            scorchedImage = whiteImage;
        R_SetCodeImageTexture(source, layer + 29, scorchedImage);
    }
}

unsigned int __cdecl R_TessTrianglesPreTessList(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext)
{
    IDirect3DIndexBuffer9 *ib; // [esp+0h] [ebp-28h]
    GfxDepthRangeType depthRangeType; // [esp+8h] [ebp-20h]
    GfxCmdBufContext context; // [esp+Ch] [ebp-1Ch]
    const GfxDrawSurfListInfo *info; // [esp+14h] [ebp-14h]
    const GfxBackEndData *data; // [esp+18h] [ebp-10h]
    GfxCmdBufSourceState *commonSource; // [esp+1Ch] [ebp-Ch]
    unsigned __int8 baseTechType; // [esp+23h] [ebp-5h]
    const unsigned int *primDrawSurfPos; // [esp+24h] [ebp-4h]

    context = listArgs->context;
    commonSource = listArgs->context.source;
    if (prepassContext.state
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            2828,
            0,
            "%s",
            "prepassContext.state == NULL"))
    {
        __debugbreak();
    }
    info = listArgs->info;
    R_SetTerrainScorchTextures(listArgs->context.source, listArgs->context.state->material);
    R_SetupPassCriticalPixelShaderArgs(context);
    baseTechType = info->baseTechType;
    R_SetObjectIdentityPlacement(commonSource);
    R_ChangeDepthHackNearClip(commonSource, 0);
    R_SetVertexDeclTypeWorld(context.state);
    depthRangeType = (GfxDepthRangeType)((commonSource->cameraView != 0) - 1);
    if (depthRangeType != context.state->depthRangeType)
        R_ChangeDepthRange(context.state, depthRangeType);
    data = commonSource->input.data;
    ib = data->preTessIb->buffer;
    if (context.state->prim.indexBuffer != ib)
        R_ChangeIndices(&context.state->prim, ib);
    primDrawSurfPos = &data->primDrawSurfsBuf[(unsigned __int16)*(_DWORD *)&info->drawSurfs[listArgs->firstDrawSurfIndex].fields];
    R_TrackPrims(&context.state->prim, GFX_PRIM_STATS_WORLD);
    if (baseTechType == 10)
        R_DrawBspDrawSurfsLitPreTess(primDrawSurfPos, context);
    else
        R_DrawBspDrawSurfsPreTess(primDrawSurfPos, context);
    context.state->prim.primStats = 0;
    return 1;
}

unsigned int __cdecl R_TessBModel(const GfxDrawSurfListArgs *listArgs, GfxCmdBufContext prepassContext)
{
    GfxCmdBufState *state; // eax
    int v3; // eax
    int vertexCount; // [esp+20h] [ebp-12Ch]
    const GfxScaledPlacement *remotePlacement; // [esp+28h] [ebp-124h]
    GfxDepthRangeType v7; // [esp+38h] [ebp-114h]
    GfxDepthRangeType depthRangeType; // [esp+40h] [ebp-10Ch]
    int baseIndex; // [esp+44h] [ebp-108h]
    int changed; // [esp+48h] [ebp-104h]
    float fade; // [esp+4Ch] [ebp-100h]
    float burn; // [esp+50h] [ebp-FCh]
    GfxCmdBufContext context; // [esp+54h] [ebp-F8h] BYREF
    int updatedConstant; // [esp+5Ch] [ebp-F0h]
    const GfxDrawSurfListInfo *info; // [esp+60h] [ebp-ECh]
    GfxDrawSurf drawSurf; // [esp+64h] [ebp-E8h]
    const GfxBackEndData *data; // [esp+70h] [ebp-DCh]
    GfxCmdBufSourceState *commonSource; // [esp+74h] [ebp-D8h]
    const srfTriangles_t *tris; // [esp+78h] [ebp-D4h]
    ShaderConstantSet savedConstantSet; // [esp+7Ch] [ebp-D0h] BYREF
    unsigned __int64 drawSurfSubKey; // [esp+FCh] [ebp-50h]
    GfxDrawSurf drawSurfSubMask; // [esp+104h] [ebp-48h]
    unsigned __int8 baseTechType; // [esp+10Fh] [ebp-3Dh]
    unsigned int drawSurfIndex; // [esp+110h] [ebp-3Ch]
    GfxDrawSurf drawSurfMask; // [esp+114h] [ebp-38h]
    ShaderConstantSet *constantSet; // [esp+120h] [ebp-2Ch]
    const GfxSurface *surf; // [esp+124h] [ebp-28h]
    const BModelSurface *bmodelSurf; // [esp+128h] [ebp-24h]
    const GfxDrawSurf *drawSurfList; // [esp+12Ch] [ebp-20h]
    GfxDrawPrimArgs args; // [esp+130h] [ebp-1Ch] BYREF
    unsigned __int64 drawSurfKey; // [esp+13Ch] [ebp-10h]
    int baseVertex; // [esp+144h] [ebp-8h]
    unsigned int drawSurfCount; // [esp+148h] [ebp-4h]

    R_InitShaderConstantSet(&savedConstantSet);
    state = listArgs->context.state;
    context.source = listArgs->context.source;
    context.state = state;
    commonSource = context.source;
    if (prepassContext.state
        && commonSource != prepassContext.source
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
            2939,
            0,
            "%s",
            "prepassContext.local.state == NULL || commonSource == prepassContext.local.source"))
    {
        __debugbreak();
    }
    info = listArgs->info;
    drawSurfList = &info->drawSurfs[listArgs->firstDrawSurfIndex];
    drawSurfCount = info->drawSurfCount - listArgs->firstDrawSurfIndex;
    R_SetTerrainScorchTextures(listArgs->context.source, listArgs->context.state->material);
    R_SetupPassCriticalPixelShaderArgs(context);
    data = commonSource->input.data;
    baseTechType = info->baseTechType;
    R_ChangeDepthHackNearClip(commonSource, 0);
    R_SetVertexDeclTypeWorld(context.state);
    depthRangeType = (GfxDepthRangeType)((commonSource->cameraView != 0) - 1);
    if (depthRangeType != context.state->depthRangeType)
        R_ChangeDepthRange(context.state, depthRangeType);
    if (prepassContext.state)
    {
        R_SetupPassCriticalPixelShaderArgs(prepassContext);
        R_SetVertexDeclTypeWorld(prepassContext.state);
        v7 = (GfxDepthRangeType)((commonSource->cameraView != 0) - 1);
        if (v7 != prepassContext.state->depthRangeType)
            R_ChangeDepthRange(prepassContext.state, v7);
        R_SetupPassPerObjectArgs(prepassContext);
    }
    drawSurf.fields = drawSurfList->fields;
    drawSurfMask.packed = -267452416;
    drawSurfSubMask.packed = -65536;
    if (baseTechType != 10)
    {
        *(_DWORD *)&drawSurfSubMask.fields &= 0xF1FFFFFF;
        *(_DWORD *)&drawSurfSubMask.fields &= 0xFE0FFFFF;
        R_SetupPassPerObjectArgs(context);
    }
    drawSurfKey = drawSurfMask.packed & drawSurf.packed;
    R_TrackPrims(&context.state->prim, GFX_PRIM_STATS_BMODEL);
    drawSurfIndex = 0;
    do
    {
        if (baseTechType == 10)
        {
            R_SetLightmap(context, (drawSurf.packed >> 20) & 0x1F);
            R_SetReflectionProbe(context, (drawSurf.packed >> 25) & 7);
            R_SetupPassPerObjectArgs(context);
        }
        drawSurfSubKey = drawSurfSubMask.packed & drawSurf.packed;
        baseVertex = -1;
        args.triCount = 0;
        args.vertexCount = 0;
        args.baseIndex = -1;
        fade = -1.0f;
        burn = -1.0f;
        constantSet = 0;
        do
        {
            bmodelSurf = (const BModelSurface *)((char *)data + 4 * LOWORD(drawSurf.packed));
            surf = bmodelSurf->surf;
            tris = &surf->tris;
            if (!surf->tris.triCount
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp", 3038, 0, "%s", "tris->triCount"))
            {
                __debugbreak();
            }
            changed = (commonSource->objectPlacement - bmodelSurf->placement)
                | (LODWORD(bmodelSurf->bmodelFadeAmt) - LODWORD(fade))
                | (LODWORD(bmodelSurf->bmodelBurnAmt) - LODWORD(burn))
                | ((char *)bmodelSurf->shaderConstSet - (char *)constantSet);
            if (3 * tris->triCount + gfxBuf.dynamicIndexBuffer->used > gfxBuf.dynamicIndexBuffer->total)
            {
                if (args.triCount > 0)
                    R_DrawBModelPrim(&prepassContext, &context, &args);
                args.triCount = 0;
                changed = 1;
            }
            baseIndex = R_SetIndexData(&context.state->prim, (unsigned char*)&g_worldDraw->indices[tris->baseIndex], tris->triCount);
            if (changed | (baseVertex - tris->firstVertex) | (args.baseIndex + 3 * args.triCount - baseIndex))
            {
                if (args.triCount > 0)
                    R_DrawBModelPrim(&prepassContext, &context, &args);
                args.triCount = 0;
                args.baseIndex = baseIndex;
                baseVertex = tris->firstVertex;
                args.vertexCount = 0;
                burn = bmodelSurf->bmodelBurnAmt;
                fade = bmodelSurf->bmodelFadeAmt;
                updatedConstant = R_UpdateCodeConstant(
                    commonSource,
                    CONST_SRC_CODE_DESTRUCTIBLE_PARMS,
                    burn,
                    r_burnedDestructibleColor->current.value,
                    fade,
                    0.0);
                if (bmodelSurf->shaderConstSet)
                    constantSet = bmodelSurf->shaderConstSet;
                else
                    constantSet = 0;
                RB_ApplyShaderConstantSet(commonSource, &savedConstantSet);
                RB_SaveCurrentShaderConstantSetValues(&savedConstantSet, commonSource, constantSet);
                v3 = RB_ApplyShaderConstantSet(commonSource, constantSet);
                updatedConstant |= v3;
                R_SetupPassCriticalPixelShaderArgs(context);
                if (updatedConstant)
                    R_SetupPassVertexShaderArgs(context);
                remotePlacement = bmodelSurf->placement;
                if (commonSource->objectPlacement != bmodelSurf->placement)
                {
                    commonSource->objectPlacement = remotePlacement;
                    R_ChangeObjectPlacementRemote(commonSource, remotePlacement);
                }
                R_SetStreamsForBspSurface(&context.state->prim, tris);
                R_SetupPassPerPrimArgs(context);
                if (prepassContext.state)
                {
                    R_SetStreamsForBspSurface(&prepassContext.state->prim, tris);
                    R_SetupPassPerPrimArgs(prepassContext);
                    if (!Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                        3129,
                        0,
                        "prepass dynamic indices not supported here"))
                        __debugbreak();
                }
            }
            args.triCount += tris->triCount;
            if (args.vertexCount < tris->vertexCount)
                vertexCount = tris->vertexCount;
            else
                vertexCount = args.vertexCount;
            args.vertexCount = vertexCount;
            if (!context.state->prim.primStats
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_tess.cpp",
                    3141,
                    0,
                    "%s",
                    "context.state->prim.primStats"))
            {
                __debugbreak();
            }
            context.state->prim.frameStats.geoIndexCount += 3 * tris->triCount;
            context.state->prim.primStats->dynamicIndexCount += 3 * tris->triCount;
            context.state->prim.primStats->dynamicVertexCount += tris->vertexCount;
            if (++drawSurfIndex == drawSurfCount)
                break;
            //drawSurf.fields = (GfxDrawSurfFields)drawSurfList[drawSurfIndex];
            drawSurf.packed = drawSurfList[drawSurfIndex].packed;
        } while ((drawSurfSubMask.packed & drawSurf.packed) == drawSurfSubKey);
        if (args.triCount > 0)
            R_DrawBModelPrim(&prepassContext, &context, &args);
    } while (drawSurfIndex != drawSurfCount && (drawSurfMask.packed & drawSurf.packed) == drawSurfKey);
    RB_ApplyShaderConstantSet(commonSource, &savedConstantSet);
    context.state->prim.primStats = 0;
    return drawSurfIndex;
}

void __cdecl R_DrawBModelPrim(GfxCmdBufContext *prepassContext, GfxCmdBufContext *context, GfxDrawPrimArgs *args)
{
    if ( prepassContext->state )
        R_DrawIndexedPrimitive(&prepassContext->state->prim, args);
    R_DrawIndexedPrimitive(&context->state->prim, args);
}

