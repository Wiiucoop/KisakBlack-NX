// nx_gl_mesa20.cpp -- the desktop-only GL calls, for builds against Mesa 20.1.
//
// Mesa 20.1 for the Switch (switch-mesa) ships libEGL, libglapi and
// libGLESv2 but no libGL: every GL entry point that also exists in GLES comes
// from libGLESv2 and dispatches through glapi to whichever context is current,
// the desktop core context included. The three the renderer calls that GLES
// lacks are fetched from the driver with eglGetProcAddress on first use.
// Built only with -DNX_MESA20_DIR (cmake/switch.cmake).
#ifdef NX_MESA20
#include <EGL/egl.h>
#include <GL/gl.h>
#include <stdio.h>

template <typename Fn>
static Fn nxGlProc(Fn &slot, const char *name)
{
    if (!slot) {
        slot = (Fn)eglGetProcAddress(name);
        if (!slot)
            printf("[nx-gl] Mesa 20: %s not available\n", name);
    }
    return slot;
}

extern "C" void glClearDepth(GLdouble depth)
{
    static void (*fn)(GLdouble);
    if (nxGlProc(fn, "glClearDepth"))
        fn(depth);
}

extern "C" void glDrawBuffer(GLenum buf)
{
    static void (*fn)(GLenum);
    if (nxGlProc(fn, "glDrawBuffer"))
        fn(buf);
}

extern "C" void glPolygonMode(GLenum face, GLenum mode)
{
    static void (*fn)(GLenum, GLenum);
    if (nxGlProc(fn, "glPolygonMode"))
        fn(face, mode);
}
#endif
