#include "r_draw_staticmodel.h"
#include "r_shade.h"
#include "r_foliage.h"
#include "r_state.h"
#include "rb_shade.h"
#include "r_state_utils.h"
#include <ik/ik_math.h>
#include "r_model_lighting.h"
#include "r_xsurface.h"
#include "r_staticmodelcache.h"
#include "r_add_staticmodel.h"   // R_ReadPrimDrawSurfPtr
#include <xanim/xmodel_utils.h>

void __cdecl R_DrawStaticModelSurfLit(
                const unsigned int *primDrawSurfPos,
                GfxCmdBufContext context,
                GfxCmdBufContext prepassContext,
                const GfxDrawSurfListInfo *info)
{
    const GfxBackEndData *data; // [esp+0h] [ebp-3Ch]
    GfxStaticModelDrawStream drawStream; // [esp+4h] [ebp-38h] BYREF
    XSurface *surf; // [esp+38h] [ebp-4h] BYREF

    drawStream.dynSModelView = 0;
    drawStream.dynSModelState = 0;
    data = context.source->input.data;
    drawStream.viewInfoIndex = 0;   // nx-port: was word 10 &= ~3, the x86 offset of this bitfield
    drawStream.primDrawSurfPos = primDrawSurfPos;
    drawStream.reflectionProbeTexture = context.state->samplerTexture[15];
    drawStream.customSamplerFlags = context.state->pass->customSamplerFlags;
    drawStream.usingCrossFade = data->splitscreen == 0;
    drawStream.frameStats = &context.state->prim.frameStats;
    drawStream.primStats = context.state->prim.primStats;
    if ( info && info->viewInfo )
    {
        drawStream.dynSModelView = info->viewInfo->dynSModelView;
        drawStream.dynSModelState = info->viewInfo->dynSModelState;
    }
    else
    {
        drawStream.dynSModelView = 0;
        drawStream.dynSModelState = 0;
    }
    while ( R_GetNextStaticModelSurf(&drawStream, &surf) )
        R_DrawStaticModelDrawSurfLightingNonOptimized(&drawStream, context);
}

int __cdecl R_GetNextStaticModelSurf(GfxStaticModelDrawStream *drawStream, XSurface **outSurf)
{
    XSurface *xsurf; // [esp+0h] [ebp-Ch]
    const unsigned int *primDrawSurfPos; // [esp+4h] [ebp-8h]

    drawStream->smodelCount = *drawStream->primDrawSurfPos++ & 0xFFFFFFF;
    if ( !drawStream->smodelCount )
        return 0;
    primDrawSurfPos = drawStream->primDrawSurfPos;
    drawStream->primDrawSurfPos += ((drawStream->smodelCount + 1) >> 1) + R_PRIM_PTR_WORDS;
    xsurf = (XSurface *)R_ReadPrimDrawSurfPtr(primDrawSurfPos);
    drawStream->smodelList = (const unsigned __int16 *)(primDrawSurfPos + R_PRIM_PTR_WORDS);
    drawStream->localSurf = xsurf;
    if ( drawStream->primStats )
    {
        drawStream->frameStats->geoIndexCount += 3 * drawStream->smodelCount * xsurf->triCount;
        if ( !drawStream->primStats
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp",
                        529,
                        0,
                        "%s",
                        "drawStream->primStats") )
        {
            __debugbreak();
        }
        drawStream->primStats->dynamicIndexCount += 3 * drawStream->smodelCount * xsurf->triCount;
        drawStream->primStats->dynamicVertexCount += drawStream->smodelCount * xsurf->vertCount;
    }
    *outSurf = xsurf;
    return 1;
}

void __cdecl R_DrawStaticModelSurf(
                const unsigned int *primDrawSurfPos,
                GfxCmdBufContext context,
                const GfxDrawSurfListInfo *info)
{
    const GfxBackEndData *data; // [esp+0h] [ebp-3Ch]
    GfxStaticModelDrawStream drawStream; // [esp+4h] [ebp-38h] BYREF
    XSurface *surf; // [esp+38h] [ebp-4h] BYREF

    drawStream.dynSModelView = 0;
    drawStream.dynSModelState = 0;
    data = context.source->input.data;
    drawStream.viewInfoIndex = 0;   // nx-port: was word 10 &= ~3, the x86 offset of this bitfield
    drawStream.primDrawSurfPos = primDrawSurfPos;
    drawStream.reflectionProbeTexture = context.state->samplerTexture[15];
    drawStream.customSamplerFlags = context.state->pass->customSamplerFlags;
    drawStream.usingCrossFade = data->splitscreen == 0;
    drawStream.frameStats = &context.state->prim.frameStats;
    drawStream.primStats = context.state->prim.primStats;
    if ( info && info->viewInfo )
    {
        drawStream.dynSModelView = info->viewInfo->dynSModelView;
        drawStream.dynSModelState = info->viewInfo->dynSModelState;
    }
    else
    {
        drawStream.dynSModelView = 0;
        drawStream.dynSModelState = 0;
    }
    while ( R_GetNextStaticModelSurf(&drawStream, &surf) )
        R_DrawStaticModelDrawSurfNonOptimized(&drawStream, context);
}

void __cdecl R_DrawStaticModelDrawSurfNonOptimized(GfxStaticModelDrawStream *drawStream, GfxCmdBufContext context)
{
    GfxStaticModelDrawInst *smodelDrawInsts; // [esp+4h] [ebp-24h]
    unsigned int mapDynSModelCount; // [esp+8h] [ebp-20h]
    const unsigned __int16 *list; // [esp+Ch] [ebp-1Ch]
    unsigned int smodelCount; // [esp+10h] [ebp-18h]
    unsigned int index; // [esp+14h] [ebp-14h]
    XSurface *xsurf; // [esp+18h] [ebp-10h]
    GfxDrawPrimArgs args; // [esp+1Ch] [ebp-Ch] BYREF

    xsurf = drawStream->localSurf;
    R_SetStaticModelPrimArgs(xsurf, &args);
    R_SetStaticModelIndexBuffer(&context.state->prim, xsurf);
    R_SetStaticModelVertexBuffer(&context.state->prim, xsurf);
    smodelCount = drawStream->smodelCount;
    smodelDrawInsts = rgp.world->dpvs.smodelDrawInsts;
    list = drawStream->smodelList;
    mapDynSModelCount = drawStream->dynSModelState->maxDynSModelIndexInMap;
    for ( index = 0; index < smodelCount; ++index )
    {
        R_DrawStaticModelDrawSurfPlacement(&smodelDrawInsts[list[index]], context.source);
        R_SetupPassPerPrimArgs(context);
        if ( list[index] < mapDynSModelCount )
            R_FoliageSetStaticModelShaderConstants(
                &context,
                list[index],
                drawStream->dynSModelView,
                drawStream->dynSModelState);
        //BLOPS_NULLSUB();
        R_DrawIndexedPrimitive(&context.state->prim, &args);
    }
}

void __cdecl R_SetStaticModelVertexBuffer(GfxCmdBufPrimState *primState, XSurface *xsurf)
{
    IDirect3DVertexBuffer9 *vb; // [esp+0h] [ebp-4h]

    if ( !xsurf
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp", 548, 0, "%s", "xsurf") )
    {
        __debugbreak();
    }
    vb = xsurf->vb0;
    if ( !vb
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp", 551, 0, "%s", "vb") )
    {
        __debugbreak();
    }
    R_SetStreamSource(primState, vb, 0, 0x20u);
}

void __cdecl R_SetStaticModelIndexBuffer(GfxCmdBufPrimState *primState, XSurface *xsurf)
{
    IDirect3DIndexBuffer9 *ib; // [esp+0h] [ebp-4h]

    if ( !xsurf
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp", 564, 0, "%s", "xsurf") )
    {
        __debugbreak();
    }
    ib = xsurf->indexBuffer;
    if ( !ib
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp", 568, 0, "%s", "ib") )
    {
        __debugbreak();
    }
    if ( primState->indexBuffer != ib )
        R_ChangeIndices(primState, ib);
}

void __cdecl R_SetStaticModelPrimArgs(const XSurface *xsurf, GfxDrawPrimArgs *args)
{
    R_SetStaticModelVertexPrimArgs(xsurf, args);
    args->triCount = xsurf->triCount;
    args->baseIndex = 0;
}

void __cdecl R_SetStaticModelVertexPrimArgs(const XSurface *xsurf, GfxDrawPrimArgs *args)
{
    if ( !xsurf
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp", 580, 0, "%s", "xsurf") )
    {
        __debugbreak();
    }
    args->vertexCount = xsurf->vertCount;
}

void __cdecl R_DrawStaticModelDrawSurfPlacement(
                const GfxStaticModelDrawInst *smodelDrawInst,
                GfxCmdBufSourceState *source)
{
    float v2; // [esp+38h] [ebp-64h]
    float v3; // [esp+40h] [ebp-5Ch]
    float v4; // [esp+44h] [ebp-58h]
    float v5; // [esp+48h] [ebp-54h]
    float v6; // [esp+50h] [ebp-4Ch]
    float v7; // [esp+54h] [ebp-48h]
    float v8; // [esp+58h] [ebp-44h]
    GfxCmdBufSourceState *matrix; // [esp+64h] [ebp-38h]
    float origin[3]; // [esp+68h] [ebp-34h] BYREF
    float scale; // [esp+74h] [ebp-28h]
    float axis[3][3]; // [esp+78h] [ebp-24h] BYREF

    v2 = smodelDrawInst->placement.axis[0][2];
    v3 = smodelDrawInst->placement.axis[1][0];
    v4 = smodelDrawInst->placement.axis[1][1];
    v5 = smodelDrawInst->placement.axis[1][2];
    v6 = smodelDrawInst->placement.axis[2][0];
    v7 = smodelDrawInst->placement.axis[2][1];
    v8 = smodelDrawInst->placement.axis[2][2];
    *(_QWORD *)&axis[0][0] = *(_QWORD *)&smodelDrawInst->placement.axis[0][0];
    axis[0][2] = v2;
    axis[1][0] = v3;
    axis[1][1] = v4;
    axis[1][2] = v5;
    axis[2][0] = v6;
    axis[2][1] = v7;
    axis[2][2] = v8;
    scale = smodelDrawInst->placement.scale;
    matrix = R_GetActiveWorldMatrix(source);
    origin[0] = smodelDrawInst->placement.origin[0] - source->eyeOffset[0];
    origin[1] = smodelDrawInst->placement.origin[1] - source->eyeOffset[1];
    origin[2] = smodelDrawInst->placement.origin[2] - source->eyeOffset[2];
    ikMatrixSet44((float (*)[4])matrix, origin, axis, scale);
}

void __cdecl R_DrawStaticModelDrawSurfLightingNonOptimized(
                GfxStaticModelDrawStream *drawStream,
                GfxCmdBufContext context)
{
    const GfxStaticModelDrawInst *smodelDrawInst; // [esp+0h] [ebp-2Ch]
    GfxStaticModelDrawInst *smodelDrawInsts; // [esp+4h] [ebp-28h]
    unsigned int mapDynSModelCount; // [esp+8h] [ebp-24h]
    const unsigned __int16 *list; // [esp+Ch] [ebp-20h]
    unsigned int smodelCount; // [esp+10h] [ebp-1Ch]
    unsigned int index; // [esp+14h] [ebp-18h]
    XSurface *xsurf; // [esp+18h] [ebp-14h]
    GfxDrawPrimArgs args; // [esp+1Ch] [ebp-10h] BYREF
    unsigned __int16 lightingHandle; // [esp+28h] [ebp-4h]

    xsurf = drawStream->localSurf;
    R_SetStaticModelPrimArgs(xsurf, &args);
    R_SetStaticModelIndexBuffer(&context.state->prim, xsurf);
    R_SetStaticModelVertexBuffer(&context.state->prim, xsurf);
    smodelCount = drawStream->smodelCount;
    smodelDrawInsts = rgp.world->dpvs.smodelDrawInsts;
    list = drawStream->smodelList;
    mapDynSModelCount = drawStream->dynSModelState->maxDynSModelIndexInMap;
    for ( index = 0; index < smodelCount; ++index )
    {
        smodelDrawInst = &smodelDrawInsts[list[index]];
        R_SetReflectionProbe(context, smodelDrawInst->reflectionProbeIndex);
        R_DrawStaticModelDrawSurfPlacement(smodelDrawInst, context.source);
        lightingHandle = smodelDrawInst->lightingHandle;
        R_SetModelLightingCoordsForSource(lightingHandle, context.source);
        R_SetupPassPerPrimArgs(context);
        if ( list[index] < mapDynSModelCount )
            R_FoliageSetStaticModelShaderConstants(
                &context,
                list[index],
                drawStream->dynSModelView,
                drawStream->dynSModelState);
        //BLOPS_NULLSUB();
        R_DrawIndexedPrimitive(&context.state->prim, &args);
    }
}

void __cdecl R_DrawStaticModelSkinnedSurfLit(
                const unsigned int *primDrawSurfPos,
                GfxCmdBufContext context,
                const GfxDrawSurfListInfo *info)
{
    const GfxBackEndData *data; // [esp+0h] [ebp-3Ch]
    GfxStaticModelDrawStream drawStream; // [esp+4h] [ebp-38h] BYREF
    XSurface *surf; // [esp+38h] [ebp-4h] BYREF

    drawStream.dynSModelView = 0;
    drawStream.dynSModelState = 0;
    R_SetupPassPerObjectArgs(context);
    data = context.source->input.data;
    drawStream.viewInfoIndex = 0;   // nx-port: was word 10 &= ~3, the x86 offset of this bitfield
    drawStream.primDrawSurfPos = primDrawSurfPos;
    drawStream.reflectionProbeTexture = context.state->samplerTexture[15];
    drawStream.customSamplerFlags = context.state->pass->customSamplerFlags;
    drawStream.usingCrossFade = data->splitscreen == 0;
    drawStream.frameStats = &context.state->prim.frameStats;
    drawStream.primStats = context.state->prim.primStats;
    if ( info && info->viewInfo )
    {
        drawStream.dynSModelView = info->viewInfo->dynSModelView;
        drawStream.dynSModelState = info->viewInfo->dynSModelState;
    }
    else
    {
        drawStream.dynSModelView = 0;
        drawStream.dynSModelState = 0;
    }
    while ( R_GetNextStaticModelSurf(&drawStream, &surf) )
        R_DrawStaticModelsSkinnedDrawSurfLighting(&drawStream, context);
    context.state->samplerTexture[15] = drawStream.reflectionProbeTexture;
}

void __cdecl R_DrawStaticModelsSkinnedDrawSurfLighting(GfxStaticModelDrawStream *drawStream, GfxCmdBufContext context)
{
    const GfxStaticModelDrawInst *smodelDrawInst; // [esp+0h] [ebp-30h]
    IDirect3DVertexBuffer9 *vb; // [esp+4h] [ebp-2Ch]
    unsigned int vertexOffset; // [esp+8h] [ebp-28h]
    GfxStaticModelDrawInst *smodelDrawInsts; // [esp+Ch] [ebp-24h]
    const unsigned __int16 *list; // [esp+10h] [ebp-20h]
    unsigned int smodelCount; // [esp+14h] [ebp-1Ch]
    unsigned int index; // [esp+18h] [ebp-18h]
    XSurface *xsurf; // [esp+1Ch] [ebp-14h]
    GfxDrawPrimArgs args; // [esp+20h] [ebp-10h] BYREF
    unsigned __int16 lightingHandle; // [esp+2Ch] [ebp-4h]

    xsurf = drawStream->localSurf;
    R_SetStaticModelSkinnedPrimArgs(&context.state->prim, xsurf, &args);
    R_CheckVertexDataOverflow(32 * args.vertexCount);
    vertexOffset = R_SetVertexData(context.state, xsurf->verts0, args.vertexCount, 32);
    vb = gfxBuf.dynamicVertexBuffer->buffer;
    if ( !vb
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp", 2806, 0, "%s", "vb") )
    {
        __debugbreak();
    }
    R_SetStreamSource(&context.state->prim, vb, vertexOffset, 0x20u);
    smodelCount = drawStream->smodelCount;
    smodelDrawInsts = rgp.world->dpvs.smodelDrawInsts;
    list = drawStream->smodelList;
    for ( index = 0; index < smodelCount; ++index )
    {
        smodelDrawInst = &smodelDrawInsts[list[index]];
        lightingHandle = smodelDrawInst->lightingHandle;
        R_DrawStaticModelSkinnedDrawSurfLighting(
            list[index],
            smodelDrawInst,
            lightingHandle,
            &args,
            context,
            drawStream->dynSModelView,
            drawStream->dynSModelState);
    }
}

void __cdecl R_DrawStaticModelSkinnedDrawSurfLighting(
                int smodelIndex,
                const GfxStaticModelDrawInst *smodelDrawInst,
                unsigned __int16 lightingHandle,
                GfxDrawPrimArgs *args,
                GfxCmdBufContext context,
                const DynSModelClientView *dynSModelView,
                const DynSModelGfxState *dynSModelState)
{
    R_SetReflectionProbe(context, smodelDrawInst->reflectionProbeIndex);
    R_DrawStaticModelDrawSurfPlacement(smodelDrawInst, context.source);
    R_SetModelLightingCoordsForSource(lightingHandle, context.source);
    R_SetupPassPerPrimArgs(context);
    if ( smodelIndex < (signed int)dynSModelState->maxDynSModelIndexInMap )
        R_FoliageSetStaticModelShaderConstants(&context, smodelIndex, dynSModelView, dynSModelState);
    R_DrawIndexedPrimitive(&context.state->prim, args);
}

void __cdecl R_SetStaticModelSkinnedPrimArgs(GfxCmdBufPrimState *state, const XSurface *xsurf, GfxDrawPrimArgs *args)
{
    if ( !xsurf
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp", 2778, 0, "%s", "xsurf") )
    {
        __debugbreak();
    }
    args->triCount = XSurfaceGetNumTris(xsurf);
    args->vertexCount = XSurfaceGetNumVerts(xsurf);
    args->baseIndex = R_SetIndexData(state, (unsigned __int8 *)xsurf->triIndices, args->triCount);
}

void __cdecl R_DrawStaticModelSkinnedSurf(
                const unsigned int *primDrawSurfPos,
                GfxCmdBufContext context,
                const GfxDrawSurfListInfo *info)
{
    const GfxBackEndData *data; // [esp+0h] [ebp-3Ch]
    GfxStaticModelDrawStream drawStream; // [esp+4h] [ebp-38h] BYREF
    XSurface *surf; // [esp+38h] [ebp-4h] BYREF

    drawStream.dynSModelView = 0;
    drawStream.dynSModelState = 0;
    R_SetupPassPerObjectArgs(context);
    data = context.source->input.data;
    drawStream.viewInfoIndex = 0;   // nx-port: was word 10 &= ~3, the x86 offset of this bitfield
    drawStream.primDrawSurfPos = primDrawSurfPos;
    drawStream.reflectionProbeTexture = context.state->samplerTexture[15];
    drawStream.customSamplerFlags = context.state->pass->customSamplerFlags;
    drawStream.usingCrossFade = data->splitscreen == 0;
    drawStream.frameStats = &context.state->prim.frameStats;
    drawStream.primStats = context.state->prim.primStats;
    if ( info && info->viewInfo )
    {
        drawStream.dynSModelView = info->viewInfo->dynSModelView;
        drawStream.dynSModelState = info->viewInfo->dynSModelState;
    }
    else
    {
        drawStream.dynSModelView = 0;
        drawStream.dynSModelState = 0;
    }
    while ( R_GetNextStaticModelSurf(&drawStream, &surf) )
        R_DrawStaticModelsSkinnedDrawSurf(&drawStream, context);
    if ( context.state->samplerTexture[15] != drawStream.reflectionProbeTexture
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp",
                    2890,
                    0,
                    "%s",
                    "context.state->samplerTexture[TEXTURE_DEST_CODE_REFLECTION_PROBE] == drawStream.reflectionProbeTexture") )
    {
        __debugbreak();
    }
}

void __cdecl R_DrawStaticModelsSkinnedDrawSurf(GfxStaticModelDrawStream *drawStream, GfxCmdBufContext context)
{
    IDirect3DVertexBuffer9 *vb; // [esp+4h] [ebp-28h]
    unsigned int vertexOffset; // [esp+8h] [ebp-24h]
    GfxStaticModelDrawInst *smodelDrawInsts; // [esp+Ch] [ebp-20h]
    const unsigned __int16 *list; // [esp+10h] [ebp-1Ch]
    unsigned int smodelCount; // [esp+14h] [ebp-18h]
    unsigned int index; // [esp+18h] [ebp-14h]
    XSurface *xsurf; // [esp+1Ch] [ebp-10h]
    GfxDrawPrimArgs args; // [esp+20h] [ebp-Ch] BYREF

    xsurf = drawStream->localSurf;
    R_SetStaticModelSkinnedPrimArgs(&context.state->prim, xsurf, &args);
    R_CheckVertexDataOverflow(32 * args.vertexCount);
    vertexOffset = R_SetVertexData(context.state, xsurf->verts0, args.vertexCount, 32);
    vb = gfxBuf.dynamicVertexBuffer->buffer;
    if ( !vb
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp", 2843, 0, "%s", "vb") )
    {
        __debugbreak();
    }
    R_SetStreamSource(&context.state->prim, vb, vertexOffset, 0x20u);
    smodelCount = drawStream->smodelCount;
    smodelDrawInsts = rgp.world->dpvs.smodelDrawInsts;
    list = drawStream->smodelList;
    for ( index = 0; index < smodelCount; ++index )
        R_DrawStaticModelSkinnedDrawSurf(
            list[index],
            &smodelDrawInsts[list[index]],
            &args,
            context,
            drawStream->dynSModelView,
            drawStream->dynSModelState);
}

void __cdecl R_DrawStaticModelSkinnedDrawSurf(
                int smodelIndex,
                const GfxStaticModelDrawInst *smodelDrawInst,
                GfxDrawPrimArgs *args,
                GfxCmdBufContext context,
                const DynSModelClientView *dynSModelView,
                const DynSModelGfxState *dynSModelState)
{
    R_DrawStaticModelDrawSurfPlacement(smodelDrawInst, context.source);
    R_SetupPassPerPrimArgs(context);
    if ( smodelIndex < (signed int)dynSModelState->maxDynSModelIndexInMap )
        R_FoliageSetStaticModelShaderConstants(&context, smodelIndex, dynSModelView, dynSModelState);
    R_DrawIndexedPrimitive(&context.state->prim, args);
}

void __cdecl R_DrawStaticModelCachedSurfLit(const unsigned int *primDrawSurfPos, GfxCmdBufContext context)
{
    const GfxBackEndData *data; // [esp+0h] [ebp-38h]
    GfxStaticModelDrawStream drawStream; // [esp+4h] [ebp-34h] BYREF

    drawStream.dynSModelView = 0;
    drawStream.dynSModelState = 0;
    R_SetupCachedStaticModelLighting(context.source);
    R_SetupPassPerObjectArgs(context);
    data = context.source->input.data;
    drawStream.viewInfoIndex = 0;   // nx-port: was word 10 &= ~3, the x86 offset of this bitfield
    drawStream.primDrawSurfPos = primDrawSurfPos;
    drawStream.reflectionProbeTexture = context.state->samplerTexture[15];
    drawStream.customSamplerFlags = context.state->pass->customSamplerFlags;
    drawStream.usingCrossFade = data->splitscreen == 0;
    drawStream.frameStats = &context.state->prim.frameStats;
    drawStream.primStats = context.state->prim.primStats;
    drawStream.dynSModelView = 0;
    drawStream.dynSModelState = 0;
    while ( R_GetNextStaticModelCachedSurf(&drawStream) )
        R_DrawStaticModelsCachedDrawSurfLighting(&drawStream, context);
    context.state->samplerTexture[15] = drawStream.reflectionProbeTexture;
}

int __cdecl R_GetNextStaticModelCachedSurf(GfxStaticModelDrawStream *drawStream)
{
    const GfxStaticModelDrawInst *smodelDrawInst; // [esp+0h] [ebp-Ch]
    XSurface *xsurf; // [esp+8h] [ebp-4h]

    drawStream->smodelCount = *drawStream->primDrawSurfPos;
    if ( drawStream->smodelCount > rgp.world->dpvs.smodelCount )
        return 0;
    ++drawStream->primDrawSurfPos;
    if ( !drawStream->smodelCount )
        return 0;
    xsurf = (XSurface *)R_ReadPrimDrawSurfPtr(drawStream->primDrawSurfPos);
    drawStream->primDrawSurfPos += R_PRIM_PTR_WORDS;
    drawStream->smodelList = (const unsigned __int16 *)drawStream->primDrawSurfPos;
    drawStream->primDrawSurfPos += (drawStream->smodelCount + 1) >> 1;
    smodelDrawInst = &rgp.world->dpvs.smodelDrawInsts[R_GetCachedSModelSurf(*drawStream->smodelList)->cachedSurf.smodelIndex];
    drawStream->localSurf = xsurf;
    drawStream->reflectionProbeIndex = smodelDrawInst->reflectionProbeIndex;
    drawStream->frameStats->geoIndexCount += 3 * drawStream->smodelCount * xsurf->triCount;
    if ( !drawStream->primStats
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp",
                    2967,
                    0,
                    "%s",
                    "drawStream->primStats") )
    {
        __debugbreak();
    }
    drawStream->primStats->dynamicIndexCount += 3 * drawStream->smodelCount * xsurf->triCount;
    drawStream->primStats->dynamicVertexCount += drawStream->smodelCount * xsurf->vertCount;
    return 1;
}

void __cdecl R_DrawStaticModelsCachedDrawSurfLighting(GfxStaticModelDrawStream *drawStream, GfxCmdBufContext context)
{
    unsigned int copyBaseIndex; // [esp+0h] [ebp-30h]
    unsigned int baseIndex; // [esp+4h] [ebp-2Ch]
    unsigned int surfBaseIndex; // [esp+8h] [ebp-28h]
    unsigned int reflectionProbeIndex; // [esp+10h] [ebp-20h] BYREF
    const unsigned __int16 *list; // [esp+14h] [ebp-1Ch]
    unsigned int smodelCount; // [esp+18h] [ebp-18h]
    unsigned int index; // [esp+1Ch] [ebp-14h]
    const XSurface *xsurf; // [esp+20h] [ebp-10h]
    GfxDrawPrimArgs args; // [esp+24h] [ebp-Ch] BYREF

    xsurf = R_GetCurrentStaticModelCachedSurf(drawStream, &reflectionProbeIndex);
    list = drawStream->smodelList;
    smodelCount = drawStream->smodelCount;
    R_SetStaticModelCachedPrimArgs(xsurf, &args);
    R_SetStaticModelCachedBuffer(context.state, *list);
    R_SetupPassPerPrimArgs(context);
    R_SetReflectionProbe(context, reflectionProbeIndex);
    surfBaseIndex = 3 * xsurf->baseTriIndex;
    args.triCount = smodelCount * xsurf->triCount;
    args.baseIndex = R_ReserveIndexData(&context.state->prim, args.triCount);
    index = 0;
    do
    {
        baseIndex = surfBaseIndex + 4 * R_GetCachedSModelSurf(list[index])->cachedSurf.baseVertIndex;
        if ( baseIndex >= 0x100000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp",
                        3050,
                        0,
                        "%s",
                        "baseIndex < SMC_MAX_INDEX_IN_CACHE") )
        {
            __debugbreak();
        }
        if ( baseIndex + 3 * xsurf->triCount > 0x100000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp",
                        3051,
                        0,
                        "%s",
                        "baseIndex + xsurf->triCount * 3 <= SMC_MAX_INDEX_IN_CACHE") )
        {
            __debugbreak();
        }
        copyBaseIndex = R_SetIndexData(
                                            &context.state->prim,
                                            (unsigned __int8 *)&gfxBuf.smodelCache.indices[baseIndex],
                                            xsurf->triCount);
        if ( copyBaseIndex != args.baseIndex + index * 3 * xsurf->triCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp",
                        3054,
                        1,
                        "copyBaseIndex == args.baseIndex + xsurf->triCount * 3 * index\n\t%i, %i",
                        copyBaseIndex,
                        args.baseIndex + index * 3 * xsurf->triCount) )
        {
            __debugbreak();
        }
        ++index;
    }
    while ( index < smodelCount );
    R_DrawIndexedPrimitive(&context.state->prim, &args);
}

XSurface *__cdecl R_GetCurrentStaticModelCachedSurf(
                GfxStaticModelDrawStream *drawStream,
                unsigned int *reflectionProbeIndex)
{
    if ( reflectionProbeIndex )
        *reflectionProbeIndex = drawStream->reflectionProbeIndex;
    return drawStream->localSurf;
}

void __cdecl R_SetStaticModelCachedPrimArgs(const XSurface *xsurf, GfxDrawPrimArgs *args)
{
    if ( !xsurf
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp", 2995, 0, "%s", "xsurf") )
    {
        __debugbreak();
    }
    args->vertexCount = 0x10000;
    args->triCount = xsurf->triCount;
}

void __cdecl R_SetStaticModelCachedBuffer(GfxCmdBufState *state, unsigned int cachedIndex)
{
    R_SetStreamSource(&state->prim, gfxBuf.smodelCacheVb, ((cachedIndex - 1) & 0xFFFFF000) << 9, 0x20u);
}

void __cdecl R_DrawStaticModelCachedSurf(const unsigned int *primDrawSurfPos, GfxCmdBufContext context)
{
    const GfxBackEndData *data; // [esp+0h] [ebp-38h]
    GfxStaticModelDrawStream drawStream; // [esp+4h] [ebp-34h] BYREF

    drawStream.dynSModelView = 0;
    drawStream.dynSModelState = 0;
    R_SetupPassPerObjectArgs(context);
    data = context.source->input.data;
    drawStream.viewInfoIndex = 0;   // nx-port: was word 10 &= ~3, the x86 offset of this bitfield
    drawStream.primDrawSurfPos = primDrawSurfPos;
    drawStream.reflectionProbeTexture = context.state->samplerTexture[15];
    drawStream.customSamplerFlags = context.state->pass->customSamplerFlags;
    drawStream.usingCrossFade = data->splitscreen == 0;
    drawStream.frameStats = &context.state->prim.frameStats;
    drawStream.primStats = context.state->prim.primStats;
    drawStream.dynSModelView = 0;
    drawStream.dynSModelState = 0;
    while ( R_GetNextStaticModelCachedSurf(&drawStream) )
        R_DrawStaticModelsCachedDrawSurf(&drawStream, context);
    if ( context.state->samplerTexture[15] != drawStream.reflectionProbeTexture
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp",
                    3188,
                    0,
                    "%s",
                    "context.state->samplerTexture[TEXTURE_DEST_CODE_REFLECTION_PROBE] == drawStream.reflectionProbeTexture") )
    {
        __debugbreak();
    }
}

void __cdecl R_DrawStaticModelsCachedDrawSurf(GfxStaticModelDrawStream *drawStream, GfxCmdBufContext context)
{
    unsigned int copyBaseIndex; // [esp+0h] [ebp-2Ch]
    unsigned int baseIndex; // [esp+4h] [ebp-28h]
    unsigned int surfBaseIndex; // [esp+8h] [ebp-24h]
    const unsigned __int16 *list; // [esp+10h] [ebp-1Ch]
    unsigned int smodelCount; // [esp+14h] [ebp-18h]
    unsigned int index; // [esp+18h] [ebp-14h]
    const XSurface *xsurf; // [esp+1Ch] [ebp-10h]
    GfxDrawPrimArgs args; // [esp+20h] [ebp-Ch] BYREF

    xsurf = R_GetCurrentStaticModelCachedSurf(drawStream, 0);
    list = drawStream->smodelList;
    smodelCount = drawStream->smodelCount;
    R_SetStaticModelCachedPrimArgs(xsurf, &args);
    R_SetStaticModelCachedBuffer(context.state, *list);
    R_SetupPassPerPrimArgs(context);
    surfBaseIndex = 3 * xsurf->baseTriIndex;
    args.triCount = smodelCount * xsurf->triCount;
    args.baseIndex = R_ReserveIndexData(&context.state->prim, args.triCount);
    index = 0;
    do
    {
        baseIndex = surfBaseIndex + 4 * R_GetCachedSModelSurf(list[index])->cachedSurf.baseVertIndex;
        if ( baseIndex >= 0x100000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp",
                        3122,
                        0,
                        "%s",
                        "baseIndex < SMC_MAX_INDEX_IN_CACHE") )
        {
            __debugbreak();
        }
        if ( baseIndex + 3 * xsurf->triCount > 0x100000
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp",
                        3123,
                        0,
                        "%s",
                        "baseIndex + xsurf->triCount * 3 <= SMC_MAX_INDEX_IN_CACHE") )
        {
            __debugbreak();
        }
        copyBaseIndex = R_SetIndexData(
                                            &context.state->prim,
                                            (unsigned __int8 *)&gfxBuf.smodelCache.indices[baseIndex],
                                            xsurf->triCount);
        if ( copyBaseIndex != args.baseIndex + index * 3 * xsurf->triCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp",
                        3126,
                        1,
                        "copyBaseIndex == args.baseIndex + xsurf->triCount * 3 * index\n\t%i, %i",
                        copyBaseIndex,
                        args.baseIndex + index * 3 * xsurf->triCount) )
        {
            __debugbreak();
        }
        ++index;
    }
    while ( index < smodelCount );
    R_DrawIndexedPrimitive(&context.state->prim, &args);
}

void __cdecl R_DrawStaticModelPreTessSurfLit(const unsigned int *primDrawSurfPos, GfxCmdBufContext context)
{
    GfxReadCmdBuf readCmdBuf; // [esp+0h] [ebp-10h] BYREF
    unsigned int firstIndex; // [esp+4h] [ebp-Ch] BYREF
    unsigned int count; // [esp+8h] [ebp-8h] BYREF
    GfxStaticModelPreTessSurf pretessSurf; // [esp+Ch] [ebp-4h] BYREF

    R_SetupCachedStaticModelLighting(context.source);
    R_SetupPassPerObjectArgs(context);
    readCmdBuf.primDrawSurfPos = primDrawSurfPos;
    while ( R_ReadStaticModelPreTessDrawSurf(&readCmdBuf, &pretessSurf, &firstIndex, &count) )
        R_DrawStaticModelsPreTessDrawSurfLighting(pretessSurf, firstIndex, count, context);
}

int __cdecl R_ReadStaticModelPreTessDrawSurf(
                GfxReadCmdBuf *readCmdBuf,
                GfxStaticModelPreTessSurf *pretessSurf,
                unsigned int *firstIndex,
                unsigned int *count)
{
    *count = R_ReadPrimDrawSurfInt(readCmdBuf);
    if ( !*count )
        return 0;
    pretessSurf->packed = R_ReadPrimDrawSurfInt(readCmdBuf);
    *firstIndex = R_ReadPrimDrawSurfInt(readCmdBuf);
    if ( *firstIndex >= 0x100000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp",
                    2989,
                    0,
                    "*firstIndex doesn't index R_MAX_PRETESS_INDICES\n\t%i not in [0, %i)",
                    *firstIndex,
                    0x100000) )
    {
        __debugbreak();
    }
    return 1;
}

void __cdecl R_DrawStaticModelsPreTessDrawSurfLighting(
                GfxStaticModelPreTessSurf pretessSurf,
                unsigned int firstIndex,
                unsigned int count,
                GfxCmdBufContext context)
{
    IDirect3DIndexBuffer9 *ib; // [esp+0h] [ebp-28h]
    const GfxStaticModelDrawInst *smodelDrawInst; // [esp+18h] [ebp-10h]
    GfxDrawPrimArgs args; // [esp+1Ch] [ebp-Ch] BYREF

    if ( !count
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp", 3070, 0, "%s", "count") )
    {
        __debugbreak();
    }
    smodelDrawInst = R_SetupCachedSModelSurface(
                                         context.state,
                                         pretessSurf.fields.cachedIndex,
                                         pretessSurf.fields.lod,
                                         pretessSurf.fields.surfIndex,
                                         count,
                                         &args,
                                         0);
    R_SetupPassPerPrimArgs(context);
    R_SetReflectionProbe(context, smodelDrawInst->reflectionProbeIndex);
    ib = context.source->input.data->preTessIb->buffer;
    if ( context.state->prim.indexBuffer != ib )
        R_ChangeIndices(&context.state->prim, ib);
    args.baseIndex = firstIndex;
    R_DrawIndexedPrimitive(&context.state->prim, &args);
}

const GfxStaticModelDrawInst *__cdecl R_SetupCachedSModelSurface(
                GfxCmdBufState *state,
                unsigned int cachedIndex,
                unsigned int lod,
                unsigned int surfIndex,
                unsigned int count,
                GfxDrawPrimArgs *args,
                unsigned int *baseIndex)
{
    const GfxStaticModelDrawInst *smodelDrawInst; // [esp+4h] [ebp-Ch]
    const XSurface *xsurf; // [esp+Ch] [ebp-4h]

    if ( !cachedIndex
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp",
                    2912,
                    0,
                    "%s",
                    "cachedIndex") )
    {
        __debugbreak();
    }
    smodelDrawInst = &rgp.world->dpvs.smodelDrawInsts[R_GetCachedSModelSurf(cachedIndex)->cachedSurf.smodelIndex];
    xsurf = XModelGetSurface(smodelDrawInst->model, lod, surfIndex);
    if ( baseIndex )
        *baseIndex = 3 * xsurf->baseTriIndex;
    args->vertexCount = 0x10000;
    args->triCount = count * xsurf->triCount;
    R_SetStreamSource(&state->prim, gfxBuf.smodelCacheVb, ((cachedIndex - 1) & 0xFFFFF000) << 9, 0x20u);
    state->prim.frameStats.geoIndexCount += 3 * count * xsurf->triCount;
    if ( !state->prim.primStats
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp",
                    2930,
                    0,
                    "%s",
                    "state->prim.primStats") )
    {
        __debugbreak();
    }
    state->prim.primStats->dynamicIndexCount += 3 * count * xsurf->triCount;
    state->prim.primStats->dynamicVertexCount += count * xsurf->vertCount;
    return smodelDrawInst;
}

void __cdecl R_DrawStaticModelPreTessSurf(const unsigned int *primDrawSurfPos, GfxCmdBufContext context)
{
    GfxReadCmdBuf readCmdBuf; // [esp+0h] [ebp-10h] BYREF
    unsigned int firstIndex; // [esp+4h] [ebp-Ch] BYREF
    unsigned int count; // [esp+8h] [ebp-8h] BYREF
    GfxStaticModelPreTessSurf pretessSurf; // [esp+Ch] [ebp-4h] BYREF

    R_SetupPassPerObjectArgs(context);
    readCmdBuf.primDrawSurfPos = primDrawSurfPos;
    while ( R_ReadStaticModelPreTessDrawSurf(&readCmdBuf, &pretessSurf, &firstIndex, &count) )
        R_DrawStaticModelsPreTessDrawSurf(pretessSurf, firstIndex, count, context);
}

void __cdecl R_DrawStaticModelsPreTessDrawSurf(
                GfxStaticModelPreTessSurf pretessSurf,
                unsigned int firstIndex,
                unsigned int count,
                GfxCmdBufContext context)
{
    IDirect3DIndexBuffer9 *ib; // [esp+0h] [ebp-24h]
    GfxDrawPrimArgs args; // [esp+18h] [ebp-Ch] BYREF

    if ( !count
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_staticmodel.cpp", 3140, 0, "%s", "count") )
    {
        __debugbreak();
    }
    R_SetupCachedSModelSurface(
        context.state,
        pretessSurf.fields.cachedIndex,
        pretessSurf.fields.lod,
        pretessSurf.fields.surfIndex,
        count,
        &args,
        0);
    R_SetupPassPerPrimArgs(context);
    ib = context.source->input.data->preTessIb->buffer;
    if ( context.state->prim.indexBuffer != ib )
        R_ChangeIndices(&context.state->prim, ib);
    args.baseIndex = firstIndex;
    R_DrawIndexedPrimitive(&context.state->prim, &args);
}

