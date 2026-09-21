// nx_d3d9_null.cpp -- null Direct3D 9 implementation for the Switch build.
//
// Resources are backed by real memory (Lock/Unlock return writable buffers
// with correct pitches, including DXT block formats), queries complete
// immediately, TestCooperativeLevel is always D3D_OK. The renderer runs "for
// real" -- and a growing slice of the API now reaches the screen over EGL and
// Mesa's OpenGL: Clear, the swap chain's Present, and indexed geometry
// (vertex and index buffers, the position and colour elements of the vertex
// declaration, DrawIndexedPrimitive) drawn in its own vertex colours and
// composited with the blend state the engine asked for. Shaders and textures
// are still discarded, so what lands on screen is untextured and unlit by
// construction, and most of the D3DRS file is recorded but not acted on.
#include <d3d9.h>
#include <d3dx9.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <switch.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
// gl.h stops at 1.1; everything the geometry path uses -- buffers, shaders,
// vertex arrays -- lives past that, in the glext.h that gl.h pulls in at its
// end. This Mesa build exports those entry points from libGL, so prototypes
// are all that is missing, and the switch for them has to be set before gl.h
// reaches that include.
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>

#include <qcommon/threads.h>

// Counters for the D3D9 call census. The point is to learn which subset of
// the API the engine actually uses before writing any real backend.
// The two presentation paths are counted apart: RB_SwapBuffers goes through
// the swap chain (rb_backend.cpp:4710), while the device's own Present is only
// reached by Bink video playback (dx9rad3d.cpp:321).
static unsigned s_nPresent, s_nSwapPresent;
static unsigned s_nBeginScene, s_nDrawIndexed, s_nDrawPrim, s_nDrawPrimUP;
static unsigned s_nSetRenderState, s_nSetTexture, s_nSetVertexShader;
static unsigned s_nSetPixelShader, s_nSetStreamSource, s_nSetIndices;
static unsigned s_nClear;

// What the engine last asked Clear for, so Present's 60-frame report can say
// whether a black screen is our bug or simply the colour we were handed.
static D3DCOLOR s_lastClearColor;
static DWORD s_lastClearFlags;

static void nxDumpCallCensus(void)
{
    printf("[d3d census] swapPresent=%u devPresent=%u beginScene=%u clear=%u\n"
           "             drawIndexed=%u drawPrim=%u drawPrimUP=%u\n"
           "             setRenderState=%u setTexture=%u\n"
           "             vs=%u ps=%u streamSource=%u indices=%u\n",
           s_nSwapPresent, s_nPresent, s_nBeginScene, s_nClear,
           s_nDrawIndexed, s_nDrawPrim, s_nDrawPrimUP,
           s_nSetRenderState, s_nSetTexture,
           s_nSetVertexShader, s_nSetPixelShader,
           s_nSetStreamSource, s_nSetIndices);
}

// ===========================================================================
// EGL / OpenGL presentation
// ===========================================================================
// Just enough of a backend to prove the swap chain: Clear becomes
// glClearColor + glClear, and the swap chain's Present becomes
// eglSwapBuffers. Nothing else is wired -- no shaders, no vertices, no
// textures -- so every draw call below still discards.
//
// initEgl/deinitEgl are the devkitPro simple_triangle example's
// ($DEVKITPRO/examples/switch/graphics/opengl/simple_triangle), kept as they
// are: desktop OpenGL core 4.3 on an NWindow surface. The example loads its
// entry points through glad; this Mesa build exports them from libGL, so
// there is no loader.
//
// THREAD OWNERSHIP. An EGL context belongs to the thread that made it
// current, and it cannot be released from any other thread, so it matters a
// great deal which thread gets here first.
//
// Clear and Present are always on the same thread as each other: both sit
// inside one RB_* sequence -- RB_BeginFrame, RB_CallExecuteRenderCommands
// (Clear), RB_EndFrame -> RB_SwapBuffers (Present). Which thread runs that
// sequence is decided by R_HandOffToBackend (r_rendercmds.cpp:493): with
// r_smp_backend and sys_smp_allowed both on it goes to RB_RenderThread, and
// with either off it runs inline on the caller. On this port both are on --
// r_smp_backend defaults to 1, and sys_smp_allowed defaults to CpuCount > 1
// where nx_wincompat's GetProcessAffinityMask reports 0x7, so three.
//
// The renderer is also *started* on the render thread, not the main one:
// Com_Init calls R_InitThreads, which spawns RB_RenderThread; CL_InitRenderer
// then signals the rgRegistered event, and the render thread picks it up and
// runs R_BeginRegistrationInternal -> R_Init -> R_InitGraphicsApi. So the
// device, the first R_ToggleSmpFrame and the first Clear are all on it.
//
// None of that is assumed here. The context is created on whichever thread
// gets here first and that thread is named in the log, so the first run says
// out loud which one it was. If a later call arrives from another thread --
// which would mean the engine changed modes underneath us -- it says so once
// instead of failing silently.
static EGLDisplay s_display;
static EGLContext s_context;
static EGLSurface s_surface;

static Mutex s_glLock;
static bool s_glReady;
static bool s_glFailed;
static bool s_glWrongThread;
static unsigned int s_glThreadId;

static const char *nxThreadName(void)
{
    if (Sys_IsRenderThread()) return "render";
    if (Sys_IsMainThread())   return "main";
    return "other";
}

static bool initEgl(NWindow *win)
{
    // The window first. Nothing in this tree calls consoleInit, gfxInitDefault
    // or framebufferCreate -- grep says so, and nx_main.cpp sends stdout to
    // nxlink or to sdmc:/switch/kisakblack/kisakblack.log, never to a console
    // layer -- so the default NWindow should be unowned and ours to bind. If
    // that ever stops being true this is where it shows: a console holds the
    // layer, and eglCreateWindowSurface below gets a window it cannot present
    // through.
    u32 winW = 0, winH = 0;
    Result rc = nwindowGetDimensions(win, &winW, &winH);
    printf("[nx-gl] NWindow %p (default %p) valid=%d dims=%ux%u rc=0x%x\n",
           (void *)win, (void *)nwindowGetDefault(),
           win ? (int)nwindowIsValid(win) : -1, winW, winH, (unsigned)rc);

    // The engine is hardwired to 1280x720 (GetViewport, the back buffer, the
    // depth surface). libnx would otherwise let the window inherit the layer
    // size, which is 1920x1080 docked.
    rc = nwindowSetDimensions(win, 1280, 720);
    if (R_FAILED(rc))
        printf("[nx-gl] nwindowSetDimensions(1280,720) failed: 0x%x\n", (unsigned)rc);
    nwindowGetDimensions(win, &winW, &winH);
    printf("[nx-gl] NWindow now %ux%u\n", winW, winH);
    fflush(stdout);

    // Connect to the EGL default display
    s_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (!s_display) {
        printf("[nx-gl] could not connect to display! error: %d\n", eglGetError());
        goto _fail0;
    }

    // Initialize the EGL display connection
    eglInitialize(s_display, nullptr, nullptr);

    // Select OpenGL (Core) as the desired graphics API
    if (eglBindAPI(EGL_OPENGL_API) == EGL_FALSE) {
        printf("[nx-gl] could not set API! error: %d\n", eglGetError());
        goto _fail1;
    }

    // Get an appropriate EGL framebuffer configuration
    EGLConfig config;
    EGLint numConfigs;
    static const EGLint framebufferAttributeList[] =
    {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_RED_SIZE,     8,
        EGL_GREEN_SIZE,   8,
        EGL_BLUE_SIZE,    8,
        EGL_ALPHA_SIZE,   8,
        EGL_DEPTH_SIZE,   24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };
    eglChooseConfig(s_display, framebufferAttributeList, &config, 1, &numConfigs);
    if (numConfigs == 0) {
        printf("[nx-gl] no config found! error: %d\n", eglGetError());
        goto _fail1;
    }

    // Create an EGL window surface
    s_surface = eglCreateWindowSurface(s_display, config, win, nullptr);
    if (!s_surface) {
        printf("[nx-gl] surface creation failed! error: %d\n", eglGetError());
        goto _fail1;
    }

    // Create an EGL rendering context
    static const EGLint contextAttributeList[] =
    {
        EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,
        EGL_CONTEXT_MAJOR_VERSION_KHR, 4,
        EGL_CONTEXT_MINOR_VERSION_KHR, 3,
        EGL_NONE
    };
    s_context = eglCreateContext(s_display, config, EGL_NO_CONTEXT, contextAttributeList);
    if (!s_context) {
        printf("[nx-gl] context creation failed! error: %d\n", eglGetError());
        goto _fail2;
    }

    // Connect the context to the surface. The example ignores the result;
    // here it decides whether GL is usable at all, so it is checked -- and so
    // is what EGL says is current afterwards, because a make-current that
    // reports success but binds nothing is exactly the shape of "EGL is up
    // and the screen never changes".
    if (eglMakeCurrent(s_display, s_surface, s_surface, s_context) == EGL_FALSE) {
        printf("[nx-gl] eglMakeCurrent failed! error: 0x%x\n", eglGetError());
        goto _fail3;
    }
    if (eglGetCurrentContext() != s_context
     || eglGetCurrentSurface(EGL_DRAW) != s_surface
     || eglGetCurrentSurface(EGL_READ) != s_surface) {
        printf("[nx-gl] eglMakeCurrent succeeded but bound something else: "
               "ctx %p (want %p) draw %p read %p (want %p)\n",
               eglGetCurrentContext(), s_context,
               eglGetCurrentSurface(EGL_DRAW), eglGetCurrentSurface(EGL_READ),
               s_surface);
        goto _fail3;
    }
    return true;

_fail3:
    eglDestroyContext(s_display, s_context);
    s_context = nullptr;

_fail2:
    eglDestroySurface(s_display, s_surface);
    s_surface = nullptr;
_fail1:
    eglTerminate(s_display);
    s_display = nullptr;
_fail0:
    return false;
}

static void deinitEgl(void)
{
    if (s_display) {
        eglMakeCurrent(s_display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (s_context) {
            eglDestroyContext(s_display, s_context);
            s_context = nullptr;
        }
        if (s_surface) {
            eglDestroySurface(s_display, s_surface);
            s_surface = nullptr;
        }
        eglTerminate(s_display);
        s_display = nullptr;
    }
}

// True when GL may be used from the calling thread. Creates the context on
// first use; after that, only the creating thread gets a yes.
static bool nxGlAcquire(void)
{
    unsigned int self = Sys_GetCurrentThreadId();

    if (!s_glReady && !s_glFailed) {
        mutexLock(&s_glLock);
        if (!s_glReady && !s_glFailed) {
            if (initEgl(nwindowGetDefault())) {
                s_glThreadId = self;
                s_glReady = true;
                EGLint sw = -1, sh = -1;
                eglQuerySurface(s_display, s_surface, EGL_WIDTH, &sw);
                eglQuerySurface(s_display, s_surface, EGL_HEIGHT, &sh);
                printf("[nx-gl] EGL up on the %s thread (id %u)\n"
                       "        vendor   : %s\n"
                       "        renderer : %s\n"
                       "        version  : %s\n"
                       "        surface  : %dx%d\n",
                       nxThreadName(), self,
                       (const char *)glGetString(GL_VENDOR),
                       (const char *)glGetString(GL_RENDERER),
                       (const char *)glGetString(GL_VERSION),
                       sw, sh);
            } else {
                s_glFailed = true;
                printf("[nx-gl] EGL init failed; staying headless\n");
            }
            fflush(stdout);
        }
        mutexUnlock(&s_glLock);
    }

    if (!s_glReady)
        return false;

    if (self != s_glThreadId) {
        if (!s_glWrongThread) {
            s_glWrongThread = true;
            printf("[nx-gl] the context belongs to thread %u, but the %s thread "
                   "(id %u) is presenting. An EGL context cannot be released "
                   "from another thread, so these calls are being dropped.\n",
                   s_glThreadId, nxThreadName(), self);
            fflush(stdout);
        }
        return false;
    }
    return true;
}
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
    GLuint glName;     // 0 until the first upload on the GL thread
    bool glDirty;      // bits changed since the last glBufferData
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
        // Only the owning thread may delete it. Anywhere else the name leaks,
        // which is the lesser of the two outcomes.
        if (b->glName && s_glReady && nxGlAcquire())
            glDeleteBuffers(1, &b->glName);
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
// geometry
// ===========================================================================
// One step past Clear: an indexed primitive on the screen, in flat colour.
//
// The game's own vertex and pixel shaders are D3D9 bytecode and translating
// them is a chapter of its own, so CreateVertexShader/CreatePixelShader keep
// handing out stubs and SetVertexShader/SetPixelShader keep discarding. In
// their place sits one hardwired GLSL program: a position in, a fixed colour
// out. What that proves is everything underneath the shading -- that vertex
// and index buffers reach the GPU with the right bytes, that the vertex
// declaration says where the position lives, and that the index run
// DrawIndexedPrimitive asks for is the run GL draws.
//
// THE TRANSFORM. D3D9 hands a vertex shader a matrix as four consecutive
// float4 constants, laid out so that `m4x4 oPos, v0, c0` means
// oPos.x = dot(v0, c0): c0 is the first row of a row-vector-times-matrix
// transform. glUniformMatrix4fv with transpose=GL_TRUE reads its sixteen
// floats in that same order, so the registers can go to GL untouched -- the
// row-major/column-major difference is paid for by one flag.
//
// Clip space does differ: D3D9 wants z in [0,w], GL wants [-w,w], so the
// shader ends with z = 2z - w. Clip-space y points up in both, and both map
// +1 to the top of the default framebuffer, so nothing is flipped; if the
// first geometry arrives upside down anyway, that assumption is the thing to
// doubt first.
//
// WHICH four registers hold the transform is not fixed. The dest register
// comes from the material's shader arguments (r_shade.cpp:217), assigned when
// the shader asset was built, so c0 here is a guess -- the usual answer, but a
// guess. That is where the 60-frame report earns its keep: it prints every
// four-register window that would land the last drawn vertex inside the NDC
// box, so if c0 is wrong the log names the alternatives instead of leaving a
// black screen to interpret.

// ---------------------------------------------------------------------------
// Render state
// ---------------------------------------------------------------------------
// SetRenderState used to be a counter and nothing else, which made every draw
// opaque: the last quad written to a pixel was the one that stayed, so a menu
// built out of stacked translucent panels showed only its topmost layer.
//
// The whole D3DRS file is recorded here -- it is one small array and the cost
// of keeping all of it is the same as keeping three -- but only the states
// that decide whether and how a draw reaches the framebuffer are acted on:
// the blend equation, its factors, and the colour write mask. Depth, stencil,
// culling and alpha test are still ignored on purpose, for the reason
// nxGlEnsurePipeline gives: nothing here fills a depth buffer or tracks a
// winding order yet, so honouring them could only discard geometry for
// reasons that have nothing to do with whether the geometry is right.
enum { NX_RS_COUNT = 256 };
static DWORD s_rs[NX_RS_COUNT];

// D3D9's own defaults, not zero. Zero is not a legal D3DBLEND, and a blend
// state that has never been set has to read as "opaque, source only" or the
// first draw before the engine touches these would vanish.
static void nxInitRenderStates(void)
{
    s_rs[D3DRS_ALPHABLENDENABLE] = FALSE;
    s_rs[D3DRS_SRCBLEND]  = D3DBLEND_ONE;
    s_rs[D3DRS_DESTBLEND] = D3DBLEND_ZERO;
    s_rs[D3DRS_BLENDOP]   = D3DBLENDOP_ADD;
    s_rs[D3DRS_SEPARATEALPHABLENDENABLE] = FALSE;
    s_rs[D3DRS_SRCBLENDALPHA]  = D3DBLEND_ONE;
    s_rs[D3DRS_DESTBLENDALPHA] = D3DBLEND_ZERO;
    s_rs[D3DRS_BLENDOPALPHA]   = D3DBLENDOP_ADD;
    s_rs[D3DRS_BLENDFACTOR]    = 0xFFFFFFFFu;
    s_rs[D3DRS_COLORWRITEENABLE] = 0xF;   // all four channels
}

// BLENDFACTOR and INVBLENDFACTOR need glBlendColor, which is one piece of
// state for the whole equation rather than per-factor, so it is set from
// D3DRS_BLENDFACTOR whenever blending is on and costs nothing when unused.
static GLenum nxBlendToGl(DWORD b)
{
    switch (b) {
    case D3DBLEND_ZERO:            return GL_ZERO;
    case D3DBLEND_ONE:             return GL_ONE;
    case D3DBLEND_SRCCOLOR:        return GL_SRC_COLOR;
    case D3DBLEND_INVSRCCOLOR:     return GL_ONE_MINUS_SRC_COLOR;
    case D3DBLEND_SRCALPHA:        return GL_SRC_ALPHA;
    case D3DBLEND_INVSRCALPHA:     return GL_ONE_MINUS_SRC_ALPHA;
    case D3DBLEND_DESTALPHA:       return GL_DST_ALPHA;
    case D3DBLEND_INVDESTALPHA:    return GL_ONE_MINUS_DST_ALPHA;
    case D3DBLEND_DESTCOLOR:       return GL_DST_COLOR;
    case D3DBLEND_INVDESTCOLOR:    return GL_ONE_MINUS_DST_COLOR;
    case D3DBLEND_SRCALPHASAT:     return GL_SRC_ALPHA_SATURATE;
    case D3DBLEND_BLENDFACTOR:     return GL_CONSTANT_COLOR;
    case D3DBLEND_INVBLENDFACTOR:  return GL_ONE_MINUS_CONSTANT_COLOR;
    default:                       return GL_ONE;
    }
}

static GLenum nxBlendOpToGl(DWORD op)
{
    switch (op) {
    case D3DBLENDOP_SUBTRACT:    return GL_FUNC_SUBTRACT;
    case D3DBLENDOP_REVSUBTRACT: return GL_FUNC_REVERSE_SUBTRACT;
    case D3DBLENDOP_MIN:         return GL_MIN;
    case D3DBLENDOP_MAX:         return GL_MAX;
    default:                     return GL_FUNC_ADD;
    }
}

// Applied per draw rather than inside SetRenderState, because SetRenderState
// can arrive on any thread and only the draw path is guaranteed to hold the
// context. It is a handful of redundant GL calls per draw; correctness first,
// and a state cache is a later, separate change.
static void nxGlApplyBlend(void)
{
    DWORD mask = s_rs[D3DRS_COLORWRITEENABLE];
    glColorMask((mask & 1) ? GL_TRUE : GL_FALSE,    // RED
                (mask & 2) ? GL_TRUE : GL_FALSE,    // GREEN
                (mask & 4) ? GL_TRUE : GL_FALSE,    // BLUE
                (mask & 8) ? GL_TRUE : GL_FALSE);   // ALPHA

    if (!s_rs[D3DRS_ALPHABLENDENABLE]) {
        glDisable(GL_BLEND);
        return;
    }
    glEnable(GL_BLEND);

    GLenum src = nxBlendToGl(s_rs[D3DRS_SRCBLEND]);
    GLenum dst = nxBlendToGl(s_rs[D3DRS_DESTBLEND]);
    GLenum op  = nxBlendOpToGl(s_rs[D3DRS_BLENDOP]);

    // With the separate-alpha switch off, D3D9 runs the colour factors on the
    // alpha channel too; glBlendFuncSeparate has no such mode, so the same
    // pair is passed twice.
    GLenum srcA = src, dstA = dst, opA = op;
    if (s_rs[D3DRS_SEPARATEALPHABLENDENABLE]) {
        srcA = nxBlendToGl(s_rs[D3DRS_SRCBLENDALPHA]);
        dstA = nxBlendToGl(s_rs[D3DRS_DESTBLENDALPHA]);
        opA  = nxBlendOpToGl(s_rs[D3DRS_BLENDOPALPHA]);
    }
    glBlendFuncSeparate(src, dst, srcA, dstA);
    glBlendEquationSeparate(op, opA);

    // Same ARGB packing Clear reads, for the same reason.
    DWORD f = s_rs[D3DRS_BLENDFACTOR];
    glBlendColor(((f >> 16) & 0xff) / 255.0f,
                 ((f >>  8) & 0xff) / 255.0f,
                 ((f      ) & 0xff) / 255.0f,
                 ((f >> 24) & 0xff) / 255.0f);
}

// Vertex shader constant registers, recorded as the engine sets them. Only the
// float file is kept: the transform is all this step needs from it.
enum { NX_VS_CONST_ROWS = 256 };
static float s_vsConst[NX_VS_CONST_ROWS][4];
static bool s_vsConstWritten[NX_VS_CONST_ROWS];
static unsigned s_vsConstBase;   // the register quad used as the transform

// c0 was only ever a guess, and the comment above says why: the register the
// transform lands in comes from the material's shader arguments, not from any
// fixed convention. Two things narrow it down without guessing.
//
// The first is generic. A matrix constant reaches the device as one
// SetVertexShaderConstantF of four rows (r_shade.cpp:269 uploads
// routingData->u.codeConst.rowCount rows in a single call), while every
// non-matrix code constant is a one-row write. So the start register of the
// most recent four-row write is where the engine last put a matrix.
static bool s_vsMatrixSeen;
static unsigned s_vsMatrixBase;    // start register of the last 4-row write
static unsigned s_nVsMatrixWrites;

// The second is exact, and only works for the 2D view -- which is all that is
// on screen right now. R_CmdBufSet2D (r_state_utils.cpp:370) builds the UI
// projection by hand:
//
//   m[0][0] = 2/W      m[1][1] = -2/H     m[3][2] = 1     m[3][3] = 1
//   m[3][0] = -1 - 1/W                    m[3][1] = 1 + 1/H
//
// and everything else is zero. HLSL packs a matrix constant column by column,
// so mul(pos, m) compiles to four dp4s, one per register, and register i ends
// up holding COLUMN i of that matrix:
//
//   base+0  ( 2/W,    0,  0,  -1 - 1/W )
//   base+1  (   0, -2/H,  0,   1 + 1/H )
//   base+2  (   0,    0,  0,         1 )
//   base+3  (   0,    0,  0,         1 )
//
// Two registers that are (0,0,0,1) back to back is already a rare shape, and
// the agreement between the 2/W in the first slot and the -1-1/W in the last
// pins it down completely -- W and H fall out of the match rather than having
// to be known in advance, so this needs nothing from the viewport.
static int s_vs2dBase = -1;
static float s_vs2dWidth, s_vs2dHeight;

static bool nxNear(float a, float b, float tol)
{
    float d = a - b;
    return (d < 0 ? -d : d) <= tol;
}

static bool nxLooksLike2dProjection(unsigned base, float *outW, float *outH)
{
    if (base + 4 > NX_VS_CONST_ROWS)
        return false;
    for (unsigned i = 0; i < 4; ++i)
        if (!s_vsConstWritten[base + i])
            return false;

    const float *c0 = s_vsConst[base + 0];
    const float *c1 = s_vsConst[base + 1];
    const float *c2 = s_vsConst[base + 2];
    const float *c3 = s_vsConst[base + 3];

    // The two constant columns first: cheapest, and they reject almost
    // everything else in the file on their own.
    if (!nxNear(c2[0], 0.0f, 1e-6f) || !nxNear(c2[1], 0.0f, 1e-6f)
     || !nxNear(c2[2], 0.0f, 1e-6f) || !nxNear(c2[3], 1.0f, 1e-6f))
        return false;
    if (!nxNear(c3[0], 0.0f, 1e-6f) || !nxNear(c3[1], 0.0f, 1e-6f)
     || !nxNear(c3[2], 0.0f, 1e-6f) || !nxNear(c3[3], 1.0f, 1e-6f))
        return false;

    if (!nxNear(c0[1], 0.0f, 1e-6f) || !nxNear(c0[2], 0.0f, 1e-6f))
        return false;
    if (!nxNear(c1[0], 0.0f, 1e-6f) || !nxNear(c1[2], 0.0f, 1e-6f))
        return false;

    float sx = c0[0];    // 2/W, so a viewport wider than one pixel
    float sy = c1[1];    // -2/H, negative because D3D9's 2D y points down
    if (!(sx > 0.0f) || !(sy < 0.0f))
        return false;
    if (!nxNear(c0[3], -1.0f - sx * 0.5f, 1e-4f))
        return false;
    if (!nxNear(c1[3],  1.0f - sy * 0.5f, 1e-4f))
        return false;

    if (outW) *outW = 2.0f / sx;
    if (outH) *outH = -2.0f / sy;
    return true;
}

enum { NX_MAX_STREAMS = 4 };
static NxBuffer *s_streamVB[NX_MAX_STREAMS];
static UINT s_streamOffset[NX_MAX_STREAMS];
static UINT s_streamStride[NX_MAX_STREAMS];
static NxBuffer *s_indexBuf;
static NxVDecl *s_vdecl;

static GLuint s_prog, s_vao;
static GLint s_locTransform = -1;
static bool s_progFailed;
enum { NX_ATTR_POS = 0, NX_ATTR_COLOR = 1 };

// Draw-path tally. The skip counters matter as much as the success one: a
// report that says ok=0 is useless without the reason, and every early return
// in nxGlDrawIndexed has its own.
static unsigned s_nGlDrawOk, s_nGlDrawFailed;
static unsigned s_nSkipNoGl, s_nSkipNoProgram, s_nSkipNoDecl, s_nSkipNoPos;
static unsigned s_nSkipNoBuffer, s_nSkipBadType;
static GLenum s_lastGlDrawError;

// How the colour attribute resolved, per draw. The split matters: a frame
// that is entirely s_nColorDefault is a frame still drawing in flat white,
// and that is a declaration problem, not a blending one.
static unsigned s_nColorAttrib, s_nColorDefault;

// The shape of the last draw that made it through, for the report.
static BYTE s_lastDrawColorType;
static bool s_lastDrawHadColor;
static GLenum s_lastDrawMode;
static GLsizei s_lastDrawCount;
static GLenum s_lastDrawIdxType;
static UINT s_lastDrawStride, s_lastDrawPosOffset;
static BYTE s_lastDrawPosType;

// Four components now, not three, and the fourth watched across the whole
// frame rather than sampled once. R_SetVertex2d (rb_backend.cpp:378) writes
// xyzw[3] = 1 for an ordinary UI quad, but RB_DrawStretchPicW writes a real w
// per vertex, so a span that is not exactly 1..1 says which of the two the
// menu is actually built from.
static float s_lastDrawPos[4];
static int s_lastDrawPosComponents;
static bool s_lastDrawPosValid;
static float s_frameMinPosW, s_frameMaxPosW;
static bool s_framePosWSeen;

// One frame's worth of every draw, not one vertex out of the last of them.
// A sampled vertex landing on screen says nothing about the other thousand
// draws in the frame; these say whether the frame as a whole does.
//
// Transform source splits by the three branches nxGlDrawIndexed picks from.
// The box is in NDC over every index the frame drew, transformed on the CPU
// with the same registers the shader got: a box that is a speck in the
// middle means most draws collapse and the sampled vertex was lucky, a box
// that spans [-1,1] means the geometry is where it belongs and whatever is
// missing is missing in colour. z is the D3D value (clip z / w, 0..1), kept
// because 1.0 is exactly GL's far plane after the conversion.
//
// The alpha figures come from the raw D3DCOLOR bytes in the vertex buffer,
// not from anything GL decoded, so they hold whichever way the swizzle is.
struct NxFrameStats {
    unsigned draws, via2d, viaMatrix, viaDefault;
    unsigned drawsBlended;
    unsigned drawsNoReadback;          // non-float position: not in the box
    unsigned refs, refsBehind, refsInside, refsOutOfRange;
    float minX, maxX, minY, maxY, minZ, maxZ;
    bool boxSeen;
    unsigned colRefs, alphaZero, alphaLow, alphaFull;   // low: 1..31
    unsigned alphaMin, alphaMax;
    unsigned alphaZeroBlended;         // alpha 0 in a draw that blends
    // The same byte-order check, done on byte 0 instead of byte 3: if
    // "alpha" lands in 200..255 here and not up there, the channels are
    // reordered; if neither end is plausible, they are not colour at all.
    // Eight buckets of 32.
    unsigned alphaHiHist[8], alphaLoHist[8];
};
// Cleared at every swap-chain Present, which prints it first when the
// report is due -- so the figures are always the frame just finished.
static NxFrameStats s_frame;

// First vertex of the last draw with a colour element, raw.
static DWORD s_lastDrawColorRaw;
static bool s_lastDrawColorRawValid;

static void nxTransformPoint(const float *regs, const float *p, float *out);

// Walks the index run a draw just used and folds every vertex it referenced
// into s_frame. The same bytes GL read, from the same client-side copies the
// buffers were uploaded from, so this sees what the GPU saw.
static void nxFrameAccumulate(const NxBuffer *ib, UINT idxSize, UINT startIndex,
                              GLsizei count, INT baseVertexIndex,
                              const float *regs,
                              const NxBuffer *vb, UINT posStride, UINT posOffset,
                              int posComps, bool posIsFloat,
                              const NxBuffer *cb, UINT colStride, UINT colOffset,
                              bool blended)
{
    for (GLsizei i = 0; i < count; ++i) {
        size_t ioff = ((size_t)startIndex + (size_t)i) * idxSize;
        if (ioff + idxSize > (size_t)ib->length) { ++s_frame.refsOutOfRange; break; }
        const BYTE *ip = (const BYTE *)ib->bits + ioff;
        UINT idx;
        if (idxSize == 4) { uint32_t v; memcpy(&v, ip, 4); idx = v; }
        else              { uint16_t v; memcpy(&v, ip, 2); idx = v; }
        long long vert = (long long)baseVertexIndex + (long long)idx;
        if (vert < 0) { ++s_frame.refsOutOfRange; continue; }

        if (posIsFloat) {
            size_t off = (size_t)vert * posStride + posOffset;
            if (off + (size_t)posComps * sizeof(float) > (size_t)vb->length) {
                ++s_frame.refsOutOfRange;
            } else {
                float p[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
                memcpy(p, (const BYTE *)vb->bits + off, (size_t)posComps * sizeof(float));
                // The shader drops the fourth component and uses 1; so does this.
                p[3] = 1.0f;
                float c[4];
                nxTransformPoint(regs, p, c);
                ++s_frame.refs;
                if (!(c[3] > 0.0f)) {
                    ++s_frame.refsBehind;
                } else {
                    float x = c[0] / c[3], y = c[1] / c[3], z = c[2] / c[3];
                    if (!s_frame.boxSeen) {
                        s_frame.minX = s_frame.maxX = x;
                        s_frame.minY = s_frame.maxY = y;
                        s_frame.minZ = s_frame.maxZ = z;
                        s_frame.boxSeen = true;
                    } else {
                        if (x < s_frame.minX) s_frame.minX = x;
                        if (x > s_frame.maxX) s_frame.maxX = x;
                        if (y < s_frame.minY) s_frame.minY = y;
                        if (y > s_frame.maxY) s_frame.maxY = y;
                        if (z < s_frame.minZ) s_frame.minZ = z;
                        if (z > s_frame.maxZ) s_frame.maxZ = z;
                    }
                    if (x >= -1.0f && x <= 1.0f && y >= -1.0f && y <= 1.0f)
                        ++s_frame.refsInside;
                }
            }
        }

        if (cb) {
            size_t off = (size_t)vert * colStride + colOffset;
            if (off + 4 <= (size_t)cb->length) {
                DWORD raw;
                memcpy(&raw, (const BYTE *)cb->bits + off, 4);
                unsigned a = raw >> 24;   // D3DCOLOR is 0xAARRGGBB
                ++s_frame.alphaHiHist[a >> 5];
                ++s_frame.alphaLoHist[(raw & 0xff) >> 5];
                if (!s_frame.colRefs) s_frame.alphaMin = s_frame.alphaMax = a;
                if (a < s_frame.alphaMin) s_frame.alphaMin = a;
                if (a > s_frame.alphaMax) s_frame.alphaMax = a;
                ++s_frame.colRefs;
                if (a == 0) {
                    ++s_frame.alphaZero;
                    if (blended) ++s_frame.alphaZeroBlended;
                } else if (a < 32) {
                    ++s_frame.alphaLow;
                } else if (a == 255) {
                    ++s_frame.alphaFull;
                }
            }
        }
    }
}

// The colour attribute arrives already in the right order and range: a
// D3DCOLOR is four bytes read back as normalized BGRA (see nxDeclTypeToGl),
// so by the time it reaches the shader it is an ordinary vec4 RGBA in 0..1.
//
// A declaration with no COLOR element leaves the attribute array disabled and
// the constant white that nxGlDrawIndexed sets stands in. White is the
// identity for the modulate a texture will bring next, so that fallback keeps
// working unchanged once sampling lands.
static const char *s_vsSrc =
    "#version 330 core\n"
    "layout(location = 0) in vec4 nxPos;\n"
    "layout(location = 1) in vec4 nxColor;\n"
    "uniform mat4 nxTransform;\n"
    "out vec4 vColor;\n"
    "void main() {\n"
    "    vec4 p = nxTransform * vec4(nxPos.xyz, 1.0);\n"
    "    p.z = 2.0 * p.z - p.w;\n"          // D3D9 clip z [0,w] -> GL [-w,w]
    "    gl_Position = p;\n"
    "    vColor = nxColor;\n"
    "}\n";

static const char *s_fsSrc =
    "#version 330 core\n"
    "in vec4 vColor;\n"
    "out vec4 nxFrag;\n"
    "void main() { nxFrag = vColor; }\n";

static GLuint nxGlCompile(GLenum type, const char *src, const char *what)
{
    GLuint sh = glCreateShader(type);
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    GLint ok = GL_FALSE;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        log[0] = 0;
        glGetShaderInfoLog(sh, sizeof(log) - 1, nullptr, log);
        printf("[nx-gl] %s shader did not compile: %s\n", what, log);
        fflush(stdout);
        glDeleteShader(sh);
        return 0;
    }
    return sh;
}

// The one-time setup the draw path needs. It lives here rather than in initEgl
// because it has to run on the thread that owns the context, after nxGlAcquire
// has said this is that thread.
static bool nxGlEnsurePipeline(void)
{
    if (s_prog) return true;
    if (s_progFailed) return false;

    GLuint vs = nxGlCompile(GL_VERTEX_SHADER, s_vsSrc, "vertex");
    GLuint fs = vs ? nxGlCompile(GL_FRAGMENT_SHADER, s_fsSrc, "fragment") : 0;
    if (!vs || !fs) {
        if (vs) glDeleteShader(vs);
        s_progFailed = true;
        return false;
    }

    s_prog = glCreateProgram();
    glAttachShader(s_prog, vs);
    glAttachShader(s_prog, fs);
    glBindAttribLocation(s_prog, NX_ATTR_POS, "nxPos");
    glBindAttribLocation(s_prog, NX_ATTR_COLOR, "nxColor");
    glLinkProgram(s_prog);
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint linked = GL_FALSE;
    glGetProgramiv(s_prog, GL_LINK_STATUS, &linked);
    if (!linked) {
        char log[1024];
        log[0] = 0;
        glGetProgramInfoLog(s_prog, sizeof(log) - 1, nullptr, log);
        printf("[nx-gl] flat-colour program did not link: %s\n", log);
        fflush(stdout);
        glDeleteProgram(s_prog);
        s_prog = 0;
        s_progFailed = true;
        return false;
    }
    s_locTransform = glGetUniformLocation(s_prog, "nxTransform");

    // The core profile refuses to draw with no vertex array object bound, and
    // nothing else here ever binds one, so this one stays current for good.
    glGenVertexArrays(1, &s_vao);
    glBindVertexArray(s_vao);

    // Every one of these is still state this file discards: no depth buffer is
    // ever filled, no winding order is tracked, SetScissorRect is a no-op. So
    // leaving them on could only throw the geometry away for reasons that have
    // nothing to do with whether the geometry arrived.
    //
    // GL_BLEND has left this list: nxGlApplyBlend now sets it per draw, from
    // the state the engine actually asked for.
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);

    // SetViewport is a no-op too, so nobody has called glViewport. The surface
    // is the only viewport that means anything at this stage.
    EGLint sw = 0, sh = 0;
    eglQuerySurface(s_display, s_surface, EGL_WIDTH, &sw);
    eglQuerySurface(s_display, s_surface, EGL_HEIGHT, &sh);
    glViewport(0, 0, sw, sh);

    printf("[nx-gl] vertex-colour pipeline up: program %u vao %u "
           "nxTransform at %d, viewport %dx%d\n",
           s_prog, s_vao, s_locTransform, sw, sh);
    fflush(stdout);
    return true;
}

// Uploads a buffer's bytes if they changed, creating its GL name on first use.
// Both halves need the context, so this only works on the GL thread: Unlock
// calls it when it happens to be there, and the draw path -- which always is --
// picks up whatever is still dirty.
static bool nxGlSyncBuffer(NxBuffer *b, GLenum target)
{
    if (!b || !b->bits || !b->length)
        return false;
    if (!b->glName) {
        glGenBuffers(1, &b->glName);
        if (!b->glName)
            return false;
        b->glDirty = true;
    }
    glBindBuffer(target, b->glName);
    if (b->glDirty) {
        glBufferData(target, (GLsizeiptr)b->length, b->bits,
                     (b->usage & D3DUSAGE_DYNAMIC) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
        b->glDirty = false;
    }
    return true;
}

// Called from Unlock. A buffer the engine filled off the GL thread cannot be
// uploaded from there; marking it dirty is enough, because the draw that needs
// it runs on the right thread and syncs it then.
static void nxGlBufferDirty(NxBuffer *b, GLenum target)
{
    b->glDirty = true;
    if (nxGlAcquire())
        nxGlSyncBuffer(b, target);
}

struct NxAttrFormat {
    GLint size;
    GLenum type;
    GLboolean normalized;
};

#define NX_ATTR_FMT(sz, ty, nrm) \
    do { out->size = (sz); out->type = (ty); out->normalized = (nrm); return true; } while (0)

static bool nxDeclTypeToGl(BYTE type, NxAttrFormat *out)
{
    switch (type) {
    case D3DDECLTYPE_FLOAT1:    NX_ATTR_FMT(1, GL_FLOAT, GL_FALSE);
    case D3DDECLTYPE_FLOAT2:    NX_ATTR_FMT(2, GL_FLOAT, GL_FALSE);
    case D3DDECLTYPE_FLOAT3:    NX_ATTR_FMT(3, GL_FLOAT, GL_FALSE);
    case D3DDECLTYPE_FLOAT4:    NX_ATTR_FMT(4, GL_FLOAT, GL_FALSE);
    case D3DDECLTYPE_FLOAT16_2: NX_ATTR_FMT(2, GL_HALF_FLOAT, GL_FALSE);
    case D3DDECLTYPE_FLOAT16_4: NX_ATTR_FMT(4, GL_HALF_FLOAT, GL_FALSE);
    case D3DDECLTYPE_SHORT2:    NX_ATTR_FMT(2, GL_SHORT, GL_FALSE);
    case D3DDECLTYPE_SHORT4:    NX_ATTR_FMT(4, GL_SHORT, GL_FALSE);
    case D3DDECLTYPE_SHORT2N:   NX_ATTR_FMT(2, GL_SHORT, GL_TRUE);
    case D3DDECLTYPE_SHORT4N:   NX_ATTR_FMT(4, GL_SHORT, GL_TRUE);
    case D3DDECLTYPE_USHORT2N:  NX_ATTR_FMT(2, GL_UNSIGNED_SHORT, GL_TRUE);
    case D3DDECLTYPE_USHORT4N:  NX_ATTR_FMT(4, GL_UNSIGNED_SHORT, GL_TRUE);
    case D3DDECLTYPE_UBYTE4:    NX_ATTR_FMT(4, GL_UNSIGNED_BYTE, GL_FALSE);
    case D3DDECLTYPE_UBYTE4N:   NX_ATTR_FMT(4, GL_UNSIGNED_BYTE, GL_TRUE);
    // D3DCOLOR is four bytes in B,G,R,A order, normalized. GL_BGRA is not a
    // component count -- it is the one legal non-numeric `size`, and it means
    // exactly that reordering, which is why the swizzle costs nothing in the
    // shader. It has been core since 3.2 (ARB_vertex_array_bgra) and the rule
    // that comes with it is that `normalized` must be GL_TRUE, as it is here.
    //
    // This mapping is right and is the one place the order is decided: the
    // engine packs vertex colour through Byte4PackVertexColor, which writes
    // B, G, R, A -- the PC layout, and on a little-endian target no swap is
    // owed on top of it. When the menu once came through with alpha 1..31
    // almost everywhere, the bytes were not reordered but garbage: that
    // function had been compiled to fill three of the four from the stack
    // (see the comment on it in fx_convert.cpp). Swizzling here to match
    // would have hidden a broken packer behind a wrong declaration.
    case D3DDECLTYPE_D3DCOLOR:  NX_ATTR_FMT(GL_BGRA, GL_UNSIGNED_BYTE, GL_TRUE);
    default: return false;   // UDEC3, DEC3N: nothing bound here uses them
    }
}
#undef NX_ATTR_FMT

static bool nxPrimToGl(D3DPRIMITIVETYPE type, UINT primCount, GLenum *mode, GLsizei *count)
{
    if (!primCount)
        return false;
    switch (type) {
    case D3DPT_POINTLIST:     *mode = GL_POINTS;         *count = (GLsizei)primCount;     return true;
    case D3DPT_LINELIST:      *mode = GL_LINES;          *count = (GLsizei)primCount * 2; return true;
    case D3DPT_LINESTRIP:     *mode = GL_LINE_STRIP;     *count = (GLsizei)primCount + 1; return true;
    case D3DPT_TRIANGLELIST:  *mode = GL_TRIANGLES;      *count = (GLsizei)primCount * 3; return true;
    case D3DPT_TRIANGLESTRIP: *mode = GL_TRIANGLE_STRIP; *count = (GLsizei)primCount + 2; return true;
    case D3DPT_TRIANGLEFAN:   *mode = GL_TRIANGLE_FAN;   *count = (GLsizei)primCount + 2; return true;
    default: return false;
    }
}

static const D3DVERTEXELEMENT9 *nxFindUsage(const NxVDecl *d, BYTE usage, BYTE usageIndex)
{
    for (UINT i = 0; i < d->count; ++i) {
        if (d->elements[i].Usage == usage
         && d->elements[i].UsageIndex == usageIndex)
            return &d->elements[i];
    }
    return nullptr;
}

// Binds one declaration element as a vertex attribute, syncing the stream it
// lives in. Returns false when the element is missing, in a stream this file
// does not track, of a type with no GL equivalent, or backed by no buffer --
// every one of which the caller answers by leaving the attribute disabled.
static bool nxGlBindAttrib(GLuint attr, const D3DVERTEXELEMENT9 *elem)
{
    if (!elem || elem->Stream >= NX_MAX_STREAMS)
        return false;

    NxAttrFormat fmt;
    if (!nxDeclTypeToGl(elem->Type, &fmt))
        return false;

    NxBuffer *vb = s_streamVB[elem->Stream];
    if (!nxGlSyncBuffer(vb, GL_ARRAY_BUFFER))
        return false;

    // glVertexAttribPointer takes its buffer from whatever is bound to
    // GL_ARRAY_BUFFER, which nxGlSyncBuffer has just left as this stream's.
    UINT offset = s_streamOffset[elem->Stream] + elem->Offset;
    glEnableVertexAttribArray(attr);
    glVertexAttribPointer(attr, fmt.size, fmt.type, fmt.normalized,
                          (GLsizei)s_streamStride[elem->Stream],
                          (const void *)(uintptr_t)offset);
    return true;
}

static void nxGlDrawIndexed(D3DPRIMITIVETYPE type, INT baseVertexIndex,
                            UINT startIndex, UINT primCount)
{
    if (!nxGlAcquire())        { ++s_nSkipNoGl;      return; }
    if (!nxGlEnsurePipeline()) { ++s_nSkipNoProgram; return; }
    if (!s_vdecl)              { ++s_nSkipNoDecl;    return; }

    // Position and colour. Normals, tangents and blend weights still have
    // nothing to feed; texcoords are the next thing to arrive here.
    const D3DVERTEXELEMENT9 *pos = nxFindUsage(s_vdecl, D3DDECLUSAGE_POSITION, 0);
    if (!pos || pos->Stream >= NX_MAX_STREAMS) { ++s_nSkipNoPos; return; }

    NxAttrFormat fmt;
    if (!nxDeclTypeToGl(pos->Type, &fmt)) { ++s_nSkipBadType; return; }

    GLenum mode;
    GLsizei count;
    if (!nxPrimToGl(type, primCount, &mode, &count)) { ++s_nSkipBadType; return; }

    NxBuffer *vb = s_streamVB[pos->Stream];
    NxBuffer *ib = s_indexBuf;
    if (!vb || !ib) { ++s_nSkipNoBuffer; return; }

    GLenum idxType = GL_UNSIGNED_SHORT;
    UINT idxSize = 2;
    if (ib->format == D3DFMT_INDEX32) { idxType = GL_UNSIGNED_INT; idxSize = 4; }

    if (!nxGlSyncBuffer(ib, GL_ELEMENT_ARRAY_BUFFER)) { ++s_nSkipNoBuffer; return; }

    // Which registers hold the transform, best answer first: the window that
    // matches the 2D projection exactly, else wherever the engine last wrote
    // a four-row matrix, else the old c0 guess. Everything on screen at this
    // stage is the menu, so the 2D window being preferred is a help and not
    // yet a lie; the first frame with a world in it will have to pick per
    // draw instead, from the technique the material selected.
    s_vsConstBase = s_vs2dBase >= 0 ? (unsigned)s_vs2dBase
                  : s_vsMatrixSeen  ? s_vsMatrixBase
                                    : 0u;

    glUseProgram(s_prog);
    if (s_locTransform >= 0) {
        // transpose=GL_TRUE makes register r mathematical row r of the mat4,
        // so GLSL's nxTransform * p is dot(register r, p) per component --
        // the same arithmetic nxTransformPoint does for the report, and the
        // same four dp4s the HLSL original compiled to.
        glUniformMatrix4fv(s_locTransform, 1, GL_TRUE, &s_vsConst[s_vsConstBase][0]);
    }

    // The two attributes may live in different streams, and each of these
    // calls binds its own before handing the pointer over, so the order they
    // go in does not matter: glVertexAttribPointer captures the buffer that
    // is bound at its own call, not at draw time.
    const D3DVERTEXELEMENT9 *col = nxFindUsage(s_vdecl, D3DDECLUSAGE_COLOR, 0);
    if (nxGlBindAttrib(NX_ATTR_COLOR, col)) {
        ++s_nColorAttrib;
        s_lastDrawColorType = col->Type;
        s_lastDrawHadColor = true;
    } else {
        // No usable COLOR element. The disabled array reads back as the
        // current generic attribute, so white it is -- see s_vsSrc.
        ++s_nColorDefault;
        glDisableVertexAttribArray(NX_ATTR_COLOR);
        glVertexAttrib4f(NX_ATTR_COLOR, 1.0f, 1.0f, 1.0f, 1.0f);
        s_lastDrawHadColor = false;
    }

    if (!nxGlBindAttrib(NX_ATTR_POS, pos)) { ++s_nSkipNoBuffer; return; }

    nxGlApplyBlend();

    UINT stride = s_streamStride[pos->Stream];
    UINT attrOffset = s_streamOffset[pos->Stream] + pos->Offset;

    glGetError();   // clear anything stale, so the reading below is this draw's
    glDrawElementsBaseVertex(mode, count, idxType,
                             (const void *)(uintptr_t)(startIndex * idxSize),
                             baseVertexIndex);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        s_lastGlDrawError = err;
        ++s_nGlDrawFailed;
        return;
    }
    ++s_nGlDrawOk;

    ++s_frame.draws;
    if (s_vs2dBase >= 0)     ++s_frame.via2d;
    else if (s_vsMatrixSeen) ++s_frame.viaMatrix;
    else                     ++s_frame.viaDefault;
    bool blended = s_rs[D3DRS_ALPHABLENDENABLE] != 0;
    if (blended) ++s_frame.drawsBlended;
    if (fmt.type != GL_FLOAT) ++s_frame.drawsNoReadback;

    // Only D3DCOLOR is read raw: it is all the UI declares, and its byte
    // order is the question.
    const NxBuffer *cb = nullptr;
    UINT colStride = 0, colOffset = 0;
    if (s_lastDrawHadColor && col->Type == D3DDECLTYPE_D3DCOLOR) {
        cb = s_streamVB[col->Stream];
        colStride = s_streamStride[col->Stream];
        colOffset = s_streamOffset[col->Stream] + col->Offset;
        if (cb && cb->bits) {
            UINT v = (UINT)(baseVertexIndex > 0 ? baseVertexIndex : 0);
            size_t off = (size_t)v * colStride + colOffset;
            s_lastDrawColorRawValid = off + 4 <= (size_t)cb->length;
            if (s_lastDrawColorRawValid)
                memcpy(&s_lastDrawColorRaw, (const BYTE *)cb->bits + off, 4);
        } else {
            cb = nullptr;
        }
    }
    if (ib->bits && vb->bits)
        nxFrameAccumulate(ib, idxSize, startIndex, count, baseVertexIndex,
                          &s_vsConst[s_vsConstBase][0],
                          vb, stride, attrOffset, fmt.size, fmt.type == GL_FLOAT,
                          cb, colStride, colOffset, blended);

    s_lastDrawMode = mode;
    s_lastDrawCount = count;
    s_lastDrawIdxType = idxType;
    s_lastDrawStride = stride;
    s_lastDrawPosOffset = attrOffset;
    s_lastDrawPosType = pos->Type;

    // One vertex, kept so the report can say where the transform puts it. Only
    // the float forms are read back: a packed position would need unpacking
    // that nothing else here needs.
    s_lastDrawPosValid = false;
    if (fmt.type == GL_FLOAT && stride) {
        UINT v = (UINT)(baseVertexIndex > 0 ? baseVertexIndex : 0);
        size_t byteOff = (size_t)v * stride + attrOffset;
        int comps = fmt.size;
        if (byteOff + (size_t)comps * sizeof(float) <= (size_t)vb->length) {
            s_lastDrawPos[0] = s_lastDrawPos[1] = s_lastDrawPos[2] = 0.0f;
            // The missing components read as the homogeneous default, so a
            // FLOAT2 or FLOAT3 declaration still transforms correctly.
            s_lastDrawPos[3] = 1.0f;
            memcpy(s_lastDrawPos, (const BYTE *)vb->bits + byteOff,
                   (size_t)comps * sizeof(float));
            s_lastDrawPosComponents = comps;
            s_lastDrawPosValid = true;

            float w = s_lastDrawPos[3];
            if (!s_framePosWSeen) {
                s_frameMinPosW = s_frameMaxPosW = w;
                s_framePosWSeen = true;
            } else {
                if (w < s_frameMinPosW) s_frameMinPosW = w;
                if (w > s_frameMaxPosW) s_frameMaxPosW = w;
            }
        }
    }
}

// One clip component per register: component r is register r dotted with the
// position, w included.
//
// This used to read the four registers as the ROWS of a matrix and multiply a
// row vector by it, which is the transpose of what the shader does -- and the
// shader is the one that is right. glUniformMatrix4fv is given transpose=TRUE,
// so register r becomes mathematical row r of nxTransform, and GLSL's
// nxTransform * p is then exactly dot(register r, p) per component. That is
// also what the constant file holds: HLSL packs a matrix column by column and
// mul(pos, m) compiles to one dp4 per register, so a register IS a column of
// the D3D matrix and dotting it with the position is the whole operation.
//
// While the two disagreed, every clip and ndc figure this report printed --
// and every candidate it ruled in or out -- described a draw that never
// happened.
static void nxTransformPoint(const float *regs, const float *p, float *out)
{
    for (int r = 0; r < 4; ++r)
        out[r] = p[0] * regs[r * 4 + 0] + p[1] * regs[r * 4 + 1]
               + p[2] * regs[r * 4 + 2] + p[3] * regs[r * 4 + 3];
}

static const char *nxGlModeName(GLenum mode)
{
    switch (mode) {
    case GL_POINTS:         return "POINTS";
    case GL_LINES:          return "LINES";
    case GL_LINE_STRIP:     return "LINE_STRIP";
    case GL_TRIANGLES:      return "TRIANGLES";
    case GL_TRIANGLE_STRIP: return "TRIANGLE_STRIP";
    case GL_TRIANGLE_FAN:   return "TRIANGLE_FAN";
    default:                return "?";
    }
}

static void nxGlDumpGeometry(void)
{
    printf("[nx-gl] geometry: glDrawElements ok=%u failed=%u lastError 0x%x\n"
           "        skipped: noGl=%u noProgram=%u noDecl=%u noPosition=%u "
           "noBuffer=%u badType=%u\n",
           s_nGlDrawOk, s_nGlDrawFailed, (unsigned)s_lastGlDrawError,
           s_nSkipNoGl, s_nSkipNoProgram, s_nSkipNoDecl, s_nSkipNoPos,
           s_nSkipNoBuffer, s_nSkipBadType);

    printf("        colour: from vertices=%u default white=%u\n",
           s_nColorAttrib, s_nColorDefault);
    printf("        blend: %s src=%u dst=%u op=%u sepAlpha=%u "
           "colorWrite=0x%x\n",
           s_rs[D3DRS_ALPHABLENDENABLE] ? "on" : "off",
           (unsigned)s_rs[D3DRS_SRCBLEND], (unsigned)s_rs[D3DRS_DESTBLEND],
           (unsigned)s_rs[D3DRS_BLENDOP],
           (unsigned)s_rs[D3DRS_SEPARATEALPHABLENDENABLE],
           (unsigned)s_rs[D3DRS_COLORWRITEENABLE]);

    if (s_nGlDrawOk) {
        if (s_lastDrawHadColor)
            printf("        last colour element: decltype %u\n",
                   (unsigned)s_lastDrawColorType);
        else
            printf("        last colour element: none, drew white\n");
        printf("        last draw: %s, %d %s, stride %u, position at +%u "
               "decltype %u\n",
               nxGlModeName(s_lastDrawMode), (int)s_lastDrawCount,
               s_lastDrawIdxType == GL_UNSIGNED_INT ? "u32 indices" : "u16 indices",
               s_lastDrawStride, s_lastDrawPosOffset, (unsigned)s_lastDrawPosType);
    }

    // Where the transform was taken from, and on whose authority.
    if (s_vs2dBase >= 0)
        printf("        transform c%d..c%d, matched R_CmdBufSet2D exactly "
               "(viewport %.0fx%.0f)\n",
               s_vs2dBase, s_vs2dBase + 3, s_vs2dWidth, s_vs2dHeight);
    else if (s_vsMatrixSeen)
        printf("        transform c%u..c%u, last of %u four-row writes; "
               "no window matched R_CmdBufSet2D\n",
               s_vsMatrixBase, s_vsMatrixBase + 3, s_nVsMatrixWrites);
    else
        printf("        transform c0..c3 by default: no four-row write has "
               "arrived at all\n");

    const float *regs = &s_vsConst[s_vsConstBase][0];
    for (int r = 0; r < 4; ++r)
        printf("          c%-3u % .4f % .4f % .4f % .4f\n",
               s_vsConstBase + r,
               regs[r * 4 + 0], regs[r * 4 + 1], regs[r * 4 + 2], regs[r * 4 + 3]);

    if (s_framePosWSeen) {
        printf("        position w since the last report: %.4f .. %.4f (%s)\n",
               s_frameMinPosW, s_frameMaxPosW,
               s_frameMinPosW == s_frameMaxPosW ? "constant" : "varies per vertex");
        s_framePosWSeen = false;   // next window starts clean
    }

    // The frame as a whole. Everything below this block is one vertex.
    const NxFrameStats &f = s_frame;
    printf("        this frame: %u draws | transform: R_CmdBufSet2D match=%u "
           "last 4-row write=%u c0 default=%u | blended=%u\n",
           f.draws, f.via2d, f.viaMatrix, f.viaDefault, f.drawsBlended);
    printf("          indices %u: in front %u, on screen %u, w<=0 %u, "
           "out of buffer %u, draws not read back (non-float pos) %u\n",
           f.refs, f.refs - f.refsBehind, f.refsInside, f.refsBehind,
           f.refsOutOfRange, f.drawsNoReadback);
    if (f.boxSeen) {
        printf("          ndc box x [% .3f, % .3f] y [% .3f, % .3f] "
               "d3d z [% .4f, % .4f]\n",
               f.minX, f.maxX, f.minY, f.maxY, f.minZ, f.maxZ);
    } else {
        printf("          ndc box: no vertex in front of the camera\n");
    }
    if (f.colRefs) {
        printf("          D3DCOLOR alpha over %u indices: min %u max %u | "
               "0: %u (in blended draws %u)  1..31: %u  255: %u\n",
               f.colRefs, f.alphaMin, f.alphaMax, f.alphaZero,
               f.alphaZeroBlended, f.alphaLow, f.alphaFull);
        const unsigned *h[2] = { f.alphaHiHist, f.alphaLoHist };
        const char *name[2] = { "high byte (D3DCOLOR alpha)", "low byte" };
        for (int k = 0; k < 2; ++k)
            printf("          %-26s 0-31:%u 32-63:%u 64-95:%u 96-127:%u "
                   "128-159:%u 160-191:%u 192-223:%u 224-255:%u\n",
                   name[k], h[k][0], h[k][1], h[k][2], h[k][3],
                   h[k][4], h[k][5], h[k][6], h[k][7]);
    } else {
        printf("          D3DCOLOR alpha: no D3DCOLOR element this frame\n");
    }
    if (s_lastDrawColorRawValid) {
        // Raw DWORD as the engine packed it, then what GL_BGRA hands the
        // shader. If the alpha here is 0 while the quad should be visible,
        // the bytes are not in the order D3DCOLOR promises.
        DWORD c = s_lastDrawColorRaw;
        printf("          last draw vertex colour raw 0x%08x -> shader rgba "
               "(%.3f %.3f %.3f %.3f)\n",
               (unsigned)c,
               ((c >> 16) & 0xff) / 255.0f, ((c >> 8) & 0xff) / 255.0f,
               (c & 0xff) / 255.0f, (c >> 24) / 255.0f);
    }

    if (s_lastDrawPosValid) {
        float clip[4];
        nxTransformPoint(regs, s_lastDrawPos, clip);
        printf("        vertex %df raw (% .2f % .2f % .2f % .2f)\n",
               s_lastDrawPosComponents, s_lastDrawPos[0], s_lastDrawPos[1],
               s_lastDrawPos[2], s_lastDrawPos[3]);
        printf("               -> clip (% .3f % .3f % .3f % .3f)",
               clip[0], clip[1], clip[2], clip[3]);
        if (clip[3] != 0.0f)
            printf(" -> ndc (% .3f % .3f % .3f)",
                   clip[0] / clip[3], clip[1] / clip[3],
                   2.0f * clip[2] / clip[3] - 1.0f);
        printf("\n");

        // If the chosen window is still wrong, the right answer is very likely
        // among the ones that put this vertex on screen. Naming them costs one
        // scan of a 256-register file and saves a blind iteration. A 2D
        // projection leaves w at exactly 1, so that is worth marking: a
        // candidate whose w drifts is a perspective matrix that happens to fit.
        char cands[160];
        int used = 0, found = 0;
        for (unsigned b = 0; b + 3 < NX_VS_CONST_ROWS && found < 6; ++b) {
            if (!s_vsConstWritten[b] || !s_vsConstWritten[b + 1]
             || !s_vsConstWritten[b + 2] || !s_vsConstWritten[b + 3])
                continue;
            float c[4];
            nxTransformPoint(&s_vsConst[b][0], s_lastDrawPos, c);
            if (c[3] <= 0.0f) continue;
            if (c[0] < -c[3] || c[0] > c[3] || c[1] < -c[3] || c[1] > c[3]) continue;
            int n = snprintf(cands + used, sizeof(cands) - used, "%sc%u%s",
                             found ? " " : "", b,
                             nxNear(c[3], 1.0f, 1e-4f) ? "(w=1)" : "");
            if (n < 0 || used + n >= (int)sizeof(cands)) break;
            used += n;
            ++found;
        }
        printf("        candidates (this vertex lands on screen): %s\n",
               found ? cands : "none");
    }
    fflush(stdout);
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
    // Before the engine's first SetRenderState, and well before the first
    // draw reads the file.
    nxInitRenderStates();
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
HRESULT IDirect3DDevice9::Present(const RECT *, const RECT *, HWND, const void *)
{
    ++s_nPresent;
    return D3D_OK;
}
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
HRESULT IDirect3DDevice9::BeginScene() { ++s_nBeginScene; return D3D_OK; }
HRESULT IDirect3DDevice9::EndScene() { return D3D_OK; }
HRESULT IDirect3DDevice9::Clear(
    DWORD, const D3DRECT *, DWORD flags, D3DCOLOR color, float depth, DWORD stencil)
{
    ++s_nClear;
    s_lastClearColor = color;
    s_lastClearFlags = flags;

    // RB_SwapBuffers skips the swap chain entirely while rg.renderHiResShot or
    // dx.resizeWindow is set (rb_backend.cpp:4756). If that is where we are,
    // Present never runs, its 60-frame report never prints, and the log looks
    // identical to GL being broken. Distinguish the two.
    if (s_nClear == 300 && s_nSwapPresent == 0) {
        printf("[nx-gl] 300 clears and not one Present: RB_SwapBuffers is not "
               "reaching the swap chain (rg.renderHiResShot or dx.resizeWindow).\n");
        fflush(stdout);
    }

    if (!nxGlAcquire())
        return D3D_OK;

    // D3DCOLOR is 0xAARRGGBB -- Byte4PackPixelColor (r_state.cpp:3122) writes
    // B,G,R,A into ascending bytes and the packed word is read back
    // little-endian, so this is the colour R_ClearScreen was given.
    // glClear obeys the colour write mask; D3D9's Clear does not. Now that
    // nxGlApplyBlend actually sets that mask, a frame whose last draw had
    // channels switched off would leave the next clear partly undone, so the
    // mask is opened here first. The next draw sets it again anyway.
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    GLbitfield mask = 0;
    if (flags & D3DCLEAR_TARGET) {
        glClearColor(((color >> 16) & 0xff) / 255.0f,
                     ((color >>  8) & 0xff) / 255.0f,
                     ((color      ) & 0xff) / 255.0f,
                     ((color >> 24) & 0xff) / 255.0f);
        mask |= GL_COLOR_BUFFER_BIT;
    }
    if (flags & D3DCLEAR_ZBUFFER) {
        glClearDepth(depth);
        mask |= GL_DEPTH_BUFFER_BIT;
    }
    if (flags & D3DCLEAR_STENCIL) {
        glClearStencil((GLint)stencil);
        mask |= GL_STENCIL_BUFFER_BIT;
    }

    // The rect list is ignored: clearing the whole framebuffer is the point of
    // this step, and honouring it would need the scissor state the rest of the
    // backend does not track yet.
    if (mask)
        glClear(mask);
    return D3D_OK;
}
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
// Recorded, not applied: the draw path reads this file through nxGlApplyBlend
// on the thread that owns the context. Anything outside NX_RS_COUNT is a state
// this D3DRS enum does not define, and dropping it is what the old no-op did
// to all of them.
HRESULT IDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE state, DWORD value)
{
    ++s_nSetRenderState;
    if ((unsigned)state < NX_RS_COUNT)
        s_rs[state] = value;
    return D3D_OK;
}
// Now that the values are kept, hand back the real one. Returning zero for
// everything was safe only while nothing was tracked; with a state file in
// place it would be the one answer guaranteed to disagree with the draw.
HRESULT IDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE state, DWORD *value)
{
    if (value)
        *value = (unsigned)state < NX_RS_COUNT ? s_rs[state] : 0;
    return D3D_OK;
}
HRESULT IDirect3DDevice9::SetTexture(DWORD, IDirect3DBaseTexture9 *) { ++s_nSetTexture; return D3D_OK; }
HRESULT IDirect3DDevice9::SetTextureStageState(DWORD, D3DTEXTURESTAGESTATETYPE, DWORD) { return D3D_OK; }
HRESULT IDirect3DDevice9::SetSamplerState(DWORD, D3DSAMPLERSTATETYPE, DWORD) { return D3D_OK; }
HRESULT IDirect3DDevice9::SetScissorRect(const RECT *) { return D3D_OK; }
HRESULT IDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE, UINT, UINT) { ++s_nDrawPrim; return D3D_OK; }
// MinVertexIndex and NumVertices are dropped: they describe the range for the
// driver to validate or copy, and glDrawElementsBaseVertex needs neither.
HRESULT IDirect3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE type, INT baseVertexIndex,
                                              UINT, UINT, UINT startIndex, UINT primCount)
{
    ++s_nDrawIndexed;
    nxGlDrawIndexed(type, baseVertexIndex, startIndex, primCount);
    return D3D_OK;
}
HRESULT IDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE, UINT, const void *, UINT) { ++s_nDrawPrimUP; return D3D_OK; }
// The three bindings the draw path reads back. None of them takes a reference:
// the engine holds every one of these alive for as long as it is bound, and a
// reference taken here would only change when the object dies, not whether.
HRESULT IDirect3DDevice9::SetVertexDeclaration(IDirect3DVertexDeclaration9 *decl)
{
    s_vdecl = (NxVDecl *)decl;
    return D3D_OK;
}
HRESULT IDirect3DDevice9::SetFVF(DWORD) { return D3D_OK; }
HRESULT IDirect3DDevice9::SetVertexShader(IDirect3DVertexShader9 *) { ++s_nSetVertexShader; return D3D_OK; }
// The shader itself is discarded, but its constants are not: four consecutive
// registers out of this file are the transform the flat-colour program uses.
HRESULT IDirect3DDevice9::SetVertexShaderConstantF(UINT startRegister, const float *data, UINT count)
{
    if (!data)
        return D3D_OK;
    for (UINT i = 0; i < count && startRegister + i < NX_VS_CONST_ROWS; ++i) {
        memcpy(s_vsConst[startRegister + i], data + i * 4, 4 * sizeof(float));
        s_vsConstWritten[startRegister + i] = true;
    }

    // Four rows in one call is a matrix; one row is a plain code constant.
    if (count >= 4 && startRegister + 4 <= NX_VS_CONST_ROWS) {
        s_vsMatrixBase = startRegister;
        s_vsMatrixSeen = true;
        ++s_nVsMatrixWrites;

        // A single call can carry more than one matrix, so every window it
        // just filled gets the 2D test, not only the first.
        for (UINT i = 0; i + 4 <= count && startRegister + i + 4 <= NX_VS_CONST_ROWS; ++i) {
            float w, h;
            if (nxLooksLike2dProjection(startRegister + i, &w, &h)) {
                s_vs2dBase = (int)(startRegister + i);
                s_vs2dWidth = w;
                s_vs2dHeight = h;
            }
        }
    }
    return D3D_OK;
}
HRESULT IDirect3DDevice9::SetPixelShader(IDirect3DPixelShader9 *) { ++s_nSetPixelShader; return D3D_OK; }
HRESULT IDirect3DDevice9::SetPixelShaderConstantF(UINT, const float *, UINT) { return D3D_OK; }
HRESULT IDirect3DDevice9::SetStreamSource(UINT streamNumber, IDirect3DVertexBuffer9 *vb,
                                          UINT offsetInBytes, UINT stride)
{
    ++s_nSetStreamSource;
    if (streamNumber < NX_MAX_STREAMS) {
        s_streamVB[streamNumber] = (NxBuffer *)vb;
        s_streamOffset[streamNumber] = offsetInBytes;
        s_streamStride[streamNumber] = stride;
    }
    return D3D_OK;
}
HRESULT IDirect3DDevice9::SetIndices(IDirect3DIndexBuffer9 *ib)
{
    ++s_nSetIndices;
    s_indexBuf = (NxBuffer *)ib;
    return D3D_OK;
}
HRESULT IDirect3DDevice9::EvictManagedResources() { return D3D_OK; }

// ===========================================================================
// IDirect3DSwapChain9
// ===========================================================================
HRESULT IDirect3DSwapChain9::Present(const RECT *, const RECT *, HWND, const void *, DWORD)
{
    bool report = (++s_nSwapPresent % 60) == 1;
    if (report) nxDumpCallCensus();
    if (report) nxGlDumpGeometry();
    memset(&s_frame, 0, sizeof(s_frame));   // the next frame starts here

    if (!nxGlAcquire()) {
        if (report) {
            printf("[nx-gl] present %u: no GL on this thread, nothing swapped\n",
                   s_nSwapPresent);
            fflush(stdout);
        }
        return D3D_OK;
    }

    // Whatever is still pending from the frame the engine just drew, read
    // before the swap so the report can attribute it to that frame.
    GLenum errBeforeSwap = glGetError();

    EGLBoolean swapped = eglSwapBuffers(s_display, s_surface);
    EGLint eglErr = eglGetError();
    GLenum errAfterSwap = glGetError();

    if (report) {
        EGLint sw = -1, sh = -1;
        eglQuerySurface(s_display, s_surface, EGL_WIDTH, &sw);
        eglQuerySurface(s_display, s_surface, EGL_HEIGHT, &sh);
        u32 winW = 0, winH = 0;
        nwindowGetDimensions(nwindowGetDefault(), &winW, &winH);
        printf("[nx-gl] present %u: glGetError before swap 0x%x | eglSwapBuffers %s "
               "eglGetError 0x%x | glGetError after swap 0x%x\n"
               "        surface %dx%d  nwindow %ux%u  ctx %s  clears seen %u\n"
               "        last Clear: color 0x%08x (a=%u r=%u g=%u b=%u) flags 0x%x%s%s%s\n",
               s_nSwapPresent, (unsigned)errBeforeSwap,
               swapped == EGL_TRUE ? "TRUE" : "FALSE",
               (unsigned)eglErr, (unsigned)errAfterSwap,
               sw, sh, winW, winH,
               eglGetCurrentContext() == s_context ? "current" : "NOT CURRENT",
               s_nClear,
               (unsigned)s_lastClearColor,
               (unsigned)((s_lastClearColor >> 24) & 0xff),
               (unsigned)((s_lastClearColor >> 16) & 0xff),
               (unsigned)((s_lastClearColor >>  8) & 0xff),
               (unsigned)((s_lastClearColor      ) & 0xff),
               (unsigned)s_lastClearFlags,
               (s_lastClearFlags & D3DCLEAR_TARGET)  ? " TARGET"  : "",
               (s_lastClearFlags & D3DCLEAR_ZBUFFER) ? " ZBUFFER" : "",
               (s_lastClearFlags & D3DCLEAR_STENCIL) ? " STENCIL" : "");
        fflush(stdout);
    }
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
HRESULT IDirect3DVertexBuffer9::Unlock()
{
    nxGlBufferDirty((NxBuffer *)this, GL_ARRAY_BUFFER);
    return D3D_OK;
}
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
HRESULT IDirect3DIndexBuffer9::Unlock()
{
    nxGlBufferDirty((NxBuffer *)this, GL_ELEMENT_ARRAY_BUFFER);
    return D3D_OK;
}
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
