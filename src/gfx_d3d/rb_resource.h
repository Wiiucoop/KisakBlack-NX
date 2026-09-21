#pragma once

#include "r_bsp.h"

#define MATERIAL_MAX_INDEX_BUFFER_SIZE 0x200000

enum Action_t : __int32 // not a real enum
{
    ACTION_CREATE_TEXTURE = 1,
    ACTION_RELEASE = 2,
    ACTION_LOADTEXTURE = 3,
    ACTION_RELOADTEXTURE = 4,
    ACTION_CALLBACK = 5,
    ACTION_CALLBACKPARAM = 6,
    ACTION_CREATEVERTEXDECL = 7,
    ACTION_CREATEVERTEXSHADER = 8,
    ACTION_CREATEPIXELSHADER = 9,
    ACTION_LOADVERTEXBUFFER = 10,
    ACTION_LOADINDEXBUFFER = 11,
};

struct r_resource_action // sizeof=0x18
{                                       // XREF: .data:r_resource_action * resourceActions/r
    Action_t action;
    void *resource;
    void *data;
    int p1;
    int p2;
    int p3;
};

void __cdecl RB_Resource_Lock();
void __cdecl RB_Resource_Unlock();
void __cdecl RB_Resource_CreateTexture(
                GfxImage *image,
                void *imageBuffer,
                int mipCount,
                int flags,
                _D3DFORMAT imageFormat);
// The body of ACTION_CREATE_TEXTURE, callable without the queue. Both the
// queue and RB_Resource_CreateTexture's inline path run this.
void __cdecl RB_Resource_DoCreateTexture(
                GfxImage *image,
                void *imageBuffer,
                int mipCount,
                int flags,
                _D3DFORMAT imageFormat);
r_resource_action *__cdecl RB_Resource_AllocEntry();
#ifdef KISAK_NX
typedef IUnknown9Like RbD3DUnknown; // null-D3D9 objects have no COM IUnknown
#else
typedef IUnknown RbD3DUnknown;
#endif
void __cdecl RB_Resource_Release(RbD3DUnknown *resource);
void __cdecl RB_Resource_ReloadTexture(GfxImage *image, void *data);
void __cdecl RB_Resource_Callback(void (__cdecl *callback)());
void __cdecl RB_Resource_CallbackParam(void (__cdecl *callback)(void *), void *data);
void __cdecl RB_Resource_CreateVertexDeclaration(
                _D3DVERTEXELEMENT9 *elements,
                IDirect3DVertexDeclaration9 **declaration);
void __cdecl RB_Resource_CreateVertexShader(unsigned int *function, IDirect3DVertexShader9 **shader);
void __cdecl RB_Resource_CreatePixelShader(unsigned int *function, IDirect3DPixelShader9 **shader);
void __cdecl RB_Resource_LoadVertexBuffer(IDirect3DVertexBuffer9 **vb, void *bufferData, int sizeInBytes);
void __cdecl RB_Resource_Update(int msec);
void RB_Resource_Update_Internal();
void __cdecl RB_Resource_Flush();
