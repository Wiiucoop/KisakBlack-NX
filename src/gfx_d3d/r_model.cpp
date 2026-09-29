#include "r_model.h"
#include <xanim/xmodel.h>
#include <universal/com_memory.h>
#include <qcommon/common.h>
#include "r_staticmodel.h"
#include <DynEntity/DynEntity_load_obj.h>
#include <xanim/xmodel_utils.h>
#include "r_dvars.h"
#include <xanim/dobj_utils.h>
#include <cgame_mp/cg_pose_mp.h>
#include "r_debug.h"
#include "r_buffers.h"
#include <qcommon/threads.h>
#include "r_rendercmds.h"
#include <win32/win_net.h>
#include "r_model_lod.h"
#include <EffectsCore/fx_beam.h>

#include <algorithm>
#include "r_warn.h"
#include "r_model_pose.h"
#include <cgame/cg_pose_utils.h>
#include "r_dobj_skin.h"
#include "r_xsurface_optimize.h"

const int boxVerts[24][3] =
{
  { 0, 0, 0 },
  { 1, 0, 0 },
  { 0, 0, 0 },
  { 0, 1, 0 },
  { 1, 1, 0 },
  { 1, 0, 0 },
  { 1, 1, 0 },
  { 0, 1, 0 },
  { 0, 0, 1 },
  { 1, 0, 1 },
  { 0, 0, 1 },
  { 0, 1, 1 },
  { 1, 1, 1 },
  { 1, 0, 1 },
  { 1, 1, 1 },
  { 0, 1, 1 },
  { 0, 0, 0 },
  { 0, 0, 1 },
  { 1, 0, 0 },
  { 1, 0, 1 },
  { 0, 1, 0 },
  { 0, 1, 1 },
  { 1, 1, 0 },
  { 1, 1, 1 }
};



void __cdecl R_ModelList_f()
{
    const char *Name; // eax
    const char *v1; // [esp+8h] [ebp-212Ch]
    const char *fmt; // [esp+Ch] [ebp-2128h]
    int inData; // [esp+104h] [ebp-2030h] BYREF
    XModel *v4[2050]; // [esp+108h] [ebp-202Ch] BYREF
    XModel *model; // [esp+2110h] [ebp-24h]
    int v6; // [esp+2114h] [ebp-20h]
    int v7; // [esp+2118h] [ebp-1Ch]
    int XModelUsageCount; // [esp+211Ch] [ebp-18h]
    int MemoryUsage; // [esp+2120h] [ebp-14h]
    int MemUsage; // [esp+2124h] [ebp-10h]
    int i; // [esp+2128h] [ebp-Ch]
    float v12; // [esp+212Ch] [ebp-8h]
    int modelCount; // [esp+2130h] [ebp-4h] BYREF

    v7 = 0;
    v6 = 0;
    inData = 0;
    DB_EnumXAssets(ASSET_TYPE_XMODEL, (void (__cdecl *)(XAssetHeader, void *))R_GetModelList, &inData, 1);
    //std::_Sort<XModel * *,int,bool (__cdecl *)(XModel * &,XModel * &)>(v4, &v4[inData], (4 * inData) >> 2, R_ModelSort);

    std::sort(v4, v4 + inData, R_ModelSort);

    Com_Printf(8, "---------------------------\n");
    Com_Printf(8, "SM# is the number of static model instances\n");
    Com_Printf(8, "instKB is static model instance usage\n");
    Com_Printf(8, "DE# is the number of dyn entity instances\n");
    Com_Printf(8, "     SM#    instKB     DE#     geoKB    name\n");
    for ( i = 0; i < inData; ++i )
    {
        model = v4[i];
        MemUsage = XModelGetMemUsage(model);
        v7 += MemUsage;
        MemoryUsage = R_StaticModelGetMemoryUsage(model, &modelCount);
        if ( MemoryUsage )
        {
            v6 += MemoryUsage;
            v12 = (float)MemoryUsage / 1024.0;
            Com_Printf(8, "    %4i    ", modelCount);
            if ( v12 >= 10.0 )
                fmt = "%6.0f";
            else
                fmt = "%6.1f";
            Com_Printf(8, fmt, v12);
        }
        else
        {
            Com_Printf(8, "                            ");
        }
        XModelUsageCount = DynEnt_GetXModelUsageCount(model);
        if ( XModelUsageCount )
            Com_Printf(8, "    %4i    ", XModelUsageCount);
        else
            Com_Printf(8, "                ");
        v12 = (float)MemUsage / 1024.0;
        if ( v12 >= 10.0 )
            v1 = "%6.0f";
        else
            v1 = "%6.1f";
        Com_Printf(8, v1, v12);
        Name = XModelGetName(model);
        Com_Printf(8, "    %s\n", Name);
    }
    Com_Printf(8, "---------------------------\n");
    Com_Printf(8, "current inst total    %4.1f MB\n", (float)((float)v6 / 1048576.0));
    Com_Printf(8, "current geo total     %4.1f MB\n", (float)((float)v7 / 1048576.0));
    Com_Printf(8, "current total             %4.1f MB\n", (float)((float)(v7 + v6) / 1048576.0));
    Com_Printf(8, "Related commands: meminfo, imagelist, gfx_world, gfx_model, cg_drawfps, com_statmon, tempmeminfo\n");
}

bool __cdecl R_ModelSort(const XModel *model1, const XModel *model2)
{
    return XModelGetMemUsage(model1) < XModelGetMemUsage(model2);
}

void __cdecl R_GetModelList(XAssetHeader header, XAssetHeader *data)
{
    if ( data->xmodelPieces >= (XModelPieces *)0x800
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model.cpp",
                    96,
                    0,
                    "%s",
                    "modelList->count < ARRAY_COUNT( modelList->sorted )") )
    {
        __debugbreak();
    }
    data[(int)data->xmodelPieces++ + 1] = header;
}

XModel *__cdecl R_RegisterModel(char *name)
{
    return XModelPrecache(
                     name,
                     (void *(__cdecl *)(int))Hunk_AllocXModelPrecache,
                     (void *(__cdecl *)(int))Hunk_AllocXModelPrecacheColl);
}

unsigned __int8 *__cdecl Hunk_AllocXModelPrecache(unsigned int size)
{
    return Hunk_Alloc(size, "Hunk_AllocXModelPrecache", 22);
}

unsigned __int8 *__cdecl Hunk_AllocXModelPrecacheColl(unsigned int size)
{
    return Hunk_Alloc(size, "Hunk_AllocXModelPrecacheColl", 28);
}

void __cdecl R_FinishLoadingModels()
{
    R_OptimizeAllModels();
}

void __cdecl R_XModelDebug(const DObj *obj, int *partBits)
{
    if ( r_xdebug->current.integer == 1 )
        R_XModelDebugBoxes(obj, partBits);
    if ( r_xdebug->current.integer == 2 )
        R_XModelDebugAxes(obj, partBits);
}

float cutOffLimit;
void __cdecl R_XModelDebugBoxes(const DObj *obj, int *partBits)
{
    DObjAnimMat *boneMatrix; // [esp+D8h] [ebp-2D0h]
    unsigned int boxEdge; // [esp+DCh] [ebp-2CCh]
    XBoneInfo *boneInfoArray[160]; // [esp+E0h] [ebp-2C8h] BYREF
    int boneIndex; // [esp+360h] [ebp-48h]
    int boneCount; // [esp+364h] [ebp-44h]
    float start[3]; // [esp+368h] [ebp-40h] BYREF
    float end[3]; // [esp+374h] [ebp-34h] BYREF
    const int (*tempBoxVerts)[3]; // [esp+380h] [ebp-28h]
    float org[3]; // [esp+384h] [ebp-24h] BYREF
    XBoneInfo *boneInfo; // [esp+390h] [ebp-18h]
    float color[4]; // [esp+394h] [ebp-14h] BYREF
    unsigned int animPartBit; // [esp+3A4h] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model.cpp", 226, 0, "%s", "obj") )
        __debugbreak();
    boneMatrix = DObjGetRotTransArray(obj);
    if ( boneMatrix )
    {
        boneCount = DObjNumBones(obj);
        iassert(boneCount <= DOBJ_MAX_PARTS);
        DObjGetBoneInfo(obj, boneInfoArray);
        color[0] = 1.0f;
        color[1] = 1.0f;
        color[2] = 1.0f;
        color[3] = 0.0f;
        animPartBit = 0x80000000;
        boneIndex = 0;
        while ( boneIndex < boneCount )
        {
            if ( (animPartBit & partBits[boneIndex >> 5]) != 0 )
            {
                boneInfo = boneInfoArray[boneIndex];
                tempBoxVerts = boxVerts;
                if ( boneInfo->radiusSquared >= 0.001 && (float)(cutOffLimit * cutOffLimit) <= boneInfo->radiusSquared )
                {
                    for ( boxEdge = 0; boxEdge < 0xC; ++boxEdge )
                    {
                        org[0] = boneInfo->bounds[(*tempBoxVerts)[0]][0];
                        org[1] = boneInfo->bounds[(*tempBoxVerts)[1]][1];
                        org[2] = boneInfo->bounds[(*tempBoxVerts)[2]][2];
                        MatrixTransformVectorQuatTrans(org, &boneMatrix[boneIndex], start);
                        org[0] = boneInfo->bounds[(*++tempBoxVerts)[0]][0];
                        org[1] = boneInfo->bounds[(*tempBoxVerts)[1]][1];
                        org[2] = boneInfo->bounds[(*tempBoxVerts)[2]][2];
                        MatrixTransformVectorQuatTrans(org, &boneMatrix[boneIndex], end);
                        ++tempBoxVerts;
                        R_AddDebugLine(&frontEndDataOut->debugGlobals, start, end, color, 0);
                    }
                }
            }
            ++boneIndex;
            animPartBit = (animPartBit << 31) | (animPartBit >> 1);
        }
    }
}

void __cdecl R_XModelDebugAxes(const DObj *obj, int *partBits)
{
    DObjAnimMat *boneMatrix; // [esp+D8h] [ebp-60h]
    int boneIndex; // [esp+DCh] [ebp-5Ch]
    float translation[3][3]; // [esp+E0h] [ebp-58h] BYREF
    int boneCount; // [esp+104h] [ebp-34h]
    float start[3]; // [esp+108h] [ebp-30h] BYREF
    float end[3]; // [esp+114h] [ebp-24h] BYREF
    int axis; // [esp+120h] [ebp-18h]
    float color[4]; // [esp+124h] [ebp-14h] BYREF
    unsigned int animPartBit; // [esp+134h] [ebp-4h]

    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model.cpp", 358, 0, "%s", "obj") )
        __debugbreak();
    boneMatrix = DObjGetRotTransArray(obj);
    if ( boneMatrix )
    {
        //*(_QWORD *)&translation[0][0] = __PAIR64__(0, LODWORD(6.0f));
        translation[0][0] = 6.0f;
        translation[0][1] = 0.0f;
        translation[0][2] = 0.0f;
        //*(_QWORD *)&translation[1][0] = __PAIR64__(LODWORD(6.0f), 0);
        translation[1][0] = 0.0f;
        translation[1][1] = 6.0f;
        translation[1][2] = 0.0f;
        translation[2][0] = 0.0f;
        //*(_QWORD *)&translation[2][1] = __PAIR64__(LODWORD(6.0f), 0);
        translation[2][1] = 0.0f;
        translation[2][2] = 6.0f;
        boneCount = DObjNumBones(obj);
        animPartBit = 0x80000000;
        for ( boneIndex = 0; boneIndex < boneCount; ++boneIndex )
        {
            if ( (animPartBit & partBits[boneIndex >> 5]) != 0 )
            {
                for ( axis = 0; axis < 3; ++axis )
                {
                    memset(color, 0, sizeof(color));
                    color[axis] = 1.0f;
                    MatrixTransformVectorQuatTrans(vec3_origin, &boneMatrix[boneIndex], start);
                    MatrixTransformVectorQuatTrans(translation[axis], &boneMatrix[boneIndex], end);
                    R_AddDebugLine(&frontEndDataOut->debugGlobals, start, end, color, 0);
                }
            }
            animPartBit = (animPartBit << 31) | (animPartBit >> 1);
        }
    }
}

void __cdecl R_LockSkinnedCache()
{
    PROF_SCOPED("R_LockSkinnedCache"); // LWSS ADD

    iassert(!gfxBuf.skinnedCacheLockAddr);

    if (Sys_QueryD3DDeviceOKEvent())
    {
        {
            PROF_SCOPED("SkinCache Frame Loop"); // LWSS ADD
            while (*frontEndDataOut->dynamicBufferCurrentFrame
                && *frontEndDataOut->dynamicBufferCurrentFrame < frontEndDataOut->frameCount)
            {
                NET_Sleep(1u);
            }
        }

        *frontEndDataOut->dynamicBufferCurrentFrame = frontEndDataOut->frameCount;
        gfxBuf.skinnedCacheLockAddr = frontEndDataOut->skinnedCacheVb->verts;
        gfxBuf.oldSkinnedCacheNormalsAddr = gfxBuf.skinnedCacheNormals[gfxBuf.skinnedCacheNormalsFrameCount & 1];
        ++gfxBuf.skinnedCacheNormalsFrameCount;
        gfxBuf.skinnedCacheNormalsAddr = gfxBuf.skinnedCacheNormals[gfxBuf.skinnedCacheNormalsFrameCount & 1];
    }
}

void __cdecl R_UnlockSkinnedCache()
{
    if ( gfxBuf.skinnedCacheLockAddr )
        gfxBuf.skinnedCacheLockAddr = 0;
}

int    R_SkinXModel(
                XModelDrawInfo *modelInfo,
                const XModel *model,
                const DObj *obj,
                const GfxPlacement *placement,
                float scale,
                __int16 gfxEntIndex,
                bool offscreen)
{
    //void *v8; // esp
    unsigned __int32 v10; // [esp+1Ch] [ebp-1C7Ch]
    int v11; // [esp+28h] [ebp-1C70h]
    unsigned int j; // [esp+34h] [ebp-1C64h]
    int v14; // [esp+38h] [ebp-1C60h]
    XSurface *v15; // [esp+3Ch] [ebp-1C5Ch]
    int i; // [esp+44h] [ebp-1C54h]
    unsigned __int8 *v17; // [esp+48h] [ebp-1C50h]
    alignas(8) unsigned __int8 v18[128 * sizeof(GfxModelRigidSurface) + 16];   // nx-port: was [7180], 128 x86 rigid surfaces // [esp+4Ch] [ebp-1C4Ch] BYREF
    unsigned int v19[5]; // [esp+1C58h] [ebp-40h] BYREF
    int surfaceCount; // [esp+1C6Ch] [ebp-2Ch]
    XSurface *surfaces; // [esp+1C70h] [ebp-28h] BYREF
    int LodForDist; // [esp+1C74h] [ebp-24h]
    float adjustedDist; // [esp+1C78h] [ebp-20h]
    XModelLodRampType LodRampType; // [esp+1C7Ch] [ebp-1Ch]
    float dist; // [esp+1C80h] [ebp-18h]
    float BaseLodDist; // [esp+1C84h] [ebp-14h]
    //int v27; // [esp+1C8Ch] [ebp-Ch]
    //void *v28; // [esp+1C90h] [ebp-8h]
    //void *retaddr; // [esp+1C98h] [ebp+0h]

    //v27 = a1;
    //v28 = retaddr;
    //v8 = alloca(7288);
    iassert(model);

    if ( !useFastFile->current.enabled && XModelBad(model) )
        return 0;

    BaseLodDist = R_GetBaseLodDist(placement->origin);
    dist = I_fres(scale) * BaseLodDist;
    LodRampType = XModelGetLodRampType(model);
    adjustedDist = R_GetAdjustedLodDist(dist, LodRampType);
    if ( offscreen )
        adjustedDist = adjustedDist * r_offscreenCasterLodScale->current.value;
    LodForDist = XModelGetLodForDist(model, adjustedDist, dist, 0);
    if ( LodForDist < 0 )
        return 0;
    surfaceCount = XModelGetSurfaces(model, &surfaces, LodForDist);
    if ( !surfaceCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model.cpp", 544, 0, "%s", "surfaceCount") )
    {
        __debugbreak();
    }
    if ( obj )
        DObjGetHidePartBits(obj, v19);
    v17 = v18;
    for ( i = 0; i < surfaceCount; ++i )
    {
        v15 = &surfaces[i];
        if ( !obj )
            goto LABEL_42;
        if ( !r_useHidePartbits->current.enabled )
            goto LABEL_42;
        v14 = 0;
        for ( j = 0; j < 5; ++j )
            v14 |= v15->partBits[j] & v19[j];
        if ( v14 )
        {
            *(unsigned int *)v17 = -3;
            v17 += 4;
        }
        else
        {
LABEL_42:
            if ( (v15->flags & 0x80) == 0 && useFastFile->current.enabled )
                v11 = -2;
            else
                v11 = -1;
            // nx-port: was written at x86 offsets with a 56-byte stride
            GfxModelRigidSurface *rigidSurf = (GfxModelRigidSurface *)v17;
            rigidSurf->surf.skinnedCachedOffset = v11;
            rigidSurf->surf.xsurf = v15;
            rigidSurf->surf.info.gfxEntIndex = gfxEntIndex;
            rigidSurf->surf.info.lightingHandle = 0;
            rigidSurf->placement.base = *placement;
            rigidSurf->placement.scale = scale;
            v17 += sizeof(GfxModelRigidSurface);
        }
    }
    v10 = _InterlockedExchangeAdd(&frontEndDataOut->surfPos, v17 - v18);
    if ( v17 - v18 + v10 <= 0x40000 )
    {
        if ( (v10 & 3) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model.cpp",
                        619,
                        0,
                        "%s",
                        "!(startSurfPos & 3)") )
        {
            __debugbreak();
        }
        modelInfo->surfId = v10 >> 2;
        memcpy(&frontEndDataOut->surfsBuffer[v10], v18, v17 - v18);
        modelInfo->lod = LodForDist;
        return 1;
    }
    else
    {
        R_WarnOncePerFrame(R_WARN_MAX_SCENE_SURFS_SIZE);
        return 0;
    }
}

int __cdecl R_SkinAndBoundSceneEnt(GfxSceneEntity *sceneEnt)
{
    DObjAnimMat *boneMatrix; // [esp+0h] [ebp-Ch]
    const DObj *obj; // [esp+4h] [ebp-8h] BYREF
    GfxSceneEntity *localSceneEnt; // [esp+8h] [ebp-4h] BYREF
    int savedregs; // [esp+Ch] [ebp+0h] BYREF

    boneMatrix = R_UpdateSceneEntBounds(sceneEnt, &localSceneEnt, &obj, 1);
    if ( boneMatrix )
    {
        if ( !localSceneEnt
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model.cpp", 691, 0, "%s", "localSceneEnt") )
        {
            __debugbreak();
        }
        CG_CullIn(localSceneEnt->info.pose);
        R_SkinSceneDObj(sceneEnt, localSceneEnt, obj, boneMatrix, 0);
        return 1;
    }
    else
    {
        if ( localSceneEnt )
            CG_UsedDObjCalcPose(localSceneEnt->info.pose);
        return 0;
    }
}

void __cdecl R_ShutdownModels()
{
    R_ReleaseAllModels();
}

void __cdecl R_ReleaseAllModels()
{
    DB_EnumXAssets(ASSET_TYPE_XMODEL, (void (__cdecl *)(XAssetHeader, void *))R_ReleaseModel, 0, 1);
}

void __cdecl R_ReleaseModel(XAssetHeader header)
{
    if ( !XModelBad(header.model) )
        XModelUnoptimize(header.model);
}

void __cdecl R_OptimizeAllModels()
{
    if ( r_loadForRenderer->current.enabled )
        DB_EnumXAssets(ASSET_TYPE_XMODEL, (void (__cdecl *)(XAssetHeader, void *))R_OptimizeModel, 0, 1);
}

void __cdecl R_OptimizeModel(XAssetHeader header)
{
    if ( !XModelBad(header.model) )
        XModelOptimize(header.model);
}

void __cdecl XSurfaceReleaseResources(XSurface *surf)
{
    if ( !surf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model.cpp", 832, 0, "%s", "surf") )
        __debugbreak();
    if ( surf->vb0 )
    {
        R_FreeStaticVertexBuffer(surf->vb0);
        surf->vb0 = 0;
    }
    if ( surf->indexBuffer )
    {
        R_FreeStaticIndexBuffer(surf->indexBuffer);
        surf->indexBuffer = 0;
    }
}

void __cdecl XSurfaceRenderString(const float *pos, char *string)
{
    float color[4]; // [esp+8h] [ebp-10h] BYREF

    color[0] = 1.0f;
    color[1] = 1.0f;
    color[2] = 1.0f;
    color[3] = 1.0f;
    R_AddDebugString(&frontEndDataOut->debugGlobals, pos, color, 0.5, string);
}

