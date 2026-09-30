#include "ui_atoms.h"
#include <gfx_d3d/r_material.h>
#include <client_mp/cl_cgame_mp.h>
#include <client/cl_main.h>
#include <universal/com_memory.h>
#include <universal/q_shared.h>
#include <database/db_file_load.h>
#include "ui_shared.h"
#include <qcommon/common.h>
#ifdef KISAK_NX
#include <client/client.h>
#include <gfx_d3d/r_cinematic.h>
#endif

void __cdecl UI_DrawHandlePic(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                const float *color,
                Material *material)
{
    float t0; // [esp+30h] [ebp-10h]
    float t1; // [esp+34h] [ebp-Ch]
    float s1; // [esp+38h] [ebp-8h]
    float s0; // [esp+3Ch] [ebp-4h]

#ifdef KISAK_NX
    // SP's loading menu ("briefing") shows the Bink load movie through the
    // cinematic material. The Switch never starts that movie (SV_SpawnServer),
    // so while loading, draw the level's loadscreen image in its place -- what
    // the online/co-op loading path shows -- instead of a black panel.
    if ( material && sharedUiInfo.loadingScreen && !R_Cinematic_IsStarted()
        && CL_GetLocalClientConnectionState(0) < CA_ACTIVE
        && !strcmp(Material_GetName(material), "cinematic") )
    {
        material = sharedUiInfo.loadingScreen;
    }
#endif

    if ( w >= 0.0 )
    {
        s0 = 0.0f;
        s1 = 1.0f;
    }
    else
    {
        //LODWORD(w) ^= _mask__NegFloat_;
        w = -w;
        s0 = 1.0f;
        s1 = 0.0f;
    }
    if ( h >= 0.0 )
    {
        t0 = 0.0f;
        t1 = 1.0f;
    }
    else
    {
        //LODWORD(h) ^= _mask__NegFloat_;
        h = -h;
        t0 = 1.0f;
        t1 = 0.0f;
    }
    CL_DrawStretchPic(scrPlace, x, y, w, h, horzAlign, vertAlign, s0, t0, s1, t1, color, material);
}

void __cdecl UI_DrawHandlePicRotated(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                const float *color,
                float angle,
                Material *material)
{
    float t0; // [esp+2Ch] [ebp-10h]
    float t1; // [esp+30h] [ebp-Ch]
    float s1; // [esp+34h] [ebp-8h]
    float s0; // [esp+38h] [ebp-4h]

    if ( w >= 0.0 )
    {
        s0 = 0.0f;
        s1 = 1.0f;
    }
    else
    {
        //LODWORD(w) ^= _mask__NegFloat_;
        w = -w;
        s0 = 1.0f;
        s1 = 0.0f;
    }
    if ( h >= 0.0 )
    {
        t0 = 0.0f;
        t1 = 1.0f;
    }
    else
    {
        //LODWORD(h) ^= _mask__NegFloat_;
        h = -h;
        t0 = 1.0f;
        t1 = 0.0f;
    }
    ScrPlace_ApplyRect(scrPlace, &x, &y, &w, &h, horzAlign, vertAlign);
    CL_DrawStretchPicPhysicalRotateXY(x, y, w, h, s0, t0, s1, t1, angle, color, material);
}

void __cdecl UI_DrawHandlePicFramed(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                float frameSize,
                float frameTexSize,
                int frameSides,
                const float *color,
                Material *material)
{
    float thicknessW; // [esp+28h] [ebp-8h]
    float thicknessH; // [esp+2Ch] [ebp-4h]

    ScrPlace_ApplyRect(scrPlace, &x, &y, &w, &h, horzAlign, vertAlign);
    thicknessW = ScrPlace_ApplyW(scrPlace, frameSize, horzAlign);
    thicknessH = ScrPlace_ApplyH(scrPlace, frameSize, vertAlign);
    CL_DrawFramedPicPhysical(
        x,
        y,
        w,
        h,
        frameSize,
        (float)(thicknessH / thicknessW) * frameSize,
        frameTexSize,
        frameSides,
        color,
        material);
}

void __cdecl UI_DrawLoadBar(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                const float *color,
                Material *material)
{
    double v9; // st7
    float percentDone; // [esp+3Ch] [ebp-10h]

    if ( useFastFile->current.enabled )
        v9 = ((double (__cdecl *)(double (__cdecl *)()))UI_LoadBarProgress_FastFile)(UI_LoadBarProgress_FastFile);
    else
        v9 = ((double (__cdecl *)(double (__cdecl *)()))UI_LoadBarProgress_LoadObj)(UI_LoadBarProgress_LoadObj);
    percentDone = v9;
    CL_DrawStretchPic(
        scrPlace,
        x,
        y,
        w * percentDone,
        h,
        horzAlign,
        vertAlign,
        0.0,
        0.0,
        percentDone,
        1.0,
        color,
        material);
}

double __cdecl UI_LoadBarProgress_LoadObj()
{
    float v2; // [esp+4h] [ebp-8h]
    float v3; // [esp+8h] [ebp-4h]

    if ( com_expectedHunkUsage <= 0 )
        return 0.0;
    v2 = (float)com_expectedHunkUsage;
    v3 = (float)Hunk_Used() / v2;
    if ( (float)(v3 - 1.0) < 0.0 )
        return v3;
    else
        return 1.0f;
}

double __cdecl UI_LoadBarProgress_FastFile()
{
    return DB_GetLoadedFraction();
}

void __cdecl UI_FillRectPhysical(float x, float y, float width, float height, const float *color)
{
    if ( sharedUiInfo.assets.whiteMaterial )
        CL_DrawStretchPicPhysical(x, y, width, height, 0.0, 0.0, 0.0, 0.0, color, sharedUiInfo.assets.whiteMaterial);
}

void __cdecl UI_FillRect(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float width,
                float height,
                int horzAlign,
                int vertAlign,
                const float *color)
{
    if ( sharedUiInfo.assets.whiteMaterial )
        CL_DrawStretchPic(
            scrPlace,
            x,
            y,
            width,
            height,
            horzAlign,
            vertAlign,
            0.0,
            0.0,
            0.0,
            0.0,
            color,
            sharedUiInfo.assets.whiteMaterial);
}

void __cdecl UI_DrawSpinner(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float width,
                float height,
                int horzAlign,
                int vertAlign,
                const float *color)
{
    CL_DrawSpinner(scrPlace, x, y, width, height, horzAlign, vertAlign, color);
}

void __cdecl UI_DrawSpinnerLoadbar(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float width,
                float height,
                int horzAlign,
                int vertAlign,
                const float *color)
{
    double v8; // st7
    float percentDone; // [esp+28h] [ebp-4h]

    if ( useFastFile->current.enabled )
        v8 = ((double (__cdecl *)(double (__cdecl *)()))UI_LoadBarProgress_FastFile)(UI_LoadBarProgress_FastFile);
    else
        v8 = ((double (__cdecl *)(double (__cdecl *)()))UI_LoadBarProgress_LoadObj)(UI_LoadBarProgress_LoadObj);
    percentDone = v8;
    CL_DrawSpinnerLoadbar(scrPlace, x, y, width, height, horzAlign, vertAlign, color, percentDone);
}

