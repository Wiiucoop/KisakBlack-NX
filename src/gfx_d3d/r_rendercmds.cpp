#include "r_rendercmds.h"
#include "r_pointlights.h"
#include "r_dpvs.h"
#include "r_model_lighting.h"
#include "r_model.h"
#include "r_debug.h"
#include <universal/com_memory.h>
#include "rb_logfile.h"
#include "r_dvars.h"
#include <qcommon/threads.h>
#include <universal/com_workercmds.h>
#include "rb_backend.h"
#include <win32/win_net.h>
#include "r_ui3d.h"
#include <cgame/cg_perf.h>
#include "r_foliage.h"
#include <cgame/cg_drawtools.h>
#include "r_material_load_obj.h"
#include "r_drawsurf.h"
#include "r_draw_method.h"
#include <client/splitscreen.h>
#include <xanim/xmodel_utils.h>
#include <client/cl_debugdata.h>

GfxBackEndData *frontEndDataOut;
GfxCmdArray g_frontEndCmds[2];
GfxBackEndData s_backEndData[2];
GfxCmdArray *s_cmdList;

int g_currCodeMesh;
GfxMeshData g_codeMesh[2];

GfxViewInfo g_viewInfo[2][4];
unsigned int s_smpFrame;
DynSModelClientView g_dynSModelClientView[2][4];
unsigned int g_frameIndex;

void __cdecl R_InitRenderCommands()
{
    int renderCmdBufferSize; // [esp+4h] [ebp-Ch]
    int renderCmdWarnSize; // [esp+8h] [ebp-8h]
    unsigned int dataIndex; // [esp+Ch] [ebp-4h]

    renderCmdBufferSize = 0x18000 * gfxCfg.maxClientViews;
    renderCmdWarnSize = (signed int)(0x48000 * gfxCfg.maxClientViews) / 4;
    GenerateLightGridBasisDirs();
    R_InitModelLightingGlobals();
    for ( dataIndex = 0; dataIndex < 2; ++dataIndex )
    {
        g_frontEndCmds[dataIndex].byteSize = renderCmdBufferSize;
        g_frontEndCmds[dataIndex].warnSize = renderCmdWarnSize;
        g_frontEndCmds[dataIndex].cmds = (unsigned __int8 *)R_AllocGlobalVariable(renderCmdBufferSize, "rendercmds");
        R_InitDebugEntry(&s_backEndData[dataIndex].debugGlobals);
    }
    R_InitSceneBuffers();
    if ( frontEndDataOut )
    {
        R_ClearCmdList();
    }
    else
    {
        R_UnlockSkinnedCache();
        R_ToggleSmpFrame();
    }
}

void __cdecl R_ShutdownRenderCommands()
{
    GfxBackEndData *data; // [esp+0h] [ebp-8h]
    unsigned int dataIndex; // [esp+4h] [ebp-4h]

    R_ShutdownModelLightingGlobals();
    for ( dataIndex = 0; dataIndex < 2; ++dataIndex )
    {
        data = &s_backEndData[dataIndex];
        if ( data->commands )
            R_FreeGlobalVariable(data->commands->cmds);
        R_ShutdownDebugEntry(&data->debugGlobals);
    }
    R_ShutdownSceneBuffers();
}

void __cdecl R_RecreateGlassRenderBuffers()
{
    int dataIndex; // [esp+0h] [ebp-Ch]

    for ( dataIndex = 0; dataIndex < 2; ++dataIndex )
    {
        if ( s_backEndData[dataIndex].glassMesh.vertSize )
            R_InitDynamicVertexBufferState(
                &s_backEndData[dataIndex].glassMesh.vb,
                s_backEndData[dataIndex].glassMesh.vb.total);
    }
}

void __cdecl R_InitRenderBuffers()
{
    float w; // [esp+8h] [ebp-3Ch]
    float h; // [esp+Ch] [ebp-38h]
    GfxBackEndData *data; // [esp+34h] [ebp-10h]
    unsigned int partitionIndex; // [esp+38h] [ebp-Ch]
    unsigned int dataIndex; // [esp+3Ch] [ebp-8h]
    unsigned int dataIndexa; // [esp+3Ch] [ebp-8h]
    unsigned int viewIndex; // [esp+40h] [ebp-4h]
    unsigned int viewIndexa; // [esp+40h] [ebp-4h]

    for ( dataIndex = 0; dataIndex < 2; ++dataIndex )
    {
        R_InitDynamicMesh(&g_codeMesh[dataIndex], 0xF000u, 0x4000u, 0x20u);
        g_codeMesh[dataIndex].vb.verts = (unsigned __int8 *)Z_VirtualAlloc(0x80000, "codeMesh", 19);
    }
    for ( dataIndexa = 0; dataIndexa < 2; ++dataIndexa )
    {
        data = &s_backEndData[dataIndexa];
        data->codeMeshPtr = &g_codeMesh[dataIndexa];
        R_InitDynamicMesh(&data->markMesh, 0x2400u, 0x1800u, 0x2Cu);
        data->markMesh.vb.verts = (unsigned __int8 *)Z_VirtualAlloc(270336, "markMesh", 19);
        gfxBuf.dynamicBufferCurrentFrame[dataIndexa] = 0;
        for ( viewIndex = 0; viewIndex < 4; ++viewIndex )
        {
            for ( partitionIndex = 0; partitionIndex < 4; ++partitionIndex )
                R_InitDynamicMesh(&g_viewInfo[dataIndexa][viewIndex].pointLightMeshData[partitionIndex], 6u, 4u, 0x20u);
        }
    }
    for ( viewIndexa = 0; viewIndexa < 4; ++viewIndexa )
    {
        R_InitDynamicMesh(&gfxMeshGlob.fullSceneViewMesh[viewIndexa].meshData, 6u, 4u, 0x20u);
        h = (float)vidConfig.sceneHeight;
        w = (float)vidConfig.sceneWidth;
        R_SetQuadMesh(&gfxMeshGlob.fullSceneViewMesh[viewIndexa], 0.0, 0.0, w, h, 0.0, 0.0, 1.0, 1.0, 0xFFFFFFFF);
    }
    R_InitSpotShadowMeshes();
    R_InitDynamicIndices(&gfxBuf.smodelCache, 0x100000);
    R_RecreateGlassRenderBuffers();
}

void __cdecl R_InitDynamicMesh(
                GfxMeshData *mesh,
                unsigned int indexCount,
                unsigned int vertCount,
                unsigned int vertSize)
{
    mesh->thisPtr = mesh;
    mesh->indices = (unsigned __int16 *)R_AllocGlobalVariable(2 * indexCount, "R_InitDynamicMesh");
    mesh->totalIndexCount = indexCount;
    mesh->indexCount = 0;
    mesh->vertSize = vertSize;
    R_InitDynamicVertexBufferState(&mesh->vb, vertSize * vertCount);
}

unsigned int R_InitSpotShadowMeshes()
{
    unsigned int result; // eax
    unsigned int sunShadowIndex; // [esp+24h] [ebp-10h]
    float y; // [esp+2Ch] [ebp-8h]
    float ya; // [esp+2Ch] [ebp-8h]
    unsigned int spotShadowIndex; // [esp+30h] [ebp-4h]

    y = 0.0f;
    for ( spotShadowIndex = 0; spotShadowIndex < 4; ++spotShadowIndex )
    {
        R_InitDynamicMesh(&gfxMeshGlob.spotShadowClearMeshData[spotShadowIndex], 6u, 4u, 0x20u);
        R_SetQuadMeshData(
            &gfxMeshGlob.spotShadowClearMeshData[spotShadowIndex],
            0.0,
            y,
            512.0,
            512.0,
            0.0,
            0.0,
            1.0,
            1.0,
            0xFFFFFFFF);
        y = y + 512.0;
        result = spotShadowIndex + 1;
    }
    ya = 0.0f;
    for ( sunShadowIndex = 0; sunShadowIndex < 2; ++sunShadowIndex )
    {
        R_InitDynamicMesh(&gfxMeshGlob.sunShadowClearMeshData[sunShadowIndex], 6u, 4u, 0x20u);
        R_SetQuadMeshData(
            &gfxMeshGlob.sunShadowClearMeshData[sunShadowIndex],
            0.0,
            ya,
            (float)dx.sunShadowmapSize,
            (float)dx.sunShadowmapSize,
            0.0,
            0.0,
            1.0,
            1.0,
            0xFFFFFFFF);
        ya = (float)dx.sunShadowmapSize + ya;
        result = sunShadowIndex + 1;
    }
    return result;
}

void __cdecl R_InitGlassRenderBuffers(unsigned int numIndices, unsigned int numVerts, unsigned int vertexSize)
{
    int dataIndex; // [esp+0h] [ebp-4h]

    for ( dataIndex = 0; dataIndex < 2; ++dataIndex )
    {
        R_InitDynamicMesh(&s_backEndData[dataIndex].glassMesh, numIndices, numVerts, vertexSize);
        s_backEndData[dataIndex].glassMesh.vb.verts = (unsigned __int8 *)Z_VirtualAlloc(
                                                                                                                                             vertexSize * numVerts,
                                                                                                                                             "glassMesh",
                                                                                                                                             19);
    }
}

void __cdecl R_FreeGlassRenderBuffers()
{
    GfxMeshData *p_glassMesh; // eax
    int dataIndex; // [esp+0h] [ebp-4h]

    for ( dataIndex = 0; dataIndex < 2; ++dataIndex )
    {
        Z_VirtualFree(s_backEndData[dataIndex].glassMesh.vb.verts, 19);
        s_backEndData[dataIndex].glassMesh.vb.verts = 0;
        R_ShutdownDynamicMesh(&s_backEndData[dataIndex].glassMesh);
        p_glassMesh = &s_backEndData[dataIndex].glassMesh;
        p_glassMesh->thisPtr = 0;
        p_glassMesh->indexCount = 0;
        p_glassMesh->totalIndexCount = 0;
        p_glassMesh->indices = 0;
        p_glassMesh->vb.used = 0;
        p_glassMesh->vb.total = 0;
        p_glassMesh->vb.buffer = 0;
        p_glassMesh->vb.verts = 0;
        p_glassMesh->vertSize = 0;
    }
}

void __cdecl R_ShutdownDynamicMesh(GfxMeshData *mesh)
{
    IDirect3DVertexBuffer9 *varCopy; // [esp+0h] [ebp-4h]

    R_FreeGlobalVariable(mesh->indices);
    mesh->indices = 0;
    if ( mesh->vb.buffer )
    {
        do
        {
            if ( r_logFile )
            {
                if ( r_logFile->current.integer )
                    RB_LogPrint("mesh->vb.buffer->Release()\n");
            }
            varCopy = mesh->vb.buffer;
            mesh->vb.buffer = 0;
            R_ReleaseAndSetNULL<IDirect3DIndexBuffer9>(
                (IDirect3DSurface9 *)varCopy,
                "mesh->vb.buffer",
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                291);
        }
        while ( alwaysfails );
    }
}

void __cdecl R_ReleaseGlassRenderBuffers()
{
    int dataIndex; // [esp+0h] [ebp-4h]

    for ( dataIndex = 0; dataIndex < 2; ++dataIndex )
    {
        Z_VirtualFree(s_backEndData[dataIndex].glassMesh.vb.verts, 19);
        s_backEndData[dataIndex].glassMesh.vb.verts = 0;
        R_ShutdownDynamicMesh(&s_backEndData[dataIndex].glassMesh);
    }
}

void __cdecl R_ShutdownRenderBuffers()
{
    GfxBackEndData *data; // [esp+0h] [ebp-10h]
    unsigned int partitionIndex; // [esp+4h] [ebp-Ch]
    int dataIndex; // [esp+8h] [ebp-8h]
    int dataIndexa; // [esp+8h] [ebp-8h]
    unsigned int viewIndex; // [esp+Ch] [ebp-4h]
    unsigned int viewIndexa; // [esp+Ch] [ebp-4h]

    for ( dataIndex = 0; dataIndex < 2; ++dataIndex )
    {
        R_ShutdownDynamicMesh(&g_codeMesh[dataIndex]);
        Z_VirtualFree(g_codeMesh[dataIndex].vb.verts, 19);
        g_codeMesh[dataIndex].vb.verts = 0;
    }
    for ( dataIndexa = 0; dataIndexa < 2; ++dataIndexa )
    {
        data = &s_backEndData[dataIndexa];
        data->endFence = 0;
        gfxBuf.dynamicBufferCurrentFrame[dataIndexa] = 0;
        data->preTessIb = 0;
        Z_VirtualFree(data->markMesh.vb.verts, 19);
        data->markMesh.vb.verts = 0;
        R_ShutdownDynamicMesh(&data->markMesh);
        for ( viewIndex = 0; viewIndex < 4; ++viewIndex )
        {
            for ( partitionIndex = 0; partitionIndex < 4; ++partitionIndex )
                R_ShutdownDynamicMesh(&g_viewInfo[dataIndexa][viewIndex].pointLightMeshData[partitionIndex]);
        }
    }
    for ( viewIndexa = 0; viewIndexa < 4; ++viewIndexa )
        R_ShutdownDynamicMesh(&gfxMeshGlob.fullSceneViewMesh[viewIndexa].meshData);
    R_ShutdownSpotShadowMeshes();
    R_ShutdownDynamicIndices(&gfxBuf.smodelCache);
    R_ReleaseGlassRenderBuffers();
}

void R_ShutdownSpotShadowMeshes()
{
    unsigned int sunShadowIndex; // [esp+0h] [ebp-8h]
    unsigned int spotShadowIndex; // [esp+4h] [ebp-4h]

    for ( spotShadowIndex = 0; spotShadowIndex < 4; ++spotShadowIndex )
        R_ShutdownDynamicMesh(&gfxMeshGlob.spotShadowClearMeshData[spotShadowIndex]);
    for ( sunShadowIndex = 0; sunShadowIndex < 2; ++sunShadowIndex )
        R_ShutdownDynamicMesh(&gfxMeshGlob.sunShadowClearMeshData[sunShadowIndex]);
}

void __cdecl R_InitTempSkinBuf()
{
    GfxBackEndData *data; // [esp+0h] [ebp-8h]
    unsigned int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 2; ++i )
    {
        data = &s_backEndData[i];
        if ( data->tempSkinPos
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                        751,
                        0,
                        "%s",
                        "!data->tempSkinPos") )
        {
            __debugbreak();
        }
        if ( data->tempSkinBuf )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                            752,
                            0,
                            "%s",
                            "!data->tempSkinBuf") )
                __debugbreak();
        }
        data->tempSkinBuf = (unsigned __int8 *)Z_VirtualReserve(0x800000);
    }
}

void __cdecl R_ShutdownTempSkinBuf()
{
    GfxBackEndData *data; // [esp+0h] [ebp-8h]
    unsigned int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < 2; ++i )
    {
        data = &s_backEndData[i];
        if ( data->tempSkinBuf )
        {
            Z_VirtualFree(data->tempSkinBuf, 22);
            data->tempSkinBuf = 0;
            data->tempSkinPos = 0;
        }
    }
}

void __cdecl R_InitRenderThread()
{
    if ( !Sys_SpawnRenderThread((void (__cdecl *)(unsigned int))RB_RenderThread) )
        Com_Error(ERR_FATAL, "Failed to create render thread");
}

void __cdecl R_SyncRenderThread()
{
    PROF_SCOPED("R_SyncRenderThread");

    if ( !Sys_IsRenderThread() )
    {
        if ( !Sys_IsMainThread()
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                        813,
                        0,
                        "%s",
                        "Sys_IsMainThread()") )
        {
            __debugbreak();
        }
        if ( rg.registered && sys_smp_allowed->current.enabled )
        {
            if ( !dx.device
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp", 826, 0, "%s", "dx.device") )
            {
                __debugbreak();
            }
            if ( r_glob.remoteScreenUpdateNesting
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                            830,
                            0,
                            "%s",
                            "r_glob.remoteScreenUpdateNesting == 0") )
            {
                __debugbreak();
            }
            if ( r_glob.startedRenderThread )
            {
                Sys_FrontEndSleep();
                Sys_WaitRenderer();
            }
        }
    }
}

GfxCmdArray *R_ClearCmdList()
{
    GfxCmdArray *commands; // edx
    GfxCmdArray *result; // eax

    commands = frontEndDataOut->commands;
    commands->usedTotal = 0;
    result = commands;
    commands->usedCritical = 0;
    commands->lastCmd = 0;
    return result;
}

void __cdecl R_IssueRenderCommands(unsigned int type)
{

    if ( !Sys_IsMainThread()
        && !Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    1197,
                    0,
                    "%s",
                    "Sys_IsMainThread() || Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    frontEndDataOut->drawType = type;
    if ( !R_HandOffToBackend(type) )
    {
        if ( (type & 2) != 0 )
            R_PerformanceCounters();
        if ( Sys_IsMainThread() )
            R_WaitFrontendWorkerCmds();
        R_UpdateSkinCacheUsage();
        R_UnlockSkinnedCache();
        Sys_WaitRenderer();
        RB_BeginFrame(frontEndDataOut);
        RB_Draw3D();
        RB_CallExecuteRenderCommands();
        if ( Sys_IsRenderThread() )
            Sys_RenderCompleted();
        RB_EndFrame(frontEndDataOut->drawType);
        R_ToggleSmpFrame();
    }
}

void R_PerformanceCounters()
{
    return;
    //unsigned int result; // eax
    //
    ////PIXBeginNamedEvent(-1, "dev perf counters");
    //result = GetCurrentThreadId();
    //if ( result == (unsigned int)g_DXDeviceThread )
    //{
    //    result = 0;
    //    if ( !HIDWORD(g_DXDeviceThread) )
    //        return //D3DPERF_EndEvent();
    //}
    //return result;
}

void R_UpdateSkinCacheUsage()
{
    iassert(frontEndDataOut->skinnedCacheVb);

    rg.skinnedCacheReachedThreshold = frontEndDataOut->skinnedCacheVb->used >= 0x6AACC0u;
}

char __cdecl R_HandOffToBackend(char type)
{
    bool v2; // [esp+3h] [ebp-1h]

    if ( r_smp_backend->current.enabled )
        v2 = sys_smp_allowed->current.enabled && !r_glob.isRenderingRemoteUpdate;
    else
        v2 = 0;
    if ( v2 )
    {
        R_ToggleSmpFrameCmd(type);
        return 1;
    }
    else
    {
        if ( Sys_IsMainThread() )
            R_SyncRenderThread();
        *frontEndDataOut->dynamicBufferCurrentFrame = 0;
        return 0;
    }
}

void __cdecl R_ToggleSmpFrameCmd(char type)
{
    GfxBackEndData *front_end_data; // [esp+14h] [ebp-Ch]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    993,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    //BLOPS_NULLSUB();
    {
        PROF_SCOPED("wait renderer");
        Sys_WaitRenderer();
    }
    
    RB_CopyBackendStats();
    if ((type & 2) != 0)
    {
        R_PerformanceCounters();
    }

    {
        PROF_SCOPED("wait frontend workercmds");
        while (!R_FinishedFrontendWorkerCmds())
            NET_Sleep(1u);
    }

    R_UpdateSkinCacheUsage();
    //BLOPS_NULLSUB();
    R_UnlockSkinnedCache();
    front_end_data = frontEndDataOut;
    R_ToggleSmpFrame();
    //PIXSetMarker(-1, "wake renderer");
    Sys_WakeRenderer((void *)front_end_data);
}

void __cdecl R_AbortRenderCommands()
{
    if ( !Sys_IsMainThread()
        && !Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    1276,
                    0,
                    "%s",
                    "Sys_IsMainThread() || Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    if ( rg.registered )
    {
        R_ClearCmdList();
        R_UnlockSkinnedCache();
        rg.viewInfoCount = 0;
        Sys_WaitRenderer();
        if ( Sys_IsRenderThread() )
        {
            if ( Sys_RendererSleep() )
                Sys_RenderCompleted();
        }
        R_ToggleSmpFrame();
        rg.inFrame = 0;
    }
}

void __cdecl R_BeginClientCmdList2D()
{
    frontEndDataOut->viewInfo[frontEndDataOut->viewInfoCount].cmds = &s_cmdList->cmds[s_cmdList->usedTotal];
}

void __cdecl R_ClearClientCmdList2D()
{
    frontEndDataOut->viewInfo[frontEndDataOut->viewInfoCount].cmds = 0;
}

void __cdecl R_BeginSharedCmdList()
{
    frontEndDataOut->cmds = &s_cmdList->cmds[s_cmdList->usedTotal];
}

void __cdecl R_BeginCompositingCmdList()
{
    frontEndDataOut->compositingCmds = &s_cmdList->cmds[s_cmdList->usedTotal];
}

// nx-port: every R_GetCommandBuffer call below passed its command's literal
// *x86* sizeof() as the byte count. On LP64 each command that holds a pointer
// is wider, so those literals under-reserve and the field writes that follow
// run past the reservation into the next command in the list. Reserve the real
// size instead, rounded up so the next command stays 8-aligned for its pointer
// members. The back end walks the list by this same header->byteCount
// (rb_backend.cpp: `execState->cmd += *(unsigned __int16 *)execState->cmd`),
// so producer and walk stay in step on both ABIs.
static int R_CmdBytes(size_t size)
{
    return (int)((size + 7) & ~(size_t)7);
}

void __cdecl R_AddCmdEndOfList()
{
    R_GetCommandBuffer(RC_END_OF_LIST, R_CmdBytes(sizeof(GfxCmdHeader)));
}

GfxCmdHeader *__cdecl R_GetCommandBuffer(GfxRenderCommand renderCmd, int bytes)
{
    const char *v2; // eax
    GfxUI3DStack *UI3DStack; // eax
    unsigned __int8 v5; // [esp+8h] [ebp-10h]
    GfxCmdHeader *header; // [esp+Ch] [ebp-Ch]
    signed int ui3dWindowIndex; // [esp+10h] [ebp-8h]
    int sizeLimit; // [esp+14h] [ebp-4h]

    if ( !s_cmdList
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp", 1316, 0, "%s", "s_cmdList") )
    {
        __debugbreak();
    }
    if ( (unsigned int)renderCmd > RC_DRAW_FRAMED
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    1317,
                    0,
                    "%s\n\t(renderCmd) = %i",
                    "(renderCmd >= 0 && renderCmd < RC_COUNT)",
                    renderCmd) )
    {
        __debugbreak();
    }
    if ( (bytes & 3) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    1318,
                    0,
                    "%s\n\t(bytes) = %i",
                    "((bytes & 3) == 0)",
                    bytes) )
    {
        __debugbreak();
    }
    if ( bytes >= s_cmdList->byteSize
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    1319,
                    0,
                    "%s\n\t(bytes) = %i",
                    "(bytes < s_cmdList->byteSize)",
                    bytes) )
    {
        __debugbreak();
    }
    if ( bytes != (unsigned __int16)bytes
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    1320,
                    0,
                    "%s\n\t(bytes) = %i",
                    "(bytes == static_cast< unsigned short >( bytes ))",
                    bytes) )
    {
        __debugbreak();
    }
    if ( !s_cmdList->cmds
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    1321,
                    0,
                    "%s",
                    "s_cmdList->cmds") )
    {
        __debugbreak();
    }
    if ( !rg.inFrame
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp", 1322, 0, "%s", "rg.inFrame") )
    {
        __debugbreak();
    }
    if ( renderCmd < RC_FIRST_NONCRITICAL && s_cmdList->usedCritical < 7680 && bytes + s_cmdList->usedCritical >= 7680 )
        Com_PrintWarning(8, "RENDERCOMMAND_CRITICAL_WARN_SIZE (%i bytes) reached\n", 7680);
    if ( s_cmdList->usedTotal < s_cmdList->warnSize && bytes + s_cmdList->usedTotal >= s_cmdList->warnSize )
        Com_PrintWarning(8, "RENDERCOMMAND_WARN_SIZE (%.0f KB) reached\n", (float)((float)s_cmdList->warnSize / 1024.0));
    sizeLimit = s_cmdList->byteSize - s_cmdList->usedTotal;
    if ( renderCmd >= RC_FIRST_NONCRITICAL )
        sizeLimit -= 0x2000 - s_cmdList->usedCritical;
    if ( bytes <= sizeLimit )
    {
        //PIXSetMarker(0, gfxRenderCommandNames[renderCmd]);
        header = (GfxCmdHeader *)&s_cmdList->cmds[s_cmdList->usedTotal];
        s_cmdList->usedTotal += bytes;
        s_cmdList->usedCritical += renderCmd >= RC_FIRST_NONCRITICAL ? 0 : bytes;
        s_cmdList->lastCmd = header;
        header->id = renderCmd;
        UI3DStack = R_GetUI3DStack();
        ui3dWindowIndex = R_UI3DStack_Top(UI3DStack);
        if ( ui3dWindowIndex > 127
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                        1362,
                        0,
                        "%s",
                        "ui3dWindowIndex <= GFXCMDHEADER_UI3D_MASK_WINDOW_INDEX") )
        {
            __debugbreak();
        }
        if ( ui3dWindowIndex >= 0 )
            v5 = ui3dWindowIndex | 0x80;
        else
            v5 = 0;
        header->ui3d = v5;
        R_UI3D_IncrementRenderCmdCount(ui3dWindowIndex);
        header->byteCount = bytes;
        return header;
    }
    else
    {
        if ( renderCmd < RC_FIRST_NONCRITICAL )
        {
            v2 = va("rc %i used %i critical %i bytes %i", renderCmd, s_cmdList->usedTotal, s_cmdList->usedCritical, bytes);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                            1344,
                            0,
                            "%s\n\t%s",
                            "renderCmd >= RC_FIRST_NONCRITICAL",
                            v2) )
                __debugbreak();
        }
        s_cmdList->lastCmd = 0;
        return 0;
    }
}

void R_ToggleSmpFrame()
{
    unsigned intresult; // eax
    volatile int surfPos; // [esp+0h] [ebp-20h]
    DebugGlobals *debugGlobalsEntry; // [esp+1Ch] [ebp-4h]

    PROF_SCOPED("R_ToggleSmpFrame");
    if ( !rg.viewInfoCount )
        CG_PerfUpdate();
    s_smpFrame = (s_smpFrame + 1) % 2;
    ++rg.frontEndFrameCount;
    gfxBuf.dynamicBufferFrame = (gfxBuf.dynamicBufferFrame + 1) % 2;
    gfxBuf.preTessBufferFrame = (gfxBuf.preTessBufferFrame + 1) % 2;
    frontEndDataOut = &s_backEndData[s_smpFrame];
    if ( !rg.frontEndFrameCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    1512,
                    0,
                    "%s",
                    "rg.frontEndFrameCount > 0") )
    {
        __debugbreak();
    }
    R_FreeTempSkinBuffer();
    if ( !frontEndDataOut
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    1518,
                    0,
                    "%s",
                    "frontEndDataOut") )
    {
        __debugbreak();
    }
    frontEndDataOut->frameCount = rg.frontEndFrameCount;
    frontEndDataOut->viewInfoCount = rg.viewInfoCount;
    frontEndDataOut->viewInfo = g_viewInfo[s_smpFrame];
    frontEndDataOut->commands = &g_frontEndCmds[s_smpFrame];
    frontEndDataOut->dynSModelClientViewArray = g_dynSModelClientView[s_smpFrame];
    if ( !rg.viewInfoCount )
        R_ToggleModelLightingFrame();
    frontEndDataOut->dynamicBufferCurrentFrame = &gfxBuf.dynamicBufferCurrentFrame[gfxBuf.dynamicBufferFrame];
    frontEndDataOut->skinnedCacheVb = &gfxBuf.skinnedCacheVbPool[gfxBuf.dynamicBufferFrame];
    gfxBuf.preTessIndexBuffer = &gfxBuf.preTessIndexBufferPool[gfxBuf.preTessBufferFrame];
    frontEndDataOut->preTessIb = &gfxBuf.preTessIndexBufferPool[gfxBuf.preTessBufferFrame];
    frontEndDataOut->smcPatchCount = 0;
    frontEndDataOut->smcPatchVertsUsed = 0;
    frontEndDataOut->modelLightingPatchCount = 0;
    frontEndDataOut->skinnedCacheVb->used = 0;
    s_cmdList = frontEndDataOut->commands;

    {
        PROF_SCOPED("dev clear buffers");
        if (frontEndDataOut->drawSurfCount > 0x8000u
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                1607,
                0,
                "frontEndDataOut->drawSurfCount not in [0, static_cast< int >( ARRAY_COUNT( frontEndDataOut->drawSurfs ) )]\n"
                "\t%i not in [%i, %i]",
                frontEndDataOut->drawSurfCount,
                0,
                0x8000))
        {
            __debugbreak();
        }
        if (frontEndDataOut->surfPos < 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                1608,
                0,
                "%s\n\t(frontEndDataOut->surfPos) = %i",
                "(frontEndDataOut->surfPos >= 0)",
                frontEndDataOut->surfPos))
        {
            __debugbreak();
        }
        if (frontEndDataOut->surfPos > 0x40000)
            surfPos = 0x40000;
        else
            surfPos = frontEndDataOut->surfPos;
        frontEndDataOut->surfPos = surfPos;
        if (frontEndDataOut->cloudCount > 0x100u
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                1610,
                0,
                "frontEndDataOut->cloudCount not in [0, GFX_PARTICLE_CLOUD_LIMIT]\n\t%i not in [%i, %i]",
                frontEndDataOut->cloudCount,
                0,
                256))
        {
            __debugbreak();
        }
        Com_Memset((unsigned int *)frontEndDataOut->drawSurfs, 176, 8 * frontEndDataOut->drawSurfCount);
        Com_Memset((unsigned int *)frontEndDataOut->surfsBuffer, 176, frontEndDataOut->surfPos);
        Com_Memset((unsigned int *)frontEndDataOut->clouds, 176, frontEndDataOut->cloudCount << 6);
        Com_Memset(&frontEndDataOut->codeMeshes[0].triCount, 176, 0x8000);
        Com_Memset(frontEndDataOut->primDrawSurfsBuf, 176, 4 * frontEndDataOut->primDrawSurfPos);
        Com_Memset((unsigned int *)&frontEndDataOut->fogSettings, 176, 80);
    }
    
    frontEndDataOut->drawSurfCount = 0;
    frontEndDataOut->primDrawSurfPos = 0;
    frontEndDataOut->surfPos = 0;
    frontEndDataOut->gfxEntCount = 1;
    frontEndDataOut->textureOverrideCount = 0;
    frontEndDataOut->cloudCount = 0;
    frontEndDataOut->shaderConstantSetCount = 1;
    frontEndDataOut->codeMeshPtr = &g_codeMesh[g_currCodeMesh];
    g_currCodeMesh = (g_currCodeMesh + 1) % 2;
    frontEndDataOut->codeMeshArgsCount = 0;
    frontEndDataOut->codeMeshCount = 0;
    R_ResetMesh(frontEndDataOut->codeMeshPtr);
    frontEndDataOut->markMeshCount = 0;
    R_ResetMesh(&frontEndDataOut->markMesh);
    frontEndDataOut->viewParmCount = 0;
    frontEndDataOut->cmds = 0;
    frontEndDataOut->compositingCmds = 0;
    frontEndDataOut->drawType = 0;
    frontEndDataOut->hideMatureContent = 0;
    frontEndDataOut->splitscreen = 0;
    debugGlobalsEntry = &frontEndDataOut->debugGlobals;
    frontEndDataOut->debugGlobals.lineCount = 0;
    debugGlobalsEntry->stringCount = 0;
    debugGlobalsEntry->polySet.vertCount = 0;
    R_DynSModelInitGfxState(&frontEndDataOut->dynSModelState);
}

void R_FreeTempSkinBuffer()
{
    if ( frontEndDataOut->tempSkinPos )
    {
        Z_VirtualDecommit((char *)frontEndDataOut->tempSkinBuf, frontEndDataOut->tempSkinPos, 22);
        frontEndDataOut->tempSkinPos = 0;
    }
}

unsigned int __cdecl R_GetFrameCount()
{
    return frontEndDataOut->frameCount;
}

GfxViewParms *__cdecl R_AllocViewParms()
{
    if ( !frontEndDataOut
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    1743,
                    0,
                    "%s",
                    "frontEndDataOut") )
    {
        __debugbreak();
    }
    if ( frontEndDataOut->viewParmCount >= 4u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    1744,
                    0,
                    "frontEndDataOut->viewParmCount doesn't index ARRAY_COUNT( frontEndDataOut->viewParms )\n\t%i not in [0, %i)",
                    frontEndDataOut->viewParmCount,
                    4) )
    {
        __debugbreak();
    }
    return &frontEndDataOut->viewParms[frontEndDataOut->viewParmCount++];
}

void __cdecl R_AddCmdDrawStretchComposite(
                float x,
                float y,
                float w,
                float h,
                float s0,
                float t0,
                float s1,
                float t1,
                const float *color,
                GfxImage *composite)
{
    GfxCmdStretchComposite *cmd; // [esp+0h] [ebp-4h]

    cmd = (GfxCmdStretchComposite *)R_GetCommandBuffer(RC_STRETCH_COMPOSITE, R_CmdBytes(sizeof(GfxCmdStretchComposite)));
    if ( cmd )
    {
        cmd->image = composite;
        cmd->x = x;
        cmd->y = y;
        cmd->w = w;
        cmd->h = h;
        cmd->s0 = s0;
        cmd->t0 = t0;
        cmd->s1 = s1;
        cmd->t1 = t1;
        R_ConvertColorToBytes(color, (unsigned __int8 *)&cmd->color);
    }
}

void __cdecl R_AddCmdDrawStretchPicW(
                float x,
                float y,
                float w0,
                float w,
                float h,
                float s0,
                float t0,
                float s1,
                float t1,
                const float *color,
                Material *material)
{
    const char *v11; // eax
    const char *Name; // eax
    Material *defaultMaterial; // [esp+0h] [ebp-1Ch]
    Material *actualMaterial; // [esp+14h] [ebp-8h]
    GfxCmdStretchPic *cmd; // [esp+18h] [ebp-4h]

    if ( material )
        defaultMaterial = (Material *)Material_FromHandle(material);
    else
        defaultMaterial = rgp.defaultMaterial;
    actualMaterial = defaultMaterial;
    if ( !Material_HasAnyFogableTechnique(defaultMaterial) || Material_IsDefault(defaultMaterial) )
    {
        if ( (defaultMaterial->stateFlags & 0x10) != 0 )
        {
            Name = Material_GetName(material);
            Com_PrintWarning(
                8,
                "R_AddCmdDrawStretchPic: NOT DRAWING WITH MATERIAL \"%s\", because it uses the depth buffer. Set materialType to 2d.\n",
                Name);
            actualMaterial = rgp.defaultMaterial;
        }
    }
    else
    {
        v11 = Material_GetName(material);
        Com_PrintWarning(
            8,
            "R_AddCmdDrawStretchPic: NOT DRAWING WITH MATERIAL \"%s\", because it has a fogable technique.\n",
            v11);
        actualMaterial = rgp.defaultMaterial;
    }
    if ( (actualMaterial->stateFlags & 0x10) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    1796,
                    0,
                    "%s",
                    "!Material_UsesDepthBuffer( actualMaterial )") )
    {
        __debugbreak();
    }
    cmd = (GfxCmdStretchPic *)R_GetCommandBuffer(RC_FIRST_NONCRITICAL, R_CmdBytes(sizeof(GfxCmdStretchPic)));
    if ( cmd )
    {
        cmd->material = actualMaterial;
        cmd->x = x;
        cmd->y = y;
        cmd->w0 = w0;
        cmd->w = w;
        cmd->h = h;
        cmd->s0 = s0;
        cmd->t0 = t0;
        cmd->s1 = s1;
        cmd->t1 = t1;
        R_ConvertColorToBytes(color, (unsigned __int8 *)&cmd->color);
    }
}

bool __cdecl Material_HasAnyFogableTechnique(const Material *material)
{
    return Material_GetTechnique(material, 0xAu) || Material_GetTechnique(material, 5u);
}

void __cdecl R_AddCmdDrawStretchPic(
                float x,
                float y,
                float w,
                float h,
                float s0,
                float t0,
                float s1,
                float t1,
                const float *color,
                Material *material)
{
    R_AddCmdDrawStretchPicW(x, y, 1.0, w, h, s0, t0, s1, t1, color, material);
}

void __cdecl R_AddCmdDrawStretchPicRotateXYW(
                float x,
                float y,
                float w0,
                float w,
                float h,
                float s0,
                float t0,
                float s1,
                float t1,
                float angle,
                const float *color,
                Material *material)
{
    Material *defaultMaterial; // [esp+4h] [ebp-8h]
    GfxCmdStretchPicRotateXY *cmd; // [esp+8h] [ebp-4h]

    cmd = (GfxCmdStretchPicRotateXY *)R_GetCommandBuffer(RC_STRETCH_PIC_ROTATE_XY, R_CmdBytes(sizeof(GfxCmdStretchPicRotateXY)));
    if ( cmd )
    {
        if ( material )
            defaultMaterial = (Material *)Material_FromHandle(material);
        else
            defaultMaterial = rgp.defaultMaterial;
        cmd->material = defaultMaterial;
        cmd->x = x;
        cmd->y = y;
        cmd->w0 = w0;
        cmd->w = w;
        cmd->h = h;
        cmd->s0 = s0;
        cmd->t0 = t0;
        cmd->s1 = s1;
        cmd->t1 = t1;
        R_ConvertColorToBytes(color, (unsigned __int8 *)&cmd->color);
        cmd->rotation = AngleNormalize360(angle);
    }
}

void __cdecl R_AddCmdDrawStretchPicRotateXY(
                float x,
                float y,
                float w,
                float h,
                float s0,
                float t0,
                float s1,
                float t1,
                float angle,
                const float *color,
                Material *material)
{
    R_AddCmdDrawStretchPicRotateXYW(x, y, 1.0, w, h, s0, t0, s1, t1, angle, color, material);
}

void __cdecl R_AddCmdDrawStretchPicRotateST(
                float x,
                float y,
                float w,
                float h,
                float centerS,
                float centerT,
                float radiusST,
                float scaleFinalS,
                float scaleFinalT,
                float angle,
                const float *color,
                Material *material)
{
    Material *defaultMaterial; // [esp+4h] [ebp-8h]
    GfxCmdStretchPicRotateST *cmd; // [esp+8h] [ebp-4h]

    cmd = (GfxCmdStretchPicRotateST *)R_GetCommandBuffer(RC_STRETCH_PIC_ROTATE_ST, R_CmdBytes(sizeof(GfxCmdStretchPicRotateST)));
    if ( cmd )
    {
        if ( material )
            defaultMaterial = (Material *)Material_FromHandle(material);
        else
            defaultMaterial = rgp.defaultMaterial;
        cmd->material = defaultMaterial;
        cmd->x = x;
        cmd->y = y;
        cmd->w = w;
        cmd->h = h;
        cmd->centerS = centerS;
        cmd->centerT = centerT;
        cmd->radiusST = radiusST;
        cmd->scaleFinalS = scaleFinalS;
        cmd->scaleFinalT = scaleFinalT;
        R_ConvertColorToBytes(color, (unsigned __int8 *)&cmd->color);
        cmd->rotation = AngleNormalize360(angle);
    }
}

void __cdecl R_AddCmdDrawTextWithCursor(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float w,
                float xScale,
                float yScale,
                float rotation,
                const float *color,
                int style,
                int cursorPos,
                char cursor)
{
    AddBaseDrawTextCmd(text, maxChars, font, x, y, w, xScale, yScale, rotation, color, style, cursorPos, cursor);
}

GfxCmdDrawText2D *__cdecl AddBaseDrawTextCmd(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float w,
                float xScale,
                float yScale,
                float rotation,
                const float *color,
                int style,
                int cursorPos,
                char cursor)
{
    unsigned int len; // [esp+10h] [ebp-Ch]
    GfxCmdDrawText2D *cmd; // [esp+18h] [ebp-4h]

    if ( maxChars <= 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp", 1938, 0, "%s", "maxChars > 0") )
    {
        __debugbreak();
    }
    if ( !text && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp", 1939, 0, "%s", "text") )
        __debugbreak();
    if ( !*text && cursorPos < 0 )
        return 0;
    len = strlen(text);
    cmd = (GfxCmdDrawText2D *)R_GetCommandBuffer(RC_DRAW_TEXT_2D, R_CmdBytes(sizeof(GfxCmdDrawText2D) + len));
    if ( !cmd )
        return 0;
    cmd->x = x;
    cmd->y = y;
    cmd->w = w;
    cmd->rotation = rotation;
    cmd->font = font;
    cmd->xScale = xScale;
    cmd->yScale = yScale;
    R_ConvertColorToBytes(color, (unsigned __int8 *)&cmd->color);
    cmd->maxChars = maxChars;
    cmd->renderFlags = 0;
    switch ( style )
    {
        case 3:
            cmd->renderFlags |= 4u;
            break;
        case 6:
            cmd->renderFlags |= 0xCu;
            break;
        case 128:
            cmd->renderFlags |= 1u;
            break;
    }
    if ( cursorPos > -1 )
    {
        cmd->renderFlags |= 2u;
        cmd->cursorPos = cursorPos;
        cmd->cursorLetter = cursor;
    }
    memcpy((unsigned __int8 *)cmd->text, (unsigned __int8 *)text, len);
    cmd->text[len] = 0;
    return cmd;
}

void __cdecl R_AddCmdDrawText(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                float rotation,
                const float *color,
                int style)
{
    R_AddCmdDrawTextWithCursor(text, maxChars, font, x, y, 1.0, xScale, yScale, rotation, color, style, -1, 0);
}

void __cdecl R_AddCmdDrawTextW(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float w,
                float xScale,
                float yScale,
                float rotation,
                const float *color,
                int style)
{
    R_AddCmdDrawTextWithCursor(text, maxChars, font, x, y, w, xScale, yScale, rotation, color, style, -1, 0);
}

void __cdecl R_AddCmdDrawTextSubtitle(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                float rotation,
                const float *color,
                int style,
                const float *glowColor,
                bool cinematic)
{
    GfxCmdDrawText2D *cmd; // [esp+28h] [ebp-4h]

    cmd = AddBaseDrawTextCmd(text, maxChars, font, x, y, 1.0, xScale, yScale, rotation, color, style, -1, 0);
    if ( cmd )
    {
        if ( cinematic )
            cmd->renderFlags |= 0x200u;
        cmd->renderFlags |= 0x100u;
        SetDrawText2DGlowParms(cmd, color, glowColor);
    }
}

char __cdecl SetDrawText2DGlowParms(GfxCmdDrawText2D *cmd, const float *color, const float *glowColor)
{
    float scaledGlowColor[4]; // [esp+0h] [ebp-10h] BYREF

    if ( !glowColor )
        return 0;
    if ( glowColor[3] == 0.0 )
        return 0;
    cmd->renderFlags |= 0x30u;
    scaledGlowColor[0] = *glowColor * 0.1;
    scaledGlowColor[1] = glowColor[1] * 0.1;
    scaledGlowColor[2] = glowColor[2] * 0.1;
    scaledGlowColor[3] = glowColor[3] * color[3];
    R_ConvertColorToBytes(scaledGlowColor, (unsigned __int8 *)&cmd->glowForceColor);
    return 1;
}

void __cdecl R_AddCmdDrawTextWithEffects(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float w,
                float xScale,
                float yScale,
                float rotation,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration)
{
    GfxCmdDrawText2D *cmd; // [esp+28h] [ebp-4h]

    cmd = AddBaseDrawTextCmd(text, maxChars, font, x, y, w, xScale, yScale, rotation, color, style, -1, 0);
    if ( cmd )
    {
        SetDrawText2DGlowParms(cmd, color, glowColor);
        SetDrawText2DTypewriterFXParms(
            cmd,
            fxMaterial,
            fxMaterialGlow,
            fxBirthTime,
            fxLetterTime,
            fxDecayStartTime,
            fxDecayDuration);
    }
}

char __cdecl SetDrawText2DTypewriterFXParms(
                GfxCmdDrawText2D *cmd,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration)
{
    if ( !fxMaterial )
        return 0;
    if ( !fxMaterialGlow )
        return 0;
    if ( !fxBirthTime )
        return 0;
    cmd->renderFlags |= 0x880u;
    cmd->fxMaterial = fxMaterial;
    cmd->fxMaterialGlow = fxMaterialGlow;
    cmd->fxBirthTime = fxBirthTime;
    cmd->fxLetterTime = fxLetterTime;
    cmd->fxDecayStartTime = fxDecayStartTime;
    cmd->fxDecayDuration = fxDecayDuration;
    cmd->padding = 0.0f;
    return 1;
}

void __cdecl R_AddCmdDrawTextWithCOD7DecodeEffects(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                float rotation,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration)
{
    GfxCmdDrawText2D *cmd; // [esp+28h] [ebp-4h]

    cmd = AddBaseDrawTextCmd(text, maxChars, font, x, y, 1.0, xScale, yScale, rotation, color, style, -1, 0);
    if ( cmd )
    {
        SetDrawText2DGlowParms(cmd, color, glowColor);
        SetDrawText2DCOD7DecodeFXParms(
            cmd,
            fxMaterial,
            fxMaterialGlow,
            fxBirthTime,
            fxLetterTime,
            fxDecayStartTime,
            fxDecayDuration);
    }
}

char __cdecl SetDrawText2DCOD7DecodeFXParms(
                GfxCmdDrawText2D *cmd,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration)
{
    if ( !fxBirthTime )
        return 0;
    cmd->renderFlags |= 0x10000u;
    cmd->fxMaterial = fxMaterial;
    cmd->fxMaterialGlow = fxMaterialGlow;
    cmd->fxBirthTime = fxBirthTime;
    cmd->fxLetterTime = fxLetterTime;
    cmd->fxDecayStartTime = fxDecayStartTime;
    cmd->fxDecayDuration = fxDecayDuration;
    cmd->padding = 0.0f;
    return 1;
}

void __cdecl R_AddCmdDrawTextWithCOD7TypeWriterEffects(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                float rotation,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration)
{
    GfxCmdDrawText2D *cmd; // [esp+28h] [ebp-4h]

    cmd = AddBaseDrawTextCmd(text, maxChars, font, x, y, 1.0, xScale, yScale, rotation, color, style, -1, 0);
    if ( cmd )
    {
        SetDrawText2DGlowParms(cmd, color, glowColor);
        SetDrawText2DCOD7TypeWriterFXParms(
            cmd,
            fxMaterial,
            fxMaterialGlow,
            fxBirthTime,
            fxLetterTime,
            fxDecayStartTime,
            fxDecayDuration);
    }
}

char __cdecl SetDrawText2DCOD7TypeWriterFXParms(
                GfxCmdDrawText2D *cmd,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration)
{
    if ( !fxBirthTime )
        return 0;
    cmd->renderFlags |= 0x1000u;
    cmd->fxMaterial = fxMaterial;
    cmd->fxMaterialGlow = fxMaterialGlow;
    cmd->fxBirthTime = fxBirthTime;
    cmd->fxLetterTime = fxLetterTime;
    cmd->fxDecayStartTime = fxDecayStartTime;
    cmd->fxDecayDuration = fxDecayDuration;
    cmd->padding = 0.0f;
    return 1;
}

void __cdecl R_AddCmdDrawTextWithRedactEffects(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                float rotation,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                int fxRedactDecayStartTime,
                int fxRedactDecayDuration)
{
    GfxCmdDrawText2D *cmd; // [esp+28h] [ebp-4h]

    cmd = AddBaseDrawTextCmd(text, maxChars, font, x, y, 1.0, xScale, yScale, rotation, color, style, -1, 0);
    if ( cmd )
    {
        SetDrawText2DGlowParms(cmd, color, glowColor);
        SetDrawText2DRedactFXParms(
            cmd,
            fxMaterial,
            fxMaterialGlow,
            fxBirthTime,
            fxLetterTime,
            fxDecayStartTime,
            fxDecayDuration,
            fxRedactDecayStartTime,
            fxRedactDecayDuration);
    }
}

char __cdecl SetDrawText2DRedactFXParms(
                GfxCmdDrawText2D *cmd,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                int fxRedactDecayStartTime,
                int fxRedactDecayDuration)
{
    if ( !fxBirthTime )
        return 0;
    cmd->renderFlags |= 0x4000u;
    cmd->fxMaterial = fxMaterial;
    cmd->fxMaterialGlow = fxMaterialGlow;
    cmd->fxBirthTime = fxBirthTime;
    cmd->fxLetterTime = fxLetterTime;
    cmd->fxDecayStartTime = fxDecayStartTime;
    cmd->fxDecayDuration = fxDecayDuration;
    cmd->fxRedactDecayStartTime = fxRedactDecayStartTime;
    cmd->fxRedactDecayDuration = fxRedactDecayDuration;
    cmd->padding = 0.0f;
    return 1;
}

void __cdecl R_AddCmdDrawTextWithPopInEffects(
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                float rotation,
                const float *color,
                int style,
                const float *glowColor,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration)
{
    GfxCmdDrawText2D *cmd; // [esp+28h] [ebp-4h]

    cmd = AddBaseDrawTextCmd(text, maxChars, font, x, y, 1.0, xScale, yScale, rotation, color, style, -1, 0);
    if ( cmd )
    {
        SetDrawText2DGlowParms(cmd, color, glowColor);
        SetDrawText2DPopInWriteFXParms(
            cmd,
            fxMaterial,
            fxMaterialGlow,
            fxBirthTime,
            fxLetterTime,
            fxDecayStartTime,
            fxDecayDuration);
    }
}

char __cdecl SetDrawText2DPopInWriteFXParms(
                GfxCmdDrawText2D *cmd,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration)
{
    if ( !fxBirthTime )
        return 0;
    cmd->renderFlags |= 0x2000u;
    cmd->fxMaterial = fxMaterial;
    cmd->fxMaterialGlow = fxMaterialGlow;
    cmd->fxBirthTime = fxBirthTime;
    cmd->fxLetterTime = fxLetterTime;
    cmd->fxDecayStartTime = fxDecayStartTime;
    cmd->fxDecayDuration = fxDecayDuration;
    cmd->padding = 0.0f;
    return 1;
}

void __cdecl R_AddCmdDrawConsoleText(
                char *textPool,
                int poolSize,
                int firstChar,
                int charCount,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                const float *color,
                int style)
{
    AddBaseDrawConsoleTextCmd(textPool, poolSize, firstChar, charCount, font, x, y, xScale, yScale, color, style);
}

GfxCmdDrawText2D *__cdecl AddBaseDrawConsoleTextCmd(
                char *textPool,
                int poolSize,
                int firstChar,
                int charCount,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                const float *color,
                int style)
{
    GfxCmdDrawText2D *cmd; // [esp+4h] [ebp-4h]

    if ( charCount < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    2266,
                    0,
                    "%s",
                    "charCount >= 0") )
    {
        __debugbreak();
    }
    if ( !textPool
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp", 2267, 0, "%s", "textPool") )
    {
        __debugbreak();
    }
    if ( !charCount )
        return 0;
    cmd = (GfxCmdDrawText2D *)R_GetCommandBuffer(RC_DRAW_TEXT_2D, R_CmdBytes(sizeof(GfxCmdDrawText2D) + charCount));
    if ( !cmd )
        return 0;
    cmd->x = x;
    cmd->y = y;
    cmd->w = 1.0f;
    cmd->rotation = 0.0f;
    cmd->font = font;
    cmd->xScale = xScale;
    cmd->yScale = yScale;
    R_ConvertColorToBytes(color, (unsigned __int8 *)&cmd->color);
    cmd->maxChars = 0x7FFFFFFF;
    cmd->renderFlags = 0;
    switch ( style )
    {
        case 3:
            cmd->renderFlags |= 4u;
            break;
        case 6:
            cmd->renderFlags |= 0xCu;
            break;
        case 128:
            cmd->renderFlags |= 1u;
            break;
    }
    CopyPoolTextToCmd(textPool, poolSize, firstChar, charCount, cmd);
    return cmd;
}

void __cdecl CopyPoolTextToCmd(char *textPool, int poolSize, int firstChar, int charCount, GfxCmdDrawText2D *cmd)
{
    unsigned int poolRemaining; // [esp+0h] [ebp-4h]

    if ( !cmd && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp", 2243, 0, "%s", "cmd") )
        __debugbreak();
    poolRemaining = poolSize - firstChar;
    if ( charCount > poolSize - firstChar )
    {
        memcpy((unsigned __int8 *)cmd->text, (unsigned __int8 *)&textPool[firstChar], poolRemaining);
        memcpy((unsigned __int8 *)&cmd->text[poolRemaining], (unsigned __int8 *)textPool, charCount - poolRemaining);
    }
    else
    {
        memcpy((unsigned __int8 *)cmd->text, (unsigned __int8 *)&textPool[firstChar], charCount);
    }
    cmd->text[charCount] = 0;
}

void __cdecl R_AddCmdDrawConsoleTextTypewriterFX(
                char *textPool,
                int poolSize,
                int firstChar,
                int charCount,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                const float *color,
                int style,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration)
{
    GfxCmdDrawText2D *cmd; // [esp+18h] [ebp-4h]

    cmd = AddBaseDrawConsoleTextCmd(textPool, poolSize, firstChar, charCount, font, x, y, xScale, yScale, color, style);
    if ( cmd )
    {
        cmd->renderFlags |= 0x880u;
        cmd->fxBirthTime = fxBirthTime;
        cmd->fxLetterTime = fxLetterTime;
        cmd->fxDecayStartTime = fxDecayStartTime;
        cmd->fxDecayDuration = fxDecayDuration;
        cmd->padding = 1.0f;
    }
}

void __cdecl R_AddCmdDrawConsoleTextSubtitle(
                char *textPool,
                int poolSize,
                int firstChar,
                int charCount,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                const float *color,
                int style,
                const float *glowColor)
{
    GfxCmdDrawText2D *cmd; // [esp+18h] [ebp-4h]

    cmd = AddBaseDrawConsoleTextCmd(textPool, poolSize, firstChar, charCount, font, x, y, xScale, yScale, color, style);
    if ( cmd )
    {
        cmd->renderFlags |= 0x100u;
        SetDrawText2DGlowParms(cmd, color, glowColor);
    }
}

void __cdecl R_AddCmdDrawConsoleTextPulseFX(
                char *textPool,
                int poolSize,
                int firstChar,
                int charCount,
                Font_s *font,
                float x,
                float y,
                float xScale,
                float yScale,
                const float *color,
                int style,
                const float *glowColor,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration,
                Material *fxMaterial,
                Material *fxMaterialGlow)
{
    GfxCmdDrawText2D *cmd; // [esp+18h] [ebp-4h]

    cmd = AddBaseDrawConsoleTextCmd(textPool, poolSize, firstChar, charCount, font, x, y, xScale, yScale, color, style);
    if ( cmd )
    {
        SetDrawText2DGlowParms(cmd, color, glowColor);
        SetDrawText2DPulseFXParms(
            cmd,
            fxMaterial,
            fxMaterialGlow,
            fxBirthTime,
            fxLetterTime,
            fxDecayStartTime,
            fxDecayDuration);
    }
}

char __cdecl SetDrawText2DPulseFXParms(
                GfxCmdDrawText2D *cmd,
                Material *fxMaterial,
                Material *fxMaterialGlow,
                int fxBirthTime,
                int fxLetterTime,
                int fxDecayStartTime,
                int fxDecayDuration)
{
    if ( !fxMaterial )
        return 0;
    if ( !fxMaterialGlow )
        return 0;
    if ( !fxBirthTime )
        return 0;
    cmd->renderFlags |= 0xC0u;
    cmd->fxMaterial = fxMaterial;
    cmd->fxMaterialGlow = fxMaterialGlow;
    cmd->fxBirthTime = fxBirthTime;
    cmd->fxLetterTime = fxLetterTime;
    cmd->fxDecayStartTime = fxDecayStartTime;
    cmd->fxDecayDuration = fxDecayDuration;
    cmd->padding = 0.0f;
    return 1;
}

void __cdecl R_AddCmdDrawQuadPicW(const float (*verts)[2], float w, const float *color, Material *material)
{
    Material *defaultMaterial; // [esp+0h] [ebp-14h]
    int cornerIndex; // [esp+Ch] [ebp-8h]
    GfxCmdDrawQuadPic *cmd; // [esp+10h] [ebp-4h]

    cmd = (GfxCmdDrawQuadPic *)R_GetCommandBuffer(RC_DRAW_QUAD_PIC, R_CmdBytes(sizeof(GfxCmdDrawQuadPic)));
    if ( cmd )
    {
        if ( material )
            defaultMaterial = (Material *)Material_FromHandle(material);
        else
            defaultMaterial = rgp.defaultMaterial;
        cmd->material = defaultMaterial;
        for ( cornerIndex = 0; cornerIndex < 4; ++cornerIndex )
        {
            cmd->verts[cornerIndex][0] = (*verts)[2 * cornerIndex];
            cmd->verts[cornerIndex][1] = (*verts)[2 * cornerIndex + 1];
        }
        cmd->w = w;
        R_ConvertColorToBytes(color, (unsigned __int8 *)&cmd->color);
    }
}

void __cdecl R_AddCmdDrawQuadPic(const float (*verts)[2], const float *color, Material *material)
{
    R_AddCmdDrawQuadPicW(verts, 1.0, color, material);
}

void __cdecl R_AddCmdSetCustomConstant(unsigned int type, const float *vec)
{
    GfxCmdSetCustomConstant *cmd; // [esp+4h] [ebp-4h]

    cmd = (GfxCmdSetCustomConstant *)R_GetCommandBuffer(RC_SET_CUSTOM_CONSTANT, R_CmdBytes(sizeof(GfxCmdSetCustomConstant)));

    iassert(cmd);

    cmd->type = (CodeConstant)type;
    cmd->vec[0] = *vec;
    cmd->vec[1] = vec[1];
    cmd->vec[2] = vec[2];
    cmd->vec[3] = vec[3];
}

void __cdecl R_AddCmdSetScissorValues(bool enabled, int x, int y, int width, int height)
{
    GfxCmdSetScissor *cmd; // [esp+0h] [ebp-4h]

    if ( enabled
        && width <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    2467,
                    0,
                    "%s\n\t(width) = %i",
                    "(!enabled || width > 0)",
                    width) )
    {
        __debugbreak();
    }
    if ( enabled
        && height <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    2468,
                    0,
                    "%s\n\t(height) = %i",
                    "(!enabled || height > 0)",
                    height) )
    {
        __debugbreak();
    }
    cmd = (GfxCmdSetScissor *)R_GetCommandBuffer(RC_SET_SCISSOR, R_CmdBytes(sizeof(GfxCmdSetScissor)));
    if ( !cmd && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp", 2472, 0, "%s", "cmd") )
        __debugbreak();
    cmd->enabled = enabled;
    cmd->scissor.x = x;
    cmd->scissor.y = y;
    cmd->scissor.width = width;
    cmd->scissor.height = height;
}

void __cdecl R_AddCmdResolveComposite(void (__cdecl *callback)(GfxImage *))
{
    GfxCmdResolveComposite *cmd; // [esp+0h] [ebp-4h]

    cmd = (GfxCmdResolveComposite *)R_GetCommandBuffer(RC_RESOLVE_COMPOSITE, R_CmdBytes(sizeof(GfxCmdResolveComposite)));
    if ( !cmd && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp", 2489, 0, "%s", "cmd") )
        __debugbreak();
    cmd->callback = callback;
}

void __cdecl R_AddCmdPCCopyImageGenMIP(void (__cdecl *callback)(void *), GfxImage *image, void *job)
{
    GfxCmdPCCopyImageGenMIP *cmd; // [esp+0h] [ebp-4h]

    cmd = (GfxCmdPCCopyImageGenMIP *)R_GetCommandBuffer(RC_PC_COPY_IMAGE_GEN_MIP, R_CmdBytes(sizeof(GfxCmdPCCopyImageGenMIP)));
    if ( !cmd && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp", 2500, 0, "%s", "cmd") )
        __debugbreak();
    cmd->callback = callback;
    cmd->image = image;
    cmd->job = job;
}

void __cdecl R_BeginFrame()
{
    const char *v0; // eax
    bool v1; // [esp+0h] [ebp-468h]
    bool v2; // [esp+4h] [ebp-464h]
    _D3DADAPTER_IDENTIFIER9 id; // [esp+10h] [ebp-458h] BYREF
    int n; // [esp+464h] [ebp-4h]

    PROF_SCOPED("R_BeginFrame"); // LWSS ADD

    if ( rg.registered )
    {
        iassert(!rg.inFrame);

        rg.inFrame = 1;
        rg.lodParms.valid = 0;
        for ( n = 0; n < 4; ++n )
            rg.correctedLodParms[n].valid = 0;
        if ( Sys_IsMainThread() )
            R_UpdateFrontEndDvarOptions();
        if ( rgp.world && rgp.needSortMaterials )
        {
            rgp.needSortMaterials = 0;
            rgp.needMaterialPreload = 1;
            if ( Sys_IsMainThread() )
                R_SyncRenderThread();
            Sys_WaitRenderer();
            Material_Sort();
            R_SortWorldSurfaces();
            dx.d3d9->GetAdapterIdentifier(dx.adapterIndex, 0, &id);
            v0 = strstr(id.Description, "NVIDIA");
            if ( v0 )
                Dvar_SetBool((dvar_s *)r_warm_dpvs, 1);
        }
        v2 = r_skinCache->current.enabled && useFastFile->current.enabled;
        gfxBuf.skinCache = v2;
        v1 = v2 && r_fastSkin->current.enabled;
        gfxBuf.fastSkin = v1;
        if ( gfxBuf.skinCache )
            R_LockSkinnedCache();
    }
}

float s_debugShaderConsts[5][4] =
{
  { 0.0, 0.0, 0.0, 0.0 },
  { 0.0, 0.0, 0.0, 1.0 },
  { 1.0, 0.0, 0.0, 0.0 },
  { 0.0, 1.0, 0.0, 0.0 },
  { 0.0, 0.0, 1.0, 0.0 }
};

void R_UpdateFrontEndDvarOptions()
{
    bool v2; // [esp+0h] [ebp-10h]
    GfxImage *image; // [esp+4h] [ebp-Ch]

    PROF_SCOPED("R_UpdateFrontEndDvarOptions"); // LWSS ADD

    if (R_LightTweaksModified())
        R_UpdateLightsFromDvars();
    if (r_sun_from_dvars->current.enabled && rgp.world)
        R_SetSunFromDvars(&rgp.world->sun);
    if (R_GpuSyncModified())
        R_UpdateGpuSyncType();
    R_SetTestLods();
    rg.hasAnyImageOverrides = R_AreAnyImageOverridesActive();
    if (R_CheckDvarModified(r_showMissingLightGrid))
    {
        R_SyncRenderThread();
        R_ResetModelLighting();
    }
    if (r_fullbright->modified || r_debugShader->modified || r_lightConflicts->modified)
    {
        Dvar_ClearModified(r_fullbright);
        Dvar_ClearModified(r_debugShader);
        Dvar_ClearModified(r_lightConflicts);
        R_SyncRenderThread();
        R_InitDrawMethod();
    }
    if (R_CheckDvarModified(r_outdoorFeather))
        R_SetOutdoorFeatherConst();
    R_SetInputCodeConstantFromVec4(&gfxCmdBufInput, 0x36u, (float *)s_debugShaderConsts[r_debugShader->current.integer]);
    if (R_CheckDvarModified(r_envMapOverride)
        || R_CheckDvarModified(r_envMapMinIntensity)
        || R_CheckDvarModified(r_envMapMaxIntensity)
        || R_CheckDvarModified(r_envMapExponent)
        || R_CheckDvarModified(r_envMapSunIntensity))
    {
        R_EnvMapOverrideConstants();
    }
    v2 = r_distortion->current.enabled && CL_LocalClient_GetActiveCount() == 1;
    if (rg.distortion != v2)
        R_SyncRenderThread();
    rg.distortion = v2;

    R_SetInputCodeImage(&gfxCmdBufInput, TEXTURE_SRC_CODE_RESOLVED_POST_SUN, gfxRenderTargets[R_RENDERTARGET_RESOLVED_POST_SUN].image);

    rg.drawWorld = r_drawWorld->current.enabled;
    rg.drawBModels = r_drawBModels->current.enabled;
    rg.drawSModels = r_drawSModels->current.enabled;
    rg.drawXModels = r_drawXModels->current.enabled;
}

bool __cdecl R_LightTweaksModified()
{
    char v1; // bl
    char v2; // bl
    char v3; // bl
    char v4; // bl
    char v5; // bl
    char v6; // bl

    if ( !rgp.world )
        return 0;
    if ( !r_lightTweakAmbient
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    2516,
                    0,
                    "%s",
                    "r_lightTweakAmbient") )
    {
        __debugbreak();
    }
    if ( !r_lightTweakDiffuseFraction
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    2517,
                    0,
                    "%s",
                    "r_lightTweakDiffuseFraction") )
    {
        __debugbreak();
    }
    if ( !r_lightTweakSunLight
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    2518,
                    0,
                    "%s",
                    "r_lightTweakSunLight") )
    {
        __debugbreak();
    }
    if ( !r_lightTweakAmbientColor
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    2519,
                    0,
                    "%s",
                    "r_lightTweakAmbientColor") )
    {
        __debugbreak();
    }
    if ( !r_lightTweakSunColor
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    2520,
                    0,
                    "%s",
                    "r_lightTweakSunColor") )
    {
        __debugbreak();
    }
    if ( !r_lightTweakSunDiffuseColor
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    2521,
                    0,
                    "%s",
                    "r_lightTweakSunDiffuseColor") )
    {
        __debugbreak();
    }
    if ( !r_lightTweakSunDirection
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    2522,
                    0,
                    "%s",
                    "r_lightTweakSunDirection") )
    {
        __debugbreak();
    }
    v1 = R_CheckDvarModified(r_lightTweakAmbient);
    v2 = R_CheckDvarModified(r_lightTweakDiffuseFraction) | v1;
    v3 = R_CheckDvarModified(r_lightTweakSunLight) | v2;
    v4 = R_CheckDvarModified(r_lightTweakAmbientColor) | v3;
    v5 = R_CheckDvarModified(r_lightTweakSunColor) | v4;
    v6 = R_CheckDvarModified(r_lightTweakSunDiffuseColor) | v5;

    return R_CheckDvarModified((const dvar_s *)r_lightTweakSunDirection) | v6;
}

bool __cdecl R_GpuSyncModified()
{
    char v0; // bl

    if ( !r_gpuSync
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp", 2541, 0, "%s", "r_gpuSync") )
    {
        __debugbreak();
    }
    if ( !r_multiGpu
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp", 2542, 0, "%s", "r_multiGpu") )
    {
        __debugbreak();
    }
    v0 = R_CheckDvarModified(r_gpuSync);
    return R_CheckDvarModified(r_multiGpu) | v0;
}

void R_SetTestLods()
{
    float dist; // [esp+4h] [ebp-8h]
    signed int i; // [esp+8h] [ebp-4h]

    if ( r_forceLod->current.integer == r_forceLod->reset.integer )
    {
        XModelSetTestLods(0, r_highLodDist->current.value);
        XModelSetTestLods(1u, r_mediumLodDist->current.value);
        XModelSetTestLods(2u, r_lowLodDist->current.value);
        XModelSetTestLods(3u, r_lowestLodDist->current.value);
    }
    else
    {
        for ( i = 0; i < 4; ++i )
        {
            if ( i == r_forceLod->current.integer )
                dist = 0.0f;
            else
                dist = 0.001f;
            XModelSetTestLods(i, dist);
        }
    }
}

bool __cdecl R_AreAnyImageOverridesActive()
{
    if ( r_colorMap->current.integer != 1 )
        return 1;
    if ( r_normalMap->current.integer == 1 )
        return r_specularRoughnessMap->current.integer != 0;
    return 1;
}

void R_SetOutdoorFeatherConst()
{
    R_SetInputCodeConstant(
        &gfxCmdBufInput,
        0x43u,
        r_outdoorFeather->current.value,
        r_outdoorFeather->current.value,
        r_outdoorFeather->current.value,
        r_outdoorFeather->current.value);
}

void R_EnvMapOverrideConstants()
{
    if ( r_envMapMaxIntensity->current.value <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    2595,
                    0,
                    "%s",
                    "r_envMapMaxIntensity->current.value > 0.0f") )
    {
        __debugbreak();
    }
    if ( r_envMapOverride->current.enabled )
        R_SetInputCodeConstant(
            &gfxCmdBufInput,
            0x45u,
            r_envMapMinIntensity->current.value,
            r_envMapMaxIntensity->current.value,
            r_envMapExponent->current.value,
            r_envMapSunIntensity->current.value);
}

void __cdecl R_EndFrame()
{
    if ( rg.registered )
    {
        CL_UpdateDebugClientData();
        CL_FlushDebugClientData();
        R_TransferDebugGlobals(&frontEndDataOut->debugGlobals);
        R_AddCmdEndOfList();
        R_ClearCmdList();
        rg.viewInfoCount = 0;
        g_frameIndex = (g_frameIndex + 1) % 2;
        if ( !rg.inFrame
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp", 2795, 0, "%s", "rg.inFrame") )
        {
            __debugbreak();
        }
        rg.inFrame = 0;
    }
    else if ( rg.inFrame
                 && !Assert_MyHandler(
                             "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                             2768,
                             0,
                             "%s",
                             "!rg.inFrame") )
    {
        __debugbreak();
    }
}

void __cdecl R_AddCmdClearScreen(int whichToClear, const float *color, float depth, unsigned __int8 stencil)
{
    GfxCmdClearScreen *cmd; // [esp+Ch] [ebp-4h]

    if ( !whichToClear
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp", 2919, 0, "%s", "whichToClear") )
    {
        __debugbreak();
    }
    if ( (whichToClear & 0xFFFFFFF8) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    2920,
                    0,
                    "%s\n\t(whichToClear) = %i",
                    "((whichToClear & ~(0x00000001l | 0x00000002l | 0x00000004l)) == 0)",
                    whichToClear) )
    {
        __debugbreak();
    }
    if ( !color
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp", 2921, 0, "%s", "color") )
    {
        __debugbreak();
    }
    if ( (depth < 0.0 || depth > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    2922,
                    0,
                    "%s\n\t(depth) = %g",
                    "(depth >= 0.0f && depth <= 1.0f)",
                    depth) )
    {
        __debugbreak();
    }
    cmd = (GfxCmdClearScreen *)R_GetCommandBuffer(RC_CLEAR_SCREEN, R_CmdBytes(sizeof(GfxCmdClearScreen)));
    if ( !cmd && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp", 2926, 0, "%s", "cmd") )
        __debugbreak();
    cmd->whichToClear = whichToClear;
    if ( cmd->whichToClear != whichToClear
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    2929,
                    1,
                    "%s",
                    "cmd->whichToClear == whichToClear") )
    {
        __debugbreak();
    }
    cmd->stencil = stencil;
    cmd->depth = depth;
    cmd->color[0] = *color;
    cmd->color[1] = color[1];
    cmd->color[2] = color[2];
    cmd->color[3] = color[3];
}

GfxCmdDrawQuadList2D *__cdecl R_AddCmdDrawQuadList2D(Material *materialHandle, int quadCount)
{
    Material *defaultMaterial; // [esp+0h] [ebp-10h]
    GfxCmdDrawQuadList2D *cmd; // [esp+Ch] [ebp-4h]

    if ( materialHandle )
        defaultMaterial = (Material *)Material_FromHandle(materialHandle);
    else
        defaultMaterial = rgp.defaultMaterial;
    if ( !defaultMaterial
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp", 3161, 0, "%s", "material") )
    {
        __debugbreak();
    }
    cmd = (GfxCmdDrawQuadList2D *)R_GetCommandBuffer(
                RC_DRAW_QUADLIST_2D,
                R_CmdBytes(sizeof(GfxCmdDrawQuadList2D) + 4 * sizeof(GfxQuadVertex) * quadCount));
    if ( !cmd )
        return 0;
    cmd->material = defaultMaterial;
    cmd->quadCount = quadCount;
    return cmd + 1;
}

void __cdecl R_AddCmdDrawEmblemLayer(
                Material *materialHandle,
                const GfxImage *image,
                int colorIdx,
                float outlineSize,
                float smoothSize,
                GfxQuadVertex *verts)
{
    Material *defaultMaterial; // [esp+8h] [ebp-10h]
    GfxCmdDrawEmblemLayer *cmd; // [esp+14h] [ebp-4h]

    if ( materialHandle )
        defaultMaterial = (Material *)Material_FromHandle(materialHandle);
    else
        defaultMaterial = rgp.defaultMaterial;
    if ( !defaultMaterial
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp", 3185, 0, "%s", "material") )
    {
        __debugbreak();
    }
    cmd = (GfxCmdDrawEmblemLayer *)R_GetCommandBuffer(RC_DRAW_EMBLEM_LAYER, R_CmdBytes(sizeof(GfxCmdDrawEmblemLayer)));
    if ( cmd )
    {
        cmd->material = defaultMaterial;
        cmd->image = image;
        cmd->colorIdx = colorIdx;
        cmd->smoothSize = smoothSize;
        cmd->outlineSize = outlineSize;
        memcpy(cmd->verts, verts, sizeof(cmd->verts));
    }
}

void __cdecl R_AddCmdProjectionSet2D()
{
    R_AddCmdProjectionSet(GFX_PROJECTION_2D);
}

void __cdecl R_AddCmdProjectionSet(GfxProjectionTypes projection)
{
    GfxCmdProjectionSet *cmd; // [esp+0h] [ebp-4h]

    cmd = (GfxCmdProjectionSet *)R_GetCommandBuffer(RC_PROJECTION_SET, R_CmdBytes(sizeof(GfxCmdProjectionSet)));
    if ( cmd )
        cmd->projection = projection;
}

void __cdecl R_AddCmdDrawFramed(
                float x,
                float y,
                float w,
                float h,
                float thicknessW,
                float thicknessH,
                float thicknessTex,
                int sides,
                const float *color,
                Material *material)
{
    Material *defaultMaterial; // [esp+0h] [ebp-8h]
    GfxCmdDrawFramed2D *cmd; // [esp+4h] [ebp-4h]

    cmd = (GfxCmdDrawFramed2D *)R_GetCommandBuffer(RC_DRAW_FRAMED, R_CmdBytes(sizeof(GfxCmdDrawFramed2D)));
    if ( cmd )
    {
        if ( material )
            defaultMaterial = (Material *)Material_FromHandle(material);
        else
            defaultMaterial = rgp.defaultMaterial;
        cmd->material = defaultMaterial;
        R_ConvertColorToBytes(color, (unsigned __int8 *)&cmd->color);
        cmd->x = x;
        cmd->y = y;
        cmd->w = w;
        cmd->h = h;
        cmd->thicknessW = thicknessW;
        cmd->thicknessH = thicknessH;
        cmd->thicknessTex = thicknessTex;
        cmd->sides = sides;
    }
}

void __cdecl R_BeginRemoteScreenUpdate()
{
    if ( useFastFile->current.enabled && Sys_IsMainThread() && sys_smp_allowed->current.enabled )
    {
        if ( r_glob.remoteScreenUpdateNesting < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                        3329,
                        0,
                        "%s",
                        "r_glob.remoteScreenUpdateNesting >= 0") )
        {
            __debugbreak();
        }
        if ( r_glob.startedRenderThread && (!CL_IsLocalClientInGame(0) || r_glob.remoteScreenUpdateInGame) )
        {
            if ( r_glob.remoteScreenUpdateNesting )
            {
                ++r_glob.remoteScreenUpdateNesting;
            }
            else
            {
                if ( r_glob.screenUpdateNotify )
                {
                    if ( !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                                    3351,
                                    0,
                                    "%s",
                                    "!r_glob.screenUpdateNotify") )
                        __debugbreak();
                }
                ++r_glob.remoteScreenUpdateNesting;
                //BLOPS_NULLSUB();
                Sys_NotifyRenderer();
            }
        }
    }
}

void __cdecl R_EndRemoteScreenUpdate(void (__cdecl *pumpfunc)())
{

    if ( useFastFile->current.enabled && Sys_IsMainThread() && sys_smp_allowed->current.enabled )
    {
        if ( r_glob.remoteScreenUpdateNesting < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                        3372,
                        0,
                        "%s",
                        "r_glob.remoteScreenUpdateNesting >= 0") )
        {
            __debugbreak();
        }
        if ( r_glob.startedRenderThread )
        {
            if ( !CL_IsLocalClientInGame(0) || r_glob.remoteScreenUpdateInGame )
            {
                if ( r_glob.remoteScreenUpdateNesting <= 0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                                3387,
                                0,
                                "%s",
                                "r_glob.remoteScreenUpdateNesting > 0") )
                {
                    __debugbreak();
                }
                if ( r_glob.remoteScreenUpdateNesting == 1 )
                {
                    while ( !r_glob.screenUpdateNotify )
                    {
                        NET_Sleep(1u);
                        if ( pumpfunc )
                            pumpfunc();
                        Sys_WaitRenderer();
                    }
                    r_glob.screenUpdateNotify = 0;
                    if ( r_glob.remoteScreenUpdateNesting <= 0
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                                    3414,
                                    0,
                                    "%s",
                                    "r_glob.remoteScreenUpdateNesting > 0") )
                    {
                        __debugbreak();
                    }
                    --r_glob.remoteScreenUpdateNesting;
                    while ( !r_glob.screenUpdateNotify )
                    {
                        ++g_mainThreadBlocked;
                        NET_Sleep(1u);
                        if ( pumpfunc )
                            pumpfunc();
                        Sys_WaitRenderer();
                        --g_mainThreadBlocked;
                    }
                    r_glob.screenUpdateNotify = 0;
                }
                else
                {
                    --r_glob.remoteScreenUpdateNesting;
                }
            }
            else if ( r_glob.remoteScreenUpdateNesting
                         && !Assert_MyHandler(
                                     "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                                     3383,
                                     0,
                                     "%s",
                                     "r_glob.remoteScreenUpdateNesting == 0") )
            {
                __debugbreak();
            }
        }
        else if ( r_glob.remoteScreenUpdateNesting )
        {
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                            3377,
                            0,
                            "%s",
                            "r_glob.remoteScreenUpdateNesting == 0") )
                __debugbreak();
        }
    }
}

void __cdecl R_PushRemoteScreenUpdate(int remoteScreenUpdateNesting)
{
    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    3439,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    if ( remoteScreenUpdateNesting < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    3440,
                    0,
                    "%s\n\t(remoteScreenUpdateNesting) = %i",
                    "(remoteScreenUpdateNesting >= 0)",
                    remoteScreenUpdateNesting) )
    {
        __debugbreak();
    }
    while ( remoteScreenUpdateNesting )
    {
        R_BeginRemoteScreenUpdate();
        --remoteScreenUpdateNesting;
    }
}

int __cdecl R_PopRemoteScreenUpdate()
{
    volatile int remoteScreenUpdateNesting; // [esp+0h] [ebp-4h]

    if ( !Sys_IsMainThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    3453,
                    0,
                    "%s",
                    "Sys_IsMainThread()") )
    {
        __debugbreak();
    }
    remoteScreenUpdateNesting = r_glob.remoteScreenUpdateNesting;
    while ( r_glob.remoteScreenUpdateNesting )
        R_EndRemoteScreenUpdate(0);
    if ( remoteScreenUpdateNesting < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    3460,
                    0,
                    "%s\n\t(remoteScreenUpdateNesting) = %i",
                    "(remoteScreenUpdateNesting >= 0)",
                    remoteScreenUpdateNesting) )
    {
        __debugbreak();
    }
    return remoteScreenUpdateNesting;
}

bool __cdecl R_IsInRemoteScreenUpdate()
{
    if ( !Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_rendercmds.cpp",
                    3468,
                    0,
                    "%s",
                    "Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    return r_glob.isRenderingRemoteUpdate;
}

bool __cdecl R_IsRemoteScreenUpdateActive()
{
    return r_glob.remoteScreenUpdateNesting > 0;
}

//GfxViewInfo *__thiscall GfxViewInfo::GfxViewInfo(GfxViewInfo *this)
//{
//    int v3; // [esp+4h] [ebp-14h]
//    GfxDrawSurfListInfo *j; // [esp+8h] [ebp-10h]
//    int v5; // [esp+Ch] [ebp-Ch]
//    PointLightPartition *i; // [esp+10h] [ebp-8h]
//
//    v5 = 4;
//    for ( i = this->pointLightPartitions; --v5 >= 0; ++i )
//        `vector constructor iterator'(i->info.group, 8u, 3, (void *(__thiscall *)(void *))jqBatchGroup::jqBatchGroup);
//    v3 = 14;
//    for ( j = this->drawList; --v3 >= 0; ++j )
//        `vector constructor iterator'(j->group, 8u, 3, (void *(__thiscall *)(void *))jqBatchGroup::jqBatchGroup);
//    return this;
//}

