#include "glass_client.h"
#include "glass_renderer.h"
#include <universal/com_memory.h>

#include <new>
#include <cgame/cg_sound.h>
#include <EffectsCore/fx_marks.h>
#include <cgame/cg_scr_main.h>
#include <EffectsCore/fx_system.h>
#include "glass_load_obj.h"
#include <client/splitscreen.h>
#include <bgame/bg_misc.h>

GlassesClient *clGlasses;

StackAllocator GlassesClient::allocator;

void __cdecl GlassesClient::InitAllocator(Glasses *glasses)
{
    unsigned int workMemorySize; // [esp+0h] [ebp-8h]

    workMemorySize = glasses->workMemorySize;
    GlassesClient::allocator.memory = glasses->workMemory;
    GlassesClient::allocator.size = workMemorySize;
    GlassesClient::allocator.pos = 0;
}

unsigned int *__cdecl GlassesClient::Allocate(int size, const char *file, int line)
{
    const char *v3; // eax

    v3 = va("Glass work memory %d %s(%d)", size, file, line);
    return Z_Malloc(size, v3, 9);
}

void __cdecl GlassesClient::Free(char *ptr)
{
    Z_Free(ptr, 9);
}

unsigned int __cdecl GlassesClient::GetFreeMem()
{
    return GlassesClient::allocator.size - GlassesClient::allocator.pos;
}

void *GlassesClient::operator new(size_t size)
{
    return GlassesClient::Allocate(size, "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_client.cpp", 69);
}

void GlassesClient::operator delete(void *ptr)
{
    GlassesClient::Free((char *)ptr);
}


GlassesClient::GlassesClient(const Glasses *glss)
{
    GlassRenderer *v3; // [esp+0h] [ebp-10h]
    GlassRenderer *v5; // [esp+8h] [ebp-8h]
    unsigned int i; // [esp+Ch] [ebp-4h]

    this->numGlasses = glss->numGlasses;
    this->glasses = (GlassClient *)GlassesClient::Allocate(
                                                                     sizeof(GlassClient) * this->numGlasses,   // nx-port: was 12 *
                                                                     "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_client.cpp",
                                                                     80);

    this->renderer = new GlassRenderer(glss); // Calls custom allocator in class
    this->lastPreShatter = 0;
    for (i = 0; i < this->numGlasses; ++i)
    {
        //GlassClient::Init(&this->glasses[i], &glss->glasses[i]);
        this->glasses[i].Init(&glss->glasses[i]);
    }
}

GlassesClient::~GlassesClient()
{
    GlassesClient::Free((char *)this->glasses);
    if (this->renderer)
    {
        //GlassRenderer::~GlassRenderer(renderer);
        //GlassesClient::operator delete((char *)renderer);
        delete this->renderer;
    }
}

const GlassClient *GlassesClient::GetGlass(int idx)
{
    return &this->glasses[idx];
}

void __thiscall GlassesClient::Reset()
{
    unsigned int i; // [esp+4h] [ebp-4h]

    for (i = 0; i < this->numGlasses; ++i)
    {
        //GlassClient::Reset(&this->glasses[i]);
        this->glasses[i].Reset();
    }
}

void __thiscall GlassesClient::Update(int localClientNum)
{
    unsigned int i; // [esp+Ch] [ebp-8h]

    PROF_SCOPED("GlassesClient.Update");
    for (i = 0; i < this->numGlasses; ++i)
    {
        //GlassClient::Update(&this->glasses[i], localClientNum);
        this->glasses[i].Update(localClientNum);
    }
}

void __thiscall GlassesClient::ParseSnapshot(int localClientNum, msg_t *msg, bool gameState)
{
    GlassState::State newState; // [esp+4h] [ebp-50h]
    float v6[3]; // [esp+8h] [ebp-4Ch] BYREF
    float v7[3]; // [esp+14h] [ebp-40h] BYREF
    unsigned int index; // [esp+20h] [ebp-34h]
    unsigned int j; // [esp+24h] [ebp-30h]
    unsigned int numChanges; // [esp+28h] [ebp-2Ch]
    unsigned int state; // [esp+2Ch] [ebp-28h]
    float hitDir[3]; // [esp+30h] [ebp-24h] BYREF
    float hitPos[3]; // [esp+3Ch] [ebp-18h] BYREF
    unsigned int i; // [esp+48h] [ebp-Ch]
    unsigned int num; // [esp+4Ch] [ebp-8h]
    unsigned int check; // [esp+50h] [ebp-4h]

    // LWSS HACK
    //check = MSG_ReadShort(msg);
    //if (check != 13337)
    //{
    //    iassert(0);
    //}

    if ( MSG_ReadBit(msg) == 1 )
    {
        if ( MSG_ReadBit(msg) == 1 )
        {
            num = MSG_ReadShort(msg);
            if ( num != this->numGlasses )
            {
                Com_PrintError(
                    14,
                    "GlassesClient::ParseSnapshot - number of glasses mismatch: %d != %d\n",
                    num,
                    this->numGlasses);
                MSG_Discard(msg);
                return;
            }
            for ( i = 0; i < this->numGlasses; ++i )
            {
                state = MSG_ReadBits(msg, 2u);
                if ( state == 2 )
                {
                    hitPos[0] = MSG_ReadFloat(msg);
                    hitPos[1] = MSG_ReadFloat(msg);
                    hitPos[2] = MSG_ReadFloat(msg);
                    hitDir[0] = MSG_ReadFloat(msg);
                    hitDir[1] = MSG_ReadFloat(msg);
                    hitDir[2] = MSG_ReadFloat(msg);
                }
                //GlassClient::SetState(&this->glasses[i], localClientNum, (GlassState::State)state, hitPos, hitDir, gameState);
                this->glasses[i].SetState(localClientNum, (GlassState::State)state, hitPos, hitDir, gameState);
            }
        }
        else
        {
            numChanges = MSG_ReadShort(msg);
            for ( j = 0; j < numChanges; ++j )
            {
                index = MSG_ReadBits(msg, 0xAu);
                if ( index >= this->numGlasses )
                {
                    Com_PrintError(
                        14,
                        "GlassesClient::ParseSnapshot - glass index out of range: %d .= %d\n",
                        index,
                        this->numGlasses);
                    MSG_Discard(msg);
                    return;
                }
                newState = (GlassState::State)MSG_ReadBits(msg, 2u);
                if ( newState == GlassState::State::SHATTERED )
                {
                    v7[0] = MSG_ReadFloat(msg);
                    v7[1] = MSG_ReadFloat(msg);
                    v7[2] = MSG_ReadFloat(msg);
                    v6[0] = MSG_ReadFloat(msg);
                    v6[1] = MSG_ReadFloat(msg);
                    v6[2] = MSG_ReadFloat(msg);
                }
                //GlassClient::SetState(&this->glasses[index], localClientNum, newState, v7, v6, gameState);
                this->glasses[index].SetState(localClientNum, newState, v7, v6, gameState);
            }
        }
    }
    check = MSG_ReadShort(msg);
    if ( check != 30154 )
    {
        Com_PrintError(14, "GlassesClient::ParseSnapshot - unexpected end of message: %x\n", check);
        MSG_Discard(msg);
    }
}

void __thiscall GlassesClient::WriteDemoSnapshot(msg_t *msg)
{
    GlassClient *glass; // [esp+10h] [ebp-Ch]
    unsigned int i; // [esp+14h] [ebp-8h]

    PROF_SCOPED("GlassesClient.WriteDemoSnapshot");
    MSG_WriteBit1(msg);
    MSG_WriteBit1(msg);
    MSG_WriteShort(msg, this->numGlasses);
    for ( i = 0; i < this->numGlasses; ++i )
    {
        glass = &this->glasses[i];
        if ( (glass->state.val.i & 0xF) == 3
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_client.cpp",
                        203,
                        0,
                        "%s",
                        "glass->state.GetState() != GlassState::INVALID") )
        {
            __debugbreak();
        }
        MSG_WriteBits(msg, glass->state.val.i & 0xF, 2u);
        if ( (glass->state.val.i & 0xF) == 2 )
        {
            MSG_WriteFloat(msg, 0.0);
            MSG_WriteFloat(msg, 0.0);
            MSG_WriteFloat(msg, 0.0);
            MSG_WriteFloat(msg, 0.0);
            MSG_WriteFloat(msg, 0.0);
            MSG_WriteFloat(msg, 0.0);
        }
    }
    MSG_WriteShort(msg, 30154);
}

void __thiscall GlassesClient::PreShatterNext()
{
    GlassClient *gls; // [esp+14h] [ebp-Ch]
    unsigned int i; // [esp+18h] [ebp-8h]

    PROF_SCOPED("GlassesClient.PreShatterNext");

    for ( i = 0; i < this->numGlasses; ++i )
    {
        gls = &this->glasses[this->lastPreShatter++];
        if ( this->lastPreShatter >= this->numGlasses )
            this->lastPreShatter = 0;
        //if ( GlassClient::PreShatter(gls) )
        if ( gls->PreShatter() )
        {
            return;
        }
    }
}

void __thiscall GlassClient::Init(const Glass *gls)
{
    const float *absmax; // [esp+8h] [ebp-10h]
    const float *absmin; // [esp+Ch] [ebp-Ch]
    GfxBrushModel *bmodel; // [esp+14h] [ebp-4h]

    this->state.val.i = this->state.val.i & 0xFFFFFFF0 | 3;
    this->glass = gls;
    this->outlines = 0;
    bmodel = R_GetBrushModel(this->glass->brushModel);
    if ( !bmodel
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_client.cpp", 273, 0, "%s", "bmodel") )
    {
        __debugbreak();
    }
    bmodel->writable.mip1radiusSq = 1000000.0f;
    absmin = this->glass->absmin;
    bmodel->writable.mins[0] = *absmin;
    bmodel->writable.mins[1] = absmin[1];
    bmodel->writable.mins[2] = absmin[2];
    absmax = this->glass->absmax;
    bmodel->writable.maxs[0] = *absmax;
    bmodel->writable.maxs[1] = absmax[1];
    bmodel->writable.maxs[2] = absmax[2];
}

void __thiscall GlassClient::Reset()
{
    if ( this->outlines )
    {
        //GlassRenderer::FreeShardMemory(clGlasses->renderer, &this->outlines->numOutlines
        clGlasses->renderer->FreeShardMemory((unsigned int*)this->outlines);
        this->outlines = 0;
    }
}

void __thiscall GlassClient::SetState(
                int localClientNum,
                GlassState::State newState,
                const float *hitPos,
                float *hitDir,
                bool gameState)
{
    char *autoShatterShound; // [esp+8h] [ebp-24h]

    if ( newState != (this->state.val.i & 0xF) )
    {
        this->state.val.i = newState & 0xF | this->state.val.i & 0xFFFFFFF0;
        if ( newState )
        {
            if ( newState == GlassState::State::CRACKED )
            {
                //GlassRenderer::RemoveGlassShards(clGlasses->renderer, this->glass->index);
                clGlasses->renderer->RemoveGlassShards(this->glass->index);
                GlassClient::SetBrushMaterial(GlassState::State::CRACKED);
                if (!gameState)
                {
                    CG_PlaySound(localClientNum, (float*)this->glass->origin, 0, 0, 1.0, (char *)this->glass->glassDef->crackSound);
                }
            }
            else if ( newState == GlassState::State::SHATTERED )
            {
                //GlassRenderer::AddShatterAction(clGlasses->renderer, this, hitPos, hitDir, gameState);
                clGlasses->renderer->AddShatterAction(this, hitPos, hitDir, gameState);
                CScr_GlassSmash((float*)this->glass->origin, hitDir);
                if ( !gameState )
                {
                    if ( (float)((float)((float)(*hitDir * *hitDir) + (float)(hitDir[1] * hitDir[1]))
                                         + (float)(hitDir[2] * hitDir[2])) == 0.0 )
                        autoShatterShound = (char *)this->glass->glassDef->autoShatterShound;
                    else
                        autoShatterShound = (char *)this->glass->glassDef->shatterShound;
                    CG_PlaySound(localClientNum, (float*)this->glass->origin, 0, 0, 1.0, autoShatterShound);
                    if ( this->glass->glassDef->shatterEffect )
                        GlassClient::PlayShatterFX(localClientNum, hitPos, hitDir);
                }
                FX_MarkGlassDetachAll(localClientNum, this->glass->index);
            }
            else
            {
                if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_client.cpp", 328, 0, va("wrong glass state: %d", newState)) )
                    __debugbreak();
            }
        }
        else
        {
            //GlassRenderer::RemoveGlassShards(clGlasses->renderer, this->glass->index);
            clGlasses->renderer->RemoveGlassShards(this->glass->index);
            GlassClient::SetBrushMaterial(GlassState::State::PRISTINE);
        }
    }
}

void __thiscall GlassClient::PlayShatterFX(
                int localClientNum,
                const float *hitPos,
                const float *hitDir)
{
    cg_s *LocalClientGlobals; // eax
    int v5; // [esp+0h] [ebp-3C4h]
    int v6; // [esp+4h] [ebp-3C0h]
    float *v8; // [esp+10h] [ebp-3B4h]
    float *v9; // [esp+18h] [ebp-3ACh]
    float v10; // [esp+1Ch] [ebp-3A8h]
    float *outlineOrigin; // [esp+20h] [ebp-3A4h]
    int v12; // [esp+2Ch] [ebp-398h]
    float *v13; // [esp+3Ch] [ebp-388h]
    float *v14; // [esp+40h] [ebp-384h]
    float *v15; // [esp+44h] [ebp-380h]
    float *v16; // [esp+4Ch] [ebp-378h]
    float *v17; // [esp+50h] [ebp-374h]
    ray2_t *p_ray; // [esp+54h] [ebp-370h]
    float *v19; // [esp+58h] [ebp-36Ch]
    float *v20; // [esp+5Ch] [ebp-368h]
    float p2d_4; // [esp+68h] [ebp-35Ch]
    float p3d[3]; // [esp+70h] [ebp-354h] BYREF
    int e; // [esp+7Ch] [ebp-348h]
    int nEffects; // [esp+80h] [ebp-344h]
    float step; // [esp+84h] [ebp-340h]
    int numEffects; // [esp+88h] [ebp-33Ch]
    int v27; // [esp+8Ch] [ebp-338h]
    int i; // [esp+90h] [ebp-334h]
    OutlineEdge outlineEdges[32]; // [esp+94h] [ebp-330h] BYREF
    int MAX_EDGES; // [esp+398h] [ebp-2Ch]
    int MAX_EFFECTS; // [esp+39Ch] [ebp-28h]
    float axis[3][3]; // [esp+3A0h] [ebp-24h] BYREF

    MAX_EDGES = 32;
    MAX_EFFECTS = 10;
    if ( this->glass->numOutlineVerts >= 0x20u
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_client.cpp",
                    362,
                    0,
                    "%s",
                    "glass->numOutlineVerts < MAX_EDGES") )
    {
        __debugbreak();
    }
    for ( i = 0; i < this->glass->numOutlineVerts; ++i )
    {
        outlineEdges[i].index = i;
        if ( i == this->glass->numOutlineVerts - 1 )
            v6 = 0;
        else
            v6 = i + 1;
        p_ray = &outlineEdges[i].ray;
        v19 = this->glass->outline[v6];
        v20 = this->glass->outline[i];
        p_ray->origin[0] = *v20;
        p_ray->origin[1] = v20[1];
        p_ray->dir[0] = *v19 - *v20;
        p_ray->dir[1] = v19[1] - v20[1];
        p_ray->len = Vec2Normalize(p_ray->dir);
    }
    qsort(
        outlineEdges,
        this->glass->numOutlineVerts,
        0x18u,
        (int (__cdecl *)(const void *, const void *))compareOutlineEdges);
    if ( (float)((float)((float)(*hitDir * *hitDir) + (float)(hitDir[1] * hitDir[1])) + (float)(hitDir[2] * hitDir[2])) == 0.0 )
    {
        if ( irand(0, 1) )
        {
            v17 = (float*)this->glass->outlineAxis[2];
            *(_QWORD *)&axis[0][0] = *(_QWORD *)v17;
            axis[0][2] = v17[2];
        }
        else
        {
            v16 = (float*)this->glass->outlineAxis[2];
            (axis[0][0]) = -v16[0];
            (axis[0][1]) = -v16[1];
            (axis[0][2]) = -v16[2];
        }
    }
    else if ( (float)((float)((float)(*hitDir * this->glass->outlineAxis[2][0])
                                                    + (float)(hitDir[1] * this->glass->outlineAxis[2][1]))
                                    + (float)(hitDir[2] * this->glass->outlineAxis[2][2])) <= 0.0 )
    {
        v14 = (float*)this->glass->outlineAxis[2];
        (axis[0][0]) = -v14[0];
        (axis[0][1]) = -v14[1];
        (axis[0][2]) = -v14[2];
    }
    else
    {
        v15 = (float*)this->glass->outlineAxis[2];
        *(_QWORD *)&axis[0][0] = *(_QWORD *)v15;
        axis[0][2] = v15[2];
    }
    v13 = (float*)this->glass->outlineAxis[1];
    axis[1][0] = *v13;
    axis[1][1] = v13[1];
    axis[1][2] = v13[2];
    Vec3Cross(axis[0], axis[1], axis[2]);
    v27 = 0;
    numEffects = 0;
    while ( v27 < this->glass->numOutlineVerts
             && numEffects < 10
             && clGlasses->renderer->shatterFxMinEdgeLength->current.value <= outlineEdges[v27].ray.len )
    {
        nEffects = (int)(float)((float)(outlineEdges[v27].ray.len / clGlasses->renderer->shatterFxMaxDist->current.value)
                                                    + 0.5);
        if ( nEffects < 3 )
            v12 = nEffects;
        else
            v12 = 3;
        if ( v12 > 1 )
            v5 = v12;
        else
            v5 = 1;
        nEffects = v5;
        step = outlineEdges[v27].ray.len / (float)v5;
        e = 0;
        while ( e < nEffects )
        {
            p2d_4 = (float)((float)((float)((float)e + 0.5) * step) * outlineEdges[v27].ray.dir[1])
                        + outlineEdges[v27].ray.origin[1];
            v9 = (float*)this->glass->outlineAxis[0];
            v10 = (float)((float)((float)((float)e + 0.5) * step) * outlineEdges[v27].ray.dir[0])
                    + outlineEdges[v27].ray.origin[0];
            outlineOrigin = (float*)this->glass->outlineOrigin;
            p3d[0] = (float)(v10 * *v9) + *outlineOrigin;
            p3d[1] = (float)(v10 * v9[1]) + outlineOrigin[1];
            p3d[2] = (float)(v10 * v9[2]) + outlineOrigin[2];
            v8 = (float*)this->glass->outlineAxis[1];
            p3d[0] = (float)(p2d_4 * *v8) + p3d[0];
            p3d[1] = (float)(p2d_4 * v8[1]) + p3d[1];
            p3d[2] = (float)(p2d_4 * v8[2]) + p3d[2];
            LocalClientGlobals = CG_GetLocalClientGlobals(localClientNum);
            FX_PlayOrientedEffect(localClientNum, this->glass->glassDef->shatterEffect, LocalClientGlobals->time, p3d, axis);
            ++e;
            ++numEffects;
        }
        ++v27;
    }
}

int __cdecl compareOutlineEdges(float *s1, float *s2)
{
    if ( s1[5] <= s2[5] )
        return 1;
    else
        return -1;
}

void __thiscall GlassClient::Update(int localClientNum)
{
    const GfxBrushModel *BrushModel; // eax
    const float *origin; // [esp-Ch] [ebp-18h]
    const float *angles; // [esp-8h] [ebp-14h]
    Material *pristineMaterial; // [esp+0h] [ebp-Ch]

    if ( (this->state.val.i & 0xF) == 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_client.cpp",
                    417,
                    0,
                    "%s",
                    "state.GetState() != GlassState::INVALID") )
    {
        __debugbreak();
    }
    if ( (this->state.val.i & 0xF) != 2 )
    {
        if ( (this->state.val.i & 0xF) != 0 )
            pristineMaterial = this->glass->glassDef->pristineMaterial;
        else
            pristineMaterial = this->glass->glassDef->crackedMaterial;
        angles = this->glass->angles;
        origin = this->glass->origin;
        BrushModel = R_GetBrushModel(this->glass->brushModel);
        R_AddGlassBrushToScene(BrushModel, origin, angles, pristineMaterial);
    }
}

void __thiscall GlassClient::SetBrushMaterial(GlassState::State state)
{
    Material *crackedMaterial; // [esp+0h] [ebp-18h]
    unsigned int i; // [esp+8h] [ebp-10h]
    const GfxBrushModel *br; // [esp+10h] [ebp-8h]
    GfxSurface *surface; // [esp+14h] [ebp-4h]

    if ( (this->state.val.i & 0xF) >= 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_client.cpp",
                    480,
                    0,
                    "%s",
                    "GetState() == GlassState::PRISTINE || GetState() == GlassState::CRACKED") )
    {
        __debugbreak();
    }
    br = R_GetBrushModel(this->glass->brushModel);
    surface = &rgp.world->dpvs.surfaces[br->startSurfIndex];
    if ( (this->state.val.i & 0xF) != 0 )
        crackedMaterial = this->glass->glassDef->crackedMaterial;
    else
        crackedMaterial = this->glass->glassDef->pristineMaterial;
    for ( i = 0; i < br->surfaceCount; ++i )
    {
        surface->material = crackedMaterial;
        ++surface;
    }
}

void __thiscall GlassClient::Shatter(const float *pos, const float *dir)
{
    int i; // [esp+34h] [ebp-65Ch]
    int numNewShards; // [esp+3Ch] [ebp-654h]
    int numNewShardsa; // [esp+3Ch] [ebp-654h]
    float glassExtent; // [esp+684h] [ebp-Ch]
    GlassShard *shard; // [esp+68Ch] [ebp-4h]

    const GlassShard *newShards[400]; // [esp+40h] [ebp-650h] BYREF

    PROF_SCOPED("GlassClient.Shatter");

    //shard = GlassRenderer::AllocShard(clGlasses->renderer);

    shard = clGlasses->renderer->AllocShard();
    if ( shard )
    {
        //if ( GlassShard::Create(shard, this->glass) )
        if ( shard->Create(this->glass) )
        {
            //glassExtent = GlassShard::Outline::Extent(&shard->outline);
            glassExtent = shard->outline.Extent();
            if ( this->outlines )
            {
                PROF_SCOPED("GlassClient.Shatter.outlines");
                //numNewShards = GlassClient::Outlines::InitShards(this->outlines, shard, newShards, 400);
                numNewShards = this->outlines->InitShards(shard, newShards, 400);
                //GlassShard::Remove(shard, (GlassShard::RemoveReason)8, 0);
                shard->Remove(GlassShard::RemoveReason::KISAK_I_HAVE_NO_CLUE_WHY, 0);
                //GlassRenderer::FreeShardMemory(clGlasses->renderer, &this->outlines->numOutlines);
                clGlasses->renderer->FreeShardMemory((unsigned int*)this->outlines);
                this->outlines = 0;
                if (numNewShards > 0)
                {
                    //GlassShard::InitPhysics(newShards[0], newShards, numNewShards, glassExtent, pos, dir);
                    ((GlassShard *)newShards[0])->InitPhysics(newShards, numNewShards, glassExtent, pos, dir);
                }
            }
            else
            {
                //numNewShardsa = GlassShard::Shatter(shard, newShards, 400);
                numNewShardsa = shard->Shatter(newShards, 400);
                if ( !shard->group
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_client.cpp",
                                570,
                                0,
                                "%s",
                                "shard->group") )
                {
                    __debugbreak();
                }
                if ( shard->group )
                {
                    //GlassShard::InitPhysics(shard, newShards, numNewShardsa, glassExtent, pos, dir);
                    shard->InitPhysics(newShards, numNewShardsa, glassExtent, pos, dir);
                }
                else
                {
                    for (i = 0; i < numNewShardsa; ++i)
                    {
                        //GlassShard::Remove(newShards[i], (GlassShard::RemoveReason)8, 0);
                        ((GlassShard *)newShards[i])->Remove(GlassShard::RemoveReason::KISAK_I_HAVE_NO_CLUE_WHY, 0);
                    }
                }
            }
        }
        else
        {
            //GlassRenderer::FreeShard(clGlasses->renderer, shard);
            clGlasses->renderer->FreeShard(shard);
        }
    }
}

char __thiscall GlassClient::PreShatter()
{
    unsigned int v2; // eax
    GlassClient::Outlines *v3; // eax
    GlassRenderer *renderer; // [esp+18h] [ebp-674h]
    int i; // [esp+34h] [ebp-658h]
    GlassShard *baseShard; // [esp+38h] [ebp-654h]
    Allocator::Memory **buffer; // [esp+3Ch] [ebp-650h]
    int numNewShards; // [esp+40h] [ebp-64Ch]
    const GlassShard *newShards[400]; // [esp+44h] [ebp-648h] BYREF
    GlassShard *shard; // [esp+688h] [ebp-4h]

    if ( (this->state.val.i & 0xF) == 2 || this->outlines )
        return 0;

    PROF_SCOPED("GlassClient.PreShatter");

    //shard = GlassRenderer::AllocShard(clGlasses->renderer);
    shard = clGlasses->renderer->AllocShard();
    if ( shard )
    {
        //if ( GlassShard::Create(shard, this->glass) )
        if ( shard->Create(this->glass) )
        {
            //numNewShards = GlassShard::Shatter(shard, newShards, 400);
            numNewShards = shard->Shatter(newShards, 400);
            renderer = clGlasses->renderer;
            v2 = GlassClient::Outlines::CalcMemorySize(newShards, numNewShards);
            //buffer = GlassRenderer::AllocateShardMemory(renderer, v2, 0);
            buffer = renderer->AllocateShardMemory(v2, NULL);
            if ( buffer )
            {
                //baseShard = GlassRenderer::AllocShard(clGlasses->renderer);
                baseShard = clGlasses->renderer->AllocShard();
                if ( baseShard )
                {
                    //if ( GlassShard::Create(baseShard, this->glass) )
                    if ( baseShard->Create(this->glass) )
                    {
                        //GlassClient::Outlines::Outlines((GlassClient::Outlines *)buffer, baseShard, newShards, numNewShards);
                        new (buffer) GlassClient::Outlines(baseShard, newShards, numNewShards);
                        //this->outlines = v3;
                        this->outlines = (GlassClient::Outlines*)buffer;
                    }
                    //GlassShard::Remove(baseShard, (GlassShard::RemoveReason)8, 0);
                    baseShard->Remove(GlassShard::RemoveReason::KISAK_I_HAVE_NO_CLUE_WHY, 0);
                }
            }
            for (i = 0; i < numNewShards; ++i)
            {
                //GlassShard::Remove(newShards[i], (GlassShard::RemoveReason)8, 0);
                ((GlassShard*)newShards[i])->Remove(GlassShard::RemoveReason::KISAK_I_HAVE_NO_CLUE_WHY, 0);
            }
        }
        else
        {
            //GlassRenderer::FreeShard(clGlasses->renderer, shard);
            clGlasses->renderer->FreeShard(shard);
        }
        return 1;
    }
    else
    {
        return 1;
    }
}

int __cdecl GlassClient::Outlines::CalcMemorySize(const GlassShard **shards, int numShards)
{
    int i; // [esp+4h] [ebp-8h]
    int numVerts; // [esp+8h] [ebp-4h]

    numVerts = 0;
    for ( i = 0; i < numShards; ++i )
        numVerts += shards[i]->outline.numVerts;
    // nx-port: was 12 * numShards + 8 * numVerts + 8, the x86 header and Outline
    return (int)(sizeof(GlassClient::Outlines) + sizeof(GlassClient::Outlines::Outline) * numShards + sizeof(float[2]) * numVerts);
}

GlassClient::Outlines::Outlines(
                const GlassShard *baseShard,
                const GlassShard **shards,
                int numShards)
{
    float *v4; // [esp+Ch] [ebp-18h]
    GlassShard::Outline::Vertex *v5; // [esp+10h] [ebp-14h]
    unsigned int v; // [esp+18h] [ebp-Ch]
    int s; // [esp+1Ch] [ebp-8h]
    float *verts; // [esp+20h] [ebp-4h]

    this->numOutlines = numShards;
    this->outlines = (GlassClient::Outlines::Outline *)&this[1];
    verts = (float *)&this->outlines[this->numOutlines].numVerts;
    for ( s = 0; s < numShards; ++s )
    {
        this->outlines[s].numVerts = shards[s]->outline.numVerts;
        this->outlines[s].verts = (float (*)[2])verts;
        this->outlines[s].edgeFlags = 0;
        for ( v = 0; v < this->outlines[s].numVerts; ++v )
        {
            v4 = this->outlines[s].verts[v];
            v5 = &shards[s]->outline.verts[v];
            *v4 = v5->edge.origin[0];
            v4[1] = v5->edge.origin[1];
            if ( shards[s]->outline.verts[v].isOriginalEdge )
                this->outlines[s].edgeFlags |= 1 << v;
        }
        verts += 2 * this->outlines[s].numVerts;
    }
}

int __thiscall GlassClient::Outlines::InitShards(
                const GlassShard *baseShard,
                const GlassShard **shards,
                int maxNewShards)
{
    bool v4; // bl
    GlassShard *v5; // eax
    int v8; // [esp+20h] [ebp-634h]
    unsigned int v; // [esp+24h] [ebp-630h]
    float offset[2]; // [esp+28h] [ebp-62Ch] BYREF
    ShardGroup *grp; // [esp+30h] [ebp-624h]
    TempOutline newOutline; // [esp+34h] [ebp-620h] BYREF
    unsigned int i; // [esp+648h] [ebp-Ch]
    int numNewShards; // [esp+64Ch] [ebp-8h]

    PROF_SCOPED("InitShards");

    numNewShards = 0;
    for ( i = 0; i < this->numOutlines; ++i )
    {
        newOutline.verts = newOutline.v;
        newOutline.numVerts = 0;
        newOutline.maxVerts = 64;
        newOutline.isClosed = 0;
        //for ( v = 0; v < this->outlines[i].numVerts; GlassShard::Outline::Add(&newOutline, this->outlines[i].verts[v++])->isOriginalEdge = v4 )
        for ( v = 0; v < this->outlines[i].numVerts; newOutline.Add(this->outlines[i].verts[v++])->isOriginalEdge = v4 )
        {
            v4 = (this->outlines[i].edgeFlags & (1 << v)) != 0;
        }
        //GlassShard::Outline::CloseOutline(&newOutline);
        newOutline.CloseOutline();
        //GlassShard::Outline::Recenter(&newOutline, 0, offset);
        newOutline.Recenter(false, offset);
        //v5 = GlassRenderer::AllocShard(clGlasses->renderer);
        v5 = clGlasses->renderer->AllocShard();
        shards[numNewShards] = v5;
        if ( !shards[numNewShards] )
            break;
        //if ( !GlassShard::Init(shards[numNewShards], baseShard, &newOutline, offset) || (grp = (ShardGroup *)GlassRenderer::GetShardGroup(
        if ( !((GlassShard *)shards[numNewShards])->Init(baseShard, &newOutline, offset) || (grp = (ShardGroup *)clGlasses->renderer->GetShardGroup(
                                                                shards[numNewShards]->origin,
                                                                baseShard->group->glassDef)) == 0 )
        {
            //GlassRenderer::FreeShard(clGlasses->renderer, shards[numNewShards]);
            clGlasses->renderer->FreeShard((GlassShard*)shards[numNewShards]);
            break;
        }
        //ShardGroup::Add(grp, shards[numNewShards++]);
        grp->Add((GlassShard *)shards[numNewShards++]);
    }
    v8 = numNewShards;
    return v8;
}

void __cdecl GlassCl_AllocateMemory()
{
    GlassesClient *v0; // [esp+0h] [ebp-Ch]
    GlassesClient *v1; // [esp+4h] [ebp-8h]
    Glasses *glasses; // [esp+8h] [ebp-4h]

    if ( !clGlasses )
    {
        glasses = GetGlasses();
        if ( glasses )
        {
            if ( glasses->numGlasses )
            {
                GlassesClient::InitAllocator(glasses);
                clGlasses = new GlassesClient(glasses); // Calls Custom allocator in class
            }
        }
    }
}

void __cdecl GlassCl_FreeMemory()
{
    if ( clGlasses )
    {
        delete clGlasses; // Custom delete
        //GlassesClient::~GlassesClient(clGlasses);
        //GlassesClient::operator delete((char *)v0);
        clGlasses = 0;
    }
}

void __cdecl GlassCl_Reset(int localClientNum)
{
    if ( CL_LocalClient_IsFirstActive(localClientNum) )
    {
        if ( clGlasses )
        {
            clGlasses->Reset();
            clGlasses->renderer->Reset();
        }
    }
}

void __cdecl GlassCl_Update(int localClientNum)
{
    if ( clGlasses )
        clGlasses->Update(localClientNum);
}

void __cdecl GlassCl_ReadGameState(int localClientNum, msg_t *msg)
{
    if ( clGlasses )
        clGlasses->ParseSnapshot(localClientNum, msg, 1);
}

void __cdecl GlassCl_ParseSnapshot(int localClientNum, msg_t *msg)
{
    if ( clGlasses )
        clGlasses->ParseSnapshot(localClientNum, msg, 0);
}

void __cdecl GlassCl_WriteDemoSnapshot(msg_t *msg)
{
    if ( clGlasses )
        clGlasses->WriteDemoSnapshot(msg);
}

void __cdecl GlassCl_WaitUpdate()
{
    PROF_SCOPED("GlassCl_WaitUpdate"); // LWSS ADD
    if ( clGlasses )
    {
        if ( clGlasses->renderer->smpGlass->current.enabled )
            clGlasses->renderer->BeginUpdate();
    }
}

void __cdecl GlassCl_BeginGenerateVerts(unsigned int localClientNum, unsigned int viewIndex)
{
    //if ( clGlasses )
    //{
    //    if ( clGlasses->renderer->smpGlass->current.enabled )
    //        NitrousVehicle::damage((NitrousVehicle *)clGlasses->renderer, localClientNum, viewIndex);
    //}

    // LWSS: above function is a nullsub
}

void __cdecl GlassCl_WaitGenerateVerts()
{
    if ( clGlasses && clGlasses->renderer->smpGlass->current.enabled )
    {
        PROF_SCOPED("GlassCl_WaitGenerateVerts");
        //GlassRenderer::BeginUpdate((colgeom_visitor_t *)clGlasses->renderer);
        clGlasses->renderer->BeginUpdate();
    }
}

void __cdecl GlassCl_GenerateVerts(int localClientNum, unsigned int viewIndex)
{
    if ( clGlasses && !clGlasses->renderer->smpGlass->current.enabled )
    {
        if ( CL_LocalClient_IsFirstActive(localClientNum) )
        {
            if ( !viewIndex )
                clGlasses->renderer->Update(0);
        }
        //GlassRenderer::GenerateVerts(clGlasses->renderer, localClientNum, viewIndex, 0);
        clGlasses->renderer->GenerateVerts(localClientNum, viewIndex, 0);
    }
}

void __cdecl GlassCl_ExplosionEvent(
                int localClientNum,
                const float *origin,
                float damageInner,
                float damageOuter,
                float radius,
                int mod)
{
    if ( clGlasses )
    {
        if ( CL_LocalClient_IsFirstActive(localClientNum) )
            clGlasses->renderer->AddExplosionAction(origin, damageInner, damageOuter, radius, mod);
    }
}

void __cdecl GlassCl_TracePoint(const pointtrace_t *clip, trace_t *results)
{
    float end[3]; // [esp+Ch] [ebp-Ch] BYREF

    if ( clGlasses )
    {
        //GlassesClient::TracePoint(clGlasses, clip, results);
        clGlasses->TracePoint(clip, results);
        Vec3Lerp(clip->extents.start.vec.v, clip->extents.end.vec.v, results->fraction, end);
        //GlassRenderer::AddTraceAction(clGlasses->renderer, clip->extents.start.vec.v, end);
        clGlasses->renderer->AddTraceAction(clip->extents.start.vec.v, end);
    }
}

void __thiscall GlassesClient::TracePoint(const pointtrace_t *clip, trace_t *results)
{
    unsigned __int16 index; // [esp+Eh] [ebp-232h]
    const Glass *glass; // [esp+10h] [ebp-230h]
    float oldFraction; // [esp+14h] [ebp-22Ch]
    unsigned int i; // [esp+18h] [ebp-228h]
    float mins[3]; // [esp+1Ch] [ebp-224h] BYREF
    const Glass *glss[128]; // [esp+28h] [ebp-218h] BYREF
    float maxs[3]; // [esp+22Ch] [ebp-14h] BYREF
    unsigned int num; // [esp+23Ch] [ebp-4h]

    PROF_SCOPED("GlassesClient.TracePoint");

    Vec3Min(clip->extents.start.vec.v, clip->extents.end.vec.v, mins);
    Vec3Max(clip->extents.start.vec.v, clip->extents.end.vec.v, maxs);
    num = GlassesClient::AreaGlasses(mins, maxs, glss, 0x80u);
    for ( i = 0; i < num; ++i )
    {
        glass = glss[i];
        oldFraction = results->fraction;
        CM_TransformedBoxTrace(
            results,
            clip->extents.start.vec.v,
            clip->extents.end.vec.v,
            vec3_origin,
            vec3_origin,
            glass->brushModel,
            clip->contentmask,
            glass->origin,
            glass->angles);
        if ( results->fraction < oldFraction )
        {
            index = glass->index;
            if ( !results
                && !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\physics\\../qcommon/cm_public.h",
                            175,
                            0,
                            "%s",
                            "trace") )
            {
                __debugbreak();
            }
            results->hitType = TRACE_HITTYPE_GLASS;
            results->hitId = index;
        }
    }
}

void __cdecl GlassCl_MeleeEvent(int localClientNum, int attackerEntNum)
{
    int integer; // [esp+4h] [ebp-4Ch]
    const centity_s *attacker; // [esp+14h] [ebp-3Ch]
    float right[3]; // [esp+1Ch] [ebp-34h] BYREF
    float forward[3]; // [esp+28h] [ebp-28h] BYREF
    float up[3]; // [esp+34h] [ebp-1Ch] BYREF
    float eyePos[3]; // [esp+40h] [ebp-10h] BYREF
    const WeaponDef *weapDef; // [esp+4Ch] [ebp-4h]

    if ( clGlasses )
    {
        if ( CL_LocalClient_IsFirstActive(localClientNum) )
        {
            attacker = CG_GetEntity(localClientNum, attackerEntNum);
            if ( attacker->nextState.weapon )
            {
                weapDef = BG_GetWeaponDef(attacker->nextState.weapon);
                CG_CalcEyePoint(localClientNum, attackerEntNum, eyePos);
                CG_GetViewDirection(localClientNum, attackerEntNum, forward, right, up);
                if ( BG_IsBayonetWeapon(attacker->nextState.weapon) )
                    integer = player_bayonetRange->current.integer;
                else
                    integer = player_meleeRange->current.integer;
                clGlasses->renderer->AddMeleeAction(eyePos, forward, right, up, *(float *)&integer);
            }
        }
    }
}

void __cdecl GlassCl_DrawDebug()
{
    if ( clGlasses )
        clGlasses->renderer->DrawDebug();
}

unsigned int __cdecl GlassCl_AreaGlasses(
                const float *mins,
                const float *maxs,
                const Glass **glasses,
                unsigned int maxGlasses)
{
    if ( clGlasses )
        return clGlasses->AreaGlasses(mins, maxs, glasses, maxGlasses);
    else
        return 0;
}

unsigned int __thiscall GlassesClient::AreaGlasses(
                const float *mins,
                const float *maxs,
                const Glass **glss,
                unsigned int maxGlasses)
{
    const GlassClient *glass; // [esp+14h] [ebp-Ch]
    unsigned int i; // [esp+18h] [ebp-8h]
    unsigned int num; // [esp+1Ch] [ebp-4h]

    num = 0;
    for ( i = 0; i < clGlasses->numGlasses && num < maxGlasses; ++i )
    {
        glass = &this->glasses[i];
        if ( (glass->state.val.i & 0xF) != 2
            && *maxs >= glass->glass->absmin[0]
            && maxs[1] >= glass->glass->absmin[1]
            && maxs[2] >= glass->glass->absmin[2]
            && glass->glass->absmax[0] >= *mins
            && glass->glass->absmax[1] >= mins[1]
            && glass->glass->absmax[2] >= mins[2] )
        {
            glss[num++] = glass->glass;
        }
    }
    return num;
}

void __cdecl GlassCl_ClipMoveTrace(const moveclip_t *clip, trace_t *results)
{
    if ( clGlasses )
    {
        if ( (clip->contentmask & 0x10) != 0 )
            clGlasses->ClipMoveTrace(clip, results);
    }
}

void __thiscall GlassesClient::ClipMoveTrace(const moveclip_t *clip, trace_t *results)
{
    float v3; // [esp+0h] [ebp-29Ch]
    float v4; // [esp+4h] [ebp-298h]
    unsigned int v5; // [esp+8h] [ebp-294h]
    float v6; // [esp+Ch] [ebp-290h]
    float v7; // [esp+10h] [ebp-28Ch]
    unsigned int v8; // [esp+14h] [ebp-288h]
    unsigned __int16 index; // [esp+26h] [ebp-276h]
    const Glass *glass; // [esp+68h] [ebp-234h]
    float oldFraction; // [esp+6Ch] [ebp-230h]
    unsigned int j; // [esp+70h] [ebp-22Ch]
    int i; // [esp+74h] [ebp-228h]
    float mins[3]; // [esp+78h] [ebp-224h] BYREF
    const Glass *glss[128]; // [esp+84h] [ebp-218h] BYREF
    float maxs[3]; // [esp+288h] [ebp-14h] BYREF
    unsigned int num; // [esp+298h] [ebp-4h]

    PROF_SCOPED("GlassesClient.ClipMoveTrace");

    for ( i = 0; i < 3; ++i )
    {
        if ( (float)(clip->extents.end.vec.v[i] - clip->extents.start.vec.v[i]) < 0.0 )
            v8 = clip->extents.end.vec.u[i];
        else
            v8 = clip->extents.start.vec.u[i];
        LODWORD(mins[i]) = v8;
        if ( (float)((float)(clip->extents.start.vec.v[i] + clip->mins[i]) - mins[i]) < 0.0 )
            v7 = clip->extents.start.vec.v[i] + clip->mins[i];
        else
            v7 = mins[i];
        mins[i] = v7;
        if ( (float)((float)(clip->extents.end.vec.v[i] + clip->mins[i]) - mins[i]) < 0.0 )
            v6 = clip->extents.end.vec.v[i] + clip->mins[i];
        else
            v6 = mins[i];
        mins[i] = v6;
        if ( (float)(clip->extents.start.vec.v[i] - clip->extents.end.vec.v[i]) < 0.0 )
            v5 = clip->extents.end.vec.u[i];
        else
            v5 = clip->extents.start.vec.u[i];
        LODWORD(maxs[i]) = v5;
        if ( (float)(maxs[i] - (float)(clip->extents.start.vec.v[i] + clip->maxs[i])) < 0.0 )
            v4 = clip->extents.start.vec.v[i] + clip->maxs[i];
        else
            v4 = maxs[i];
        maxs[i] = v4;
        if ( (float)(maxs[i] - (float)(clip->extents.end.vec.v[i] + clip->maxs[i])) < 0.0 )
            v3 = clip->extents.end.vec.v[i] + clip->maxs[i];
        else
            v3 = maxs[i];
        maxs[i] = v3;
    }
    num = GlassesClient::AreaGlasses(mins, maxs, glss, 0x80u);
    for ( j = 0; j < num; ++j )
    {
        glass = glss[j];
        oldFraction = results->fraction;
        CM_TransformedBoxTrace(
            results,
            clip->extents.start.vec.v,
            clip->extents.end.vec.v,
            clip->mins,
            clip->maxs,
            glass->brushModel,
            clip->contentmask,
            glass->origin,
            glass->angles);
        if ( results->fraction < oldFraction )
        {
            index = glass->index;
            if ( !results
                && !Assert_MyHandler(
                            "c:\\projects_pc\\cod\\codsrc\\src\\physics\\../qcommon/cm_public.h",
                            175,
                            0,
                            "%s",
                            "trace") )
            {
                __debugbreak();
            }
            results->hitType = TRACE_HITTYPE_GLASS;
            results->hitId = index;
        }
    }
}

