#include "cg_draw_net_mp.h"
#include "cg_local_mp.h"
#include "cg_main_mp.h"
#include <ui/ui_atoms.h>
#include <client_mp/cl_cgame_mp.h>
#include <cgame/cg_drawtools.h>
#include "cg_consolecmds_mp.h"
#include <client_mp/console_mp.h>

lagometer_t lagometer;

void __cdecl CG_AddLagometerFrameInfo(const cg_s *cgameGlob)
{
    lagometer.frameSamples[lagometer.frameCount & 0x7F] = cgameGlob->time - cgameGlob->latestSnapshotTime;
    ++lagometer.frameCount;
}

void __cdecl CG_AddLagometerSnapshotInfo(snapshot_s *snap)
{
    if ( snap )
    {
        lagometer.snapshotSamples[lagometer.snapshotCount & 0x7F] = snap->ping;
        lagometer.snapshotFlags[lagometer.snapshotCount & 0x7F] = snap->snapFlags;
    }
    else
    {
        lagometer.snapshotSamples[lagometer.snapshotCount & 0x7F] = -1;
    }
    ++lagometer.snapshotCount;
}

void __cdecl CG_DrawLagometer(int localClientNum)
{
    const float *hcolor; // [esp+30h] [ebp-40h]
    ScreenPlacement *scrPlace; // [esp+34h] [ebp-3Ch]
    float range; // [esp+38h] [ebp-38h]
    float rangea; // [esp+38h] [ebp-38h]
    const cgs_t *cgs; // [esp+44h] [ebp-2Ch]
    int a; // [esp+54h] [ebp-1Ch]
    int aa; // [esp+54h] [ebp-1Ch]
    float v; // [esp+68h] [ebp-8h]
    float va; // [esp+68h] [ebp-8h]
    float vb; // [esp+68h] [ebp-8h]
    float vc; // [esp+68h] [ebp-8h]
    float mid; // [esp+6Ch] [ebp-4h]

    cgs = CG_GetLocalClientStaticGlobals(localClientNum);
    if ( cg_drawLagometer->current.enabled && !cgs->localServer )
    {
        scrPlace = &scrPlaceView[localClientNum];
        UI_DrawHandlePic(scrPlace, -55.0, -140.0, 48.0, 48.0, 3, 3, 0, cgMedia.lagometerMaterial);
        range = 48.0 / 3.0;
        mid = (float)(48.0 / 3.0) + -140.0;
        for ( a = 0; (float)a < 48.0; ++a )
        {
            v = (float)lagometer.frameSamples[(LOBYTE(lagometer.frameCount) - 1 - (_BYTE)a) & 0x7F]
                * (float)((float)(48.0 / 3.0) / 300.0);
            if ( v <= 0.0 )
            {
                if ( v < 0.0 )
                {
                    va = -v;
                    if ( va > range )
                        va = 48.0 / 3.0;
                    CL_DrawStretchPic(
                        scrPlace,
                        (float)(48.0 + -55.0) - (float)a,
                        mid,
                        1.0,
                        va,
                        3,
                        3,
                        0.0,
                        0.0,
                        0.0,
                        0.0,
                        colorBlue,
                        cgMedia.whiteMaterial);
                }
            }
            else
            {
                if ( v > range )
                    v = 48.0 / 3.0;
                CL_DrawStretchPic(
                    scrPlace,
                    (float)(48.0 + -55.0) - (float)a,
                    mid - v,
                    1.0,
                    v,
                    3,
                    3,
                    0.0,
                    0.0,
                    0.0,
                    0.0,
                    colorYellow,
                    cgMedia.whiteMaterial);
            }
        }
        rangea = 48.0 / 2.0;
        for ( aa = 0; (float)aa < 48.0; ++aa )
        {
            vb = (float)lagometer.snapshotSamples[(LOBYTE(lagometer.snapshotCount) - 1 - (_BYTE)aa) & 0x7F];
            if ( vb <= 0.0 )
            {
                if ( vb < 0.0 )
                    CL_DrawStretchPic(
                        scrPlace,
                        (float)(48.0 + -55.0) - (float)aa,
                        (float)(-140.0 + 48.0) - rangea,
                        1.0,
                        rangea,
                        3,
                        3,
                        0.0,
                        0.0,
                        0.0,
                        0.0,
                        colorRed,
                        cgMedia.whiteMaterial);
            }
            else
            {
                if ( (lagometer.snapshotFlags[(LOBYTE(lagometer.snapshotCount) - 1 - (_BYTE)aa) & 0x7F] & 1) != 0 )
                    hcolor = colorYellow;
                else
                    hcolor = colorGreen;
                vc = vb * (float)((float)(48.0 / 2.0) / 900.0);
                if ( vc > rangea )
                    vc = 48.0 / 2.0;
                CL_DrawStretchPic(
                    scrPlace,
                    (float)(48.0 + -55.0) - (float)aa,
                    (float)(-140.0 + 48.0) - vc,
                    1.0,
                    vc,
                    3,
                    3,
                    0.0,
                    0.0,
                    0.0,
                    0.0,
                    hcolor,
                    cgMedia.whiteMaterial);
            }
        }
        if ( cg_nopredict->current.enabled || cg_synchronousClients->current.enabled )
            CG_DrawBigDevString(scrPlace, (float)(-55.0 + 640.0) + 48.0, -140.0 + 480.0, (char*)"snc", 1.0, 10);
    }
    CG_DrawDisconnect(localClientNum);
}

void __cdecl CG_DrawDisconnect(int localClientNum)
{
#ifdef KISAK_SP
    // Retail SP (0x0088e0f0) excludes local servers, paused games and offline play.
    if ( com_sv_running->current.enabled || cg_paused->current.integer || !Dvar_GetBool("onlinegame") )
        return;
#endif
    int i; // [esp+24h] [ebp-60h]
    Material *disconnectMaterial; // [esp+28h] [ebp-5Ch]
    Font_s *font; // [esp+2Ch] [ebp-58h]
    ScreenPlacement *scrPlace; // [esp+30h] [ebp-54h]
    const cg_s *cgameGlob; // [esp+34h] [ebp-50h]
    int cmdNum; // [esp+38h] [ebp-4Ch]
    char *s; // [esp+3Ch] [ebp-48h]
    usercmd_s cmd; // [esp+48h] [ebp-3Ch] BYREF
    int w; // [esp+80h] [ebp-4h]

    for ( i = 0; i < 2; ++i )
        cmd.button_bits.array[i] = 0;
    if ( !CL_IsServerRestarting(localClientNum) && !Demo_IsPlaying() )
    {
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        cmdNum = CL_GetCurrentCmdNumber(localClientNum) - 127;
        CL_GetUserCmd(localClientNum, cmdNum, &cmd);
        if ( cmd.serverTime > cgameGlob->nextSnap->ps.commandTime && cmd.serverTime <= cgameGlob->time )
        {
            scrPlace = &scrPlaceView[localClientNum];
            s = UI_SafeTranslateString("CGAME_CONNECTIONINTERUPTED");
            font = UI_GetFontHandle(scrPlace, 0, 0.5);
            w = UI_TextWidth(s, 0, font, 0.5);
            UI_DrawText(scrPlace, s, 0x7FFFFFFF, font, (float)((640 - w) / 2), 100.0, 0, 0, 0.5, colorWhite, 3);
            disconnectMaterial = Material_RegisterHandle("net_new_animation", 7);
            UI_DrawHandlePic(scrPlace, 288.0, 320.0, 64.0, 64.0, 0, 0, 0, disconnectMaterial);
        }
    }
}

void __cdecl CG_DrawEntityCount(int localClientNum)
{
    char *v1; // eax
    Font_s *smallDevFont; // [esp+10h] [ebp-54h]
    ScreenPlacement *scrPlace; // [esp+30h] [ebp-34h]
    float vscale; // [esp+38h] [ebp-2Ch]
    float value; // [esp+44h] [ebp-20h]
    int i; // [esp+48h] [ebp-1Ch]
    float ay; // [esp+4Ch] [ebp-18h]
    int a; // [esp+50h] [ebp-14h]
    float aw; // [esp+5Ch] [ebp-8h]
    float v; // [esp+60h] [ebp-4h]
    float va; // [esp+60h] [ebp-4h]
    float vb; // [esp+60h] [ebp-4h]

    if ( CL_LocalClient_IsCUIFlagSet(localClientNum, 32) )
    {
        CG_GetLocalClientStaticGlobals(localClientNum);
        if ( drawEntityCount->current.enabled )
        {
            scrPlace = &scrPlaceView[localClientNum];
            value = drawEntityCountPos->current.value;
            ay = drawEntityCountPos->current.vector[1];
            aw = (float)drawEntityCountSize->current.integer;
            smallDevFont = cgMedia.smallDevFont;
            v1 = ::va(
                         "Ents: %3i/%3i/%3i",
                         lagometer.changedEntitySamples[lagometer.frameCount & 0x7F],
                         lagometer.archivedEntitySamples[lagometer.frameCount & 0x7F],
                         lagometer.entitySamples[lagometer.frameCount & 0x7F]);
            UI_DrawText(
                scrPlace,
                v1,
                500,
                smallDevFont,
                value - 50.0,
                (float)(ay + aw) + 18.0,
                3,
                3,
                0.30000001,
                colorWhite,
                3);
            UI_DrawHandlePic(scrPlace, value, ay, aw, aw, 3, 3, colorBlack, cgMedia.whiteMaterial);
            vscale = aw / 650.0;
            for ( a = 0; aw > (float)a; ++a )
            {
                i = (LOBYTE(lagometer.frameCount) - 1 - (_BYTE)a) & 0x7F;
                v = (float)lagometer.entitySamples[i] * vscale;
                if ( lagometer.entitySamples[i] >= 500 )
                    CL_DrawStretchPic(
                        scrPlace,
                        (float)((float)(value + aw) - (float)a) - 1.0,
                        (float)(ay + aw) - v,
                        1.0,
                        v,
                        3,
                        3,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        colorRed,
                        cgMedia.whiteMaterial);
                else
                    CL_DrawStretchPic(
                        scrPlace,
                        (float)((float)(value + aw) - (float)a) - 1.0,
                        (float)(ay + aw) - v,
                        1.0,
                        v,
                        3,
                        3,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        colorBlue,
                        cgMedia.whiteMaterial);
                va = (float)lagometer.archivedEntitySamples[i] * vscale;
                if ( lagometer.archivedEntitySamples[i] >= 300 )
                    CL_DrawStretchPic(
                        scrPlace,
                        (float)((float)(value + aw) - (float)a) - 1.0,
                        (float)(ay + aw) - va,
                        1.0,
                        va,
                        3,
                        3,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        colorOrange,
                        cgMedia.whiteMaterial);
                else
                    CL_DrawStretchPic(
                        scrPlace,
                        (float)((float)(value + aw) - (float)a) - 1.0,
                        (float)(ay + aw) - va,
                        1.0,
                        va,
                        3,
                        3,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        colorMdCyan,
                        cgMedia.whiteMaterial);
                vb = (float)lagometer.changedEntitySamples[i] * vscale;
                CL_DrawStretchPic(
                    scrPlace,
                    (float)((float)(value + aw) - (float)a) - 1.0,
                    (float)(ay + aw) - vb,
                    1.0,
                    vb,
                    3,
                    3,
                    0.0,
                    0.0,
                    1.0,
                    1.0,
                    colorWhite,
                    cgMedia.whiteMaterial);
            }
        }
    }
}

void __cdecl CG_DrawServerBandwidth(int localClientNum)
{
    char *v1; // eax
    Font_s *smallDevFont; // [esp+10h] [ebp-60h]
    int bandwidthRequired; // [esp+30h] [ebp-40h]
    ScreenPlacement *scrPlace; // [esp+34h] [ebp-3Ch]
    int currentBandwidth; // [esp+3Ch] [ebp-34h]
    int a; // [esp+58h] [ebp-18h]
    float x; // [esp+5Ch] [ebp-14h]
    float y; // [esp+60h] [ebp-10h]
    float aw; // [esp+64h] [ebp-Ch]
    const float *color; // [esp+68h] [ebp-8h]
    float v; // [esp+6Ch] [ebp-4h]

    if ( CL_LocalClient_IsCUIFlagSet(localClientNum, 32) )
    {
        CG_GetLocalClientStaticGlobals(localClientNum);
        if ( drawServerBandwidth->current.enabled )
        {
            x = drawServerBandwidthPos->current.value;
            y = drawServerBandwidthPos->current.vector[1];
            scrPlace = &scrPlaceView[localClientNum];
            aw = (float)drawServerBandwidthSize->current.integer;
            bandwidthRequired = (int)(float)((float)((float)Live_GetNecessaryBandwidth() / 1024.0) * 0.89999998);
            currentBandwidth = lagometer.serverBandwidth[lagometer.frameCount & 0x7F];
            if ( currentBandwidth <= bandwidthRequired )
                color = colorWhite;
            else
                color = colorRed;
            smallDevFont = cgMedia.smallDevFont;
            v1 = va("kbps: %3i", currentBandwidth);
            UI_DrawText(scrPlace, v1, 500, smallDevFont, x - 50.0, (float)(y + aw) + 18.0, 3, 3, 0.30000001, color, 3);
            UI_DrawHandlePic(scrPlace, x, y, aw, aw, 3, 3, colorBlack, cgMedia.whiteMaterial);
            for ( a = 0; aw > (float)a; ++a )
            {
                v = (float)lagometer.serverBandwidth[(LOBYTE(lagometer.frameCount) - 1 - (_BYTE)a) & 0x7F]
                    * (float)(aw / (float)bandwidthRequired);
                if ( v > aw )
                    v = aw;
                if ( lagometer.serverBandwidth[(LOBYTE(lagometer.frameCount) - 1 - (_BYTE)a) & 0x7F] >= bandwidthRequired )
                    CL_DrawStretchPic(
                        scrPlace,
                        (float)((float)(x + aw) - (float)a) - 1.0,
                        (float)(y + aw) - v,
                        1.0,
                        v,
                        3,
                        3,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        colorRed,
                        cgMedia.whiteMaterial);
                else
                    CL_DrawStretchPic(
                        scrPlace,
                        (float)((float)(x + aw) - (float)a) - 1.0,
                        (float)(y + aw) - v,
                        1.0,
                        v,
                        3,
                        3,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        colorBlue,
                        cgMedia.whiteMaterial);
            }
        }
    }
}

void __cdecl CG_DrawServerKillcamData(int localClientNum)
{
    char *v1; // eax
    Font_s *smallDevFont; // [esp+10h] [ebp-64h]
    int currentKillcamTime; // [esp+30h] [ebp-44h]
    ScreenPlacement *scrPlace; // [esp+34h] [ebp-40h]
    int a; // [esp+5Ch] [ebp-18h]
    float x; // [esp+60h] [ebp-14h]
    float y; // [esp+64h] [ebp-10h]
    float aw; // [esp+68h] [ebp-Ch]
    const float *color; // [esp+6Ch] [ebp-8h]
    float v; // [esp+70h] [ebp-4h]

    if ( CL_LocalClient_IsCUIFlagSet(localClientNum, 32) )
    {
        CG_GetLocalClientStaticGlobals(localClientNum);
        if ( drawKillcamData->current.enabled )
        {
            x = drawKillcamDataPos->current.value;
            y = drawKillcamDataPos->current.vector[1];
            scrPlace = &scrPlaceView[localClientNum];
            aw = (float)drawKillcamDataSize->current.integer;
            currentKillcamTime = lagometer.killcamTimes[lagometer.frameCount & 0x7F];
            if ( currentKillcamTime >= 10000 )
                color = colorWhite;
            else
                color = colorRed;
            smallDevFont = cgMedia.smallDevFont;
            v1 = va("killcam: %3i", currentKillcamTime);
            UI_DrawText(scrPlace, v1, 500, smallDevFont, x - 50.0, (float)(y + aw) + 18.0, 3, 3, 0.30000001, color, 3);
            UI_DrawHandlePic(scrPlace, x, y, aw, aw, 3, 3, colorBlack, cgMedia.whiteMaterial);
            for ( a = 0; aw > (float)a; ++a )
            {
                v = (float)lagometer.killcamTimes[(LOBYTE(lagometer.frameCount) - 1 - (_BYTE)a) & 0x7F]
                    * (float)(aw / (float)20000);
                if ( v > aw )
                    v = aw;
                if ( lagometer.killcamTimes[(LOBYTE(lagometer.frameCount) - 1 - (_BYTE)a) & 0x7F] >= 10000 )
                    CL_DrawStretchPic(
                        scrPlace,
                        (float)((float)(x + aw) - (float)a) - 1.0,
                        (float)(y + aw) - v,
                        1.0,
                        v,
                        3,
                        3,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        colorRed,
                        cgMedia.whiteMaterial);
                else
                    CL_DrawStretchPic(
                        scrPlace,
                        (float)((float)(x + aw) - (float)a) - 1.0,
                        (float)(y + aw) - v,
                        1.0,
                        v,
                        3,
                        3,
                        0.0,
                        0.0,
                        1.0,
                        1.0,
                        colorBlue,
                        cgMedia.whiteMaterial);
            }
        }
    }
}

