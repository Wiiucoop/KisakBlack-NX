// nx_d3d9_null.cpp -- null Direct3D 9 implementation for the Switch build.
//
// Resources are backed by real memory (Lock/Unlock return writable buffers
// with correct pitches, including DXT block formats), queries complete
// immediately, draws/state changes are discarded, TestCooperativeLevel is
// always D3D_OK. The renderer runs "for real" -- it just produces no pixels.
// This is the seam where a GL/deko3d backend can be grown later.
#include <d3d9.h>
#include <d3dx9.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ===========================================================================
// format helpers
// ===========================================================================
static bool nxIsDXT(D3DFORMAT fmt)
{
    return fmt == D3DFMT_DXT1 || fmt == D3DFMT_DXT2 || fmt == D3DFMT_DXT3
        || fmt == D3DFMT_DXT4 || fmt == D3DFMT_DXT5;
}

static UINT nxBytesPerPixel(D3DFORMAT fmt)
{
    switch (fmt) {
    case D3DFMT_A8:
    case D3DFMT_L8:
    case D3DFMT_P8:
        return 1;
    case D3DFMT_R5G6B5:
    case D3DFMT_X1R5G5B5:
    case D3DFMT_A1R5G5B5:
    case D3DFMT_A4R4G4B4:
    case D3DFMT_A8L8:
    case D3DFMT_V8U8:
    case D3DFMT_L16:
    case D3DFMT_D16:
    case D3DFMT_D16_LOCKABLE:
    case D3DFMT_D15S1:
    case D3DFMT_R16F:
    case D3DFMT_INDEX16:
        return 2;
    case D3DFMT_R8G8B8:
        return 3;
    case D3DFMT_A16B16G16R16:
    case D3DFMT_A16B16G16R16F:
    case D3DFMT_G32R32F:
    case D3DFMT_Q16W16V16U16:
        return 8;
    case D3DFMT_A32B32G32R32F:
        return 16;
    default:
        return 4; // A8R8G8B8/X8R8G8B8/D24S8/G16R16/R32F/INDEX32/...
    }
}

// pitch + total size for one mip level
static void nxLevelLayout(D3DFORMAT fmt, UINT width, UINT height, UINT *pitch, UINT *size)
{
    if (nxIsDXT(fmt)) {
        UINT bw = (width + 3) / 4;
        UINT bh = (height + 3) / 4;
        UINT blockSize = fmt == D3DFMT_DXT1 ? 8 : 16;
        *pitch = bw * blockSize;
        *size = bw * bh * blockSize;
    } else {
        *pitch = width * nxBytesPerPixel(fmt);
        *size = *pitch * height;
    }
}

static UINT nxMipDim(UINT dim, UINT level)
{
    UINT d = dim >> level;
    return d ? d : 1;
}

static UINT nxCountLevels(UINT w, UINT h, UINT levels)
{
    if (levels) return levels;
    UINT n = 1;
    while (w > 1 || h > 1) {
        w = w > 1 ? w >> 1 : 1;
        h = h > 1 ? h >> 1 : 1;
        n++;
    }
    return n;
}

// ===========================================================================
// internal object records (interface pointers are casts of these)
// ===========================================================================
struct NxD3DObject {
    LONG refCount;
    D3DRESOURCETYPE type;
};

struct NxSurface {
    NxD3DObject obj;
    D3DFORMAT format;
    UINT width;
    UINT height;
    UINT pitch;
    void *bits;       // owned unless owner != NULL
    struct NxTexture *owner;
};

struct NxTexture {
    NxD3DObject obj;   // D3DRTYPE_TEXTURE / CUBETEXTURE / VOLUMETEXTURE
    D3DFORMAT format;
    UINT width, height, depth;
    UINT levels;
    UINT faces;        // 1, or 6 for cube
    void **levelBits;  // [faces * levels]
    NxSurface **surfaces; // lazily created views
};

struct NxBuffer {
    NxD3DObject obj;   // D3DRTYPE_VERTEXBUFFER / INDEXBUFFER
    UINT length;
    DWORD usage;
    D3DFORMAT format;  // index format
    DWORD fvf;
    void *bits;
};

struct NxQuery {
    NxD3DObject obj;
    D3DQUERYTYPE type;
};

struct NxShader {
    NxD3DObject obj;
};

struct NxVDecl {
    NxD3DObject obj;
    D3DVERTEXELEMENT9 elements[32];
    UINT count;
};

struct NxSwapChain {
    NxD3DObject obj;
};

struct NxDevice {
    NxD3DObject obj;
    D3DPRESENT_PARAMETERS pp;
    NxSwapChain swapChain;
    NxSurface *backBuffer;
    NxSurface *depthStencil;
};

struct NxD3D9 {
    NxD3DObject obj;
};

static NxD3D9 s_d3d9;
static NxDevice s_device;

template <typename T> static T *nxAlloc()
{
    T *p = (T *)calloc(1, sizeof(T));
    p->obj.refCount = 1;
    return p;
}

static ULONG nxAddRef(void *self)
{
    NxD3DObject *o = (NxD3DObject *)self;
    return (ULONG)__atomic_add_fetch(&o->refCount, 1, __ATOMIC_SEQ_CST);
}

static void nxDestroy(NxD3DObject *o);

static ULONG nxRelease(void *self)
{
    NxD3DObject *o = (NxD3DObject *)self;
    if (o == &s_d3d9.obj || o == &s_device.obj || o == &s_device.swapChain.obj)
        return 1; // static singletons
    LONG r = __atomic_sub_fetch(&o->refCount, 1, __ATOMIC_SEQ_CST);
    if (r <= 0) {
        nxDestroy(o);
        return 0;
    }
    return (ULONG)r;
}

static void nxDestroy(NxD3DObject *o)
{
    switch ((int)o->type) {
    case D3DRTYPE_SURFACE: {
        NxSurface *s = (NxSurface *)o;
        if (!s->owner) free(s->bits);
        free(s);
        break;
    }
    case D3DRTYPE_TEXTURE:
    case D3DRTYPE_CUBETEXTURE:
    case D3DRTYPE_VOLUMETEXTURE: {
        NxTexture *t = (NxTexture *)o;
        UINT n = t->faces * t->levels;
        for (UINT i = 0; i < n; ++i)
            free(t->levelBits[i]);
        if (t->surfaces) {
            for (UINT i = 0; i < n; ++i)
                free(t->surfaces[i]);
            free(t->surfaces);
        }
        free(t->levelBits);
        free(t);
        break;
    }
    case D3DRTYPE_VERTEXBUFFER:
    case D3DRTYPE_INDEXBUFFER: {
        NxBuffer *b = (NxBuffer *)o;
        free(b->bits);
        free(b);
        break;
    }
    default:
        free(o);
        break;
    }
}

static NxTexture *nxCreateTexture(D3DRESOURCETYPE type, D3DFORMAT fmt,
                                  UINT w, UINT h, UINT depth, UINT levels, UINT faces)
{
    NxTexture *t = nxAlloc<NxTexture>();
    t->obj.type = type;
    t->format = fmt;
    t->width = w;
    t->height = h;
    t->depth = depth ? depth : 1;
    t->levels = nxCountLevels(w, h, levels);
    t->faces = faces;
    t->levelBits = (void **)calloc(t->faces * t->levels, sizeof(void *));
    for (UINT f = 0; f < t->faces; ++f) {
        for (UINT l = 0; l < t->levels; ++l) {
            UINT pitch, size;
            nxLevelLayout(fmt, nxMipDim(w, l), nxMipDim(h, l), &pitch, &size);
            size *= nxMipDim(t->depth, l);
            t->levelBits[f * t->levels + l] = calloc(1, size ? size : 16);
        }
    }
    return t;
}

static NxSurface *nxCreateSurface(D3DFORMAT fmt, UINT w, UINT h)
{
    NxSurface *s = nxAlloc<NxSurface>();
    s->obj.type = D3DRTYPE_SURFACE;
    s->format = fmt;
    s->width = w;
    s->height = h;
    UINT size;
    nxLevelLayout(fmt, w, h, &s->pitch, &size);
    s->bits = calloc(1, size ? size : 16);
    return s;
}

static NxSurface *nxTextureSurface(NxTexture *t, UINT face, UINT level)
{
    UINT n = t->faces * t->levels;
    if (!t->surfaces)
        t->surfaces = (NxSurface **)calloc(n, sizeof(NxSurface *));
    UINT idx = face * t->levels + level;
    if (idx >= n) idx = n - 1;
    if (!t->surfaces[idx]) {
        NxSurface *s = (NxSurface *)calloc(1, sizeof(NxSurface));
        s->obj.refCount = 1;
        s->obj.type = D3DRTYPE_SURFACE;
        s->format = t->format;
        s->width = nxMipDim(t->width, level);
        s->height = nxMipDim(t->height, level);
        UINT size;
        nxLevelLayout(t->format, s->width, s->height, &s->pitch, &size);
        s->bits = t->levelBits[idx];
        s->owner = t;
        t->surfaces[idx] = s;
    }
    nxAddRef(t->surfaces[idx]);
    return t->surfaces[idx];
}

// ===========================================================================
// IUnknown-ish
// ===========================================================================
ULONG IUnknown9Like::AddRef() { return nxAddRef(this); }
ULONG IUnknown9Like::Release() { return nxRelease(this); }
DWORD IDirect3DResource9::SetPriority(DWORD) { return 0; }
void IDirect3DResource9::PreLoad() {}

// ===========================================================================
// IDirect3D9
// ===========================================================================
IDirect3D9 *Direct3DCreate9(UINT)
{
    s_d3d9.obj.refCount = 1;
    return (IDirect3D9 *)&s_d3d9;
}

UINT IDirect3D9::GetAdapterCount() { return 1; }

HRESULT IDirect3D9::GetAdapterIdentifier(UINT, DWORD, D3DADAPTER_IDENTIFIER9 *id)
{
    memset(id, 0, sizeof(*id));
    strcpy(id->Driver, "nx_d3d9_null");
    strcpy(id->Description, "Nintendo Switch (null D3D9)");
    strcpy(id->DeviceName, "\\\\.\\DISPLAY1");
    id->DriverVersion.QuadPart = 0x0001000000010000ll;
    id->VendorId = 0;      // unknown vendor: keeps NVAPI/vendor paths off
    id->DeviceId = 0;
    return D3D_OK;
}

static const D3DDISPLAYMODE s_modes[] = {
    { 1280, 720, 60, D3DFMT_X8R8G8B8 },
    { 1920, 1080, 60, D3DFMT_X8R8G8B8 },
};

UINT IDirect3D9::GetAdapterModeCount(UINT, D3DFORMAT) { return 2; }

HRESULT IDirect3D9::EnumAdapterModes(UINT, D3DFORMAT, UINT mode, D3DDISPLAYMODE *displayMode)
{
    if (mode >= 2) return D3DERR_INVALIDCALL;
    *displayMode = s_modes[mode];
    return D3D_OK;
}

HRESULT IDirect3D9::GetAdapterDisplayMode(UINT, D3DDISPLAYMODE *mode)
{
    *mode = s_modes[0];
    return D3D_OK;
}

HRESULT IDirect3D9::CheckDeviceFormat(UINT, D3DDEVTYPE, D3DFORMAT, DWORD, D3DRESOURCETYPE, D3DFORMAT checkFormat)
{
    // reject the vendor-hack FOURCCs so the engine takes vanilla paths
    switch ((DWORD)checkFormat) {
    case 0x5A544E49u: // 'INTZ'
    case 0x5A534552u: // 'RESZ'
    case 0x4C4C554Eu: // 'NULL'
    case 0x41415353u: // 'SSAA'
    case 0x434F5441u: // 'ATOC'
        return D3DERR_NOTAVAILABLE;
    default:
        return D3D_OK;
    }
}

HRESULT IDirect3D9::CheckDepthStencilMatch(UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, D3DFORMAT) { return D3D_OK; }

HRESULT IDirect3D9::CheckDeviceMultiSampleType(UINT, D3DDEVTYPE, D3DFORMAT, BOOL,
                                               D3DMULTISAMPLE_TYPE type, DWORD *qualityLevels)
{
    if (qualityLevels) *qualityLevels = 1;
    return type == D3DMULTISAMPLE_NONE ? D3D_OK : D3DERR_NOTAVAILABLE;
}

HMONITOR IDirect3D9::GetAdapterMonitor(UINT) { return (HMONITOR)(uintptr_t)1; }

static void nxFillCaps(D3DCAPS9 *caps)
{
    memset(caps, 0, sizeof(*caps));
    caps->DeviceType = D3DDEVTYPE_HAL;
    // Every mandatory bit here comes from the s_capsCheckBits table in
    // src/gfx_d3d/r_caps.cpp, whose entries the engine treats as fatal.
    // Offsets in that table map to D3DCAPS9 fields: 12=Caps2, 16=Caps3,
    // 28=DevCaps, 32=PrimitiveMiscCaps, 40=ZCmpCaps, 44/48=Src/DestBlendCaps,
    // 52=AlphaCmpCaps, 60=TextureCaps, 212=DevCaps2, 236=DeclTypes.
    caps->Caps2 = D3DCAPS2_FULLSCREENGAMMA
        | 0x20000000; /* DYNAMICTEXTURES -- fatal without it */
    caps->Caps3 = 0x20 | 0x100;  /* + LINEAR_TO_SRGB_PRESENTATION */
    caps->DevCaps = 0x000C0000 | 0x8000 | 0x10400; /* + HWRASTERIZATION | HWTNL */
    caps->PrimitiveMiscCaps = 0x2    /* MASKZ */
        | 0x70                       /* CULLNONE | CULLCW | CULLCCW */
        | 0x80                       /* COLORWRITEENABLE */
        | 0x800                      /* BLENDOP */
        | 0x20000;                   /* SEPARATEALPHABLEND */
    caps->ZCmpCaps = 0xFF;           /* all comparison modes */
    caps->SrcBlendCaps = 0x3FFF;
    caps->DestBlendCaps = 0x3FFF;
    caps->AlphaCmpCaps = 0xFF;
    caps->DevCaps2 = 0x1;            /* STREAMOFFSET */
    caps->RasterCaps = D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS | D3DPRASTERCAPS_DEPTHBIAS
        | 0x1 /*DITHER*/ | 0x10000 /*ANISOTROPY*/ | 0x100 /*ZTEST*/;
    // 0x2 (POW2) and 0x20 (SQUAREONLY) must stay CLEAR: the engine reads
    // their presence as a restriction and refuses to run.
    caps->TextureCaps = D3DPTEXTURECAPS_MIPCUBEMAP | 0x1 /* PERSPECTIVE */
        | 0x4                        /* ALPHA */
        | 0x800                      /* CUBEMAP */
        | 0x4000                     /* MIPMAP */
        | 0x2000                     /* VOLUMEMAP */
        | 0x100000;                  /* MIPVOLUMEMAP */
    caps->TextureFilterCaps = D3DPTFILTERCAPS_MINFANISOTROPIC | D3DPTFILTERCAPS_MAGFANISOTROPIC
        | 0x100 /*MINFPOINT*/ | 0x200 /*MINFLINEAR*/ | 0x1000000 /*MAGFPOINT*/ | 0x2000000 /*MAGFLINEAR*/
        | 0x10000 /*MIPFPOINT*/ | 0x20000 /*MIPFLINEAR*/;
    caps->CubeTextureFilterCaps = caps->TextureFilterCaps;
    caps->VolumeTextureFilterCaps = caps->TextureFilterCaps;
    caps->TextureAddressCaps = 0x3F;
    caps->VolumeTextureAddressCaps = 0x3F;
    caps->MaxTextureWidth = 8192;
    caps->MaxTextureHeight = 8192;
    caps->MaxVolumeExtent = 2048;
    caps->MaxTextureRepeat = 8192;
    caps->MaxTextureAspectRatio = 8192;
    caps->MaxAnisotropy = 16;
    caps->StencilCaps = 0x1FF;
    caps->TextureOpCaps = 0x3FFFFFF;
    caps->MaxTextureBlendStages = 8;
    caps->MaxSimultaneousTextures = 8;
    caps->MaxActiveLights = 8;
    caps->MaxUserClipPlanes = 6;
    caps->MaxPointSize = 256.0f;
    caps->MaxPrimitiveCount = 0xFFFFF;
    caps->MaxVertexIndex = 0xFFFFF;
    caps->MaxStreams = 16;
    caps->MaxStreamStride = 508;
    caps->VertexShaderVersion = D3DVS_VERSION(3, 0);
    caps->MaxVertexShaderConst = 256;
    caps->PixelShaderVersion = D3DPS_VERSION(3, 0);
    caps->PixelShader1xMaxValue = 8.0f;
    caps->NumSimultaneousRTs = 4;
    caps->StretchRectFilterCaps = 0x03000300;
    caps->VS20Caps.Caps = 1;
    caps->VS20Caps.DynamicFlowControlDepth = 24;
    caps->VS20Caps.NumTemps = 32;
    caps->VS20Caps.StaticFlowControlDepth = 4;
    caps->PS20Caps.Caps = 0x1F;
    caps->PS20Caps.DynamicFlowControlDepth = 24;
    caps->PS20Caps.NumTemps = 32;
    caps->PS20Caps.StaticFlowControlDepth = 4;
    caps->PS20Caps.NumInstructionSlots = 512;
    caps->VertexTextureFilterCaps = caps->TextureFilterCaps;
    caps->MaxVShaderInstructionsExecuted = 65535;
    caps->MaxPShaderInstructionsExecuted = 65535;
    caps->MaxVertexShader30InstructionSlots = 32768;
    caps->MaxPixelShader30InstructionSlots = 32768;
    caps->DeclTypes = 0x3FF;
    caps->PresentationIntervals = 0x80000001;
}

HRESULT IDirect3D9::GetDeviceCaps(UINT, D3DDEVTYPE, D3DCAPS9 *caps)
{
    nxFillCaps(caps);
    return D3D_OK;
}

HRESULT IDirect3D9::CreateDevice(UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS *pp,
                                 IDirect3DDevice9 **device)
{
    memset(&s_device, 0, sizeof(s_device));
    s_device.obj.refCount = 1;
    s_device.swapChain.obj.refCount = 1;
    if (pp) {
        s_device.pp = *pp;
        UINT w = pp->BackBufferWidth ? pp->BackBufferWidth : 1280;
        UINT h = pp->BackBufferHeight ? pp->BackBufferHeight : 720;
        s_device.backBuffer = nxCreateSurface(D3DFMT_A8R8G8B8, w, h);
        s_device.depthStencil = nxCreateSurface(D3DFMT_D24S8, w, h);
    }
    *device = (IDirect3DDevice9 *)&s_device;
    return D3D_OK;
}

// ===========================================================================
// IDirect3DDevice9
// ===========================================================================
HRESULT IDirect3DDevice9::TestCooperativeLevel() { return D3D_OK; }
UINT IDirect3DDevice9::GetAvailableTextureMem() { return 256 * 1024 * 1024; }
HRESULT IDirect3DDevice9::GetDeviceCaps(D3DCAPS9 *caps)
{
    nxFillCaps(caps);
    return D3D_OK;
}
HRESULT IDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *params)
{
    memset(params, 0, sizeof(*params));
    params->DeviceType = D3DDEVTYPE_HAL;
    return D3D_OK;
}
HRESULT IDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS *pp)
{
    if (pp) s_device.pp = *pp;
    return D3D_OK;
}
HRESULT IDirect3DDevice9::Present(const RECT *, const RECT *, HWND, const void *) { return D3D_OK; }
HRESULT IDirect3DDevice9::GetSwapChain(UINT, IDirect3DSwapChain9 **swapChain)
{
    nxAddRef(&s_device.swapChain);
    *swapChain = (IDirect3DSwapChain9 *)&s_device.swapChain;
    return D3D_OK;
}
HRESULT IDirect3DDevice9::GetBackBuffer(UINT, UINT, D3DBACKBUFFER_TYPE, IDirect3DSurface9 **surface)
{
    if (!s_device.backBuffer)
        s_device.backBuffer = nxCreateSurface(D3DFMT_A8R8G8B8, 1280, 720);
    nxAddRef(s_device.backBuffer);
    *surface = (IDirect3DSurface9 *)s_device.backBuffer;
    return D3D_OK;
}
void IDirect3DDevice9::SetGammaRamp(UINT, DWORD, const D3DGAMMARAMP *) {}

HRESULT IDirect3DDevice9::CreateTexture(UINT width, UINT height, UINT levels, DWORD, D3DFORMAT format,
                                        D3DPOOL, IDirect3DTexture9 **texture, HANDLE *)
{
    *texture = (IDirect3DTexture9 *)nxCreateTexture(D3DRTYPE_TEXTURE, format, width, height, 1, levels, 1);
    return D3D_OK;
}

HRESULT IDirect3DDevice9::CreateVolumeTexture(UINT width, UINT height, UINT depth, UINT levels, DWORD,
                                              D3DFORMAT format, D3DPOOL, IDirect3DVolumeTexture9 **texture, HANDLE *)
{
    *texture = (IDirect3DVolumeTexture9 *)nxCreateTexture(D3DRTYPE_VOLUMETEXTURE, format, width, height, depth, levels, 1);
    return D3D_OK;
}

HRESULT IDirect3DDevice9::CreateCubeTexture(UINT edgeLength, UINT levels, DWORD, D3DFORMAT format,
                                            D3DPOOL, IDirect3DCubeTexture9 **texture, HANDLE *)
{
    *texture = (IDirect3DCubeTexture9 *)nxCreateTexture(D3DRTYPE_CUBETEXTURE, format, edgeLength, edgeLength, 1, levels, 6);
    return D3D_OK;
}

HRESULT IDirect3DDevice9::CreateVertexBuffer(UINT length, DWORD usage, DWORD fvf, D3DPOOL,
                                             IDirect3DVertexBuffer9 **vb, HANDLE *)
{
    NxBuffer *b = nxAlloc<NxBuffer>();
    b->obj.type = D3DRTYPE_VERTEXBUFFER;
    b->length = length;
    b->usage = usage;
    b->fvf = fvf;
    b->bits = calloc(1, length ? length : 16);
    *vb = (IDirect3DVertexBuffer9 *)b;
    return D3D_OK;
}

HRESULT IDirect3DDevice9::CreateIndexBuffer(UINT length, DWORD usage, D3DFORMAT format, D3DPOOL,
                                            IDirect3DIndexBuffer9 **ib, HANDLE *)
{
    NxBuffer *b = nxAlloc<NxBuffer>();
    b->obj.type = D3DRTYPE_INDEXBUFFER;
    b->length = length;
    b->usage = usage;
    b->format = format;
    b->bits = calloc(1, length ? length : 16);
    *ib = (IDirect3DIndexBuffer9 *)b;
    return D3D_OK;
}

HRESULT IDirect3DDevice9::CreateRenderTarget(UINT width, UINT height, D3DFORMAT format, D3DMULTISAMPLE_TYPE,
                                             DWORD, BOOL, IDirect3DSurface9 **surface, HANDLE *)
{
    *surface = (IDirect3DSurface9 *)nxCreateSurface(format, width, height);
    return D3D_OK;
}

HRESULT IDirect3DDevice9::CreateDepthStencilSurface(UINT width, UINT height, D3DFORMAT format, D3DMULTISAMPLE_TYPE,
                                                    DWORD, BOOL, IDirect3DSurface9 **surface, HANDLE *)
{
    *surface = (IDirect3DSurface9 *)nxCreateSurface(format, width, height);
    return D3D_OK;
}

HRESULT IDirect3DDevice9::CreateOffscreenPlainSurface(UINT width, UINT height, D3DFORMAT format, D3DPOOL,
                                                      IDirect3DSurface9 **surface, HANDLE *)
{
    *surface = (IDirect3DSurface9 *)nxCreateSurface(format, width, height);
    return D3D_OK;
}

HRESULT IDirect3DDevice9::CreateVertexDeclaration(const D3DVERTEXELEMENT9 *elements, IDirect3DVertexDeclaration9 **decl)
{
    NxVDecl *d = nxAlloc<NxVDecl>();
    d->obj.type = (D3DRESOURCETYPE)100;
    UINT n = 0;
    while (elements && n < 31 && !(elements[n].Stream == 0xFF && elements[n].Type == D3DDECLTYPE_UNUSED)) {
        d->elements[n] = elements[n];
        n++;
    }
    d->count = n;
    *decl = (IDirect3DVertexDeclaration9 *)d;
    return D3D_OK;
}

HRESULT IDirect3DDevice9::CreateVertexShader(const DWORD *, IDirect3DVertexShader9 **shader)
{
    NxShader *s = nxAlloc<NxShader>();
    s->obj.type = (D3DRESOURCETYPE)101;
    *shader = (IDirect3DVertexShader9 *)s;
    return D3D_OK;
}

HRESULT IDirect3DDevice9::CreatePixelShader(const DWORD *, IDirect3DPixelShader9 **shader)
{
    NxShader *s = nxAlloc<NxShader>();
    s->obj.type = (D3DRESOURCETYPE)102;
    *shader = (IDirect3DPixelShader9 *)s;
    return D3D_OK;
}

HRESULT IDirect3DDevice9::CreateQuery(D3DQUERYTYPE type, IDirect3DQuery9 **query)
{
    if (!query) return D3D_OK; // supported-check form
    NxQuery *q = nxAlloc<NxQuery>();
    q->obj.type = (D3DRESOURCETYPE)103;
    q->type = type;
    *query = (IDirect3DQuery9 *)q;
    return D3D_OK;
}

HRESULT IDirect3DDevice9::UpdateSurface(IDirect3DSurface9 *, const RECT *, IDirect3DSurface9 *, const POINT *) { return D3D_OK; }
HRESULT IDirect3DDevice9::UpdateTexture(IDirect3DBaseTexture9 *, IDirect3DBaseTexture9 *) { return D3D_OK; }
HRESULT IDirect3DDevice9::GetRenderTargetData(IDirect3DSurface9 *, IDirect3DSurface9 *) { return D3D_OK; }
HRESULT IDirect3DDevice9::StretchRect(IDirect3DSurface9 *, const RECT *, IDirect3DSurface9 *, const RECT *, D3DTEXTUREFILTERTYPE) { return D3D_OK; }
HRESULT IDirect3DDevice9::ColorFill(IDirect3DSurface9 *, const RECT *, D3DCOLOR) { return D3D_OK; }
HRESULT IDirect3DDevice9::SetRenderTarget(DWORD, IDirect3DSurface9 *) { return D3D_OK; }
HRESULT IDirect3DDevice9::GetRenderTarget(DWORD, IDirect3DSurface9 **renderTarget)
{
    if (renderTarget) {
        if (!s_device.backBuffer)
            s_device.backBuffer = nxCreateSurface(D3DFMT_A8R8G8B8, 1280, 720);
        nxAddRef(s_device.backBuffer);
        *renderTarget = (IDirect3DSurface9 *)s_device.backBuffer;
    }
    return D3D_OK;
}
HRESULT IDirect3DDevice9::SetDepthStencilSurface(IDirect3DSurface9 *) { return D3D_OK; }
HRESULT IDirect3DDevice9::GetDepthStencilSurface(IDirect3DSurface9 **depthStencil)
{
    if (depthStencil) {
        if (!s_device.depthStencil)
            s_device.depthStencil = nxCreateSurface(D3DFMT_D24S8, 1280, 720);
        nxAddRef(s_device.depthStencil);
        *depthStencil = (IDirect3DSurface9 *)s_device.depthStencil;
    }
    return D3D_OK;
}
HRESULT IDirect3DDevice9::BeginScene() { return D3D_OK; }
HRESULT IDirect3DDevice9::EndScene() { return D3D_OK; }
HRESULT IDirect3DDevice9::Clear(DWORD, const D3DRECT *, DWORD, D3DCOLOR, float, DWORD) { return D3D_OK; }
HRESULT IDirect3DDevice9::SetViewport(const D3DVIEWPORT9 *) { return D3D_OK; }
HRESULT IDirect3DDevice9::GetViewport(D3DVIEWPORT9 *viewport)
{
    if (viewport) {
        memset(viewport, 0, sizeof(*viewport));
        viewport->Width = 1280;
        viewport->Height = 720;
        viewport->MaxZ = 1.0f;
    }
    return D3D_OK;
}
HRESULT IDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE, DWORD) { return D3D_OK; }
HRESULT IDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE, DWORD *value)
{
    if (value) *value = 0;
    return D3D_OK;
}
HRESULT IDirect3DDevice9::SetTexture(DWORD, IDirect3DBaseTexture9 *) { return D3D_OK; }
HRESULT IDirect3DDevice9::SetTextureStageState(DWORD, D3DTEXTURESTAGESTATETYPE, DWORD) { return D3D_OK; }
HRESULT IDirect3DDevice9::SetSamplerState(DWORD, D3DSAMPLERSTATETYPE, DWORD) { return D3D_OK; }
HRESULT IDirect3DDevice9::SetScissorRect(const RECT *) { return D3D_OK; }
HRESULT IDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE, UINT, UINT) { return D3D_OK; }
HRESULT IDirect3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE, INT, UINT, UINT, UINT, UINT) { return D3D_OK; }
HRESULT IDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE, UINT, const void *, UINT) { return D3D_OK; }
HRESULT IDirect3DDevice9::SetVertexDeclaration(IDirect3DVertexDeclaration9 *) { return D3D_OK; }
HRESULT IDirect3DDevice9::SetFVF(DWORD) { return D3D_OK; }
HRESULT IDirect3DDevice9::SetVertexShader(IDirect3DVertexShader9 *) { return D3D_OK; }
HRESULT IDirect3DDevice9::SetVertexShaderConstantF(UINT, const float *, UINT) { return D3D_OK; }
HRESULT IDirect3DDevice9::SetPixelShader(IDirect3DPixelShader9 *) { return D3D_OK; }
HRESULT IDirect3DDevice9::SetPixelShaderConstantF(UINT, const float *, UINT) { return D3D_OK; }
HRESULT IDirect3DDevice9::SetStreamSource(UINT, IDirect3DVertexBuffer9 *, UINT, UINT) { return D3D_OK; }
HRESULT IDirect3DDevice9::SetIndices(IDirect3DIndexBuffer9 *) { return D3D_OK; }
HRESULT IDirect3DDevice9::EvictManagedResources() { return D3D_OK; }

// ===========================================================================
// IDirect3DSwapChain9
// ===========================================================================
HRESULT IDirect3DSwapChain9::Present(const RECT *, const RECT *, HWND, const void *, DWORD)
{
    return D3D_OK;
}
HRESULT IDirect3DSwapChain9::GetBackBuffer(UINT, D3DBACKBUFFER_TYPE, IDirect3DSurface9 **surface)
{
    return s_device.backBuffer
        ? (nxAddRef(s_device.backBuffer), *surface = (IDirect3DSurface9 *)s_device.backBuffer, D3D_OK)
        : D3DERR_INVALIDCALL;
}

// ===========================================================================
// IDirect3DSurface9
// ===========================================================================
HRESULT IDirect3DSurface9::LockRect(D3DLOCKED_RECT *lockedRect, const RECT *rect, DWORD)
{
    NxSurface *s = (NxSurface *)this;
    UINT bpp = nxIsDXT(s->format) ? 0 : nxBytesPerPixel(s->format);
    lockedRect->Pitch = (INT)s->pitch;
    if (rect && bpp)
        lockedRect->pBits = (BYTE *)s->bits + rect->top * s->pitch + rect->left * bpp;
    else
        lockedRect->pBits = s->bits;
    return D3D_OK;
}
HRESULT IDirect3DSurface9::UnlockRect() { return D3D_OK; }
HRESULT IDirect3DSurface9::GetDesc(D3DSURFACE_DESC *desc)
{
    NxSurface *s = (NxSurface *)this;
    memset(desc, 0, sizeof(*desc));
    desc->Format = s->format;
    desc->Type = D3DRTYPE_SURFACE;
    desc->Width = s->width;
    desc->Height = s->height;
    return D3D_OK;
}

// ===========================================================================
// textures
// ===========================================================================
DWORD IDirect3DBaseTexture9::GetLevelCount()
{
    return ((NxTexture *)this)->levels;
}

HRESULT IDirect3DTexture9::LockRect(UINT level, D3DLOCKED_RECT *lockedRect, const RECT *rect, DWORD)
{
    NxTexture *t = (NxTexture *)this;
    if (level >= t->levels) level = t->levels - 1;
    UINT pitch, size;
    nxLevelLayout(t->format, nxMipDim(t->width, level), nxMipDim(t->height, level), &pitch, &size);
    lockedRect->Pitch = (INT)pitch;
    BYTE *bits = (BYTE *)t->levelBits[level];
    if (rect && !nxIsDXT(t->format))
        bits += rect->top * pitch + rect->left * nxBytesPerPixel(t->format);
    lockedRect->pBits = bits;
    return D3D_OK;
}
HRESULT IDirect3DTexture9::UnlockRect(UINT) { return D3D_OK; }
HRESULT IDirect3DTexture9::GetSurfaceLevel(UINT level, IDirect3DSurface9 **surface)
{
    NxTexture *t = (NxTexture *)this;
    if (level >= t->levels) level = t->levels - 1;
    *surface = (IDirect3DSurface9 *)nxTextureSurface(t, 0, level);
    return D3D_OK;
}
HRESULT IDirect3DTexture9::GetLevelDesc(UINT level, D3DSURFACE_DESC *desc)
{
    NxTexture *t = (NxTexture *)this;
    if (level >= t->levels) level = t->levels - 1;
    memset(desc, 0, sizeof(*desc));
    desc->Format = t->format;
    desc->Type = D3DRTYPE_SURFACE;
    desc->Width = nxMipDim(t->width, level);
    desc->Height = nxMipDim(t->height, level);
    return D3D_OK;
}
HRESULT IDirect3DTexture9::AddDirtyRect(const RECT *) { return D3D_OK; }

HRESULT IDirect3DCubeTexture9::LockRect(D3DCUBEMAP_FACES face, UINT level, D3DLOCKED_RECT *lockedRect,
                                        const RECT *, DWORD)
{
    NxTexture *t = (NxTexture *)this;
    if (level >= t->levels) level = t->levels - 1;
    UINT f = (UINT)face < t->faces ? (UINT)face : 0;
    UINT pitch, size;
    nxLevelLayout(t->format, nxMipDim(t->width, level), nxMipDim(t->height, level), &pitch, &size);
    lockedRect->Pitch = (INT)pitch;
    lockedRect->pBits = t->levelBits[f * t->levels + level];
    return D3D_OK;
}
HRESULT IDirect3DCubeTexture9::UnlockRect(D3DCUBEMAP_FACES, UINT) { return D3D_OK; }
HRESULT IDirect3DCubeTexture9::GetLevelDesc(UINT level, D3DSURFACE_DESC *desc)
{
    return ((IDirect3DTexture9 *)this)->GetLevelDesc(level, desc);
}
HRESULT IDirect3DCubeTexture9::GetCubeMapSurface(D3DCUBEMAP_FACES face, UINT level, IDirect3DSurface9 **surface)
{
    NxTexture *t = (NxTexture *)this;
    if (level >= t->levels) level = t->levels - 1;
    *surface = (IDirect3DSurface9 *)nxTextureSurface(t, (UINT)face, level);
    return D3D_OK;
}

HRESULT IDirect3DVolumeTexture9::LockBox(UINT level, D3DLOCKED_BOX *lockedBox, const D3DBOX *, DWORD)
{
    NxTexture *t = (NxTexture *)this;
    if (level >= t->levels) level = t->levels - 1;
    UINT pitch, size;
    nxLevelLayout(t->format, nxMipDim(t->width, level), nxMipDim(t->height, level), &pitch, &size);
    lockedBox->RowPitch = (INT)pitch;
    lockedBox->SlicePitch = (INT)size;
    lockedBox->pBits = t->levelBits[level];
    return D3D_OK;
}
HRESULT IDirect3DVolumeTexture9::UnlockBox(UINT) { return D3D_OK; }
HRESULT IDirect3DVolumeTexture9::GetLevelDesc(UINT level, D3DVOLUME_DESC *desc)
{
    NxTexture *t = (NxTexture *)this;
    if (level >= t->levels) level = t->levels - 1;
    memset(desc, 0, sizeof(*desc));
    desc->Format = t->format;
    desc->Type = D3DRTYPE_VOLUME;
    desc->Width = nxMipDim(t->width, level);
    desc->Height = nxMipDim(t->height, level);
    desc->Depth = nxMipDim(t->depth, level);
    return D3D_OK;
}

// ===========================================================================
// buffers
// ===========================================================================
HRESULT IDirect3DVertexBuffer9::Lock(UINT offset, UINT, void **data, DWORD)
{
    NxBuffer *b = (NxBuffer *)this;
    *data = (BYTE *)b->bits + offset;
    return D3D_OK;
}
HRESULT IDirect3DVertexBuffer9::Unlock() { return D3D_OK; }
HRESULT IDirect3DVertexBuffer9::GetDesc(D3DVERTEXBUFFER_DESC *desc)
{
    NxBuffer *b = (NxBuffer *)this;
    memset(desc, 0, sizeof(*desc));
    desc->Format = D3DFMT_VERTEXDATA;
    desc->Type = D3DRTYPE_VERTEXBUFFER;
    desc->Usage = b->usage;
    desc->Size = b->length;
    desc->FVF = b->fvf;
    return D3D_OK;
}

HRESULT IDirect3DIndexBuffer9::Lock(UINT offset, UINT, void **data, DWORD)
{
    NxBuffer *b = (NxBuffer *)this;
    *data = (BYTE *)b->bits + offset;
    return D3D_OK;
}
HRESULT IDirect3DIndexBuffer9::Unlock() { return D3D_OK; }
HRESULT IDirect3DIndexBuffer9::GetDesc(D3DINDEXBUFFER_DESC *desc)
{
    NxBuffer *b = (NxBuffer *)this;
    memset(desc, 0, sizeof(*desc));
    desc->Format = b->format;
    desc->Type = D3DRTYPE_INDEXBUFFER;
    desc->Usage = b->usage;
    desc->Size = b->length;
    return D3D_OK;
}

// ===========================================================================
// declarations / queries
// ===========================================================================
HRESULT IDirect3DVertexDeclaration9::GetDeclaration(D3DVERTEXELEMENT9 *decl, UINT *numElements)
{
    NxVDecl *d = (NxVDecl *)this;
    if (decl)
        memcpy(decl, d->elements, d->count * sizeof(D3DVERTEXELEMENT9));
    if (numElements) *numElements = d->count;
    return D3D_OK;
}

HRESULT IDirect3DQuery9::Issue(DWORD) { return D3D_OK; }
HRESULT IDirect3DQuery9::GetData(void *data, DWORD size, DWORD)
{
    // Queries complete immediately: EVENT fences signal, OCCLUSION returns 0
    // visible pixels (coronas/sun flares read this).
    if (data && size >= 4)
        memset(data, 0, size);
    return S_OK;
}
DWORD IDirect3DQuery9::GetDataSize() { return 4; }

// ===========================================================================
// D3DX subset + perf markers
// ===========================================================================
void *ID3DXBuffer::GetBufferPointer() { return NULL; }
DWORD ID3DXBuffer::GetBufferSize() { return 0; }
ULONG ID3DXBuffer::AddRef() { return 1; }
ULONG ID3DXBuffer::Release() { return 0; }

void *ID3DXConstantTable::GetBufferPointer() { return NULL; }
DWORD ID3DXConstantTable::GetBufferSize() { return 0; }
ULONG ID3DXConstantTable::AddRef() { return 1; }
ULONG ID3DXConstantTable::Release() { return 0; }

HRESULT D3DXCompileShader(LPCSTR, UINT, const D3DXMACRO *, LPD3DXINCLUDE, LPCSTR, LPCSTR,
                          DWORD, LPD3DXBUFFER *shader, LPD3DXBUFFER *errorMsgs, LPD3DXCONSTANTTABLE *table)
{
    if (shader) *shader = NULL;
    if (errorMsgs) *errorMsgs = NULL;
    if (table) *table = NULL;
    return E_FAIL; // runtime shader compilation unavailable; fastfiles carry prebuilt shaders
}

HRESULT D3DXCreateBuffer(DWORD, LPD3DXBUFFER *buffer)
{
    if (buffer) *buffer = NULL;
    return E_FAIL;
}

HRESULT D3DXGetShaderConstantTable(const DWORD *, LPD3DXCONSTANTTABLE *table)
{
    if (table) *table = NULL;
    return E_FAIL;
}

HRESULT D3DXGetShaderInputSemantics(const DWORD *, D3DXSEMANTIC *, UINT *count)
{
    if (count) *count = 0;
    return E_FAIL;
}

HRESULT D3DXGetShaderOutputSemantics(const DWORD *, D3DXSEMANTIC *, UINT *count)
{
    if (count) *count = 0;
    return E_FAIL;
}

HRESULT D3DXSaveSurfaceToFileA(LPCSTR, D3DXIMAGE_FILEFORMAT, IDirect3DSurface9 *, const void *, const RECT *)
{
    return E_FAIL;
}

int D3DPERF_BeginEvent(D3DCOLOR, const wchar_t *) { return 0; }
int D3DPERF_EndEvent(void) { return 0; }
void D3DPERF_SetMarker(D3DCOLOR, const wchar_t *) {}
