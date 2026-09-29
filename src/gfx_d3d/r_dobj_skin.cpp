#include "r_dobj_skin.h"
#include <universal/q_shared.h>
#include <xanim/dobj_utils.h>
#include <xanim/xmodel_utils.h>
#include "r_dvars.h"
#include "r_warn.h"
#include <cgame_mp/cg_ents_mp.h>
#include <universal/com_memory.h>
#include "r_model.h"
#include <universal/com_workercmds.h>
#include <EffectsCore/fx_marks.h>
#include "r_model_pose.h"
#include "r_buffers.h"
#include "r_rendercmds.h"

r_SkinConditions g_skinErrorFlags;

void __cdecl R_FlagXModelAsSkinned(GfxSceneEntity *sceneEnt, unsigned int surfaceCount)
{
    if ( sceneEnt->cull.state != 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dobj_skin.cpp",
                    954,
                    0,
                    "%s\n\t(sceneEnt->cull.state) = %i",
                    "(sceneEnt->cull.state == CULL_STATE_SKINNED_PENDING)",
                    sceneEnt->cull.state) )
    {
        __debugbreak();
    }
    sceneEnt->cull.state = surfaceCount + 4;
}

void __cdecl R_SkinSceneDObj(
                GfxSceneEntity *sceneEnt,
                GfxSceneEntity *localSceneEnt,
                const DObj *obj,
                DObjAnimMat *boneMatrix,
                int waitForCullState)
{
    volatile unsigned int state; // [esp+0h] [ebp-8h]
    unsigned int surfaceCount; // [esp+4h] [ebp-4h]

    if ( !localSceneEnt
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dobj_skin.cpp", 1008, 0, "%s", "localSceneEnt") )
    {
        __debugbreak();
    }
    if ( !boneMatrix
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dobj_skin.cpp", 1009, 0, "%s", "boneMatrix") )
    {
        __debugbreak();
    }
    if ( !obj && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dobj_skin.cpp", 1010, 0, "%s", "obj") )
        __debugbreak();
    if ( localSceneEnt->cull.state < 4 )
    {
        if ( _InterlockedCompareExchange((volatile unsigned __int32 *)&sceneEnt->cull, 3, 2) == 2 )
        {
            surfaceCount = R_SkinSceneDObjModels(localSceneEnt, obj, boneMatrix);
            R_FlagXModelAsSkinned(localSceneEnt, surfaceCount);
        }
        else if ( waitForCullState )
        {
            do
            {
                state = sceneEnt->cull.state;
                if ( state < 3
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dobj_skin.cpp",
                                1043,
                                0,
                                "%s\n\t(state) = %i",
                                "(state >= CULL_STATE_SKINNED_PENDING)",
                                state) )
                {
                    __debugbreak();
                }
            }
            while ( state == 3 );
            if ( state < 4
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dobj_skin.cpp",
                            1046,
                            0,
                            "%s\n\t(state) = %i",
                            "(state >= CULL_STATE)",
                            state) )
            {
                __debugbreak();
            }
        }
    }
}

unsigned int    R_SkinSceneDObjModels(
    GfxSceneEntity *sceneEnt,
    const DObj *obj,
    DObjAnimMat *boneMatrix)
{
    int v1; // ebp
    void *v2; // esp
    const char *Name; // eax
    unsigned __int8 *modelSurfs; // [esp-1D30h] [ebp-1D3Ch]
    signed __int32 startSurfPos; // [esp-1D2Ch] [ebp-1D38h]
    signed __int32 firstSurf; // [esp-1D24h] [ebp-1D30h]
    unsigned int vertsSize; // [esp-1D1Ch] [ebp-1D28h]
    int size; // [esp-1D18h] [ebp-1D24h]
    GfxModelSkinnedSurface *skinnedSurf; // [esp-1D10h] [ebp-1D1Ch]
    GfxModelSkinnedSurface *surfPos2__; // [esp-1D10h] [ebp-1D1Ch]
    unsigned int offset; // [esp-1D0Ch] [ebp-1D18h]
    unsigned int offset_1; // [esp-1D0Ch] [ebp-1D18h]
    GfxModelSkinnedSurface *surfPos2_; // [esp-1D08h] [ebp-1D14h]
    GfxModelSkinnedSurface *surfPos2; // [esp-1D08h] [ebp-1D14h]
    GfxSkinCacheEntry *SkinCacheEntry; // [esp-1D04h] [ebp-1D10h]
    unsigned int skinnedCachedOffset; // [esp-1D00h] [ebp-1D0Ch]
    int skinnedCachedOffset_; // [esp-1CFCh] [ebp-1D08h]
    int surfBufSize; // [esp-1CF0h] [ebp-1CFCh]
    unsigned int totalSurfSize; // [esp-1CF0h] [ebp-1CFCh]
    int ii; // [esp-1CECh] [ebp-1CF8h]
    unsigned int n; // [esp-1CE8h] [ebp-1CF4h]
    int partsBitsVisCheck; // [esp-1CE4h] [ebp-1CF0h]
    unsigned int partsBitCheck[5]; // [esp-1CE0h] [ebp-1CECh] BYREF
    int m; // [esp-1CCCh] [ebp-1CD8h]
    XSurface *surface; // [esp-1CC8h] [ebp-1CD4h]
    unsigned int k; // [esp-1CC4h] [ebp-1CD0h]
    int boneIndex_rem32; // [esp-1CBCh] [ebp-1CC8h]
    int boneIndex_mod32; // [esp-1CB8h] [ebp-1CC4h]
    unsigned int boneIndex_div32; // [esp-1CB4h] [ebp-1CC0h]
    unsigned int surfaceCount; // [esp-1CB0h] [ebp-1CBCh]
    XSurface *surfaces; // [esp-1CACh] [ebp-1CB8h] BYREF
    GfxModelSurfaceInfo targBoneIndexHigh; // [esp-1CA8h] [ebp-1CB4h] BYREF
    int cullLod; // [esp-1C9Ch] [ebp-1CA8h]
    const XModel *model; // [esp-1C98h] [ebp-1CA4h]
    int modelNumBones; // [esp-1C94h] [ebp-1CA0h]
    unsigned int lod; // [esp-1C90h] [ebp-1C9Ch]
    unsigned int v40[5]; // [esp-1C8Ch] [ebp-1C98h] BYREF
    SkinXModelCmd cmd; // [esp-1C78h] [ebp-1C84h] BYREF
    int j; // [esp-1C48h] [ebp-1C54h]
    unsigned int surfPartBits[9]; // [esp-1C44h] [ebp-1C50h]
    int i; // [esp-1C20h] [ebp-1C2Ch]
    unsigned int totalSurfaceCount; // [esp-1C1Ch] [ebp-1C28h]
    unsigned int NumModels; // [esp-1C18h] [ebp-1C24h]
    unsigned int boneIndex; // [esp-1C14h] [ebp-1C20h]
    unsigned int numSkinnedVerts; // [esp-1C10h] [ebp-1C1Ch] BYREF
    GfxModelSkinnedSurface *surfPos; // [esp-1C0Ch] [ebp-1C18h]
    alignas(8) unsigned __int8 surfsBuffer[7488 / 24 * sizeof(GfxModelSkinnedSurface)];   // nx-port: was 7488, 312 x86 skinned surfaces // [esp-1C08h] [ebp-1C14h] BYREF
    //int v51; // [esp+8h] [ebp-4h]
    //int retaddr; // [esp+Ch] [ebp+0h]

    //v51 = v1;
    //retaddr = pad[1];
    //v2 = alloca(7488);
    iassert(!sceneEnt->cull.skinnedSurfs.firstSurf);
    iassert(obj);
    iassert(boneMatrix);
    iassert(!IsObjFileLoad() || !DObjBad(obj));

    surfPos = (GfxModelSkinnedSurface *)surfsBuffer;
    numSkinnedVerts = 0;
    boneIndex = 0;
    NumModels = DObjGetNumModels(obj);
    if (!NumModels)
        return 0;
    totalSurfaceCount = 0;
    for (i = 0; i < 4; ++i)
        surfPartBits[i] = 0;
    for (j = 0; j < 5; ++j)
        cmd.surfacePartBits[j] = 0;
    DObjGetHidePartBits(obj, v40);
    cmd.viewoffset[0] = scene.def.viewOffset[0];
    cmd.viewoffset[1] = scene.def.viewOffset[1];
    cmd.viewoffset[2] = scene.def.viewOffset[2];
    lod = 0;
    while (lod < NumModels)
    {
        model = DObjGetModel(obj, lod);
        iassert(model);
        modelNumBones = XModelNumBones(model);
        cullLod = sceneEnt->cull.lods[lod];
        if (cullLod >= 0)
        {
            targBoneIndexHigh.boneIndex = boneIndex;
            targBoneIndexHigh.baseMat = XModelGetBasePose(model);
            targBoneIndexHigh.boneCount = modelNumBones;
            targBoneIndexHigh.gfxEntIndex = sceneEnt->gfxEntIndex;
            targBoneIndexHigh.lightingHandle = 0;
            surfaceCount = XModelGetSurfaces(model, &surfaces, cullLod);

            iassert(surfaces);
            iassert(surfaceCount);

            totalSurfaceCount += surfaceCount;
            targBoneIndexHigh.dobjModelIndex = lod;
            boneIndex_div32 = boneIndex >> 5;
            boneIndex_mod32 = boneIndex & 0x1F;
            boneIndex_rem32 = 32 - boneIndex_mod32;

            for (k = 0; k < surfaceCount; ++k)
            {
                surface = &surfaces[k];

                for (m = 0; m < 5; ++m)
                    surfPartBits[m + 4] = surface->partBits[m];

                if (boneIndex_mod32)
                {
                    partsBitCheck[0] = surfPartBits[4 - boneIndex_div32] >> boneIndex_mod32;
                    partsBitCheck[1] = (surfPartBits[5 - boneIndex_div32] >> boneIndex_mod32)
                        | (surfPartBits[4 - boneIndex_div32] << boneIndex_rem32);
                    partsBitCheck[2] = (surfPartBits[6 - boneIndex_div32] >> boneIndex_mod32)
                        | (surfPartBits[5 - boneIndex_div32] << boneIndex_rem32);
                    partsBitCheck[3] = (surfPartBits[7 - boneIndex_div32] >> boneIndex_mod32)
                        | (surfPartBits[6 - boneIndex_div32] << boneIndex_rem32);
                    partsBitCheck[4] = (surfPartBits[8 - boneIndex_div32] >> boneIndex_mod32)
                        | (surfPartBits[7 - boneIndex_div32] << boneIndex_rem32);
                }
                else
                {
                    partsBitCheck[0] = surfPartBits[4 - boneIndex_div32];
                    partsBitCheck[1] = surfPartBits[5 - boneIndex_div32];
                    partsBitCheck[2] = surfPartBits[6 - boneIndex_div32];
                    partsBitCheck[3] = surfPartBits[7 - boneIndex_div32];
                    partsBitCheck[4] = surfPartBits[8 - boneIndex_div32];
                }
                if (DObjIsRecorded(obj))
                    DObjSkelSetSkel(obj, (int *)partsBitCheck);
                if (!r_useHidePartbits->current.enabled)
                    goto NO_PARTS_BITS_CHECK;
                partsBitsVisCheck = 0;
                for (n = 0; n < 5; ++n)
                    partsBitsVisCheck |= partsBitCheck[n] & v40[n];
                if (partsBitsVisCheck)
                {
                    surfPos->skinnedCachedOffset = -3;
                    surfPos = (GfxModelSkinnedSurface *)((char *)surfPos + 4);
                }
                else
                {
                NO_PARTS_BITS_CHECK:
                    for (ii = 0; ii < 5; ++ii)
                        cmd.surfacePartBits[ii] |= partsBitCheck[ii];
                    surfBufSize = R_PreSkinXSurface(
                        obj,
                        surface,
                        &targBoneIndexHigh,
                        &numSkinnedVerts,
                        (GfxModelRigidSurface*)surfPos);
                    surfPos->xsurf = &surfaces[k];
                    surfPos->info = targBoneIndexHigh;
                    iassert(surfBufSize);
                    surfPos = (GfxModelSkinnedSurface *)((char *)surfPos + surfBufSize);
                }
            }
        }
        ++lod;
        boneIndex += modelNumBones;
    }

    iassert(DObjSkelAreBonesUpToDate(obj, cmd.surfacePartBits));

    //if (!DObjSkelAreBonesUpToDate(obj, cmd.surfacePartBits)
    //    && !Assert_MyHandler(
    //        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dobj_skin.cpp",
    //        554,
    //        0,
    //        "%s",
    //        "DObjSkelAreBonesUpToDate( obj, skinCmd.surfacePartBits )"))
    //{
    //    __debugbreak();
    //}

    if (numSkinnedVerts)
    {
        if (gfxBuf.skinCache)
        {
            skinnedCachedOffset_ = R_AllocSkinnedCachedVerts(
                numSkinnedVerts,
                (frontEndDataOut->gfxEnts[sceneEnt->gfxEntIndex].renderFxFlags & 1) != 0);
            if (skinnedCachedOffset_ < 0)
            {
                R_WarnOncePerFrame(R_WARN_MAX_SKINNED_CACHE_VERTICES);
                return 0;
            }
            skinnedCachedOffset = 0x80000001;
            if (gfxBuf.fastSkin)
            {
                SkinCacheEntry = CG_GetSkinCacheEntry(sceneEnt->info.pose);
                if (SkinCacheEntry->ageCount < 3u
                    && gfxBuf.skinnedCacheNormalsFrameCount - SkinCacheEntry->frameCount == 1
                    && SkinCacheEntry->numSkinnedVerts == numSkinnedVerts)
                {
                    skinnedCachedOffset = SkinCacheEntry->skinnedCachedOffset;
                    ++SkinCacheEntry->ageCount;
                }
                else
                {
                    SkinCacheEntry->ageCount = 0;
                }
                SkinCacheEntry->frameCount = gfxBuf.skinnedCacheNormalsFrameCount;
                SkinCacheEntry->numSkinnedVerts = numSkinnedVerts;
                if (SkinCacheEntry->numSkinnedVerts != numSkinnedVerts
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dobj_skin.cpp",
                        625,
                        0,
                        "%s",
                        "skinCacheEntry->numSkinnedVerts == numSkinnedVerts"))
                {
                    __debugbreak();
                }
                SkinCacheEntry->skinnedCachedOffset = skinnedCachedOffset_;
            }
            surfPos2_ = (GfxModelSkinnedSurface *)surfsBuffer;
            for (offset = 0; offset < totalSurfaceCount; ++offset)
            {
                skinnedSurf = surfPos2_;
                if (surfPos2_->skinnedCachedOffset == -2)
                {
                    surfPos2_ = (GfxModelSkinnedSurface *)((char *)surfPos2_ + sizeof(GfxModelRigidSurface))   /* nx-port: was 56 */;
                }
                else if (surfPos2_->skinnedCachedOffset == -3)
                {
                    surfPos2_ = (GfxModelSkinnedSurface *)((char *)surfPos2_ + 4);
                }
                else
                {
                    ++surfPos2_;
                    size = 32 * skinnedSurf->skinnedCachedOffset;
                    skinnedSurf->skinnedCachedOffset = size + skinnedCachedOffset_;
                    skinnedSurf->oldSkinnedCachedOffset = size + skinnedCachedOffset;
                }
            }
            if (surfPos != surfPos2_
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dobj_skin.cpp",
                    679,
                    0,
                    "%s",
                    "surfPos == (byte *)surfPos2"))
            {
                __debugbreak();
            }
        }
        else
        {
            vertsSize = 32 * numSkinnedVerts;
            if (!frontEndDataOut->tempSkinBuf
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dobj_skin.cpp",
                    692,
                    0,
                    "%s",
                    "frontEndDataOut->tempSkinBuf"))
            {
                __debugbreak();
            }
            firstSurf = _InterlockedExchangeAdd(&frontEndDataOut->tempSkinPos, vertsSize);
            if (vertsSize + firstSurf > 0x800000)
            {
                R_WarnOncePerFrame(R_WARN_TEMP_SKIN_BUF_SIZE);
                return 0;
            }
            Z_VirtualCommit((char*)&frontEndDataOut->tempSkinBuf[firstSurf], vertsSize, 22);
            surfPos2 = (GfxModelSkinnedSurface *)surfsBuffer;
            for (offset_1 = 0; offset_1 < totalSurfaceCount; ++offset_1)
            {
                surfPos2__ = surfPos2;
                if (surfPos2->skinnedCachedOffset == -2)
                {
                    surfPos2 = (GfxModelSkinnedSurface *)((char *)surfPos2 + sizeof(GfxModelRigidSurface))   /* nx-port: was 56 */;
                }
                else if (surfPos2->skinnedCachedOffset == -3)
                {
                    surfPos2 = (GfxModelSkinnedSurface *)((char *)surfPos2 + 4);
                }
                else
                {
                    ++surfPos2;
                    surfPos2__->skinnedVert = (GfxPackedVertex *)&frontEndDataOut->tempSkinBuf[32 * surfPos2__->skinnedCachedOffset
                        + firstSurf];   // nx-port: was (int) through oldSkinnedCachedOffset
                    surfPos2__->skinnedCachedOffset = -1;
                }
            }
            if (surfPos != surfPos2
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dobj_skin.cpp",
                    753,
                    0,
                    "%s",
                    "surfPos == (byte *)surfPos2"))
            {
                __debugbreak();
            }
        }
    }
    totalSurfSize = (char *)surfPos - (char *)surfsBuffer;
    startSurfPos = _InterlockedExchangeAdd(&frontEndDataOut->surfPos, (char *)surfPos - (char *)surfsBuffer);
    if (totalSurfSize + startSurfPos <= 0x40000)
    {
        if ((startSurfPos & 3) != 0
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dobj_skin.cpp",
                854,
                0,
                "%s",
                "!(startSurfPos & 3)"))
        {
            __debugbreak();
        }
        modelSurfs = &frontEndDataOut->surfsBuffer[startSurfPos];
        sceneEnt->cull.skinnedSurfs.firstSurf = &frontEndDataOut->surfsBuffer[startSurfPos];
        memcpy(modelSurfs, surfsBuffer, totalSurfSize);
        if (!totalSurfaceCount
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dobj_skin.cpp",
                883,
                0,
                "%s",
                "totalSurfaceCount"))
        {
            __debugbreak();
        }
        if (r_xdebug->current.integer)
            R_XModelDebug(obj, cmd.surfacePartBits);
        if (numSkinnedVerts)
        {
            cmd.modelSurfs = modelSurfs;
            cmd.surfCount = totalSurfaceCount;
            cmd.mat = boneMatrix;
            Sys_AddWorkerCmdInternal(&r_model_skinWorkerCmd, (unsigned char*)&cmd, 0);
        }
        return totalSurfaceCount;
    }
    else
    {
        Name = DObjGetName(obj);
        Com_PrintWarning(
            1,
            "FAILED to allocate surfs on frame %u: DObj %s needs %i surfs\n",
            rg.frontEndFrameCount,
            Name,
            totalSurfSize);
        R_WarnOncePerFrame(R_WARN_MAX_SCENE_SURFS_SIZE);
        return 0;
    }
}

int __cdecl R_AllocSkinnedCachedVerts(int vertCount, bool highPriority)
{
    volatile int offset; // [esp+4h] [ebp-Ch]
    unsigned int size; // [esp+8h] [ebp-8h]
    unsigned int MAX_SKINNED_CACHE_VERTICES; // [esp+Ch] [ebp-4h]

    if ( vertCount <= 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dobj_skin.cpp", 54, 0, "%s", "vertCount > 0") )
    {
        __debugbreak();
    }
    if ( !r_drawSkinnedModels->current.enabled )
        return -1;

    if ( !gfxBuf.skinnedCacheLockAddr )
        return -1;

    g_skinErrorFlags.allbits = 0;
    size = 32 * vertCount;
    MAX_SKINNED_CACHE_VERTICES = highPriority ? 0x40000 : 257024;
    while ( 1 )
    {
        offset = frontEndDataOut->skinnedCacheVb->used;
        if ( size + offset > 32 * MAX_SKINNED_CACHE_VERTICES )
            break;
        if ( _InterlockedCompareExchange(&frontEndDataOut->skinnedCacheVb->used, size + offset, offset) == offset )
            goto LABEL_13;
    }
    offset = 32 * MAX_SKINNED_CACHE_VERTICES;
LABEL_13:
    if ( size + offset > 0x6AACC0 )
        g_skinErrorFlags.allbits = (g_skinErrorFlags.allbits | 2);
    if ( size + offset > 0x7D8000 )
        g_skinErrorFlags.allbits = (g_skinErrorFlags.allbits | 1);
    if ( size + offset > 32 * MAX_SKINNED_CACHE_VERTICES )
        return -1;
    else
        return offset;
}

// local variable allocation has failed, the output may be wrong!
int    R_PreSkinXSurface(
                const DObj *obj,
                XSurface *surf,
                const GfxModelSurfaceInfo *surfaceInfo,
                unsigned int *numSkinnedVerts,
                GfxModelRigidSurface *surfPos)
{
    float origin_[3]; // [esp-8h] [ebp-D8h] OVERLAPPED BYREF
    float origin[4]; // [esp+4h] [ebp-CCh] BYREF
    DObjSkelMat skelMat; // [esp+14h] [ebp-BCh] BYREF
    const DObjAnimMat *mat_1; // [esp+60h] [ebp-70h]
    DObjSkelMat invBaseMat; // [esp+64h] [ebp-6Ch] OVERLAPPED BYREF
    const DObjAnimMat *mat; // [esp+ACh] [ebp-24h]
    int offset; // [esp+B0h] [ebp-20h]
    DObjAnimMat *RotTransArray; // [esp+B4h] [ebp-1Ch]
    GfxModelRigidSurface *rigidSurf; // [esp+B8h] [ebp-18h]
    unsigned int boneOffset; // [esp+BCh] [ebp-14h]
    const DObjAnimMat *boneMatrix; // [esp+C0h] [ebp-10h]
    //GfxModelRigidSurface *rigidSurf; // [esp+C4h] [ebp-Ch]
    //void *v19; // [esp+C8h] [ebp-8h]
    //void *retaddr; // [esp+D0h] [ebp+0h]

    //rigidSurf = a1;
    //v19 = retaddr;

    iassert(obj);
    iassert(surf);
    iassert(surfaceInfo);

    if ( (surf->flags & 0x80) == 0 && IsFastFileLoad() && surf->vertListCount == 1 )
    {
        surf->vertList = surf->vertList;
        rigidSurf = surfPos;
        surfPos->surf.skinnedCachedOffset = -2;

        iassert(&rigidSurf->surf == reinterpret_cast<GfxModelSkinnedSurface *>(surfPos));

        rigidSurf->placement.scale = 1.0f;
        RotTransArray = &DObjGetRotTransArray(obj)[surfaceInfo->boneIndex];
        offset = surf->vertList->boneOffset >> 6;
        mat = &surfaceInfo->baseMat[offset];
        ConvertQuatToInverseSkelMat(mat, &invBaseMat);
        mat_1 = &RotTransArray[offset];
        ConvertQuatToSkelMat(mat_1, &skelMat);
        QuatMultiplyInverse(mat->quat, mat_1->quat, origin);
        Vec4Normalize(origin);
        Vec4Copy(origin, rigidSurf->placement.base.quat);
        R_TransformSkelMat(invBaseMat.origin, (const float4 *)&skelMat, origin_);
        Vec3Copy(origin_, rigidSurf->placement.base.origin);
        return sizeof(GfxModelRigidSurface); // 56
    }
    else
    {
        surfPos->surf.skinnedCachedOffset = *numSkinnedVerts;
        *numSkinnedVerts += surf->vertCount;
        return sizeof(GfxModelSkinnedSurface); // 24
    }
}

void __cdecl R_TransformSkelMat(const float *in1, const float4 *in2, float *out)
{
    out[0] = (float)((float)((float)(*in1 * in2->v[0]) + (float)(in1[1] * in2[1].v[0])) + (float)(in1[2] * in2[2].v[0])) + in2[3].v[0];
    out[1] = (float)((float)((float)(*in1 * in2->v[1]) + (float)(in1[1] * in2[1].v[1])) + (float)(in1[2] * in2[2].v[1])) + in2[3].v[1];
    out[2] = (float)((float)((float)(*in1 * in2->v[2]) + (float)(in1[1] * in2[1].v[2])) + (float)(in1[2] * in2[2].v[2])) + in2[3].v[2];
}

void __cdecl R_SkinGfxEntityCmd(GfxSceneEntity **data)
{
    DObjAnimMat *boneMatrix; // [esp+0h] [ebp-14h]
    const DObj *obj; // [esp+4h] [ebp-10h] BYREF
    GfxSceneEntity *localSceneEnt; // [esp+8h] [ebp-Ch] BYREF
    GfxSceneEntity *sceneEnt; // [esp+Ch] [ebp-8h]
    GfxSceneEntity **pSceneEnt; // [esp+10h] [ebp-4h]
    int savedregs; // [esp+14h] [ebp+0h] BYREF

    if ( !data && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dobj_skin.cpp", 1092, 0, "%s", "data") )
        __debugbreak();
    pSceneEnt = data;
    sceneEnt = *data;
    boneMatrix = R_UpdateSceneEntBounds(sceneEnt, &localSceneEnt, &obj, 0);
    if ( boneMatrix )
    {
        if ( !localSceneEnt
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_dobj_skin.cpp",
                        1107,
                        0,
                        "%s",
                        "localSceneEnt") )
        {
            __debugbreak();
        }
        R_SkinSceneDObj(sceneEnt, localSceneEnt, obj, boneMatrix, 0);
    }
}

