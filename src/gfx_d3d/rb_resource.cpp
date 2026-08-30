#include "rb_resource.h"
#include <qcommon/threads.h>
#include <universal/timing.h>
#include "r_image.h"
#include "r_image_load_common.h"
#include <universal/com_memory.h>
#include "r_image_load_obj.h"
#include "r_init.h"
#include "r_buffers.h"

FastCriticalSection resourceLock;
r_resource_action resourceActions[16384];

volatile int numResourceActions;

void __cdecl RB_Resource_Lock()
{
    Sys_LockWrite(&resourceLock);
}

void __cdecl RB_Resource_Unlock()
{
    Sys_UnlockWrite(&resourceLock);
}

void __cdecl RB_Resource_CreateTexture(
                GfxImage *image,
                void *imageBuffer,
                int mipCount,
                int flags,
                _D3DFORMAT imageFormat)
{
    r_resource_action *action; // [esp+0h] [ebp-8h]

    RB_Resource_Lock();
    action = RB_Resource_AllocEntry();
    if ( !action
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_resource.cpp", 66, 0, "%s", "action") )
    {
        __debugbreak();
    }
    action->action = ACTION_CREATE_TEXTURE;
    action->resource = image;
    action->data = imageBuffer;
    action->p1 = mipCount;
    action->p2 = imageFormat;
    action->p3 = flags;
    RB_Resource_Unlock();
}

r_resource_action *__cdecl RB_Resource_AllocEntry()
{
    if ( numResourceActions >= 0x4000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_resource.cpp",
                    53,
                    0,
                    "%s",
                    "numResourceActions < R_MAX_RESOURCE_ACTIONS") )
    {
        __debugbreak();
    }
    Sys_SetResourcesQueuedEvent();
    return &resourceActions[numResourceActions++];
}

void __cdecl RB_Resource_Release(RbD3DUnknown *resource)
{
    r_resource_action *action; // [esp+0h] [ebp-8h]

    RB_Resource_Lock();
    action = RB_Resource_AllocEntry();
    if ( !action
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_resource.cpp", 87, 0, "%s", "action") )
    {
        __debugbreak();
    }
    action->action = ACTION_RELEASE;
    action->resource = resource;
    RB_Resource_Unlock();
}

void __cdecl RB_Resource_ReloadTexture(GfxImage *image, void *data)
{
    r_resource_action *action; // [esp+0h] [ebp-8h]

    RB_Resource_Lock();
    action = RB_Resource_AllocEntry();
    if ( !action
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_resource.cpp", 122, 0, "%s", "action") )
    {
        __debugbreak();
    }
    action->action = ACTION_RELOADTEXTURE;
    action->resource = image;
    action->data = data;
    RB_Resource_Unlock();
}

void __cdecl RB_Resource_Callback(void (__cdecl *callback)())
{
    r_resource_action *action; // [esp+0h] [ebp-8h]

    RB_Resource_Lock();
    action = RB_Resource_AllocEntry();
    if ( !action
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_resource.cpp", 140, 0, "%s", "action") )
    {
        __debugbreak();
    }
    action->action = ACTION_CALLBACK;
    action->resource = callback;
    RB_Resource_Unlock();
}

void __cdecl RB_Resource_CallbackParam(void (__cdecl *callback)(void *), void *data)
{
    r_resource_action *action; // [esp+0h] [ebp-8h]

    RB_Resource_Lock();
    action = RB_Resource_AllocEntry();
    if ( !action
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_resource.cpp", 157, 0, "%s", "action") )
    {
        __debugbreak();
    }
    action->action = ACTION_CALLBACKPARAM;
    action->resource = callback;
    action->data = data;
    RB_Resource_Unlock();
}

void __cdecl RB_Resource_CreateVertexDeclaration(
                _D3DVERTEXELEMENT9 *elements,
                IDirect3DVertexDeclaration9 **declaration)
{
    r_resource_action *action; // [esp+0h] [ebp-8h]

    RB_Resource_Lock();
    action = RB_Resource_AllocEntry();
    if ( !action
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_resource.cpp", 175, 0, "%s", "action") )
    {
        __debugbreak();
    }
    action->action = ACTION_CREATEVERTEXDECL;
    action->resource = declaration;
    action->data = elements;
    RB_Resource_Unlock();
}

void __cdecl RB_Resource_CreateVertexShader(unsigned int *function, IDirect3DVertexShader9 **shader)
{
    r_resource_action *action; // [esp+0h] [ebp-8h]

    RB_Resource_Lock();
    action = RB_Resource_AllocEntry();
    if ( !action
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_resource.cpp", 193, 0, "%s", "action") )
    {
        __debugbreak();
    }
    action->action = ACTION_CREATEVERTEXSHADER;
    action->resource = shader;
    action->data = function;
    RB_Resource_Unlock();
}

void __cdecl RB_Resource_CreatePixelShader(unsigned int *function, IDirect3DPixelShader9 **shader)
{
    r_resource_action *action; // [esp+0h] [ebp-8h]

    RB_Resource_Lock();
    action = RB_Resource_AllocEntry();
    if ( !action
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_resource.cpp", 211, 0, "%s", "action") )
    {
        __debugbreak();
    }
    action->action = ACTION_CREATEPIXELSHADER;
    action->resource = shader;
    action->data = function;
    RB_Resource_Unlock();
}

void __cdecl RB_Resource_LoadVertexBuffer(IDirect3DVertexBuffer9 **vb, void *bufferData, int sizeInBytes)
{
    r_resource_action *action; // [esp+0h] [ebp-8h]

    RB_Resource_Lock();
    action = RB_Resource_AllocEntry();
    if ( !action
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_resource.cpp", 229, 0, "%s", "action") )
    {
        __debugbreak();
    }
    action->action = ACTION_LOADVERTEXBUFFER;
    action->resource = vb;
    action->data = bufferData;
    action->p1 = sizeInBytes;
    RB_Resource_Unlock();
}

void __cdecl RB_Resource_Update(int msec)
{
    LARGE_INTEGER v1; // [esp+20h] [ebp-20h] BYREF
    LARGE_INTEGER PerformanceCount; // [esp+28h] [ebp-18h] BYREF
    unsigned __int64 endtime; // [esp+30h] [ebp-10h]
    unsigned __int64 curtime; // [esp+38h] [ebp-8h]

    if ( Sys_IsDatabaseReady() )
    {
        RB_Resource_Update_Internal();
    }
    else
    {
        QueryPerformanceCounter(&PerformanceCount);
        curtime = PerformanceCount.QuadPart;
        endtime = PerformanceCount.QuadPart + (unsigned __int64)((double)msec / msecPerRawTimerTick);
        do
        {
            Sys_WaitResourcesQueuedEvent((__int64)((double)(endtime - curtime) * msecPerRawTimerTick));
            RB_Resource_Update_Internal();
            QueryPerformanceCounter(&v1);
            curtime = v1.QuadPart;
        }
        while ( v1.QuadPart < endtime );
    }
}

void RB_Resource_Update_Internal()
{
    unsigned int v1; // [esp+0h] [ebp-C0h]
    unsigned int v2; // [esp+4h] [ebp-BCh]
    unsigned int v3; // [esp+8h] [ebp-B8h]
    _D3DCUBEMAP_FACES v4; // [esp+Ch] [ebp-B4h]
    unsigned __int8 mapType; // [esp+10h] [ebp-B0h]
    unsigned __int8 *indexBuffer; // [esp+2Ch] [ebp-94h]
    int rawIndexBytes; // [esp+30h] [ebp-90h]
    signed int indexBytes; // [esp+34h] [ebp-8Ch]
    IDirect3DIndexBuffer9 **ib; // [esp+38h] [ebp-88h]
    void *src; // [esp+3Ch] [ebp-84h]
    GfxImage *resource; // [esp+6Ch] [ebp-54h]
    GfxImageFileHeader *v12; // [esp+74h] [ebp-4Ch]
    GfxImage *image; // [esp+88h] [ebp-38h]
    signed int mipCount; // [esp+90h] [ebp-30h]
    unsigned __int8 *data; // [esp+94h] [ebp-2Ch]
    int faceCount; // [esp+A4h] [ebp-1Ch]
    signed int faceIndex; // [esp+A8h] [ebp-18h]
    _D3DFORMAT imageFormat; // [esp+ACh] [ebp-14h]
    signed int mipLevel; // [esp+B0h] [ebp-10h]
    int flags; // [esp+B4h] [ebp-Ch]
    r_resource_action *action; // [esp+B8h] [ebp-8h]
    int resourceIndex; // [esp+BCh] [ebp-4h]

    PROF_SCOPED("RB_Resource_Update");

    RB_Resource_Lock();
    resourceIndex = 0;
    while ( resourceIndex < numResourceActions )
    {
        action = &resourceActions[resourceIndex];
        switch ( action->action )
        {
            case ACTION_CREATE_TEXTURE:
                image = (GfxImage *)action->resource;
                data = (unsigned __int8 *)action->data;
                mipCount = action->p1;
                imageFormat = (_D3DFORMAT)(action->p2);
                flags = action->p3;
                mapType = image->mapType;
                if ( mapType == MAPTYPE_2D)
                {
                    Image_Create2DTexture_PC(image, image->width, image->height, mipCount, flags, imageFormat);
                    faceCount = 1;
                }
                else if ( mapType == MAPTYPE_3D)
                {
                    Image_Create3DTexture_PC(image, image->width, image->height, image->depth, mipCount, flags, imageFormat);
                    faceCount = 1;
                }
                else
                {
                    iassert(image->mapType == MAPTYPE_CUBE);
                    Image_CreateCubeTexture_PC(image, image->width, mipCount, imageFormat);
                    faceCount = 6;
                }
                if ( data )
                {
                    for ( faceIndex = 0; faceIndex < faceCount; ++faceIndex )
                    {
                        if ( faceCount == 1 )
                            v4 = D3DCUBEMAP_FACE_POSITIVE_X;
                        else
                            v4 = (_D3DCUBEMAP_FACES)Image_CubemapFace(faceIndex);
                        for ( mipLevel = 0; mipLevel < mipCount; ++mipLevel )
                        {
                            Image_UploadData(image, imageFormat, v4, mipLevel, data);
                            if ( image->width >> mipLevel > 1 )
                                v3 = image->width >> mipLevel;
                            else
                                v3 = 1;
                            if ( image->height >> mipLevel > 1 )
                                v2 = image->height >> mipLevel;
                            else
                                v2 = 1;
                            if ( image->depth >> mipLevel > 1 )
                                v1 = image->depth >> mipLevel;
                            else
                                v1 = 1;
                            data += Image_GetCardMemoryAmountForMipLevel(imageFormat, v3, v2, v1);
                        }
                    }
                    image->texture.basemap->PreLoad();
                    Z_VirtualFree(action->data, 20);
                }
                goto LABEL_2;
            case ACTION_RELEASE:
                (*(void (__thiscall **)(void *, void *))(*(unsigned int *)action->resource + 8))(action->resource, action->resource);
                goto LABEL_2;
            case ACTION_LOADTEXTURE:
                Image_LoadFromData(
                    (GfxImage *)action->resource,
                    (GfxImageFileHeader *)action->data,
                    (unsigned __int8 *)action->data + 48);
                Z_VirtualFree(action->data, 20);
                goto LABEL_2;
            case ACTION_RELOADTEXTURE:
                resource = (GfxImage *)action->resource;
                v12 = (GfxImageFileHeader *)action->data;
                Image_Release(resource);
                if ( resource->texture.basemap
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_resource.cpp",
                                356,
                                0,
                                "%s",
                                "!image->texture.basemap") )
                {
                    __debugbreak();
                }
                Image_LoadFromData(resource, v12, (unsigned __int8 *)&v12[1]);
                Z_VirtualFree(action->data, 20);
                goto LABEL_2;
            case ACTION_CALLBACK:
                ((void (*)(void))action->resource)();
                goto LABEL_2;
            case ACTION_CALLBACKPARAM:
                ((void (__cdecl *)(void *))action->resource)(action->data);
                goto LABEL_2;
            case ACTION_CREATEVERTEXDECL:
                dx.device->CreateVertexDeclaration(
                    (const _D3DVERTEXELEMENT9 *)action->data,
                    (IDirect3DVertexDeclaration9 **)action->resource);
                goto LABEL_2;
            case ACTION_CREATEVERTEXSHADER:
                dx.device->CreateVertexShader(
                    (const DWORD *)action->data,
                    (IDirect3DVertexShader9 **)action->resource);
                goto LABEL_2;
            case ACTION_CREATEPIXELSHADER:
                //((void (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, void *, void *))dx.device->CreatePixelShader)(
                //    dx.device,
                //    dx.device,
                //    action->data,
                //    action->resource);
                dx.device->CreatePixelShader((const DWORD*)action->data, (IDirect3DPixelShader9 **)action->resource);
                goto LABEL_2;
            case ACTION_LOADVERTEXBUFFER:
                Load_VertexBuffer((IDirect3DVertexBuffer9 **)action->resource, (unsigned __int8 *)action->data, action->p1);
                goto LABEL_2;
            case ACTION_LOADINDEXBUFFER:
                ib = (IDirect3DIndexBuffer9 **)action->resource;
                src = action->data;
                rawIndexBytes = action->p1;
                indexBytes = (rawIndexBytes + 31) & 0xFFFFFFE0;
                iassert(indexBytes <= MATERIAL_MAX_INDEX_BUFFER_SIZE);
                indexBuffer = (unsigned __int8 *)R_AllocStaticIndexBuffer(ib, indexBytes);
                if (!indexBuffer)
                    return;
                Com_Memcpy(indexBuffer, src, rawIndexBytes);
                memset(&indexBuffer[rawIndexBytes], 0, indexBytes - rawIndexBytes);
                R_FinishStaticIndexBuffer(*ib);
LABEL_2:
                ++resourceIndex;
                break;
            default:
                goto LABEL_2;
        }
    }
    Sys_ClearResourcesQueuedEvent();
    Sys_SetResourcesFlushedEvent();
    numResourceActions = 0;
    RB_Resource_Unlock();
}

void __cdecl RB_Resource_Flush()
{

    if ( Sys_IsRenderThread()
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_resource.cpp",
                    469,
                    0,
                    "%s",
                    "!Sys_IsRenderThread()") )
    {
        __debugbreak();
    }
    RB_Resource_Lock();
    if ( numResourceActions )
    {
        Sys_ClearResourcesFlushedEvent();
        RB_Resource_Unlock();
        Sys_WaitResourcesFlushedEvent();
    }
    else
    {
        RB_Resource_Unlock();
    }
}

