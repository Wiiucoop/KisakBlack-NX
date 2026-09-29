#include "ui_main_mp.h"
#include <cstddef>
#include <ui/ui_shared.h>
#include <gfx_d3d/r_material.h>
#include <qcommon/com_clients.h>
#include <client/cl_keys.h>
#include <ui/ui_emblem.h>
#include <client/splitscreen.h>
#include <ui/ui_viewer.h>
#include <ui/ui_utils.h>
#include <ui/ui_feeders.h>
#include <game_mp/g_main_mp.h>
#include <ui/ui_shared_obj.h>
#include <gfx_d3d/r_rendercmds.h>
#include <universal/com_stringtable.h>
#include <ragdoll/ragdoll_controller.h>
#include <universal/com_memory.h>
#include <gfx_d3d/r_image.h>
#include <DW/MatchRecorder.h>
#include <live/live_win.h>
#include <gfx_d3d/rb_resource.h>
#include <live/live_stats.h>
#include <ui/ui_atoms.h>
#include <universal/com_expressions_eval.h>
#include <live/live_fileshare.h>
#include <ui/ui_screenshot.h>
#include <client_mp/cl_cgame_mp.h>
#include <client_mp/cl_main_mp.h> // KISAK_SP: UI_SetActiveMenu needs CL_PauseGame
#include <gfx_d3d/r_ui3d.h>
#include <live/live_combatrecord.h>
#include <live/live_contracts.h>
#include <bgame/bg_unlockable_items.h>
#include <cgame/cg_weapon_options.h>
#include <ui/ui_playlists.h>
#include <ui/ui_main_pc.h>
#include <client/client.h>
#include <cgame/cg_main.h>
#include <server_mp/sv_init_mp.h>
#include <client_mp/cl_ui_pc_mp.h>
#include <client_mp/cl_main_pc_mp.h>
#include <ui/ui_server.h>
#include <sound/snd_public_async.h>
#include <demo/demo_playback.h>
#include <cgame_mp/cg_newDraw_mp.h>
#include <client_mp/cl_ui_mp.h>
#include <demo/demo_ui.h>
#include <database/db_file_load.h>
#include <cgame/cg_info.h>
#ifdef KISAK_SP
#include <gfx_d3d/r_cinematic.h>
#include <qcommon/common.h>
#include <ui/ui_main.h>                 // ui_menuLvlNotify, for the sendMenuNotify menu script
#endif

const char *lbTypeEnum_3[17] =
{
  "tdm",
  "dm",
  "ctf",
  "dom",
  "sab",
  "sd",
  "koth",
  "dem",
  "hctdm",
  "hcdm",
  "hcctf",
  "hcdom",
  "hcsab",
  "hcsd",
  "hckoth",
  "hcdem",
  NULL
};

const dvar_t *ui_ignoreMousePos;
const dvar_t *ui_prevTextEntryBox;
const dvar_t *ui_blurAmount;
const dvar_t *ui_blurDarkenAmount;
const dvar_t *ui_mapCount;
const dvar_t *ui_browserHardcore;
const dvar_t *ui_showEndOfGame;
const dvar_t *ui_serverBrowserMenu;
const dvar_t *ui_showAllContracts;
const dvar_t *ui_hud_hardcore;
const dvar_t *ui_radar_client;
const dvar_t *ui_allow_classchange;
const dvar_t *ui_allow_teamchange;
const dvar_t *ui_map_killstreak;
const dvar_t *ui_hud_visible;
const dvar_t *ui_party_download_bar_height;
const dvar_t *ui_party_download_bar_color;
const dvar_t *ui_closeAfterPurchase;
const dvar_t *ui_classesCurrentItemEquippedIn;
const dvar_t *ui_heatMapColor;
const dvar_t *ui_heatMapColorForPlayer;

const char *serverBrowserMenus[7] =
{
  "server_browser_ranked",
  "server_browser_unranked",
  "server_browser_wager",
  "server_browser_friends",
  "server_browser_favorites",
  "server_browser_history",
  NULL
};

char g_mapname[64];
char s_gametype[64];
bool g_showLoadingScreenMenu;

int currIndex;
int startIndex;
int endIndex;

void __cdecl UI_Project_RegisterDvars()
{
    ui_ignoreMousePos = _Dvar_RegisterBool(
                                                "ui_ignoreMousePos",
                                                0,
                                                0,
                                                "Whether there are multiple text entries in the menu.");
    ui_prevTextEntryBox = _Dvar_RegisterInt(
                                                    "ui_prevTextEntryBox",
                                                    -1,
                                                    -1,
                                                    0x7FFFFFFF,
                                                    0,
                                                    "Index of the previous text entry box.");
    ui_blurAmount = _Dvar_RegisterFloat(
                                        "ui_blurAmount",
                                        4.0,
                                        0.0,
                                        3.4028235e38,
                                        0,
                                        "Max amount to blur background menu items.");
    ui_blurDarkenAmount = _Dvar_RegisterFloat(
                                                    "ui_blurDarkenAmount",
                                                    0.28,
                                                    0.0,
                                                    1.0,
                                                    0,
                                                    "Max amount to darken the background menu items.");
    ui_mapCount = _Dvar_RegisterInt("ui_mapCount", 0, 0, 0x7FFFFFFF, 1u, "Number of maps the player has");
    ui_browserHardcore = _Dvar_RegisterInt("ui_browserHardcore", -1, 0x80000000, 0x7FFFFFFF, 1u, "Hardcore mode");
    ui_showEndOfGame = _Dvar_RegisterBool("ui_showEndOfGame", 0, 0, "Currently showing the end of game menu.");
    ui_serverBrowserMenu = _Dvar_RegisterEnum(
                                                     "ui_serverBrowserMenu",
                                                     serverBrowserMenus,
                                                     0,
                                                     0x41u,
                                                     "Last opened server browser tab.");
    ui_showAllContracts = _Dvar_RegisterBool(
                                                    "ui_showAllContracts",
                                                    0,
                                                    0x80u,
                                                    "Allow all contracts to be purchased and removes all cooldown and inventory restrictions.");
    ui_hud_hardcore = _Dvar_RegisterBool(
                                            "ui_hud_hardcore",
                                            0,
                                            0x80u,
                                            "Whether the HUD should be suppressed for hardcore mode");
    ui_radar_client = _Dvar_RegisterBool("ui_radar_client", 0, 0x80u, "Whether the UI should show radar to this client");
    ui_allow_classchange = _Dvar_RegisterBool(
                                                     "ui_allow_classchange",
                                                     0,
                                                     0x80u,
                                                     "Whether the UI should allow changing class");
    ui_allow_teamchange = _Dvar_RegisterBool("ui_allow_teamchange", 0, 0, "Whether the UI should allow changing team");
    ui_map_killstreak = _Dvar_RegisterString(
                                                "ui_map_killstreak",
                                                (char *)"",
                                                0,
                                                "Reference of the killstreak that requires map placement.");
    ui_hud_visible = _Dvar_RegisterBool("ui_hud_visible", 1, 0x80u, "Whether the 2D HUD should be displayed or not.");
    ui_party_download_bar_height = _Dvar_RegisterInt(
                                                                     "ui_party_download_bar_height",
                                                                     14,
                                                                     0,
                                                                     0x7FFFFFFF,
                                                                     0,
                                                                     "Height of each download progress bar.");
    ui_party_download_bar_color = _Dvar_RegisterVec4(
                                                                    "ui_party_download_bar_color",
                                                                    (0.69999999),
                                                                    (0.69999999),
                                                                    (0.69999999),
                                                                    (0.2),
                                                                    0.0,
                                                                    1.0,
                                                                    0,
                                                                    "Color of the party download progress bar.");
    ui_closeAfterPurchase = _Dvar_RegisterBool(
                                                        "ui_closeAfterPurchase",
                                                        0,
                                                        0,
                                                        "Whether an item has just been purchased and the item list needs to be closed.");
    ui_classesCurrentItemEquippedIn = _Dvar_RegisterString(
                                                                            "ui_classesCurrentItemEquippedIn",
                                                                            (char *)"",
                                                                            0,
                                                                            "The name of the class in which the currently selected item is equipped.");
    ui_heatMapColor = _Dvar_RegisterColor("ui_heatMapColor", 1.0, 0.0, 0.0, 1.0, 0, "Heat Map Blob Color");
    ui_heatMapColorForPlayer = _Dvar_RegisterColor(
                                                             "ui_heatMapColorForPlayer",
                                                             1.0,
                                                             1.0,
                                                             0.0,
                                                             1.0,
                                                             0,
                                                             "Heat Map Blob Color for player deaths only");
}

void __cdecl UI_Project_AssetCache()
{
    sharedUiInfo.assets.blur = Material_RegisterHandle((char*)"ui_blur", 3);
    sharedUiInfo.assets.lineGraph = Material_RegisterHandle((char *)"ui_line_graph", 3);
}

#ifdef KISAK_SP
// SP-only addition (Ghidra 0x0040FC70). Retail's own plate leaves this function deliberately
// unnamed ("naming it would be fabrication") -- confirmed statically by decompiling the address
// directly, body verified against disassembly line-for-line: unpause, fade to clear over 3
// seconds, close all menus, promote a queued cinematic (or stop playback if none is queued), mark
// com_introPlayed, and (retail only) rearm an idle/attract timer. It is the SP intro/attract-movie
// dismiss handler, reached (per retail xrefs) from UI_Project_RunMenuScript's "playerstart"
// command, from UI_SetActiveMenu's case 4 ("pregame") gate (TODO'd, not transcribed, see
// UI_SetActiveMenuSp case 4 above), and from the per-frame lifecycle helper below. The name here
// is this reconstruction's own descriptive label, not a claimed retail symbol.
//
// TWO retail statements are deliberately NOT ported:
//  - a raw global-byte write (FUN_0057fc70(0), Ghidra 0x0057fc70) with no confirmed reader
//    anywhere in the binary (get_xrefs_to found only this one write) and no corresponding state
//    in this reconstruction to hook it to;
//  - an idle/attract-timer rearm (FUN_004f46b0, Ghidra 0x004f46b0) that arms a retail-only
//    "return to attract screen after N seconds idle" timer. This reconstruction has no
//    attract-mode/idle-timeout subsystem anywhere else (same reasoning this project has already
//    applied to declining an unrelated PunkBuster port -- see ORCHESTRATOR.md Sec.10): inventing
//    one here would be exactly the kind of new-subsystem guess that's out of scope for a
//    behavior-preserving port.
// TODO(SP): FUN_0057fc70 (Ghidra 0x0057fc70) -- unidentified global write, not ported.
// TODO(SP): FUN_004f46b0 (Ghidra 0x004f46b0) -- idle/attract-timer rearm, no reconstruction
// subsystem to attach it to, not ported.
// STILL BLOCKED, re-checked 2026-08-26 against the live database (19673 functions): BOTH
// addresses are still bare FUN_* -- no name has landed on either since these TODOs were written,
// so neither is any closer to being portable. The second one is blocked on a missing subsystem
// regardless of naming, so a future name would not unblock it by itself.
static void UI_Sp_DismissCinematic(int localClientNum)
{
    uiInfo_s *uiInfo = UI_GetInfo(localClientNum);

    Dvar_SetIntByName("cl_paused", 0);
    Cbuf_AddText(localClientNum, "fade 0 0 0 0 3\n");
    Menus_CloseAll(localClientNum, &uiInfo->uiDC);
    if ( R_Cinematic_IsNextReady_Internal() )
        R_Cinematic_StartNextPlayback();
    else
        R_Cinematic_StopPlayback();
    Dvar_SetBool((dvar_s *)com_introPlayed, true);
}
#endif

void __cdecl UI_Project_Refresh(int localClientNum)
{
#ifdef KISAK_SP
    // SP-only, Ghidra 0x00612B90 (called every UI_Refresh at retail 0x004AFE0A, as the very first
    // statement -- before this reconstruction's shared UI_SetShaderTime/emblem work below, which
    // matches retail's own call ordering). Per-frame SP cinematic/menu lifecycle: while the
    // "pregame" (briefing/loading) menu is active and the current cinematic has finished with
    // nothing queued or targeted, dismiss it and fall back to UISP_NONE; otherwise, once the
    // active cinematic finishes and a next one is ready, promote it. Confirmed by decompiling the
    // address directly.
    //
    // The two dvar-handle writes in retail's dismiss branch (two unidentified cached-dvar-pointer
    // globals, DAT_02562a3c/DAT_02598f5c) are the SAME pair already flagged unresolved on
    // UI_SetActiveMenuSp's case 8 above ("likely campaign mission/act progress trackers") --
    // confirmed by decompiling UI_SetActiveMenu (0x005852c0) case 8 directly, which writes -1 to
    // both. Left unported here for the same reason: no second independent fact to name or thread
    // them through.
    //
    // Retail structures this as nested ifs with a `goto` past the second block once the dismiss
    // path is taken; every other path (any conjunct false) falls into the second block instead.
    // That is exactly an if/else, transcribed as one below with no behavior change.
    if ( (int)UI_GetActiveMenu(localClientNum) == UISP_PREGAME && !Dvar_GetString("com_errorMessage")[0] &&
         R_Cinematic_IsFinished() && !R_Cinematic_IsTargetSet() && !R_Cinematic_IsNextReady_Internal() )
    {
        UI_Sp_DismissCinematic(localClientNum);
        UI_SetActiveMenuSp(localClientNum, UISP_NONE);
        // IMPLEMENTED 2026-08-26, retail 0x00612be2-0x00612c10 (immediately after the
        // UI_SetActiveMenu(localClientNum, 0) call at 0x00612bdd, then JMP past the second
        // block). The dvar pair is the one resolved on UI_SetActiveMenuSp's case 8 above:
        //   0x00612be2  MOV EAX,[ui_controllerPulled]; MOV EAX,[EAX+0x18]   (current.integer)
        //   0x00612bed  CMP EAX,-1 ; JZ  -> skip
        //   0x00612bfa  Dvar_SetInt(ui_pendingControllerPullEvent, EAX)
        //   0x00612c08  Dvar_SetInt(ui_controllerPulled, -1)
        // i.e. promote the pulled-controller index into the pending-event dvar and re-arm the
        // sentinel.
        //
        // DIVERGENCE, stated rather than hidden: retail reads a cached dvar_s* whose registered
        // default is -1, so before anything sets it the guard is false. Neither dvar is
        // registered in this reconstruction, and Dvar_GetInt returns 0 (not -1) for a name that
        // does not resolve, which would make the guard spuriously true on the first pass. The
        // existence test below restores retail's default so the transcription is behaviourally
        // faithful; it is not extra logic retail has, it stands in for retail's registration.
        {
            int controllerPulled = Dvar_FindVar("ui_controllerPulled")
                                 ? Dvar_GetInt("ui_controllerPulled")
                                 : -1;
            if ( controllerPulled != -1 )
            {
                Dvar_SetIntByName("ui_pendingControllerPullEvent", controllerPulled);
                Dvar_SetIntByName("ui_controllerPulled", -1);
            }
        }
    }
    else if ( R_Cinematic_IsFinished() && R_Cinematic_IsNextReady_Internal() )
    {
        R_Cinematic_WaitForUpdateFrame();
        R_Cinematic_StartNextPlayback();
        R_Cinematic_UpdateFrame(1);
        R_Cinematic_WaitForUpdateFrame();
    }
    // TODO(SP): retail's tail (0x00612c3d) also calls an unidentified ~500-byte redacted-intel
    // image-viewer helper (Ghidra 0x00424730, gated on Key_IsCatcherActive(localClientNum, 0x40)).
    // It consumes K_MOUSE1/K_MOUSE2 and updates the pan/zoom globals initialized while loading
    // "<name>_redact.csv" and consumed by the timed-redaction overlay renderer. Its behavior is
    // confirmed, but no exact original source symbol is attested. Not ported in this slice.
#endif
    UI_SetShaderTime(localClientNum);
    if ( localClientNum == Com_LocalClients_GetPrimary() && Key_IsCatcherActive(localClientNum, 128) )
        UI_EmblemUpdate(localClientNum);
}

void __cdecl UI_Project_Shutdown(int localClientNum)
{
    Key_ClearStates(localClientNum);
    if ( CL_LocalClient_IsFirstActive(localClientNum) )
        UI_ViewerShutdown();
}

void __cdecl UI_UpdateListboxPos_f()
{
    int i; // [esp+0h] [ebp-6Ch]
    const char *max; // [esp+4h] [ebp-68h]
    itemDef_s *item; // [esp+8h] [ebp-64h]
    char menuName[68]; // [esp+Ch] [ebp-60h] BYREF
    int controllerIndex; // [esp+54h] [ebp-18h]
    uiInfo_s *uiInfo; // [esp+58h] [ebp-14h]
    listBoxDef_s *listPtr; // [esp+5Ch] [ebp-10h]
    menuDef_t *menu; // [esp+60h] [ebp-Ch]
    int viewmax; // [esp+64h] [ebp-8h]
    int contextIndex; // [esp+68h] [ebp-4h]

    controllerIndex = 0;
    contextIndex = Com_ControllerIndex_GetUIContextIndex(0);
    Cmd_ArgsBuffer(1, menuName, 64);
    uiInfo = UI_UIContext_GetInfo(contextIndex);
    if ( Menu_Count(&uiInfo->uiDC) )
    {
        menu = Menus_FindByName(&uiInfo->uiDC, menuName);
        if ( menu )
        {
            for ( i = 0; i < menu->itemCount; ++i )
            {
                if ( Item_IsVisible(0, contextIndex, menu->items[i]) )
                {
                    if ( Window_HasFocus(contextIndex, &menu->items[i]->window) )
                    {
                        item = menu->items[i];
                        if ( item )
                        {
                            if ( item->dataType == 4 )
                            {
                                listPtr = Item_GetListBoxDef(item);
                                if ( !listPtr
                                    && !Assert_MyHandler(
                                                "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_main_mp.cpp",
                                                391,
                                                0,
                                                "%s",
                                                "listPtr") )
                                {
                                    __debugbreak();
                                }
                                max = Item_ListBox_MaxScroll(0, contextIndex, item);
                                viewmax = Item_ListBox_Viewmax(0, contextIndex, item);
                                Item_ListBox_SetCursorPos(0, contextIndex, item, (int)max, viewmax, listPtr->cursorPos[contextIndex], 1);
                                return;
                            }
                        }
                    }
                }
            }
        }
    }
}

void __cdecl UI_KeyClearStates_f()
{
    int LocalClientNum; // eax

    LocalClientNum = Com_ControllerIndex_GetLocalClientNum(0);
    Key_ClearStates(LocalClientNum);
}

void __cdecl UI_SetMap(char *mapname, const char *gametype)
{
    const char *v2; // eax

    I_strncpyz(g_mapname, mapname, 64);
    I_strncpyz(s_gametype, gametype, 64);
    g_showLoadingScreenMenu = *mapname != 0;
    if ( !*mapname || G_ExitAfterToolComplete() )
    {
        g_showLoadingScreenMenu = 0;
    }
    else
    {
        Dvar_SetStringByName("ui_mapname", mapname);
        g_showLoadingScreenMenu = 1;
    }
    if ( !useFastFile->current.enabled )
    {
        if ( g_mapname[0] )
        {
            v2 = va("maps/mp/%s.csv", g_mapname);
            UI_MapLoadInfo(v2);
        }
    }
}

void __cdecl UI_DrawLineGraphSegment(int contextIndex, float *p1, float *p2, rectDef_s *graphRect)
{
    float p2y; // [esp+0h] [ebp-28h] BYREF
    const ScreenPlacement *scrPlace; // [esp+4h] [ebp-24h]
    GfxQuadVertex *quad; // [esp+8h] [ebp-20h]
    float p1x; // [esp+Ch] [ebp-1Ch] BYREF
    float p2x; // [esp+10h] [ebp-18h] BYREF
    float x; // [esp+14h] [ebp-14h] BYREF
    float y; // [esp+18h] [ebp-10h] BYREF
    float p1y; // [esp+1Ch] [ebp-Ch] BYREF
    float h; // [esp+20h] [ebp-8h] BYREF
    float w; // [esp+24h] [ebp-4h] BYREF

    scrPlace = &scrPlaceView[contextIndex];
    p1x = *p1;
    p1y = p1[1];
    p2x = *p2;
    p2y = p2[1];
    x = graphRect->x;
    y = graphRect->y;
    w = graphRect->w;
    h = graphRect->h;
    ScrPlace_ApplyRect(scrPlace, &p1x, &p1y, &w, &h, graphRect->horzAlign, graphRect->vertAlign);
    ScrPlace_ApplyRect(scrPlace, &p2x, &p2y, &w, &h, graphRect->horzAlign, graphRect->vertAlign);
    w = graphRect->w;
    h = graphRect->h;
    ScrPlace_ApplyRect(scrPlace, &x, &y, &w, &h, graphRect->horzAlign, graphRect->vertAlign);
    quad = (GfxQuadVertex *)R_AddCmdDrawQuadList2D(sharedUiInfo.assets.lineGraph, 1);
    quad->xy[0] = p1x;
    quad->xy[1] = p1y;
    quad->st[0] = (float)(p1x - x) / w;
    quad->st[1] = (float)(p1y - y) / h;
    quad->color.packed = -1;
    quad[3].xy[0] = p2x;
    quad[3].xy[1] = p2y;
    quad[3].st[0] = (float)(p2x - x) / w;
    quad[3].st[1] = (float)(p2y - y) / h;
    quad[3].color.packed = -1;
    quad[1].xy[0] = p1x;
    quad[1].xy[1] = y + h;
    quad[1].st[0] = (float)(p1x - x) / w;
    quad[1].st[1] = 1.0f;
    quad[1].color.packed = -1;
    quad[2].xy[0] = p2x;
    quad[2].xy[1] = y + h;
    quad[2].st[0] = (float)(p2x - x) / w;
    quad[2].st[1] = 1.0f;
    quad[2].color.packed = -1;
}

void __cdecl UI_GetGameTypeOnMapName(char *outputString, const char *inputMapName, const char *inputGameType)
{
    char *gametypeOnMap; // [esp+0h] [ebp-2Ch]
    ConversionArguments convArgs; // [esp+4h] [ebp-28h] BYREF

    gametypeOnMap = UI_SafeTranslateString("MPUI_GAMETYPE_ON_MAP");
    memset(&convArgs.args[2], 0, 28);
    convArgs.argCount = 2;
    convArgs.args[0] = UI_GetGameTypeName(inputGameType, 0);
    convArgs.args[1] = UI_GetMapName(inputMapName, 0);
    UI_ReplaceConversions(gametypeOnMap, &convArgs, outputString, 4);
}

char *__cdecl UI_GetMapName(const char *inputMapName, bool returnStringRef)
{
    const char *v2; // eax
    const StringTable *mapTable; // [esp+0h] [ebp-8h] BYREF

    StringTable_GetAsset("mp/mapsTable.csv", (XAssetHeader *)&mapTable);
    if ( !mapTable
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_main_mp.cpp", 1028, 0, "%s", "mapTable") )
    {
        __debugbreak();
    }
    if ( returnStringRef )
        return (char *)StringTable_Lookup(mapTable, 0, inputMapName, 3);
    v2 = StringTable_Lookup(mapTable, 0, inputMapName, 3);
    return UI_SafeTranslateString(v2);
}

char *__cdecl UI_GetGameTypeName(const char *inputGameType, bool returnStringRef)
{
    const char *v2; // eax
    const StringTable *gameTypeTable; // [esp+4h] [ebp-4h] BYREF

    StringTable_GetAsset("mp/gametypesTable.csv", (XAssetHeader *)&gameTypeTable);
    if ( !gameTypeTable
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_main_mp.cpp", 1044, 0, "%s", "gameTypeTable") )
    {
        __debugbreak();
    }
    if ( returnStringRef )
        return (char *)StringTable_Lookup(gameTypeTable, 0, inputGameType, 1);
    v2 = StringTable_Lookup(gameTypeTable, 0, inputGameType, 1);
    return UI_SafeTranslateString(v2);
}

unsigned int __cdecl UI_GetOffsetForTextureCoords(
                unsigned int xPos,
                unsigned int yPos,
                GfxImage *img,
                unsigned int bytesPerPixel)
{
    return bytesPerPixel * (xPos + yPos * img->width);
}

void __cdecl UI_GenerateHeatMapTextureCallback(GfxImage *param)
{
#ifdef KISAK_HEATMAP // probably requires DW? for MatchRecord stuff
    _D3DLOCKED_RECT lock; // [esp+0h] [ebp-Ch] BYREF
    GfxImage *img; // [esp+8h] [ebp-4h]

    img = param;
    ((void (__stdcall *)(unsigned int, unsigned int, _D3DLOCKED_RECT *, unsigned int, int))param->texture.basemap->__vftable[1].Release)(
        (GfxTexture)param->texture.basemap,
        0,
        &lock,
        0,
        0x2000);
    memcpy((unsigned __int8 *)lock.pBits, img->pixels, 4 * img->height * img->width);
    img->texture.basemap->__vftable[1].GetDevice(img->texture.basemap, 0);

    Z_VirtualFree(img->pixels, 19);
    img->pixels = 0;
#endif
}

void __cdecl UI_GenerateHeatMapTexture(int controllerIndex)
{
#ifdef KISAK_HEATMAP // probably requires DW? for MatchRecord stuff
    const char *String; // eax
    unsigned __int64 v2; // rax
    unsigned __int8 *pixels; // [esp+0h] [ebp-1Ch]
    LargeLocal heatmap_large_local(0x10000); // [esp+4h] [ebp-18h] BYREF
    unsigned __int8 (*heatmap)[65536]; // [esp+Ch] [ebp-10h]
    GfxImage *img; // [esp+10h] [ebp-Ch]
    unsigned __int64 xuid; // [esp+14h] [ebp-8h]

    //LargeLocal::LargeLocal(&heatmap_large_local, 0x10000);
    //heatmap = (unsigned __int8 (*)[65536])LargeLocal::GetBuf(&heatmap_large_local);
    heatmap = (unsigned __int8 (*)[65536])heatmap_large_local.GetBuf();
    img = Image_Register("heatmap", 0, 3);
    pixels = (unsigned __int8 *)Z_VirtualAlloc(4 * img->height * img->width, "UI_GenerateHeatMapTexture", 19);
    img->pixels = pixels;
    if ( !img && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_main_mp.cpp", 1258, 0, "%s", "img") )
        __debugbreak();
    if ( img->height * (unsigned int)img->width > 0x10000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_main_mp.cpp",
                    1259,
                    0,
                    "%s",
                    "img->width * img->height <= sizeof( heatmap )") )
    {
        __debugbreak();
    }
    if ( img->width > 1u || img->height > 1u )
    {
        xuid = 0;
        MatchRecord_GenerateHeatMapData((unsigned __int8 *)heatmap, 0x10000, img->width, img->height, 0);
        UI_GenerateHeatMapTextureInternal(img, (unsigned __int8 *)heatmap, 0x10000, xuid, ui_heatMapColor);
        if ( Dvar_GetBool("ui_showFriendsCombatRecord") || Dvar_GetBool("ui_playercardOpen") )
        {
            String = Dvar_GetString("selectedPlayerXuid");
            xuid = I_atoi64(String);
        }
        else
        {
            LODWORD(v2) = Live_GetXuid(controllerIndex);
            xuid = v2;
        }
        MatchRecord_GenerateHeatMapData((unsigned __int8 *)heatmap, 0x10000, img->width, img->height, xuid);
        UI_GenerateHeatMapTextureInternal(img, (unsigned __int8 *)heatmap, 0x10000, xuid, ui_heatMapColorForPlayer);
        RB_Resource_CallbackParam((void (__cdecl *)(void *))UI_GenerateHeatMapTextureCallback, img);
        LargeLocal::~LargeLocal(&heatmap_large_local);
    }
    else
    {
        LargeLocal::~LargeLocal(&heatmap_large_local);
    }
#endif
}

void __cdecl UI_GenerateHeatMapTextureInternal(
                GfxImage *img,
                unsigned __int8 *heatmap,
                int heatmapSize,
                unsigned __int64 xuid,
                const dvar_s *dvarColor)
{
    unsigned __int8 *pixel; // [esp+4h] [ebp-1Ch]
    unsigned __int8 alpha; // [esp+Bh] [ebp-15h]
    signed int xPos; // [esp+Ch] [ebp-14h]
    int yOffset; // [esp+10h] [ebp-10h]
    int i; // [esp+14h] [ebp-Ch]
    int yPos; // [esp+18h] [ebp-8h]
    __int16 green; // [esp+1Dh] [ebp-3h]
    unsigned __int8 red; // [esp+1Fh] [ebp-1h]

    if ( !img && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_main_mp.cpp", 1142, 0, "%s", "img") )
        __debugbreak();
    if ( img && img->width > 1u && img->height > 1u && img->height * img->width <= heatmapSize )
    {
        if ( !xuid )
        {
            for ( yPos = 0; yPos < img->height; ++yPos )
                memset(&img->pixels[4 * yPos * img->width], 0, 4 * img->width);
        }
        if ( dvarColor )
        {
            red = dvarColor->current.color[0];
            green = *(_WORD *)(&dvarColor->current.enabled + 1);
        }
        else
        {
            red = -1;
            green = -1;
        }
        for ( i = 0; i < img->height; ++i )
        {
            yOffset = i * img->width;
            for ( xPos = 0; xPos < img->width; ++xPos )
            {
                alpha = heatmap[xPos + yOffset];
                if ( xuid && alpha == 255 || !xuid && alpha )
                {
                    pixel = &img->pixels[UI_GetOffsetForTextureCoords(xPos, i, img, 4u)];
                    *pixel++ = HIBYTE(green);
                    *pixel++ = green;
                    *pixel = red;
                    pixel[1] = alpha;
                }
            }
        }
    }
}

void __cdecl UI_DrawStatsMilestonesFeederProgressBar(
                int controllerIndex,
                int contextIndex,
                const rectDef_s *rect,
                unsigned int index,
                int type)
{
    float v5; // [esp+24h] [ebp-6Ch]
    float v6; // [esp+28h] [ebp-68h]
    float v7; // [esp+2Ch] [ebp-64h]
    float v8; // [esp+30h] [ebp-60h]
    float v9; // [esp+34h] [ebp-5Ch]
    float v10; // [esp+38h] [ebp-58h]
    float v11; // [esp+3Ch] [ebp-54h]
    float v12; // [esp+40h] [ebp-50h]
    float v13; // [esp+44h] [ebp-4Ch]
    int i; // [esp+60h] [ebp-30h]
    challengeInfo_t *challenge; // [esp+64h] [ebp-2Ch] BYREF
    float barColorFilled[4]; // [esp+68h] [ebp-28h] BYREF
    float partitionWidth; // [esp+78h] [ebp-18h]
    float attributeValue; // [esp+7Ch] [ebp-14h]
    float barColorEmpty[4]; // [esp+80h] [ebp-10h] BYREF

    barColorEmpty[0] = 0.96f;
    barColorEmpty[1] = 0.57999998;
    barColorEmpty[2] = 0.11;
    barColorEmpty[3] = 0.2f;
    barColorFilled[0] = 0.96f;
    barColorFilled[1] = 0.57999998;
    barColorFilled[2] = 0.11;
    barColorFilled[3] = 1.0f;
    if ( LiveStats_GetChallengeInfo(&challenge, index, type) )
    {
        attributeValue = (float)(6 * challenge->currentValue) / (float)challenge->targetValue;
        partitionWidth = rect->w / 6.0;
        for ( i = 0; i < 6; ++i )
        {
            UI_DrawHandlePic(
                &scrPlaceView[contextIndex],
                (float)((float)i * partitionWidth) + rect->x,
                rect->y,
                partitionWidth - 1.0,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                barColorEmpty,
                sharedUiInfo.assets.whiteMaterial);
            v13 = (float)(i + 1);
            v12 = floor(attributeValue);
            if ( v12 >= (double)v13 )
                UI_DrawHandlePic(
                    &scrPlaceView[contextIndex],
                    (float)((float)i * partitionWidth) + rect->x,
                    rect->y,
                    partitionWidth - 1.0,
                    rect->h,
                    rect->horzAlign,
                    rect->vertAlign,
                    barColorFilled,
                    sharedUiInfo.assets.whiteMaterial);
        }
        v11 = ceil(attributeValue);
        v10 = floor(attributeValue);
        if ( v11 <= (double)v10 )
        {
            v7 = ceil(attributeValue);
            v6 = floor(attributeValue);
            if ( v7 == v6 )
            {
                v5 = floor(attributeValue);
                UI_DrawHandlePicRotated(
                    &scrPlaceView[contextIndex],
                    (float)((float)((float)(v5 * partitionWidth) + rect->x) - 3.0) - 2.0,
                    rect->y + rect->h,
                    7.0,
                    7.0,
                    rect->horzAlign,
                    rect->vertAlign,
                    barColorFilled,
                    -90.0,
                    sharedUiInfo.assets.sliderThumb);
            }
        }
        else
        {
            v9 = floor(attributeValue);
            UI_DrawHandlePic(
                &scrPlaceView[contextIndex],
                (float)(v9 * partitionWidth) + rect->x,
                rect->y,
                partitionWidth / 2.0,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                barColorFilled,
                sharedUiInfo.assets.whiteMaterial);
            v8 = floor(attributeValue);
            UI_DrawHandlePicRotated(
                &scrPlaceView[contextIndex],
                (float)((float)((float)(v8 * partitionWidth) + rect->x) + (float)(partitionWidth / 2.0)) - 3.0,
                rect->y + rect->h,
                7.0,
                7.0,
                rect->horzAlign,
                rect->vertAlign,
                barColorFilled,
                -90.0,
                sharedUiInfo.assets.sliderThumb);
        }
    }
}

void __cdecl UI_Project_OwnerDraw(
                int localClientNum,
                itemDef_s *item,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                int ownerDraw,
                int ownerDrawFlags,
                float *color,
                Material *material,
                rectDef_s parentRect,
                const char *dvarName)
{
    int ControllerIndex; // eax
    int contextIndex; // [esp+Ch] [ebp-24h]
    rectDef_s rect; // [esp+10h] [ebp-20h] BYREF
    float histSamples[2]; // [esp+28h] [ebp-8h] BYREF

    histSamples[0] = 0.0f;
    histSamples[1] = 0.0f;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    rect.horzAlign = horzAlign;
    rect.vertAlign = vertAlign;
    contextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
    if ( contextIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_main_mp.cpp",
                    1818,
                    0,
                    "%s",
                    "contextIndex != INVALID_UI_CONTEXT") )
    {
        __debugbreak();
    }
    if ( ownerDraw == 342 )
    {
        if ( LODWORD(item->typeData.textDef->textRect[0].x) )
            histSamples[0] = GetExpressionFloat(localClientNum, item, (ExpressionStatement *)item->typeData.textDef);
        if ( item->forecolorAExp.filename )
            histSamples[1] = GetExpressionFloat(localClientNum, item, &item->forecolorAExp);
    }
    if ( ownerDraw > 385 )
    {
        if ( ownerDraw == 387 )
            Live_FileShare_PrivateRatingOwnerdraw(contextIndex, &rect, item, color);
    }
    else if ( ownerDraw == 385 )
    {
        ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        Live_FileShare_MyRatingOwnerdraw(ControllerIndex, contextIndex, &rect, item, color);
    }
    else
    {
        switch ( ownerDraw )
        {
            case 131:
                //BLOPS_NULLSUB();
                break;
            case 133:
                UI_DrawBlurMaterial(contextIndex, &rect, color);
                break;
            case 208:
                UI_DrawReticlePreview(localClientNum, contextIndex, item, &rect);
                break;
            case 209:
                UI_DrawLensPreview(localClientNum, contextIndex, item, &rect, color);
                break;
            case 334:
                UI_DrawHeatMap(contextIndex, &rect, color);
                break;
            case 340:
                UI_DrawCombatRecordPieChart(contextIndex, &rect, color, dvarName);
                break;
            case 341:
                UI_DrawCombatRecordLineGraph(contextIndex, &rect, color, dvarName);
                break;
            case 342:
                UI_DrawCombatRecordHistogram(contextIndex, &rect, color, dvarName, histSamples);
                break;
            case 343:
                UI_DrawCombatRecordBarGraph(contextIndex, &rect);
                break;
            case 344:
                UI_DrawCombatRecordHorizontalBarGraph(localClientNum, contextIndex, &rect, &parentRect, color, dvarName);
                break;
            case 347:
                UI_DrawCombatRecordHitLocHeatMap(contextIndex, &rect);
                break;
            case 361:
                Live_FileShare_RatingOwnerdraw(contextIndex, &rect, item, color);
                break;
            case 362:
                Live_FileShare_SubmitRatingOwnerdraw(contextIndex, &rect, item, color);
                break;
            case 363:
                Live_FileShare_AvgSubmitRatingOwnerdraw(contextIndex, &rect, item, color);
                break;
            case 364:
                UI_ScreenshotDraw(localClientNum, contextIndex, &rect, color);
                break;
            case 381:
                UI_DrawAttributeBar(localClientNum, contextIndex, item, &rect, dvarName);
                break;
            default:
                return;
        }
    }
}

void __cdecl UI_DrawBlurMaterial(int contextIndex, rectDef_s *rect, float *color)
{
    unsigned __int16 CodeConst_GenericParam0; // ax
    float shaderConstants[4]; // [esp+20h] [ebp-14h] BYREF
    float intensity; // [esp+30h] [ebp-4h]

    intensity = color[3];
    if ( intensity > 0.0 )
    {
        shaderConstants[0] = intensity;
        shaderConstants[1] = ui_blurDarkenAmount->current.value;
        shaderConstants[2] = 0.0f;
        shaderConstants[3] = 0.0f;
        CodeConst_GenericParam0 = GetCodeConst_GenericParam0();
        CL_SetCustomConstant(CodeConst_GenericParam0, shaderConstants);
        R_UI3D_SetBlurRadius(intensity * ui_blurAmount->current.value);
        color[3] = 1.0f;
        UI_DrawHandlePic(
            &scrPlaceView[contextIndex],
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            color,
            sharedUiInfo.assets.blur);
    }
    else
    {
        R_UI3D_SetBlurRadius(0.0);
    }
}

void __cdecl UI_DrawCombatRecordPieChart(int contextIndex, rectDef_s *rect, float *color, const char *dvarName)
{
    bool Bool; // al
    unsigned __int16 CodeConst_GenericParam0; // ax
    Material *v6; // eax
    unsigned __int16 v7; // ax
    Material *v8; // eax
    float v9; // xmm0_4
    float v10; // xmm0_4
    float v11; // xmm0_4
    float v12; // xmm0_4
    float pt1[2]; // [esp+44h] [ebp-7Ch] BYREF
    float param[4]; // [esp+4Ch] [ebp-74h] BYREF
    float lineColor[4]; // [esp+5Ch] [ebp-64h] BYREF
    int totalItemTypes; // [esp+6Ch] [ebp-54h]
    int totalItems; // [esp+70h] [ebp-50h]
    int pieChartType; // [esp+74h] [ebp-4Ch]
    float pt2[2]; // [esp+78h] [ebp-48h] BYREF
    int i; // [esp+80h] [ebp-40h]
    float pieColors[3][4]; // [esp+84h] [ebp-3Ch] BYREF
    int numItems[3]; // [esp+B4h] [ebp-Ch]

    pieChartType = Dvar_GetInt(dvarName);
    currIndex = 0;
    startIndex = 0;
    endIndex = 0;
    totalItemTypes = 0;
    totalItems = 0;
    switch (pieChartType)
    {
    case 0:
        goto LABEL_4;
    case 1:
        startIndex = 3;
        endIndex = 6;
        break;
    case 2:
    LABEL_4:
        startIndex = 0;
        endIndex = 3;
        break;
    }
    while (startIndex < endIndex)
    {
        Bool = Dvar_GetBool("ui_showFriendsCombatRecord");
        numItems[currIndex] = LiveCombatRecord_GetSortedItemData(startIndex, Bool, SORT_KEY);
        if (!numItems[currIndex])
            break;
        ++totalItemTypes;
        totalItems += numItems[currIndex];
        ++startIndex;
        ++currIndex;
    }
    pt1[0] = 0.0f;
    pt1[1] = 0.0f;
    pt2[0] = 0.0f;
    pt2[1] = 0.0f;
    param[2] = 0.0f;
    param[3] = 0.0f;
    //*&pieColors[0][0] = __PAIR64__(LODWORD(0.58f), LODWORD(0.96f));
    pieColors[0][0] = 0.96f;
    pieColors[0][1] = 0.58f;
    //*&pieColors[0][2] = __PAIR64__(LODWORD(1.0f), LODWORD(0.11f));
    pieColors[0][2] = 0.11f;
    pieColors[0][3] = 1.0f;
    pieColors[1][0] = 0.17f;
    pieColors[1][1] = 0.17f;
    //*&pieColors[1][2] = __PAIR64__(LODWORD(1.0f), LODWORD(0.17f));
    pieColors[1][2] = 0.17f;
    pieColors[1][3] = 1.0f;
    pieColors[2][0] = 0.57f;
    pieColors[2][1] = 0.57f;
    pieColors[2][2] = 0.57f;
    pieColors[2][3] = 1.0f;
    //*&pieColors[2][2] = __PAIR64__(LODWORD(1.0f), LODWORD(0.57f));
    param[0] = 0.0f;
    param[1] = M_2_PI; 
    memset(lineColor, 0, 12);
    lineColor[3] = 0.8f;
    CodeConst_GenericParam0 = GetCodeConst_GenericParam0();
    CL_SetCustomConstant(CodeConst_GenericParam0, param);
    v6 = Material_RegisterHandle("piechart", 3);
    UI_DrawHandlePic(
        &scrPlaceView[contextIndex],
        rect->x,
        rect->y,
        rect->w,
        rect->h,
        rect->horzAlign,
        rect->vertAlign,
        pieColors[0],
        v6);
    param[0] = 4.712389 - ((numItems[1] / totalItems) * 6.2831855);
    for (i = 1; i < totalItemTypes; ++i)
    {
        param[1] = (numItems[i] / totalItems) * 6.2831855;
        v7 = GetCodeConst_GenericParam0();
        CL_SetCustomConstant(v7, param);
        v8 = Material_RegisterHandle("piechart", 3);
        UI_DrawHandlePic(
            &scrPlaceView[contextIndex],
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            pieColors[i],
            v8);
        pt1[0] = (rect->w / 2.0) + rect->x;
        pt1[1] = (rect->h / 2.0) + rect->y;
        //v9 = __libm_sse2_cos(param[0]);
        v9 = cos(param[0]);
        pt2[0] = ((rect->w / 2.0) + rect->x) + ((rect->w / 2.0) * v9);
        //v10 = __libm_sse2_sin(param[0]);
        v10 = sin(param[0]);
        pt2[1] = ((rect->h / 2.0) + rect->y) - ((rect->h / 2.0) * v10);
        UI_DrawPrettyLine(contextIndex, pt1, pt2, 1.0, rect->horzAlign, rect->vertAlign, lineColor);
        param[0] = param[0] + param[1];
    }
    if (totalItemTypes > 1)
    {
        //v11 = __libm_sse2_cos(param[0]);
        v11 = cos(param[0]);
        pt2[0] = ((rect->w / 2.0) + rect->x) + ((rect->w / 2.0) * v11);
        //v12 = __libm_sse2_sin(param[0]);
        v12 = sin(param[0]);
        pt2[1] = ((rect->h / 2.0) + rect->y) - ((rect->h / 2.0) * v12);
        UI_DrawPrettyLine(contextIndex, pt1, pt2, 1.0, rect->horzAlign, rect->vertAlign, lineColor);
    }
}

void __cdecl UI_DrawCombatRecordLineGraph(int contextIndex, rectDef_s *rect, float *color, const char *dvarName)
{
    Material *v4; // eax
    float v5; // xmm0_4
    double v6; // xmm0_8
    float v7; // xmm0_4
    float v8; // xmm0_4
    double v9; // xmm0_8
    float v10; // xmm0_4
    Material *v11; // eax
    float maxKDRatioValue; // [esp+20h] [ebp-C0h]
    float white[4]; // [esp+24h] [ebp-BCh] BYREF
    float pt1[2]; // [esp+34h] [ebp-ACh] BYREF
    int maxValues; // [esp+3Ch] [ebp-A4h]
    float midpoint; // [esp+40h] [ebp-A0h]
    float green[4]; // [esp+44h] [ebp-9Ch]
    int numRecentValues; // [esp+54h] [ebp-8Ch]
    float separation; // [esp+58h] [ebp-88h]
    float red[4]; // [esp+5Ch] [ebp-84h]
    float currY; // [esp+6Ch] [ebp-74h]
    int startIndex; // [esp+70h] [ebp-70h]
    bool forFriend; // [esp+77h] [ebp-69h]
    int currGraph; // [esp+78h] [ebp-68h]
    int currIndex; // [esp+7Ch] [ebp-64h]
    float recentKDRatioValues[10]; // [esp+80h] [ebp-60h]
    float pt2[2]; // [esp+A8h] [ebp-38h] BYREF
    float startpoint; // [esp+B0h] [ebp-30h]
    float nextY; // [esp+B4h] [ebp-2Ch]
    float dotColor[4]; // [esp+B8h] [ebp-28h] BYREF
    float halfHeight; // [esp+C8h] [ebp-18h]
    float dotDimension; // [esp+CCh] [ebp-14h]
    float gray[4]; // [esp+D0h] [ebp-10h] BYREF

    forFriend = Dvar_GetBool("ui_showFriendsCombatRecord");
    currGraph = Dvar_GetInt(dvarName);
    maxValues = 0;
    startIndex = 0;
    if ( currGraph == 1 )
    {
        maxValues = 10;
        startIndex = 0;
    }
    else if ( currGraph == 3 )
    {
        maxValues = 5;
        startIndex = 1;
    }
    numRecentValues = 0;
    maxKDRatioValue = 0.0f;
    for ( currIndex = 0;
                currIndex < maxValues && (int)LiveCombatRecord_GetSortedItemData(startIndex, forFriend, ITEM_INDEX);
                ++currIndex )
    {
        recentKDRatioValues[currIndex] = LiveCombatRecord_GetSortedItemData(startIndex, forFriend, SORT_KEY);
        if ( recentKDRatioValues[currIndex] > maxKDRatioValue )
            maxKDRatioValue = recentKDRatioValues[currIndex];
        ++numRecentValues;
        ++startIndex;
    }
    if ( numRecentValues >= 2 )
    {
        startpoint = rect->y;
        midpoint = (float)(rect->h / 2.0) + startpoint;
        halfHeight = rect->h / 2.0;
        currY = midpoint;
        nextY = 0.0f;
        dotDimension = rect->h / 11.5;
        separation = rect->w / (float)(maxValues - 1);
        gray[0] = 1.0f;
        gray[1] = 1.0f;
        gray[2] = 1.0f;
        gray[3] = 0.1f;
        white[0] = 1.0f;
        white[1] = 1.0f;
        white[2] = 1.0f;
        white[3] = 1.0f;
        red[0] = 0.73f;
        red[1] = 0.19f;
        red[2] = 0.19f;
        red[3] = 1.0f;
        green[0] = 0.42f;
        green[1] = 0.68f;
        green[2] = 0.46f;
        green[3] = 1.0f;
        dotColor[0] = 0.42f;
        dotColor[1] = 0.68f;
        dotColor[2] = 0.46f;
        dotColor[3] = 1.0f;
        pt1[0] = 0.0f;
        pt1[1] = 0.0f;
        pt2[0] = 0.0f;
        pt2[1] = 0.0f;
        for ( startIndex = 0; startIndex < maxValues; ++startIndex )
        {
            v4 = Material_RegisterHandle("white", 3);
            UI_DrawHandlePic(
                &scrPlaceView[contextIndex],
                (float)((float)startIndex * separation) + rect->x,
                rect->y,
                1.0,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                gray,
                v4);
        }
        currIndex = numRecentValues - 1;
        startIndex = 0;
        while ( currIndex >= 0 )
        {
            if ( recentKDRatioValues[currIndex] >= 1.0 )
            {
                if ( maxKDRatioValue > 2.0 )
                    v6 = (float)(startpoint + halfHeight)
                         - (recentKDRatioValues[currIndex] - 1.0) / (maxKDRatioValue - 1.0) * halfHeight;
                else
                    v6 = startpoint + (2.0 - recentKDRatioValues[currIndex]) * halfHeight;
                v7 = v6;
                currY = v7;
                dotColor[0] = green[0];
                dotColor[1] = green[1];
                dotColor[2] = green[2];
                dotColor[3] = green[3];
            }
            else
            {
                v5 = midpoint + (1.0 - recentKDRatioValues[currIndex]) * halfHeight;
                currY = v5;
                dotColor[0] = red[0];
                dotColor[1] = red[1];
                dotColor[2] = red[2];
                dotColor[3] = red[3];
            }
            if ( recentKDRatioValues[currIndex - 1] >= 1.0 )
            {
                if ( maxKDRatioValue > 2.0 )
                    v9 = (float)(startpoint + halfHeight)
                         - (recentKDRatioValues[currIndex - 1] - 1.0) / (maxKDRatioValue - 1.0) * halfHeight;
                else
                    v9 = startpoint + (2.0 - recentKDRatioValues[currIndex - 1]) * halfHeight;
                v10 = v9;
                nextY = v10;
            }
            else
            {
                v8 = midpoint + (1.0 - recentKDRatioValues[currIndex - 1]) * halfHeight;
                nextY = v8;
            }
            if ( startIndex < numRecentValues - 1 )
            {
                pt1[0] = (float)((float)startIndex * separation) + rect->x;
                pt1[1] = currY;
                pt2[0] = (float)((float)(startIndex + 1) * separation) + rect->x;
                pt2[1] = nextY;
                UI_DrawPrettyLine(contextIndex, pt1, pt2, 1.0, rect->horzAlign, rect->vertAlign, white);
            }
            v11 = Material_RegisterHandle("white", 3);
            UI_DrawHandlePic(
                &scrPlaceView[contextIndex],
                (float)((float)((float)startIndex * separation) + rect->x) - (float)(dotDimension / 2.0),
                currY - (float)(dotDimension / 2.0),
                dotDimension,
                dotDimension,
                rect->horzAlign,
                rect->vertAlign,
                dotColor,
                v11);
            --currIndex;
            ++startIndex;
        }
    }
}

void __cdecl UI_DrawCombatRecordHistogram(
                int contextIndex,
                rectDef_s *rect,
                float *color,
                const char *dvarName,
                float *samples)
{
    float pt1[2]; // [esp+10h] [ebp-24h] BYREF
    float separation; // [esp+18h] [ebp-1Ch]
    float lineColor[4]; // [esp+1Ch] [ebp-18h] BYREF
    float pt2[2]; // [esp+2Ch] [ebp-8h] BYREF

    pt1[0] = 0.0f;
    pt1[1] = 0.0f;
    pt2[0] = 0.0f;
    pt2[1] = 0.0f;
    lineColor[0] = 1.0f;
    lineColor[1] = 1.0f;
    lineColor[2] = 1.0f;
    lineColor[3] = 0.2f;
    separation = rect->w / 10.0;
    if ( rect->w != 0.0 && rect->h != 0.0 )
    {
        pt1[0] = rect->x;
        pt1[1] = (float)(rect->y + rect->h) - (float)(*samples * rect->h);
        pt2[0] = rect->x + separation;
        pt2[1] = (float)(rect->y + rect->h) - (float)(samples[1] * rect->h);
        UI_DrawLineGraphSegment(contextIndex, pt1, pt2, rect);
        UI_DrawPrettyLine(contextIndex, pt1, pt2, 2.0, rect->horzAlign, rect->vertAlign, lineColor);
    }
}

void __cdecl UI_DrawCombatRecordBarGraph(int contextIndex, rectDef_s *rect)
{
    double SortedItemData; // st7
    Material *v3; // eax
    float barColorRed[4]; // [esp+28h] [ebp-74h] BYREF
    int maxEarnings; // [esp+38h] [ebp-64h]
    int recentEarnings[10]; // [esp+3Ch] [ebp-60h]
    int numRecentValues; // [esp+64h] [ebp-38h]
    float separation; // [esp+68h] [ebp-34h]
    float currY; // [esp+6Ch] [ebp-30h]
    int startIndex; // [esp+70h] [ebp-2Ch]
    float barWidth; // [esp+74h] [ebp-28h]
    bool forFriend; // [esp+7Bh] [ebp-21h]
    float midPoint; // [esp+7Ch] [ebp-20h]
    int currIndex; // [esp+80h] [ebp-1Ch]
    float barColorGreen[4]; // [esp+84h] [ebp-18h] BYREF
    float barHeight; // [esp+94h] [ebp-8h]
    float (*barColor)[4]; // [esp+98h] [ebp-4h]

    forFriend = Dvar_GetBool("ui_showFriendsCombatRecord");
    numRecentValues = 0;
    maxEarnings = 0;
    startIndex = 0;
    for ( currIndex = 0;
                currIndex < 10 && (int)LiveCombatRecord_GetSortedItemData(currIndex, forFriend, ITEM_INDEX);
                ++currIndex )
    {
        SortedItemData = LiveCombatRecord_GetSortedItemData(currIndex, forFriend, PARAM1);
        recentEarnings[currIndex] = (int)SortedItemData;
        ++numRecentValues;
        if ( (int)abs(recentEarnings[currIndex]) > maxEarnings )
            maxEarnings = abs(recentEarnings[currIndex]);
    }
    if ( maxEarnings )
    {
        barColorRed[0] = 0.73f;
        barColorRed[1] = 0.19f;
        barColorRed[2] = 0.19f;
        barColorRed[3] = 1.0f;
        barColorGreen[0] = 0.42f;
        barColorGreen[1] = 0.68f;
        barColorGreen[2] = 0.46f;
        barColorGreen[3] = 1.0f;
        currY = 0.0f;
        midPoint = (float)(rect->h / 2.0) + rect->y;
        barHeight = 0.0f;
        barWidth = (float)(rect->w / 10.0) * 0.66666669;
        separation = rect->w / 10.0;
        currIndex = numRecentValues - 1;
        startIndex = 0;
        while ( currIndex >= 0 )
        {
            barHeight = (float)(fabs((float)recentEarnings[currIndex])
                                                / (float)maxEarnings)
                                * (float)(rect->h / 2.0);
            if ( recentEarnings[currIndex] <= 0 )
            {
                currY = midPoint;
                barColor = (float (*)[4])barColorRed;
            }
            else
            {
                currY = midPoint - barHeight;
                barColor = (float (*)[4])barColorGreen;
            }
            v3 = Material_RegisterHandle("white", 3);
            UI_DrawHandlePic(
                &scrPlaceView[contextIndex],
                (float)((float)startIndex * separation) + rect->x,
                currY,
                barWidth,
                barHeight,
                rect->horzAlign,
                rect->vertAlign,
                (const float *)barColor,
                v3);
            --currIndex;
            ++startIndex;
        }
    }
}

void __cdecl UI_DrawCombatRecordHorizontalBarGraph(
                int localClientNum,
                int contextIndex,
                rectDef_s *rect,
                rectDef_s *parentRect,
                float *color,
                const char *dvarName)
{
    int NumContractsInProgress; // eax
    Material *v7; // eax
    Material *v8; // eax
    Material *v9; // eax
    int v10; // [esp+20h] [ebp-68h]
    int i; // [esp+24h] [ebp-64h]
    float white[4]; // [esp+28h] [ebp-60h] BYREF
    int graphType; // [esp+38h] [ebp-50h]
    int controllerIndex; // [esp+3Ch] [ebp-4Ch]
    float green[4]; // [esp+40h] [ebp-48h] BYREF
    float redBarWidth; // [esp+50h] [ebp-38h]
    float red[4]; // [esp+54h] [ebp-34h] BYREF
    float greenBarWidth; // [esp+64h] [ebp-24h]
    bool forFriend; // [esp+6Bh] [ebp-1Dh]
    int power; // [esp+6Ch] [ebp-1Ch]
    float barHeight; // [esp+70h] [ebp-18h]
    int greenBarValue; // [esp+74h] [ebp-14h] BYREF
    int redBarValue; // [esp+78h] [ebp-10h] BYREF
    int firstDigit; // [esp+7Ch] [ebp-Ch]
    int numGridLines; // [esp+80h] [ebp-8h]
    float maxValue; // [esp+84h] [ebp-4h]

    graphType = Dvar_GetInt(dvarName);
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    greenBarWidth = 0.0f;
    redBarWidth = 0.0f;
    barHeight = rect->h / 10.0;
    greenBarValue = 0;
    redBarValue = 0;
    firstDigit = 0;
    power = 1;
    maxValue = 0.0f;
    forFriend = Dvar_GetBool("ui_showFriendsCombatRecord");
    if ( graphType == 16 )
    {
        if ( forFriend )
        {
            LiveStats_GetIntOtherPlayerStatByKey(controllerIndex, &greenBarValue, MP_PLAYERSTATSKEY_LIFETIME_EARNINGS);
            LiveStats_GetIntOtherPlayerStatByKey(controllerIndex, &redBarValue, MP_PLAYERSTATSKEY_LIFETIME_BUYIN);
        }
        else
        {
            LiveStats_GetIntPlayerStatByKey(controllerIndex, &greenBarValue, MP_PLAYERSTATSKEY_LIFETIME_EARNINGS);
            LiveStats_GetIntPlayerStatByKey(controllerIndex, &redBarValue, MP_PLAYERSTATSKEY_LIFETIME_BUYIN);
        }
    }
    else if ( graphType == 17 )
    {
        if ( forFriend )
        {
            LiveStats_GetIntOtherPlayerStatByKey(controllerIndex, &greenBarValue, MP_PLAYERSTATSKEY_CONTRACTS_COMPLETED);
            LiveStats_GetIntOtherPlayerStatByKey(controllerIndex, &redBarValue, MP_PLAYERSTATSKEY_CONTRACTS_PURCHASED);
        }
        else
        {
            LiveStats_GetIntPlayerStatByKey(controllerIndex, &greenBarValue, MP_PLAYERSTATSKEY_CONTRACTS_COMPLETED);
            LiveStats_GetIntPlayerStatByKey(controllerIndex, &redBarValue, MP_PLAYERSTATSKEY_CONTRACTS_PURCHASED);
        }
        NumContractsInProgress = LiveContracts_GetNumContractsInProgress(controllerIndex);
        redBarValue -= greenBarValue + NumContractsInProgress;
    }
    if ( greenBarValue < redBarValue )
        v10 = redBarValue;
    else
        v10 = greenBarValue;
    firstDigit = v10;
    while ( firstDigit > 10 )
    {
        firstDigit /= 10;
        power *= 10;
    }
    maxValue = (float)(power * (firstDigit + 1));
    greenBarWidth = (float)((float)((float)greenBarValue / maxValue) * rect->w) / 2.0;
    redBarWidth = (float)((float)((float)redBarValue / maxValue) * rect->w) / 2.0;
    green[0] = 0.42f;
    green[1] = 0.68f;
    green[2] = 0.46f;
    green[3] = 1.0f;
    red[0] = 0.73f;
    red[1] = 0.19f;
    red[2] = 0.19f;
    red[3] = 1.0f;
    white[0] = 1.0f;
    white[1] = 1.0f;
    white[2] = 1.0f;
    white[3] = 0.2f;
    numGridLines = 17;
    for ( i = 0; i < 17; ++i )
    {
        v7 = Material_RegisterHandle("white", 3);
        UI_DrawHandlePic(
            &scrPlaceView[contextIndex],
            (float)((float)((float)i * rect->w) / 16.0) + rect->x,
            (float)(rect->h / 4.0) + rect->y,
            1.0,
            rect->h / 2.0,
            rect->horzAlign,
            rect->vertAlign,
            white,
            v7);
    }
    v8 = Material_RegisterHandle("white", 3);
    UI_DrawHandlePic(
        &scrPlaceView[contextIndex],
        (float)((float)(rect->w / 2.0) - redBarWidth) + rect->x,
        (float)((float)(rect->h / 2.0) + rect->y) - (float)(barHeight / 2.0),
        redBarWidth,
        barHeight,
        rect->horzAlign,
        rect->vertAlign,
        red,
        v8);
    v9 = Material_RegisterHandle("white", 3);
    UI_DrawHandlePic(
        &scrPlaceView[contextIndex],
        (float)(rect->w / 2.0) + rect->x,
        (float)((float)(rect->h / 2.0) + rect->y) - (float)(barHeight / 2.0),
        greenBarWidth,
        barHeight,
        rect->horzAlign,
        rect->vertAlign,
        green,
        v9);
}

void __cdecl UI_DrawCombatRecordHitLocHeatMap(int contextIndex, rectDef_s *rect)
{
    double SortedItemData; // st7
    double v3; // st7
    Material *v4; // eax
    Material *v5; // eax
    float maxCriticalHitCount; // [esp+20h] [ebp-D4h]
    float red[4]; // [esp+24h] [ebp-D0h] BYREF
    float criticalHitLocCount[20]; // [esp+34h] [ebp-C0h]
    float hitLocCount[20]; // [esp+84h] [ebp-70h]
    bool forFriend; // [esp+D7h] [ebp-1Dh]
    int currIndex; // [esp+D8h] [ebp-1Ch]
    float maxhitCount; // [esp+DCh] [ebp-18h]
    const char *hitlocImage; // [esp+E0h] [ebp-14h]
    float yellow[4]; // [esp+E4h] [ebp-10h] BYREF

    maxhitCount = 0.0f;
    maxCriticalHitCount = 0.0f;
    forFriend = Dvar_GetBool("ui_showFriendsCombatRecord");
    for ( currIndex = 0; currIndex < 19; ++currIndex )
    {
        SortedItemData = LiveCombatRecord_GetSortedItemData(currIndex, forFriend, PARAM1);
        criticalHitLocCount[currIndex] = SortedItemData;
        v3 = LiveCombatRecord_GetSortedItemData(currIndex, forFriend, PARAM2);
        hitLocCount[currIndex] = v3;
        if ( criticalHitLocCount[currIndex] != 0.0 && criticalHitLocCount[currIndex] > maxCriticalHitCount )
            maxCriticalHitCount = criticalHitLocCount[currIndex];
        if ( hitLocCount[currIndex] != 0.0 && hitLocCount[currIndex] > maxhitCount )
            maxhitCount = hitLocCount[currIndex];
    }
    for ( currIndex = 0; currIndex < 19; ++currIndex )
    {
        criticalHitLocCount[currIndex] = criticalHitLocCount[currIndex] / maxCriticalHitCount;
        hitLocCount[currIndex] = hitLocCount[currIndex] / maxhitCount;
    }
    red[0] = 0.73f;
    red[1] = 0.19f;
    red[2] = 0.19f;
    red[3] = 1.0f;
    yellow[0] = 0.96f;
    yellow[1] = 0.58f;
    yellow[2] = 0.11f;
    yellow[3] = 1.0f;
    hitlocImage = 0;
    for ( currIndex = 1; currIndex < 18; ++currIndex )
    {
        hitlocImage = LiveCombatRecord_GetHitLocationImageRef((hitLocation_t)currIndex);
        red[3] = criticalHitLocCount[currIndex];
        yellow[3] = hitLocCount[currIndex];
        v4 = Material_RegisterHandle((char *)hitlocImage, 3);
        UI_DrawHandlePic(
            &scrPlaceView[contextIndex],
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            yellow,
            v4);
        v5 = Material_RegisterHandle((char *)hitlocImage, 3);
        UI_DrawHandlePic(
            &scrPlaceView[contextIndex],
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            red,
            v5);
    }
}

void __cdecl UI_DrawHeatMap(int contextIndex, const rectDef_s *rect, const float *color)
{
#ifdef KISAK_HEATMAP
    unsigned __int8 MapIndex; // al
    char *OverlayMapNameFromIndex; // eax
    Material *v5; // eax
    Material *material; // [esp+20h] [ebp-4h]

    if ( MatchRecord_IsHeatMapDataGenerated() )
    {
        MapIndex = MatchRecord_GetMapIndex();
        OverlayMapNameFromIndex = (char *)UI_GetOverlayMapNameFromIndex(MapIndex);
        v5 = Material_RegisterHandle(OverlayMapNameFromIndex, 3);
        UI_DrawHandlePic(
            &scrPlaceView[contextIndex],
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            color,
            v5);
        material = Material_RegisterHandle("heatmap_default", 3);
        UI_DrawHandlePic(
            &scrPlaceView[contextIndex],
            rect->x,
            rect->y,
            rect->w,
            rect->h,
            rect->horzAlign,
            rect->vertAlign,
            color,
            material);
    }
#endif
}

const char *__cdecl UI_GetOverlayMapNameFromIndex(int mapIndex)
{
    const char *v1; // eax
    const StringTable *mapTable; // [esp+0h] [ebp-4h] BYREF

    StringTable_GetAsset("mp/mapsTable.csv", (XAssetHeader *)&mapTable);
    if ( !mapTable
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_main_mp.cpp", 1059, 0, "%s", "mapTable") )
    {
        __debugbreak();
    }
    v1 = va("%d", mapIndex);
    return StringTable_Lookup(mapTable, 5, v1, 7);
}

void __cdecl UI_DrawAttributeBar(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                rectDef_s *rect,
                const char *dvarName)
{
    const char *String; // eax
    const char *v7; // eax
    const char *ItemRef; // eax
    const char *v9; // eax
    const char *v10; // eax
    const char *v11; // eax
    const char *v12; // eax
    const char *ItemAttachmentRef; // eax
    const char *v14; // eax
    const char *v15; // eax
    const char *v16; // eax
    const char *v17; // [esp+20h] [ebp-98h]
    const char *AttachmentPointName; // [esp+20h] [ebp-98h]
    char *v19; // [esp+20h] [ebp-98h]
    int v20; // [esp+24h] [ebp-94h]
    int i; // [esp+28h] [ebp-90h]
    eAttachmentPoint currAttachPoint; // [esp+2Ch] [ebp-8Ch]
    int attributeColumn; // [esp+30h] [ebp-88h]
    bool selectingWeapon; // [esp+37h] [ebp-81h]
    float remainingPartOfLastBlock; // [esp+38h] [ebp-80h]
    unsigned int currentWeaponIndex; // [esp+3Ch] [ebp-7Ch]
    bool selectingAttachments; // [esp+43h] [ebp-75h]
    unsigned int equippedAttachmentNum; // [esp+44h] [ebp-74h]
    int controllerIndex; // [esp+48h] [ebp-70h]
    float attachmentEffectXpos; // [esp+4Ch] [ebp-6Ch]
    float attachmentEffectXposa; // [esp+4Ch] [ebp-6Ch]
    ownerDrawDef_s *ownerDraw; // [esp+54h] [ebp-64h]
    float barColorFilled[4]; // [esp+58h] [ebp-60h] BYREF
    bool arrowDrawn; // [esp+6Bh] [ebp-4Dh]
    const char *currentWeaponRef; // [esp+6Ch] [ebp-4Ch]
    float attachmentEffectColor[4]; // [esp+70h] [ebp-48h] BYREF
    int currBlock; // [esp+80h] [ebp-38h]
    float attachmentEffectOnAttribute; // [esp+84h] [ebp-34h]
    const StringTable *attributesTable; // [esp+88h] [ebp-30h] BYREF
    float partitionWidth; // [esp+8Ch] [ebp-2Ch]
    float attributeValue; // [esp+90h] [ebp-28h]
    int numPartitions; // [esp+94h] [ebp-24h]
    float arrowColor[4]; // [esp+98h] [ebp-20h] BYREF
    float barColorEmpty[4]; // [esp+A8h] [ebp-10h] BYREF

    attributeValue = 0.0f;
    attachmentEffectOnAttribute = 0.0f;
    attachmentEffectXpos = 0.0f;
    remainingPartOfLastBlock = 0.0f;
    barColorFilled[0] = 0.96f;
    barColorFilled[1] = 0.58f;
    barColorFilled[2] = 0.11f;
    barColorFilled[3] = 1.0f;
    attachmentEffectColor[0] = 0.96f;
    attachmentEffectColor[1] = 0.58f;
    attachmentEffectColor[2] = 0.11f;
    attachmentEffectColor[3] = 1.0f;
    arrowColor[0] = 0.96f;
    arrowColor[1] = 0.58f;
    arrowColor[2] = 0.11f;
    arrowColor[3] = 1.0f;
    barColorEmpty[0] = 0.96f;
    barColorEmpty[1] = 0.58f;
    barColorEmpty[2] = 0.11f;
    barColorEmpty[3] = 0.2f;
    StringTable_GetAsset("mp/attributesTable.csv", (XAssetHeader*)&attributesTable);
    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    ownerDraw = Item_GetOwnerDrawDef(item);
    if (ownerDraw)
    {
        arrowDrawn = 0;
        attributeColumn = GetExpressionInt(localClientNum, item, &ownerDraw->dataExp).intVal;
        String = Dvar_GetString("ui_inside_popup");
        selectingAttachments = I_stricmp(String, "attachments") == 0;
        v7 = Dvar_GetString("ui_inside_popup");
        selectingWeapon = I_stricmp(v7, "select_weapon") == 0;
        if (selectingWeapon)
        {
            currentWeaponIndex = sharedUiInfo.itemIndex;
            ItemRef = BG_UnlockablesGetItemRef(sharedUiInfo.itemIndex);
        }
        else
        {
            v17 = Dvar_GetString("selected_loadout_slot");
            v9 = Dvar_GetString("ui_custom_name");
            currentWeaponIndex = BG_UnlockablesGetEquippedItemInSlot(controllerIndex, v9, v17);
            ItemRef = BG_UnlockablesGetItemRef(currentWeaponIndex);
        }
        currentWeaponRef = ItemRef;
        v10 = StringTable_Lookup(attributesTable, 1, ItemRef, attributeColumn);
        attributeValue = atoi(v10) / 10.0;
        if (!selectingWeapon && !selectingAttachments)
        {
            for (currAttachPoint = ATTACHMENT_POINT_TOP; currAttachPoint < ATTACHMENT_POINT_COUNT; ++currAttachPoint)
            {
                AttachmentPointName = BG_GetAttachmentPointName(currAttachPoint);
                v11 = Dvar_GetString("selected_loadout_slot");
                v19 = va("%sattachment%s", v11, AttachmentPointName);
                v12 = Dvar_GetString("ui_custom_name");
                equippedAttachmentNum = BG_UnlockablesGetEquippedItemInSlot(controllerIndex, v12, v19);
                ItemAttachmentRef = BG_UnlockablesGetItemAttachmentRef(currentWeaponIndex, equippedAttachmentNum);
                v14 = StringTable_Lookup(attributesTable, 1, ItemAttachmentRef, 9, currentWeaponRef, attributeColumn);
                attachmentEffectOnAttribute = (atoi(v14) / 10.0) + attachmentEffectOnAttribute;
            }
        }
        if (!selectingWeapon && selectingAttachments)
        {
            v15 = BG_UnlockablesGetItemAttachmentRef(currentWeaponIndex, sharedUiInfo.attachmentNum);
            v16 = StringTable_Lookup(attributesTable, 1, v15, 9, currentWeaponRef, attributeColumn);
            attachmentEffectOnAttribute = atoi(v16) / 10.0;
        }
        if (attachmentEffectOnAttribute >= 0.0)
        {
            if (attachmentEffectOnAttribute > 0.0)
            {
                attachmentEffectColor[0] = newUIGreen[0];
                attachmentEffectColor[1] = newUIGreen[1];
                attachmentEffectColor[2] = newUIGreen[2];
                attachmentEffectColor[3] = newUIGreen[3];
                arrowColor[0] = newUIGreen[0];
                arrowColor[1] = newUIGreen[1];
                arrowColor[2] = newUIGreen[2];
                arrowColor[3] = newUIGreen[3];
            }
        }
        else
        {
            attributeValue = attributeValue + attachmentEffectOnAttribute;
            attachmentEffectColor[0] = newUIRed[0];
            attachmentEffectColor[1] = newUIRed[1];
            attachmentEffectColor[2] = newUIRed[2];
            attachmentEffectColor[3] = newUIRed[3];
        }
        numPartitions = Dvar_GetInt(dvarName);
        if (numPartitions <= 0)
            v20 = 1;
        else
            v20 = numPartitions;
        numPartitions = v20;
        partitionWidth = rect->w / v20;
        for (i = 0; i < numPartitions; ++i)
        {
            UI_DrawHandlePic(
                &scrPlaceView[contextIndex],
                ((i * partitionWidth) + i) + rect->x,
                rect->y,
                partitionWidth,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                barColorEmpty,
                sharedUiInfo.assets.whiteMaterial);
            if (attributeValue >= (i + 1))
                UI_DrawHandlePic(
                    &scrPlaceView[contextIndex],
                    ((i * partitionWidth) + i) + rect->x,
                    rect->y,
                    partitionWidth,
                    rect->h,
                    rect->horzAlign,
                    rect->vertAlign,
                    barColorFilled,
                    sharedUiInfo.assets.whiteMaterial);
        }
        currBlock = 0;
        while (attributeValue > 0.0)
        {
            if (attributeValue >= 1.0)
            {
                UI_DrawHandlePic(
                    &scrPlaceView[contextIndex],
                    ((currBlock * partitionWidth) + currBlock) + rect->x,
                    rect->y,
                    partitionWidth,
                    rect->h,
                    rect->horzAlign,
                    rect->vertAlign,
                    barColorFilled,
                    sharedUiInfo.assets.whiteMaterial);
                attributeValue = attributeValue - 1.0;
                ++currBlock;
            }
            if (attributeValue == 0.0)
            {
                attachmentEffectXpos = ((currBlock * partitionWidth) + currBlock) + rect->x;
                if (attachmentEffectOnAttribute == 0.0)
                {
                    UI_DrawHandlePicRotated(
                        &scrPlaceView[contextIndex],
                        ((((currBlock * partitionWidth) + (currBlock - 1)) + rect->x) + (attributeValue * partitionWidth)) - 3.0,
                        rect->y + rect->h,
                        7.0,
                        7.0,
                        rect->horzAlign,
                        rect->vertAlign,
                        arrowColor,
                        -90.0,
                        sharedUiInfo.assets.sliderThumb);
                    return;
                }
            }
            if (attributeValue < 1.0)
            {
                UI_DrawHandlePic(
                    &scrPlaceView[contextIndex],
                    ((currBlock * partitionWidth) + currBlock) + rect->x,
                    rect->y,
                    attributeValue * partitionWidth,
                    rect->h,
                    rect->horzAlign,
                    rect->vertAlign,
                    barColorFilled,
                    sharedUiInfo.assets.whiteMaterial);
                attachmentEffectXpos = (((currBlock * partitionWidth) + currBlock) + rect->x)
                    + (attributeValue * partitionWidth);
                if (attachmentEffectOnAttribute <= 0.0)
                {
                    UI_DrawHandlePicRotated(
                        &scrPlaceView[contextIndex],
                        (((((currBlock * partitionWidth) + currBlock) + rect->x) + (attributeValue * partitionWidth)) - 3.0) - 1.0,
                        rect->y + rect->h,
                        7.0,
                        7.0,
                        rect->horzAlign,
                        rect->vertAlign,
                        arrowColor,
                        -90.0,
                        sharedUiInfo.assets.sliderThumb);
                    arrowDrawn = 1;
                }
                remainingPartOfLastBlock = 1.0 - attributeValue;
                attributeValue = 0.0f;
            }
        }
        if (attachmentEffectOnAttribute != 0.0 && attachmentEffectXpos != 0.0 && remainingPartOfLastBlock != 0.0)
        {
            //LODWORD(attachmentEffectOnAttribute) &= _mask__AbsFloat_;
            attachmentEffectOnAttribute = fabs(attachmentEffectOnAttribute);
            if (remainingPartOfLastBlock < attachmentEffectOnAttribute)
            {
                UI_DrawHandlePic(
                    &scrPlaceView[contextIndex],
                    attachmentEffectXpos,
                    rect->y,
                    remainingPartOfLastBlock * partitionWidth,
                    rect->h,
                    rect->horzAlign,
                    rect->vertAlign,
                    attachmentEffectColor,
                    sharedUiInfo.assets.whiteMaterial);
                attachmentEffectOnAttribute = attachmentEffectOnAttribute - remainingPartOfLastBlock;
                attachmentEffectXposa = ((remainingPartOfLastBlock * partitionWidth) + 1.0) + attachmentEffectXpos;
                currBlock = 0;
                while (attachmentEffectOnAttribute > 0.0)
                {
                    if (attachmentEffectOnAttribute >= 1.0)
                    {
                        UI_DrawHandlePic(
                            &scrPlaceView[contextIndex],
                            ((currBlock * partitionWidth) + currBlock) + attachmentEffectXposa,
                            rect->y,
                            partitionWidth,
                            rect->h,
                            rect->horzAlign,
                            rect->vertAlign,
                            attachmentEffectColor,
                            sharedUiInfo.assets.whiteMaterial);
                        attachmentEffectOnAttribute = attachmentEffectOnAttribute - 1.0;
                        ++currBlock;
                    }
                    if (attachmentEffectOnAttribute == 0.0 && !arrowDrawn)
                        UI_DrawHandlePicRotated(
                            &scrPlaceView[contextIndex],
                            (((((currBlock * partitionWidth) + currBlock) + attachmentEffectXposa)
                                + (attachmentEffectOnAttribute * partitionWidth))
                                - 3.0)
                            - 1.0,
                            rect->y + rect->h,
                            7.0,
                            7.0,
                            rect->horzAlign,
                            rect->vertAlign,
                            arrowColor,
                            -90.0,
                            sharedUiInfo.assets.sliderThumb);
                    if (attachmentEffectOnAttribute < 1.0)
                    {
                        UI_DrawHandlePic(
                            &scrPlaceView[contextIndex],
                            ((currBlock * partitionWidth) + currBlock) + attachmentEffectXposa,
                            rect->y,
                            attachmentEffectOnAttribute * partitionWidth,
                            rect->h,
                            rect->horzAlign,
                            rect->vertAlign,
                            attachmentEffectColor,
                            sharedUiInfo.assets.whiteMaterial);
                        if (!arrowDrawn)
                            UI_DrawHandlePicRotated(
                                &scrPlaceView[contextIndex],
                                (((((currBlock * partitionWidth) + currBlock) + attachmentEffectXposa)
                                    + (attachmentEffectOnAttribute * partitionWidth))
                                    - 3.0)
                                - 1.0,
                                rect->y + rect->h,
                                7.0,
                                7.0,
                                rect->horzAlign,
                                rect->vertAlign,
                                arrowColor,
                                -90.0,
                                sharedUiInfo.assets.sliderThumb);
                        attachmentEffectOnAttribute = 0.0f;
                    }
                }
            }
            else
            {
                UI_DrawHandlePic(
                    &scrPlaceView[contextIndex],
                    attachmentEffectXpos,
                    rect->y,
                    attachmentEffectOnAttribute * partitionWidth,
                    rect->h,
                    rect->horzAlign,
                    rect->vertAlign,
                    attachmentEffectColor,
                    sharedUiInfo.assets.whiteMaterial);
                if (!arrowDrawn)
                    UI_DrawHandlePicRotated(
                        &scrPlaceView[contextIndex],
                        (((attachmentEffectOnAttribute * partitionWidth) + attachmentEffectXpos) - 3.0) - 1.0,
                        rect->y + rect->h,
                        7.0,
                        7.0,
                        rect->horzAlign,
                        rect->vertAlign,
                        arrowColor,
                        -90.0,
                        sharedUiInfo.assets.sliderThumb);
            }
        }
    }
}

void __cdecl UI_DrawReticlePreview(int localClientNum, int contextIndex, itemDef_s *item, const rectDef_s *rect)
{
    Material *v4; // eax
    ownerDrawDef_s *ownerDraw; // [esp+20h] [ebp-1Ch]
    const char *reticleMaterial; // [esp+24h] [ebp-18h] BYREF
    int reticleColorIndex; // [esp+28h] [ebp-14h] BYREF
    float reticleColor[4]; // [esp+2Ch] [ebp-10h] BYREF

    ownerDraw = Item_GetOwnerDrawDef(item);
    if ( ownerDraw
        && GetExpressionStringInt(
                 localClientNum,
                 item,
                 &ownerDraw->dataExp,
                 (char **)&reticleMaterial,
                 (operandInternalDataUnion *)&reticleColorIndex) )
    {
        if ( CG_GetWeaponOptionReticleColor(reticleColorIndex, reticleColor) )
        {
            reticleColor[3] = 1.0f;
            v4 = Material_RegisterHandle((char *)reticleMaterial, 3);
            UI_DrawHandlePic(
                &scrPlaceView[contextIndex],
                rect->x,
                rect->y,
                rect->w,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                reticleColor,
                v4);
        }
    }
}

void __cdecl UI_DrawLensPreview(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                const rectDef_s *rect,
                const float *color)
{
    Material *v5; // eax
    float lensColor[4]; // [esp+20h] [ebp-1Ch] BYREF
    ownerDrawDef_s *ownerDraw; // [esp+30h] [ebp-Ch]
    int lensColorIndex; // [esp+34h] [ebp-8h] BYREF
    const char *lensMaterial; // [esp+38h] [ebp-4h] BYREF

    ownerDraw = Item_GetOwnerDrawDef(item);
    if ( ownerDraw
        && GetExpressionStringInt(
                 localClientNum,
                 item,
                 &ownerDraw->dataExp,
                 (char **)&lensMaterial,
                 (operandInternalDataUnion *)&lensColorIndex) )
    {
        if ( CG_GetWeaponOptionLensColor(lensColorIndex, lensColor) )
        {
            lensColor[3] = color[3];
            v5 = Material_RegisterHandle((char *)lensMaterial, 3);
            UI_DrawHandlePic(
                &scrPlaceView[contextIndex],
                rect->x,
                rect->y,
                rect->w,
                rect->h,
                rect->horzAlign,
                rect->vertAlign,
                lensColor,
                v5);
        }
    }
}

void __cdecl UI_Project_OwnerDrawText(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                float text_x,
                float text_y,
                int ownerDraw,
                int ownerDrawFlags,
                int align,
                Font_s *font,
                float scale,
                float *color,
                Material *material,
                int textStyle,
                rectDef_s parentRect,
                char textAlignMode)
{
    rectDef_s rect; // [esp+98h] [ebp-18h] BYREF

    rect.x = x + text_x;
    rect.y = y + text_y;
    rect.w = w;
    rect.h = h;
    rect.horzAlign = horzAlign;
    rect.vertAlign = vertAlign;
    switch ( ownerDraw )
    {
        case 274:
            UI_DrawPlaylistName(
                localClientNum,
                contextIndex,
                &rect,
                font,
                color,
                scale,
                textStyle,
                text_x,
                text_y,
                textAlignMode);
            break;
        case 279:
            UI_DrawCategoryName(
                localClientNum,
                contextIndex,
                &rect,
                font,
                color,
                scale,
                textStyle,
                text_x,
                text_y,
                textAlignMode);
            break;
        case 283:
            UI_DrawGameTypeOnMapName(contextIndex, &rect, font, color, scale, textStyle);
            break;
        case 314:
            UI_DrawWagerTier(contextIndex, &rect, font, color, scale, textStyle, text_x, text_y, textAlignMode);
            break;
        case 345:
            UI_DrawCombatRecordLineGraphValues(contextIndex, &rect, font, color, scale, textStyle);
            break;
        case 346:
            UI_DrawCombatRecordLineGraphGameTypes(contextIndex, &rect, font, color, scale, textStyle);
            break;
        default:
            return;
    }
}

void __cdecl UI_DrawPlaylistName(
                int localClientNum,
                int contextIndex,
                rectDef_s *rect,
                Font_s *font,
                float *color,
                float textScale,
                int style,
                float text_x,
                float text_y,
                char textAlignMode)
{
    unsigned int PlaylistIdForNum; // eax
    int controllerIndex; // [esp+1Ch] [ebp-24h]
    const char *name; // [esp+20h] [ebp-20h]
    rectDef_s textRect; // [esp+28h] [ebp-18h] BYREF

    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( controllerIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_main_mp.cpp",
                    973,
                    0,
                    "%s",
                    "controllerIndex != INVALID_CONTROLLER_PORT") )
    {
        __debugbreak();
    }
    PlaylistIdForNum = Playlist_GetPlaylistIdForNum(controllerIndex, playlist->current.integer);
    name = Playlist_GetPlaylistName(controllerIndex, PlaylistIdForNum);
    if ( name )
    {
        if ( *name )
        {
            UI_TextHeight(font, textScale);
            UI_DrawWrappedText(
                &scrPlaceView[contextIndex],
                name,
                rect,
                font,
                rect->x + text_x,
                rect->y + text_y,
                textScale,
                color,
                style,
                textAlignMode,
                &textRect);
        }
    }
}

void __cdecl UI_DrawCategoryName(
                int localClientNum,
                int contextIndex,
                rectDef_s *rect,
                Font_s *font,
                float *color,
                float textScale,
                int style,
                float text_x,
                float text_y,
                char textAlignMode)
{
    unsigned int CategoryIdForNum; // eax
    int controllerIndex; // [esp+1Ch] [ebp-28h]
    const char *name; // [esp+24h] [ebp-20h]
    rectDef_s textRect; // [esp+2Ch] [ebp-18h] BYREF

    controllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
    if ( controllerIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_main_mp.cpp",
                    990,
                    0,
                    "%s",
                    "controllerIndex != INVALID_CONTROLLER_PORT") )
    {
        __debugbreak();
    }
    if ( xblive_wagermatch->current.enabled )
        CategoryIdForNum = Playlist_GetCategoryIdForNum(wagerCategory->current.integer);
    else
        CategoryIdForNum = Playlist_GetCategoryIdForNum(category->current.integer);
    name = Playlist_GetCategoryLocalizedName(controllerIndex, CategoryIdForNum);
    if ( name )
    {
        if ( *name )
        {
            UI_TextHeight(font, textScale);
            UI_DrawWrappedText(
                &scrPlaceView[contextIndex],
                name,
                rect,
                font,
                rect->x + text_x,
                rect->y + text_y,
                textScale,
                color,
                style,
                textAlignMode,
                &textRect);
        }
    }
}

void __cdecl UI_DrawGameTypeOnMapName(
                int localClientNum,
                rectDef_s *rect,
                Font_s *font,
                float *color,
                float textScale,
                int style)
{
    const char *v6; // eax
    char *v7; // eax
    float x; // [esp+0h] [ebp-424h]
    float y; // [esp+4h] [ebp-420h]
    int horzAlign; // [esp+8h] [ebp-41Ch]
    int vertAlign; // [esp+Ch] [ebp-418h]
    const char *String; // [esp+18h] [ebp-40Ch]
    char outputString[1028]; // [esp+1Ch] [ebp-408h] BYREF

    String = Dvar_GetString("ui_gametype");
    v6 = Dvar_GetString("ui_mapname");
    UI_GetGameTypeOnMapName(outputString, v6, String);
    vertAlign = rect->vertAlign;
    horzAlign = rect->horzAlign;
    y = rect->y;
    x = rect->x;
    v7 = va("%s", outputString);
    UI_DrawText(&scrPlaceView[localClientNum], v7, 0x7FFFFFFF, font, x, y, horzAlign, vertAlign, textScale, color, style);
}

void __cdecl UI_DrawWagerTier(
                int localClientNum,
                rectDef_s *rect,
                Font_s *font,
                float *color,
                float textScale,
                int style,
                float text_x,
                float text_y,
                char textAlignMode)
{
    int Int; // eax
    const char *v10; // eax
    float x; // [esp+0h] [ebp-474h]
    float y; // [esp+4h] [ebp-470h]
    char outputString[1024]; // [esp+1Ch] [ebp-458h] BYREF
    char currentTierString[8]; // [esp+41Ch] [ebp-58h] BYREF
    char numTiersString[8]; // [esp+424h] [ebp-50h] BYREF
    ConversionArguments convArgs; // [esp+430h] [ebp-44h] BYREF
    const char *wagerTierString; // [esp+458h] [ebp-1Ch]
    rectDef_s textRect; // [esp+45Ch] [ebp-18h] BYREF

    wagerTierString = UI_SafeTranslateString("MPUI_WAGERMATCH_TIER");
    Int = Dvar_GetInt("scr_wagerTier");
    Com_sprintf(currentTierString, 5u, "%i", Int);
    Com_sprintf(numTiersString, 5u, "%i", 3);
    memset(&convArgs.args[2], 0, 28);
    convArgs.argCount = 2;
    convArgs.args[0] = currentTierString;
    convArgs.args[1] = numTiersString;
    UI_ReplaceConversions((char *)wagerTierString, &convArgs, outputString, 1024);
    y = rect->y + text_y;
    x = rect->x + text_x;
    v10 = va("%s", outputString);
    UI_DrawWrappedText(
        &scrPlaceView[localClientNum],
        v10,
        rect,
        font,
        x,
        y,
        textScale,
        color,
        style,
        textAlignMode,
        &textRect);
}

void __cdecl UI_DrawCombatRecordLineGraphGameTypes(
                int contextIndex,
                rectDef_s *rect,
                Font_s *font,
                float *color,
                float textScale,
                int style)
{
    double SortedItemData; // st7
    const char *v7; // eax
    char *v8; // eax
    float x; // [esp+0h] [ebp-5Ch]
    float y; // [esp+4h] [ebp-58h]
    int horzAlign; // [esp+8h] [ebp-54h]
    int vertAlign; // [esp+Ch] [ebp-50h]
    int maxValues; // [esp+1Ch] [ebp-40h]
    int recentGametypes[10]; // [esp+20h] [ebp-3Ch]
    int numRecentValues; // [esp+48h] [ebp-14h]
    float separation; // [esp+4Ch] [ebp-10h]
    int startIndex; // [esp+50h] [ebp-Ch]
    bool forFriend; // [esp+57h] [ebp-5h]
    int currIndex; // [esp+58h] [ebp-4h]

    maxValues = 0;
    currIndex = 0;
    numRecentValues = 0;
    startIndex = 0;
    forFriend = Dvar_GetBool("ui_showFriendsCombatRecord");
    if ( Dvar_GetInt("ui_combatCurrViewNum") == 1 )
    {
        maxValues = 10;
        startIndex = 0;
    }
    else if ( Dvar_GetInt("ui_combatCurrViewNum") == 3 )
    {
        maxValues = 5;
        startIndex = 1;
    }
    for ( currIndex = 0;
                currIndex < maxValues && (int)LiveCombatRecord_GetSortedItemData(startIndex, forFriend, ITEM_INDEX);
                ++currIndex )
    {
        SortedItemData = LiveCombatRecord_GetSortedItemData(startIndex, forFriend, PARAM2);
        recentGametypes[currIndex] = (int)SortedItemData;
        ++numRecentValues;
        ++startIndex;
    }
    separation = rect->w / (float)(maxValues - 1);
    currIndex = numRecentValues - 1;
    startIndex = 0;
    while ( currIndex >= 0 )
    {
        vertAlign = rect->vertAlign;
        horzAlign = rect->horzAlign;
        y = rect->y;
        x = (float)((float)((float)startIndex * separation) + rect->x) - 15.0;
        v7 = va("MPUI_%s_SHORT_CAPS", lbTypeEnum_3[recentGametypes[currIndex]]);
        v8 = UI_SafeTranslateString(v7);
        UI_DrawText(&scrPlaceView[contextIndex], v8, 0x7FFFFFFF, font, x, y, horzAlign, vertAlign, textScale, color, style);
        --currIndex;
        ++startIndex;
    }
}

void __cdecl UI_DrawCombatRecordLineGraphValues(
                int contextIndex,
                rectDef_s *rect,
                Font_s *font,
                float *color,
                float textScale,
                int style)
{
    const char *v6; // eax
    char *v7; // eax
    float x; // [esp+10h] [ebp-5Ch]
    float y; // [esp+14h] [ebp-58h]
    int horzAlign; // [esp+18h] [ebp-54h]
    int vertAlign; // [esp+1Ch] [ebp-50h]
    int maxValues; // [esp+2Ch] [ebp-40h]
    int numRecentValues; // [esp+30h] [ebp-3Ch]
    float separation; // [esp+34h] [ebp-38h]
    int startIndex; // [esp+38h] [ebp-34h]
    int startIndexa; // [esp+38h] [ebp-34h]
    bool forFriend; // [esp+3Fh] [ebp-2Dh]
    int currIndex; // [esp+40h] [ebp-2Ch]
    int currIndexa; // [esp+40h] [ebp-2Ch]
    float recentKDRatioValues[10]; // [esp+44h] [ebp-28h]

    maxValues = 0;
    numRecentValues = 0;
    startIndex = 0;
    forFriend = Dvar_GetBool("ui_showFriendsCombatRecord");
    if ( Dvar_GetInt("ui_combatCurrViewNum") == 1 )
    {
        maxValues = 10;
        startIndex = 0;
    }
    else if ( Dvar_GetInt("ui_combatCurrViewNum") == 3 )
    {
        maxValues = 5;
        startIndex = 1;
    }
    for ( currIndex = 0;
                currIndex < maxValues && (int)LiveCombatRecord_GetSortedItemData(startIndex, forFriend, ITEM_INDEX);
                ++currIndex )
    {
        recentKDRatioValues[currIndex] = LiveCombatRecord_GetSortedItemData(startIndex, forFriend, SORT_KEY);
        ++numRecentValues;
        ++startIndex;
    }
    if ( numRecentValues >= 2 )
    {
        separation = rect->w / (float)(maxValues - 1);
        currIndexa = numRecentValues - 1;
        startIndexa = 0;
        while ( currIndexa >= 0 )
        {
            vertAlign = rect->vertAlign;
            horzAlign = rect->horzAlign;
            y = rect->y;
            x = (float)((float)((float)startIndexa * separation) + rect->x) - 10.0;
            v6 = va("%.2f", recentKDRatioValues[currIndexa]);
            v7 = UI_SafeTranslateString(v6);
            UI_DrawText(
                &scrPlaceView[contextIndex],
                v7,
                0x7FFFFFFF,
                font,
                x,
                y,
                horzAlign,
                vertAlign,
                textScale,
                color,
                style);
            --currIndexa;
            ++startIndexa;
        }
    }
}

void __cdecl UI_Project_RunMenuScript(
                int localClientNum,
                int contextIndex,
                const char *name,
                const char **args,
                const char *actualScript)
{
    uiInfo_s *dc; // [esp+Ch] [ebp-34h]

#ifdef KISAK_SP
    // Retail UI_Project_RunMenuScript 0x00420652-0x00420684.  Shipped frontend.gsc
    // normally reaches the same command through StartMultiplayerGame(), but retail also
    // exposes this direct menu-script producer.
    if ( !I_stricmp(name, "startMultiplayer") )
    {
        Com_Printf(13, "SP UI startMultiplayer: queueing command\n");
        Cbuf_AddText(localClientNum, "startMultiplayer\n");
        return;
    }

    // SP-only, Ghidra 0x00420432 (UI_Project_RunMenuScript, retail function 0x00420240 --
    // confirmed by decompiling the retail function directly). The "playerstart" menu-script
    // command: if a cinematic has been specifically targeted (R_Cinematic_IsTargetSet) or the
    // queued "next" cinematic is ready, promote/advance it; otherwise run the same dismiss path
    // as the per-frame lifecycle helper in UI_Project_Refresh above.
    if ( !I_stricmp(name, "playerstart") )
    {
        if ( R_Cinematic_IsTargetSet() || R_Cinematic_IsNextReady_Internal() )
        {
            R_Cinematic_WaitForUpdateFrame();
            R_Cinematic_StartNextPlayback();
            R_Cinematic_UpdateFrame(1);
            R_Cinematic_WaitForUpdateFrame();
        }
        else
        {
            UI_Sp_DismissCinematic(localClientNum);
        }
        return;
    }

    // SP-only, retail UI_Project_RunMenuScript 0x00420960-0x004209aa (read by decompilation and
    // by read_memory on the operands). One of the three producers of the "cmd mlvl ..." client
    // command; without it the SP frontend never tells script that a menu item was chosen.
    //
    //   0x00420960  I_stricmp(name, "sendMenuNotify")
    //               CMP byte ptr [DAT_02598ef4 + 0x18],0 -> ui_menuLvlNotify->current.enabled.
    //               DAT_02598ef4 is identified by the registrar string-table walk run against
    //               this binary this pass: 0x008361c8 pushes 0x009b7560 ("ui_menuLvlNotify") and
    //               the register call's return lands in 0x02598ef4 at 0x008361eb (MSVC's usual
    //               delayed store). The same global gates both ui_shared.cpp emitters.
    //   0x00420992  PUSH 0x009ec1c0 -> va("cmd mlvl %s\n", buf)
    //
    // ONE token, not two. Retail runs a single String_Parse into a 1024-byte buffer and drops the
    // whole thing into one "%s", so a menu writing sendMenuNotify "startlevel 0" sends
    // `cmd mlvl startlevel 0`, which the server side then splits into argv[1]/argv[2]. Splitting
    // it here would produce `cmd mlvl startlevel` and an argc of 2, which Cmd_MenuLevelMessage_f
    // rejects.
    //
    // Retail does NOT fall through to the "unknown UI script" print when the dvar is off or the
    // parse fails -- it reaches the function's tail return -- so the unconditional return matches.
    if ( !I_stricmp(name, "sendMenuNotify") )
    {
        Com_Printf(15, "SP sendMenuNotify: localClient %i enabled %i args '%s'\n",
                   localClientNum,
                   ui_menuLvlNotify->current.enabled,
                   args ? *args : "<null>");

        // The gate here is ONE dvar, not two: retail's sendMenuNotify arm does not consult
        // cl_paused, unlike the two ui_shared.cpp emitters.
        if ( ui_menuLvlNotify->current.enabled )
        {
            char szNotify[1024];        // [esp+...] BYREF

            if ( String_Parse(args, szNotify, 1024) )
            {
                Com_Printf(15, "SP sendMenuNotify parsed: '%s'\n", szNotify);
                Cbuf_AddText(localClientNum, va("cmd mlvl %s\n", szNotify));
            }
            else
            {
                Com_Printf(15, "SP sendMenuNotify parse failed\n");
            }
        }
        return;
    }
#endif

    UI_UIContext_GetInfo(contextIndex);
    if ( I_stricmp(name, "StartServer") )
    {
        if ( I_stricmp(name, "StartListenServer") )
        {
            if ( I_stricmp(name, "resetServerSettings") )
            {
                if ( I_stricmp(name, "loadGameInfo") )
                {
                    if ( I_stricmp(name, "clearError") )
                    {
                        if ( I_stricmp(name, "ClearMods") )
                        {
                            if ( I_stricmp(name, "WagerWarning") )
                            {
                                if ( I_stricmp(name, "JoinServer") )
                                {
                                    if ( I_stricmp(name, "JoinReserved") )
                                    {
                                        if ( I_stricmp(name, "RefreshServer") )
                                        {
                                            if ( I_stricmp(name, "createFavorite") )
                                            {
                                                if ( I_stricmp(name, "createFavoriteInGame") )
                                                    Com_Printf(13, "unknown UI script %s in block:\n%s\n", name, actualScript);
                                                else
                                                    UI_RunMenuScript_CreateFavoriteInGame(localClientNum);
                                            }
                                            else
                                            {
                                                UI_RunMenuScript_CreateFavorites(localClientNum, contextIndex);
                                            }
                                        }
                                        else
                                        {
                                            UI_RunMenuScript_RefreshServer(localClientNum, contextIndex);
                                        }
                                    }
                                    else
                                    {
                                        dc = UI_UIContext_GetInfo(contextIndex);
                                        Menus_OpenByName(localClientNum, &dc->uiDC, "join_password_popmenu");
                                    }
                                }
                                else
                                {
                                    UI_RunMenuScript_JoinServer(localClientNum, contextIndex);
                                }
                            }
                            else
                            {
                                UI_RunMenuScript_WagerWarning(localClientNum, contextIndex);
                            }
                        }
                        else
                        {
                            UI_ClearMods();
                        }
                    }
                    else
                    {
                        Dvar_SetStringByName("com_errorMessage", (char *)"");
                        Dvar_SetBoolByName("com_isNotice", 0);
                        if ( CL_GetLocalClientConnectionState(localClientNum) > 0 )
                            Key_RemoveCatcher(localClientNum, -17);
                    }
                }
                else
                {
                    UI_GetGameTypesList();
                }
            }
            else
            {
                UI_RunMenuScript_ResetServerSettings();
            }
        }
        else
        {
            UI_RunMenuScript_StartListenServer();
        }
    }
    else
    {
        UI_RunMenuScript_StartServer(localClientNum);
    }
}

void __cdecl UI_RunMenuScript_StartListenServer()
{
    char *String; // eax
    char *v1; // eax
    const char *v2; // eax
    const char *map; // [esp+0h] [ebp-8h]
    parseInfo_t *value; // [esp+4h] [ebp-4h]

    String = (char *)Dvar_GetString("ui_gametype");
    Dvar_SetStringByName("g_gametype", String);
    map = Dvar_GetString("ui_mapname");
    v1 = (char *)Dvar_GetString("sv_mapRotation");
    Dvar_SetStringByName("sv_mapRotationCurrent", v1);
    for ( value = UI_GetMapRotationToken(); value && I_strcmp(value->token, map); value = UI_GetMapRotationToken() )
        ;
    //BLOPS_NULLSUB();
    v2 = va("wait ; wait ; map %s\n", map);
    Cbuf_AddText(0, v2);
}

void __cdecl UI_RunMenuScript_StartServer(int localClientNum)
{
    char *v2; // eax
    char *v3; // eax
    char *map; // [esp+0h] [ebp-8h]
    parseInfo_t *value; // [esp+4h] [ebp-4h]

    CG_SetThirdPerson(0);
    v2 = va("%i", ui_dedicated->current.integer);
    Dvar_SetFromStringByNameFromSource("dedicated", v2, DVAR_SOURCE_EXTERNAL, 0);
    Dvar_SetStringByName("g_gametype", sharedUiInfo.gameTypes[ui_netGameType->current.integer].gameType);
    map = sharedUiInfo.mapList[ui_currentNetMap->current.integer].mapLoadName;
    Dvar_SetString((dvar_s*)sv_mapRotationCurrent, sv_mapRotation->current.string);
    for (value = UI_GetMapRotationToken(); value && I_strcmp(value->token, map); value = UI_GetMapRotationToken())
        ;
    //BG_EvalVehicleName();
    //BG_EvalVehicleName();
    v3 = va("wait ; wait ; map %s\n", map);
    Cbuf_AddText(localClientNum, v3);
}

void __cdecl UI_RunMenuScript_ResetServerSettings()
{
    Dvar_SetIntByName("ui_netGameType", 0);
    Dvar_SetIntByName("ui_currentNetMap", 0);
    Dvar_SetStringByName("g_gametype", sharedUiInfo.gameTypes[ui_netGameType->current.integer].gameType);
    Dvar_SetStringByName("ui_gametype", sharedUiInfo.gameTypes[ui_netGameType->current.integer].gameType);
    Dvar_SetStringByName("ui_mapname", sharedUiInfo.mapList[ui_currentNetMap->current.integer].mapLoadName);
}

char info_6[1024];
void __cdecl UI_RunMenuScript_WagerWarning(int localClientNum, int contextIndex)
{
    char *v2; // eax
    int ControllerIndex; // eax
    uiInfo_s *uiInfo; // [esp+0h] [ebp-Ch]
    int codpoints; // [esp+4h] [ebp-8h] BYREF
    int wagerbet; // [esp+8h] [ebp-4h]

    uiInfo = UI_UIContext_GetInfo(contextIndex);
    codpoints = 0;
    LAN_GetServerInfo(
        ui_netSource->current.integer,
        sharedUiInfo.serverStatus.displayServers[sharedUiInfo.serverStatus.currentServer],
        info_6,
        1024);
    v2 = Info_ValueForKey(info_6, "wagerbet");
    wagerbet = atoi(v2);
    Dvar_SetIntByName("ui_wagerbet", wagerbet);
    if (wagerbet)
    {
        ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
        LiveStats_GetIntPlayerStat(ControllerIndex, &codpoints, "CODPOINTS");
        if (codpoints < wagerbet)
            Menus_OpenByName(localClientNum, &uiInfo->uiDC, "WagerDeadBeat");
        else
            Menus_OpenByName(localClientNum, &uiInfo->uiDC, "WagerConfirmation");
    }
}

char info_7[1024];
void __cdecl UI_RunMenuScript_JoinServer(int localClientNum, int contextIndex)
{
    char *v3; // eax
    int *count; // [esp+0h] [ebp-10h] BYREF
    serverInfo_t *servers; // [esp+4h] [ebp-Ch] BYREF
    uiInfo_s *uiInfo; // [esp+8h] [ebp-8h]

    if (Dvar_GetInt("ui_joiningwagerfrominvite"))
    {
        Dvar_SetIntByName("ui_joiningwagerfrominvite", 0);
        Live_JoinWagerFromInvite();
    }
    else
    {
        uiInfo = UI_UIContext_GetInfo(contextIndex);
        CG_SetThirdPerson(0);
        UI_UpdateDisplayServers(localClientNum, uiInfo);
        LAN_GetServerInfo(
            ui_netSource->current.integer,
            sharedUiInfo.serverStatus.displayServers[sharedUiInfo.serverStatus.currentServer],
            info_7,
            1024);
        v3 = Info_ValueForKey(info_7, "pswrd");
        if (!atoi(v3) || Menu_IsMenuOpenAndVisible(contextIndex, "join_password_popmenu"))
        {
            if (sharedUiInfo.serverStatus.currentServer >= 0
                && sharedUiInfo.serverStatus.currentServer < sharedUiInfo.serverStatus.numDisplayServers)
            {
                servers = 0;
                if (CL_GetServerList(ui_netSource->current.integer, &servers, &count)
                    && servers
                    && sharedUiInfo.serverStatus.displayServers[sharedUiInfo.serverStatus.currentServer] < *count)
                {
                    CL_Connect(&servers[sharedUiInfo.serverStatus.displayServers[sharedUiInfo.serverStatus.currentServer]]);
                }
                Dvar_SetStringByName(
                    "ui_mapname",
                    servers[sharedUiInfo.serverStatus.displayServers[sharedUiInfo.serverStatus.currentServer]].mapName);
            }
        }
        else
        {
            Menus_OpenByName(localClientNum, &uiInfo->uiDC, "join_password_popmenu");
        }
    }
}

void __cdecl UI_RunMenuScript_RefreshServer(int localClientNum, int contextIndex)
{
    uiInfo_s *uiInfo; // [esp+0h] [ebp-Ch]
    serverInfo_t *servers; // [esp+4h] [ebp-8h] BYREF
    int *count; // [esp+8h] [ebp-4h] BYREF

    if (!sharedUiInfo.serverStatus.refreshActive)
    {
        uiInfo = UI_UIContext_GetInfo(contextIndex);
        servers = 0;
        UI_UpdateDisplayServers(localClientNum, uiInfo);
        if (CL_GetServerList(ui_netSource->current.integer, &servers, &count)
            && servers
            && sharedUiInfo.serverStatus.currentServer >= 0
            && sharedUiInfo.serverStatus.currentServer < sharedUiInfo.serverStatus.numDisplayServers)
        {
            CL_RawPingServer(&servers[sharedUiInfo.serverStatus.displayServers[sharedUiInfo.serverStatus.currentServer]], 2u);
        }
    }
}

void __cdecl UI_RunMenuScript_CreateFavorites(int localClientNum, int contextIndex)
{
    const char *VariantString; // eax
    char addr[32]; // [esp+0h] [ebp-44h] BYREF
    char name[32]; // [esp+20h] [ebp-24h] BYREF

    if ( ui_netSource->current.integer == 3 )
    {
        addr[0] = 0;
        name[0] = 0;
        I_strncpyz(name, "default", 32);
        VariantString = Dvar_GetVariantString("ui_favoriteAddress");
        I_strncpyz(addr, VariantString, 32);
        UI_AddServerToFavoritesList(name, addr);
        UI_StartServerRefresh(localClientNum, contextIndex, 1);
    }
}

void __cdecl UI_RunMenuScript_CreateFavoriteInGame(int localClientNum)
{
    if ( CL_GetLocalClientConnection(localClientNum) )
        Live_AddFavourite_Ingame(0, cls.serveruid);
}

void __cdecl UI_Project_InitOnceForAllClients()
{
    UI_ScreenshotInit();
    _Dvar_RegisterBool("ui_multiplayer", 1, 0x40u, "True if the game is multiplayer");
}

#ifdef KISAK_SP
// SP retail divergence (Ghidra 0x005852c0 UI_SetActiveMenu, hand-verified 2026-08-05 -- full
// case-by-case evidence trail in this function's own Ghidra plate; read it before touching this
// block again). *** SP uses a STRUCTURALLY DIFFERENT, LARGER uiMenuCommand_t than the 11-value MP
// enum in ui_shared.h:251-263 -- this is NOT a renumbering. The SP-only uiMenuCommandSp_t enum
// below preserves the retail SP values without extending or reusing that MP-authoritative enum. SP value 2 is "main"
// (MP's UIMENU_MAIN is 1); SP has several campaign-only menus (briefing/victoryscreen/
// savegameloading/savegamesaving) with no MP counterpart at all; SP's scoreboard/menu_playercard
// analogues sit at different values (0xD/0xE) than MP's UIMENU_SCOREBOARD/UIMENU_GAMERCARD (7/8). ***
//
// 2026-08-06 FIX - the SP block above was landing every shared caller on the WRONG case, which is
// why no SP menu could ever appear. Every UI_SetActiveMenu call site in the tree passes an MP
// `uiMenuCommand_t` (~20 of them, in common.cpp, cl_keys.cpp, cl_main_mp.cpp, ui_main.cpp,
// cg_draw_mp.cpp, cg_scoreboard_mp.cpp, cg_consolecmds_mp.cpp), and those values were being fed
// straight into SP's differently-numbered switch. The concrete failure:
//   Com_UpdateMenu (common.cpp:3333, runs EVERY FRAME) passes UI_GetMenuScreen(), which returns
//   the constant 1 (ui_shared.cpp:11147-11156) meaning MP's UIMENU_MAIN. Under SP, 1 is
//   "fullscreen_error". Worse, the SP re-entrancy guard below refuses all further work once
//   currentMenuType == UISP_FULLSCREEN_ERROR, so the very first frame permanently locked the UI into
//   fullscreen_error and no later UI_SetActiveMenu call of any kind could succeed.
// Fixed by translating at the boundary rather than by #ifdef-ing ~20 call sites: a boundary
// translation fixes every caller at once and cannot be half-applied.
// Audit finding B2 (frontend-map-load audit).
//
// Mapping derived by MENU NAME - the SP case that opens the same menu the MP case opens (both
// switch bodies are in this file, so this is directly checkable):
//   UIMENU_NONE(0)            -> UISP_NONE(0)              both Menus_CloseAll
//   UIMENU_MAIN(1)            -> UISP_MAIN(2)              both open "main" + error_popmenu +
//                                                          Menus_CloseAllBehindMain + SND_FadeIn
//   UIMENU_INGAME(2)          -> UISP_PAUSEDMENU(3)        WEAKEST ROW - see TODO below
//   UIMENU_PREGAME(3)         -> UISP_PREGAME(4)           "pregame_loaderror_mp" vs
//                                                          "pregame"/"pregame_loaderror"
//   UIMENU_WM_QUICKMESSAGE(5) -> UISP_QUICKMESSAGE(6)      both open "quickmessage"
//   UIMENU_SCOREBOARD(7)      -> UISP_SCOREBOARD(0xD)      both open "scoreboard"
//   UIMENU_GAMERCARD(8)       -> UISP_MENU_PLAYERCARD(0xE) both open "menu_playercard"
//   UIMENU_ENDOFGAME(0xA)     -> UISP_ENDOFGAME(5)         both open "endofgame"
//   UIMENU_POSTGAME(4) and UIMENU_SCRIPT_POPUP(6) have no case in MP's own switch either, and
//   UIMENU_MUTEERROR(9) opens "unmute_error_popup_live" which SP has no case for -> all refused.
//
// TODO(SP): UIMENU_INGAME -> UISP_PAUSEDMENU is the one row NOT established by a matching menu
// name. MP's UIMENU_INGAME opens CG_ScriptMainMenu()'s menu (falling back to "main");
// UISP_PAUSEDMENU is the SP pause menu, and the SP header above calls it "SP-only, no MP
// counterpart". It is mapped
// here because ESC in-game (cl_keys.cpp:2040) and CL_ToggleMenu_f (cl_main_mp.cpp:3765) pass
// UIMENU_INGAME and the SP pause menu is what should appear; before this change those landed on
// UISP_MAIN ("main") by accident. Settled by decompiling SP's own ESC/pause key handler and
// reading which value it passes; Ghidra is unavailable in this session.
//
// TODO(SP): the READ side is NOT fixed by this and remains a known, separate SP defect.
// uiInfo->currentMenuType now holds SP values - which is what retail SP stores, and what the
// re-entrancy guard below and UISP_MAIN's `currentMenuType = UISP_FULLSCREEN_ERROR` assignment
// were transcribed against - but ~12 comparisons elsewhere still test it against MP enum values
// and so never match under SP:
// ui_main.cpp:228, ui_main.cpp:2120, cg_view_mp.cpp:4289, demo_playback.cpp:782,
// cg_scoreboard_mp.cpp:1786, cl_input_mp.cpp:1116, cg_gamepad.cpp:100, cl_keys.cpp:2211,
// cl_gamepad.cpp:729/750, cg_newDraw_mp.cpp:491, cl_main_mp.cpp:3759, plus ui_main.cpp:2081-2105
// which WRITES UIMENU_SCRIPT_POPUP into currentMenuType as a sentinel (UISP_QUICKMESSAGE in SP).
// All are in-game states; none block reaching the main menu, so they are left for a dedicated
// pass rather than half-converted here.
uiMenuCommandSp_t __cdecl UI_SpMenuFromMpMenu(uiMenuCommand_t menu)
{
    switch ( menu )
    {
    case UIMENU_NONE:            return UISP_NONE;
    case UIMENU_MAIN:            return UISP_MAIN;
    case UIMENU_INGAME:          return UISP_PAUSEDMENU;
    case UIMENU_PREGAME:         return UISP_PREGAME;
    case UIMENU_WM_QUICKMESSAGE: return UISP_QUICKMESSAGE;
    case UIMENU_SCOREBOARD:      return UISP_SCOREBOARD;
    case UIMENU_GAMERCARD:       return UISP_MENU_PLAYERCARD;
    case UIMENU_ENDOFGAME:       return UISP_ENDOFGAME;
    default:                     return UISP_INVALID;
    }
}

// MP-signature shim kept so every shared call site compiles and behaves unchanged.
int __cdecl UI_SetActiveMenu(int localClientNum, uiMenuCommand_t menu)
{
    uiMenuCommandSp_t spMenu = UI_SpMenuFromMpMenu(menu);

    // Refuse unmapped MP values WITHOUT touching currentMenuType, so asking for a state SP does
    // not have cannot leave a nonsense value behind for the read side.
    if ( spMenu == UISP_INVALID )
        return 0;

    return UI_SetActiveMenuSp(localClientNum, spMenu);
}

int __cdecl UI_SetActiveMenuSp(int localClientNum, uiMenuCommandSp_t menu)
{
    uiInfo_s *uiInfo;
    const char *errMsg;

    if ( localClientNum == -1 )
        return 0;

    uiInfo = UI_GetInfo(localClientNum);

    // Re-entrancy guard: confirmed via disassembly to be TWO conditions here, not just Menu_Count
    // like MP -- also refuses re-entry while a fullscreen error is already active.
    if ( static_cast<int>(uiInfo->currentMenuType) == UISP_FULLSCREEN_ERROR || Menu_Count(&uiInfo->uiDC) <= 0 )
        return 0;

    // TODO(SP): unconfirmed, see Ghidra 0x005852c0 plate addendum. Real body has a pre-switch
    // special case for menu == UISP_PAUSEDMENU ("pausedmenu"): if FUN_00415580(localClientNum) is true (a bare
    // global-flag read at DAT_02ff5354+0xa9b38, identity not resolved), it recursively calls
    // UI_SetActiveMenu(localClientNum, UISP_NONE) BEFORE proceeding to the switch below. Not transcribed.
    // STILL BLOCKED: 0x00415580 re-checked 2026-08-26 against the live database, still a bare
    // FUN_*. Naming it would not have unblocked this anyway -- the gap is what the flag MEANS,
    // and its 12-byte body is a single global read.

    // Stores the SP value, matching retail SP (and the `currentMenuType == UISP_FULLSCREEN_ERROR`
    // guard above, which
    // was transcribed from the SP binary and only makes sense against SP values). See the
    // read-side TODO(SP) above UI_SpMenuFromMpMenu.
    uiInfo->currentMenuType = static_cast<uiMenuCommand_t>(menu);
    switch ( menu )
    {
    case UISP_NONE:
        Key_RemoveCatcher(localClientNum, -17);
        Key_ClearStates(localClientNum);
        CL_PauseGame(false);
        Menus_CloseAll(localClientNum, &uiInfo->uiDC);
        return 1;

    case UISP_FULLSCREEN_ERROR:
        Key_SetCatcher(localClientNum, 16);
        Menus_OpenByName(localClientNum, &uiInfo->uiDC, "fullscreen_error");
        // NOTE: confirmed SP does NOT apply MP's `I_stricmp(errMsg, ";")` exclusion here -- only a
        // bare non-empty check (see 0x005852c0 plate addendum).
        errMsg = Dvar_GetString("com_errorMessage");
        if ( *errMsg )
            Menus_OpenByName(localClientNum, &uiInfo->uiDC, "error_popmenu");
        SND_FadeIn();
        return 1;

    case UISP_MAIN:
    {
        int contextIndex = Com_LocalClient_GetUIContextIndex(localClientNum);
        Key_ContextIndex_SetCatcher(contextIndex, 16);
        // TODO(SP): unconfirmed, see Ghidra 0x00610a60. An unnamed __fastcall helper taking one
        // register argument (writes into a stack buffer via 0x00445ff0) runs here, between the
        // catcher set and opening "main". Not transcribed -- identity not resolved.
        Menus_OpenByName(localClientNum, &uiInfo->uiDC, "main");
        if ( Dvar_GetInt("loc_language") == 8 && Dvar_GetInt("com_first_time_pc") )
            Menus_OpenByName(localClientNum, &uiInfo->uiDC, "russian_content_warning");
        Dvar_SetIntByName("com_first_time_pc", 0);
        // NOTE: confirmed SP does NOT apply MP's `I_stricmp(errMsg, ";")` exclusion here either,
        // and additionally forces currentMenuType to fullscreen_error before opening
        // "error_popmenu" -- both directly visible in disassembly, neither present in MP.
        errMsg = Dvar_GetString("com_errorMessage");
        if ( *errMsg )
        {
            uiInfo->currentMenuType = static_cast<uiMenuCommand_t>(UISP_FULLSCREEN_ERROR);
            Menus_OpenByName(localClientNum, &uiInfo->uiDC, "error_popmenu");
        }
        Menus_CloseAllBehindMain(localClientNum, &uiInfo->uiDC);
        SND_FadeIn();
        return 1;
    }

    case UISP_PAUSEDMENU:
        // TODO(SP): unconfirmed guard chain, see Ghidra 0x005852c0 plate addendum -- NOT
        // transcribed: real body refuses (returns 0) if the current map is a "menu_"/frontend map
        // (Com_IsMenuLevel, Ghidra 0x00684eb0 -- now named in the live database) or the "outro"
        // map (0x00640ce0), if the "ingameoptions" menu can't be found or is already in the menu
        // stack, or on two further raw-global flag checks gating an unresolved
        // FUN_005e2800/FUN_0048b550 pair (STILL BLOCKED: both re-checked 2026-08-26 against the
        // live database, still bare FUN_*). Deliberately not guessed at here -- an
        // incorrectly-transcribed refusal gate would silently break pause entirely, which is worse
        // than an honest gap. What IS confirmed and implemented below is the success tail.
        //
        // NARROWED 2026-08-26: the CL_PauseGame call is NOT unconditional in retail. At
        // 0x005855xx (case 3, immediately before Menus_OpenByName "pausedmenu") retail tests the
        // current.enabled byte of two cached dvar pointers and pauses only when BOTH are clear:
        //     if (!DAT_0247fed0->current.enabled && !DAT_0290bf04->current.enabled)
        //         CL_PauseGame(true);
        // Both are now identified by the registrar string-table walk, run against this binary
        // this pass (store lags the name push by one call):
        //   0x0247FED0 = "onlinegame"  -- sole WRITE 0x0082bde3 in Com_InitDvars; the name pushed
        //                for the preceding register call is 0x0082bdc6 -> 0x00A499C4. The next
        //                registration in program order is 0x00A06548 "xblive_rankedmatch", which
        //                is this tree's common.cpp ordering. Registered via the Dvar_RegisterBool
        //                registrar (0x0045bb20) with value 0, so current.enabled is the right
        //                field.
        //   0x0290BF04 = "systemlink"  -- sole WRITE 0x00590d6a in CL_InitOnceForAllClients; the
        //                name pushed for the preceding register call is 0x00590d4d -> 0x00A01E4C.
        //                The next registration is 0x00A07188 "systemlink_warning_shown". Same
        //                bool registrar. This independently reproduces the identification already
        //                recorded at cg_main_mp.cpp:3622.
        // "systemlink" is registered by retail but by NOTHING in this reconstruction, so it is
        // read by name with Dvar_GetBool, which returns false for an unregistered dvar
        // (dvar.cpp:951-957) -- the same result retail gets from its default-false global. That
        // is the identical accommodation cg_main_mp.cpp:3651 already makes for this dvar.
        //
        // STRICTLY SCOPED: only this gate is transcribed. The refusal gates above it
        // (FUN_005e2800/FUN_0048b550 and the 0x0243fd3c / 0x0286d01c pair) remain unresolved and
        // are deliberately still absent, per the paragraph above.
        if ( !onlinegame->current.enabled && !Dvar_GetBool("systemlink") )
            CL_PauseGame(true);
        if ( !Menus_OpenByName(localClientNum, &uiInfo->uiDC, "pausedmenu") )
            return 0;
        Key_SetCatcher(localClientNum, 16);
        return 1;

    case UISP_PREGAME:
        // TODO(SP): unconfirmed gate, see Ghidra 0x005852c0 plate addendum. Transcribed here is
        // only the confirmed simple path, which matches MP's own UIMENU_PREGAME shape.
        //
        // *** TWO CORRECTIONS to this marker, both re-read off retail case 4 this pass. ***
        //  (1) The two flag reads in the gate are NOT per-local-client. They are the SAME two
        //      global cached dvar pointers case 3 uses -- DAT_0247fed0 = "onlinegame" and
        //      DAT_0290bf04 = "systemlink" (identifications derived in the case 3 comment above)
        //      -- read as `*(char *)(dvar + 0x18)`, i.e. current.enabled, with no client index
        //      anywhere in the addressing. The full gate is
        //          if (!FUN_004efe20() && !onlinegame->current.enabled && !systemlink->current.enabled)
        //      and only its false side does the unidentified bookkeeping
        //      (Dvar_SetBool(DAT_0247fed8, 0); FUN_0040fc70()) and returns 1 with no menu change.
        //      FUN_004efe20 is still bare FUN_* in the live database, so the gate as a whole is
        //      still NOT transcribed -- but the reason is that one unresolved predicate, not the
        //      two dvars, which are now known.
        //  (2) This marker never mentioned case 4's PRELUDE, which runs before the gate and is
        //      unconditional with respect to it:
        //          if (Com_IsMenuLevel(0))
        //              UI_AddMenuList(localClientNum, &UI_GetInfo(localClientNum)->uiDC,
        //                             UI_LoadMenus("ui/menus.txt", 3), 1);
        //      Com_IsMenuLevel is the same already-named 0x00684eb0 predicate case 3 uses.
        //
        //      *** NOW PORTED (2026-08-27). The earlier refusal above -- "not transcribed
        //      either, because it is only coherent together with the gate it precedes" -- was
        //      wrong, and this is why. ***
        //      The prelude is NOT half of the gate; it is independent of it. Re-read off retail
        //      this pass with capstone (image base 0x00400000, .text 0x00401000-0x009a2a10):
        //          0058555b  push 0 / call 0x684eb0   ; Com_IsMenuLevel(NULL)
        //          00585565  test al,al / je 0x58558a
        //          00585569  push 3 / push 0xa1db18   ; "ui/menus.txt" (string verified)
        //          00585570  call 0x5e6d20            ; UI_LoadMenus(file, 3)
        //          00585575  push esi / call 0x46be80 ; UI_GetInfo(localClientNum)
        //          0058557d  push 1 / push ebx / push eax / push esi
        //          00585582  call 0x555bf0            ; UI_AddMenuList(lc, dc, list, 1)
        //          00585587  add esp,0x1c
        //          0058558a  call 0x4efe20            ; <-- the gate, i.e. the je TARGET
        //      The `je` lands on the gate call itself, so a non-menu level skips ONLY the two
        //      reload lines and still falls through into the gate. The prelude and the gate are
        //      therefore separable, and porting the prelude alone is a complete transcription of
        //      a complete retail control-flow region, not "half of case 4". The gate at
        //      0x004efe20 remains untranscribed for the unchanged reason (unresolved predicate).
        //
        //      WHY THIS MATTERS: ui/menus.txt holds 102 menus INCLUDING "main", and ships only in
        //      frontend.ff. UI_InitUIInfos requests it at boot, before that zone is mounted, so it
        //      fails ("Error: Could not load menufile \"ui/menus.txt\"") and registers a DEFAULT
        //      entry. frontend.ff links the real asset later ("Redundant asset:
        //      'menufile','ui/menus.txt'" -> DB_LinkXAssetEntry, which does
        //      --g_defaultAssetCount + DB_CloneXAssetEntry(.., DB_CLONE_FROM_DEFAULT) and so
        //      replaces the placeholder IN PLACE). Nothing ever asked again, so the UI context
        //      only ever held the 111 menus from ui/code.txt + ui/patch_menus.txt + the HUD
        //      menufiles -- "main" was never among them and no main menu could open. This prelude
        //      is retail's own mechanism for asking again, at the one moment the zone is mounted.
        //
        //      Deliberately reloads ONLY menus.txt, exactly as retail does -- NOT code.txt or
        //      patch_menus.txt. The three files define disjoint menu names, which is what keeps
        //      UI_AddMenu's `touchMenu == menu` assert (ui_shared.cpp) clear. Re-running this is
        //      inert: UI_AddMenu returns early on pointer identity and the second UI_LoadMenus
        //      returns the same DB asset, so a repeat `map frontend` is a no-op. No NULL check
        //      and no SP_UI_DIR, matching retail and the sibling call at ui_main.cpp:3634;
        //      UI_AddMenuList is itself NULL-safe.
        if ( Com_IsMenuLevel(0) )
            UI_AddMenuList(localClientNum, &uiInfo->uiDC, UI_LoadMenus("ui/menus.txt", 3), 1);

        // Retail SP UI_SetActiveMenu 0x005852c0 calls FUN_004efe20 here. Its full predicate also
        // handles auto-continue/lobby cases which are still outside this reconstruction, but one
        // state path is unconditional and fully established: when no cinematic is currently
        // started, FUN_004efe20 returns true and case 4 clears long_blocking_call, invokes the
        // 0x0040fc70 dismiss/promote helper, and returns without opening "pregame".
        //
        // This is the boot-critical path. Com_Init queues number_lady_intro in the NEXT slot;
        // SV_SpawnServer then reaches case 4 before anything has promoted it. Omitting this branch
        // left that movie queued until frontend.gsc's Start3DCinematic("frontend", 1, 1) replaced
        // the active target. Keep the transcription deliberately limited to the proven
        // !R_Cinematic_IsStarted() arm; the remaining auto-continue predicate is still TODO.
        if ( !R_Cinematic_IsStarted() )
        {
            Com_Printf(
                14,
                "SP cinematic boot gate: no active playback, nextReady=%i; running retail case-4 promote path\n",
                R_Cinematic_IsNextReady_Internal());
            Dvar_SetBool((dvar_s *)long_blocking_call, false);
            UI_Sp_DismissCinematic(localClientNum);
            return 1;
        }

        Key_ClearStates(localClientNum);
        Key_SetCatcher(localClientNum, 16);
        Menus_CloseAll(localClientNum, &uiInfo->uiDC);
        errMsg = Dvar_GetString("com_errorMessage");
        if ( !*errMsg )
            Menus_OpenByName(localClientNum, &uiInfo->uiDC, "pregame");
        else
            Menus_OpenByName(localClientNum, &uiInfo->uiDC, "pregame_loaderror");
        return 1;

    case UISP_ENDOFGAME:
        Key_SetCatcher(localClientNum, 16);
        Menus_CloseAll(localClientNum, &uiInfo->uiDC);
        Menus_OpenByName(localClientNum, &uiInfo->uiDC, "endofgame");
        return 1;

    case UISP_QUICKMESSAGE:
        uiInfo->uiDC.cursor.x = 639.0f;
        uiInfo->uiDC.cursor.y = 479.0f;
        UI_SetSystemCursorPos(&uiInfo->uiDC, 639.0, 479.0);
        Key_SetCatcher(localClientNum, 16);
        CL_LocalClient_SetCUIFlag(localClientNum, 64);
        Menus_CloseAll(localClientNum, &uiInfo->uiDC);
        Menus_OpenByName(localClientNum, &uiInfo->uiDC, "quickmessage");
        return 1;

    case UISP_BRIEFING:
        // IMPLEMENTED 2026-08-26. The two Dvar_SetInt(dvarPtr, -1) calls at retail 0x00585631 /
        // 0x0058563d are now identified -- the earlier "likely campaign mission/act progress
        // trackers" guess was wrong. Both globals are written by the sole registrar FUN_0051d1f0
        // and resolved by the registrar string-table walk (store lags the name push by one call):
        //   store 0x0051d35e -> DAT_02562a3c, name pushed 0x0051d359 -> 0x00A3B428
        //       = "ui_controllerPulled"
        //   store 0x0051d376 -> DAT_02598f5c, name pushed 0x0051d371 ... (see below)
        // Careful with the pairing: the name for DAT_02562a3c is the one pushed for the call that
        // PRECEDES its store, i.e. 0x00A3B428 pushed at 0x0051d33f, and DAT_02598f5c's is
        // 0x00A19AFC pushed at 0x0051d359 = "ui_pendingControllerPullEvent". Both are registered
        // through _Dvar_RegisterInt with default value -1 (PUSH -0x1 at 0x0051d33d / 0x0051d357).
        // Retail's order is ui_controllerPulled first, then ui_pendingControllerPullEvent.
        //
        // Neither dvar is registered anywhere in this reconstruction, and registering new globals
        // on this shared path is out of scope, so both are driven by name -- the same technique
        // cg_main_mp.cpp already uses for "systemlink". Dvar_SetIntByName creates the dvar as a
        // 0x4000-flagged string dvar when it is absent (dvar.cpp:3066), which Dvar_GetInt reads
        // back through Dvar_StringToInt, so the pair still round-trips.
        Dvar_SetIntByName("ui_controllerPulled", -1);
        Dvar_SetIntByName("ui_pendingControllerPullEvent", -1);
        Menus_CloseAll(localClientNum, &uiInfo->uiDC);
        Menus_OpenByName(localClientNum, &uiInfo->uiDC, "briefing");
        return 1;

    case UISP_VICTORYSCREEN:
        // Cursor-set helper independently re-decompiled this pass (0x005cdcb0): writes
        // dc->cursor.x/y then calls UI_SetSystemCursorPos -- same helper as case 6, floats decoded
        // from the literal words 0x43a00000/0x43e00000.
        uiInfo->uiDC.cursor.x = 320.0f;
        uiInfo->uiDC.cursor.y = 448.0f;
        UI_SetSystemCursorPos(&uiInfo->uiDC, 320.0, 448.0);
        Key_SetCatcher(localClientNum, 16);
        Menus_OpenByName(localClientNum, &uiInfo->uiDC, "victoryscreen");
        return 1;

    case UISP_SAVEGAMELOADING:
        Menus_OpenByName(localClientNum, &uiInfo->uiDC, "savegameloading");
        return 1;

    case UISP_SAVEGAMESAVING:
        Menus_OpenByName(localClientNum, &uiInfo->uiDC, "savegamesaving");
        Key_SetCatcher(localClientNum, 16);
        return 1;

    case UISP_SCOREBOARD:
        // NOTE: confirmed SP does NOT check com_errorMessage here at all, unlike MP's
        // UIMENU_SCOREBOARD case just below in the #else branch.
        Key_SetCatcher(localClientNum, 16);
        Menus_CloseAll(localClientNum, &uiInfo->uiDC);
        Menus_OpenByName(localClientNum, &uiInfo->uiDC, "scoreboard");
        return 1;

    case UISP_MENU_PLAYERCARD:
        Key_SetCatcher(localClientNum, 16);
        Menus_OpenByName(localClientNum, &uiInfo->uiDC, "menu_playercard");
        errMsg = Dvar_GetString("com_errorMessage");
        if ( strlen(errMsg) && I_stricmp(errMsg, ";") )
            Menus_OpenByName(localClientNum, &uiInfo->uiDC, "error_popmenu");
        return 1;

    case UISP_MAIN_SYSTEMLINK:
        Key_SetCatcher(localClientNum, 16);
        Dvar_SetBoolByName("ui_skipMainLockout", true);
        // TODO(SP): unconfirmed, see Ghidra 0x0040e110. Single-int-arg helper (called with literal
        // 0) runs here; identity not resolved.
        Menus_OpenByName(localClientNum, &uiInfo->uiDC, "main");
        Menus_OpenByName(localClientNum, &uiInfo->uiDC, "main_text");
        Menus_OpenByName(localClientNum, &uiInfo->uiDC, "main_online");
        Menus_SetFocusToItem(localClientNum, &uiInfo->uiDC, "main_online", "systemlinkselect");
        Menus_CloseAllBehindMain(localClientNum, &uiInfo->uiDC);
        SND_FadeIn();
        return 1;

    case UISP_XBOXLIVE_LOBBY:
    case UISP_XBOXLIVE_PRIVATE_LOBBY:
    case UISP_SYSTEMLINK_LOBBY:
        // TODO(SP): deliberately NOT transcribed, see Ghidra 0x005852c0 plate. These three cases'
        // decompile shows `Actor_InitAnim(self)` calls with an uninitialised `self`; this is a
        // confirmed DECOMPILER ARTIFACT (Ghidra misapplying the doubtful name/prototype at
        // 0x00651A30 -- six prior agents have flagged that address as a folded empty stub with no
        // real identity, see agents.md), re-confirmed independently this pass via
        // disassemble_function: the real call at that point (0x005857cc / 0x005858e6 / 0x00585a12)
        // has ZERO arguments pushed immediately before it, contradicting the decompiler's
        // single-argument rendering. Beyond that artifact, each case's body threads several more
        // still-unnamed FUN_* helpers (a controllerIndex getter at 0x004f3c70, a buffer-format call
        // at 0x0057cdd0, unnamed helpers at 0x00480a00/0x006850c0/0x0059b090) building up the
        // Cbuf_ExecuteBuffer command string and lobby menu stack. Not guessed at here -- would need
        // a dedicated pass. Collapsed to a safe refusal for all three distinct real behaviors.
        return 0;

    case UISP_MAIN_ONLINE:
        Key_SetCatcher(localClientNum, 16);
        SND_FadeIn();
        // TODO(SP): unconfirmed real controllerIndex, see Ghidra 0x004f3c70 -- the real body
        // threads an unnamed FUN_004f3c70(localClientNum) result through as Cbuf_ExecuteBuffer's
        // controllerIndex argument instead of a literal. 0 is used here as the only value it could
        // plausibly resolve to for this reimplementation's single-local-client build (same
        // reasoning as Com_LoadFrontEnd's own TODO in common.cpp), not as a verified literal.
        // STILL BLOCKED: 0x004f3c70 re-checked 2026-08-26 against the live database, still a bare
        // FUN_*, so nothing here has become resolvable.
        Cbuf_ExecuteBuffer(localClientNum, 0, (char *)"set systemlink 0; set splitscreen 0; set onlinegame 1;");
        Menus_OpenByName(localClientNum, &uiInfo->uiDC, "main");
        Menus_CloseAllBehindMain(localClientNum, &uiInfo->uiDC);
        return 1;

    // 7, 0xA, 0xF, 0x10, 0x12 and anything > 0x16 fall to default in the real binary too.
    default:
        return 0;
    }
}
#else
int __cdecl UI_SetActiveMenu(int localClientNum, uiMenuCommand_t menu)
{
    int result; // eax
    const char *v3; // eax
    char *v4; // eax
    uiInfo_s *uiInfo; // [esp+5Ch] [ebp-8h]
    const char *buf; // [esp+60h] [ebp-4h]
    const char *bufa; // [esp+60h] [ebp-4h]
    const char *bufb; // [esp+60h] [ebp-4h]
    const char *bufc; // [esp+60h] [ebp-4h]
    const char *bufd; // [esp+60h] [ebp-4h]

    uiInfo = UI_GetInfo(localClientNum);
    if ( Menu_Count(&uiInfo->uiDC) <= 0 )
        return 0;
    if ( menu == UIMENU_SCRIPT_POPUP
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_main_mp.cpp",
                    2609,
                    0,
                    "%s",
                    "menu != UIMENU_SCRIPT_POPUP") )
    {
        __debugbreak();
    }
    uiInfo->currentMenuType = menu;
    switch ( menu )
    {
        case UIMENU_NONE:
            Key_RemoveCatcher(localClientNum, -17);
            Dvar_SetIntByName("cl_paused", 0);
            Menus_CloseAll(localClientNum, &uiInfo->uiDC);
            result = 1;
            break;
        case UIMENU_MAIN:
            Key_SetCatcher(localClientNum, 16);
            Menus_OpenByName(localClientNum, &uiInfo->uiDC, "main");
            buf = Dvar_GetString("com_errorMessage");
            if ( strlen(buf) )
            {
                if ( I_stricmp(buf, ";") )
                    Menus_OpenByName(localClientNum, &uiInfo->uiDC, "error_popmenu");
            }
            Menus_CloseAllBehindMain(localClientNum, &uiInfo->uiDC);
            SND_FadeIn();
            result = 1;
            break;
        case UIMENU_INGAME:
            Key_SetCatcher(localClientNum, 16);
            Menus_CloseAll(localClientNum, &uiInfo->uiDC);
            if ( Demo_IsPlaying() )
            {
                if ( Demo_IsRenderingMovie() )
                {
                    v3 = va("demo_stop 1\n");
                    Cbuf_AddText(localClientNum, v3);
                    result = 1;
                }
                else
                {
                    if ( Demo_IsClipPreviewRunning() )
                        Demo_RestoreUIStateAfterPreview(localClientNum);
                    else
                        Menus_OpenByName(localClientNum, &uiInfo->uiDC, "demo_ingame");
                    result = 1;
                }
            }
            else
            {
                v4 = CG_ScriptMainMenu(localClientNum);
                if ( !Menus_OpenByName(localClientNum, &uiInfo->uiDC, v4) )
                {
                    Menus_OpenByName(localClientNum, &uiInfo->uiDC, "main");
                    Menus_CloseAllBehindMain(localClientNum, &uiInfo->uiDC);
                }
                result = 1;
            }
            break;
        case UIMENU_PREGAME:
            Key_ClearStates(localClientNum);
            if ( !*Dvar_GetString("com_errorMessage")
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\ui_mp\\ui_main_mp.cpp", 2895, 0, "%s", "buf[0]") )
            {
                __debugbreak();
            }
            Key_SetCatcher(localClientNum, 16);
            Menus_CloseAll(localClientNum, &uiInfo->uiDC);
            Menus_OpenByName(localClientNum, &uiInfo->uiDC, "pregame_loaderror_mp");
            result = 1;
            break;
        case UIMENU_WM_QUICKMESSAGE:
            if ( Menus_FindByName(&uiInfo->uiDC, "quickmessage") )
            {
                uiInfo->uiDC.cursor.x = 639.0f;
                uiInfo->uiDC.cursor.y = 479.0f;
                UI_SetSystemCursorPos(&uiInfo->uiDC, 639.0, 479.0);
                Key_SetCatcher(localClientNum, 16);
                CL_LocalClient_SetCUIFlag(localClientNum, 64);
                Menus_CloseAll(localClientNum, &uiInfo->uiDC);
                Menus_OpenByName(localClientNum, &uiInfo->uiDC, "quickmessage");
                result = 1;
            }
            else
            {
                result = 0;
            }
            break;
        case UIMENU_SCOREBOARD:
            Key_SetCatcher(localClientNum, 16);
            Menus_CloseAll(localClientNum, &uiInfo->uiDC);
            Menus_OpenByName(localClientNum, &uiInfo->uiDC, "scoreboard");
            bufa = Dvar_GetString("com_errorMessage");
            if ( strlen(bufa) && I_stricmp(bufa, ";") )
                Menus_OpenByName(localClientNum, &uiInfo->uiDC, "error_popmenu");
            result = 1;
            break;
        case UIMENU_GAMERCARD:
            Key_SetCatcher(localClientNum, 16);
            Menus_OpenByName(localClientNum, &uiInfo->uiDC, "menu_playercard");
            bufb = Dvar_GetString("com_errorMessage");
            if ( strlen(bufb) && I_stricmp(bufb, ";") )
                Menus_OpenByName(localClientNum, &uiInfo->uiDC, "error_popmenu");
            result = 1;
            break;
        case UIMENU_MUTEERROR:
            Key_SetCatcher(localClientNum, 16);
            Menus_OpenByName(localClientNum, &uiInfo->uiDC, "unmute_error_popup_live");
            bufc = Dvar_GetString("com_errorMessage");
            if ( strlen(bufc) && I_stricmp(bufc, ";") )
                Menus_OpenByName(localClientNum, &uiInfo->uiDC, "error_popmenu");
            result = 1;
            break;
        case UIMENU_ENDOFGAME:
            Key_SetCatcher(localClientNum, 16);
            Menus_CloseAll(localClientNum, &uiInfo->uiDC);
            Menus_OpenByName(localClientNum, &uiInfo->uiDC, "endofgame");
            bufd = Dvar_GetString("com_errorMessage");
            if ( strlen(bufd) && I_stricmp(bufd, ";") )
                Menus_OpenByName(localClientNum, &uiInfo->uiDC, "error_popmenu");
            result = 1;
            break;
        default:
            result = 0;
            break;
    }
    return result;
}
#endif

char *__cdecl UI_TranslateIntegerToOrdinal(int integer)
{
    char *result; // eax

    switch ( integer )
    {
        case 1:
            result = UI_SafeTranslateString("MP_1ST");
            break;
        case 2:
            result = UI_SafeTranslateString("MP_2ND");
            break;
        case 3:
            result = UI_SafeTranslateString("MP_3RD");
            break;
        case 4:
            result = UI_SafeTranslateString("MP_4TH");
            break;
        case 5:
            result = UI_SafeTranslateString("MP_5TH");
            break;
        case 6:
            result = UI_SafeTranslateString("MP_6TH");
            break;
        case 7:
            result = UI_SafeTranslateString("MP_7TH");
            break;
        case 8:
            result = UI_SafeTranslateString("MP_8TH");
            break;
        case 9:
            result = UI_SafeTranslateString("MP_9TH");
            break;
        case 10:
            result = UI_SafeTranslateString("MP_10TH");
            break;
        case 11:
            result = UI_SafeTranslateString("MP_11TH");
            break;
        case 12:
            result = UI_SafeTranslateString("MP_12TH");
            break;
        default:
            Com_PrintWarning(13, "WARNING: Could not translate integer to ordinal: %i\n", integer);
            result = (char *)"";
            break;
    }
    return result;
}

int __cdecl UI_Popup(int localClientNum, const char *menu)
{
    if (!CL_AllowPopup(localClientNum) || UI_IsFullscreen(localClientNum))
        return 0;
    if (!I_stricmp(menu, "UIMENU_WM_QUICKMESSAGE"))
        UI_SetActiveMenu(localClientNum, UIMENU_WM_QUICKMESSAGE);
    return 1;
}

bool __cdecl UI_ShouldDrawCrosshair()
{
    return ui_drawCrosshair->current.enabled;
}

char *__cdecl UI_GetGameTypeDisplayNameCaps(const char *pszGameType)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < sharedUiInfo.numGameTypes; ++i)
    {
        if (!I_stricmp(pszGameType, sharedUiInfo.gameTypes[i].gameType))
            return UI_SafeTranslateString(sharedUiInfo.gameTypes[i].gameTypeNameCaps);
    }
    return (char *)pszGameType;
}

char *__cdecl UI_GetMapDisplayNameCaps(const char *pszMap)
{
    int i; // [esp+0h] [ebp-4h]

    for (i = 0; i < sharedUiInfo.mapCount; ++i)
    {
        if (!I_stricmp(pszMap, sharedUiInfo.mapList[i].mapLoadName))
            return UI_SafeTranslateString(sharedUiInfo.mapList[i].mapNameCaps);
    }
    return (char *)pszMap;
}

void __cdecl UI_DrawConnectScreen(int localClientNum)
{
    char *GameTypeDisplayNameCaps; // eax
    char *TitleName; // eax
    char *Author; // eax
    int endTime; // [esp+4h] [ebp-824h] BYREF
    int startTime; // [esp+8h] [ebp-820h] BYREF
    const char *translation; // [esp+Ch] [ebp-81Ch] BYREF
    bool displayConnectionInfo; // [esp+13h] [ebp-815h]
    const char *s; // [esp+14h] [ebp-814h]
    uiClientState_s cstate; // [esp+18h] [ebp-810h] BYREF

    displayConnectionInfo = 1;
    CL_GetClientState(localClientNum, &cstate);
    Dvar_SetStringByName("ls_gametype", (char *)"");
    Dvar_SetStringByName("ls_mapname", (char *)"");
    if ( g_mapname[0] )
    {
        GameTypeDisplayNameCaps = UI_GetGameTypeDisplayNameCaps(s_gametype);
        Dvar_SetStringByName("ls_gametype", GameTypeDisplayNameCaps);
        translation = UI_GetMapDisplayNameCaps(g_mapname);
        Dvar_SetStringByName("ls_mapname", (char *)translation);
        displayConnectionInfo = 0;
        if ( Demo_IsPlaying() )
        {
            if ( Live_FileShare_Theater_GetName((char **)&translation) )
            {
                Dvar_SetStringByName("ls_demotitle", (char *)translation);
            }
            else
            {
                TitleName = Demo_GetTitleName();
                Dvar_SetStringByName("ls_demotitle", TitleName);
            }
            Demo_GetStartAndEndTime(&startTime, &endTime);
            Dvar_SetIntByName("ls_demoduration", (endTime - startTime) / 1000);
            if ( Demo_IsClipPlaying() )
            {
                Author = Demo_GetAuthor();
                Dvar_SetStringByName("ls_demoauthor", Author);
            }
            else
            {
                Dvar_SetStringByName("ls_demoauthor", (char *)"");
            }
        }
        if ( !I_stricmp(cstate.servername, "localhost") )
            displayConnectionInfo = 0;
        if ( DB_GetLoadedFraction() >= 1.0 )
            displayConnectionInfo = 1;
        s = 0;
        switch ( cstate.connState )
        {
            case CA_CONNECTING:
                if ( displayConnectionInfo )
                {
                    s = UI_SafeTranslateString("EXE_AWAITINGCONNECTION");
                    s = UI_ReplaceConversionInt((char *)s, cstate.connectPacketCount);
                }
                break;
            case CA_CHALLENGING:
                if ( displayConnectionInfo )
                {
                    s = UI_SafeTranslateString("EXE_AWAITINGCHALLENGE");
                    s = UI_ReplaceConversionInt((char *)s, cstate.connectPacketCount);
                }
                break;
            case CA_CONNECTED:
                if ( displayConnectionInfo )
                    s = UI_SafeTranslateString("EXE_AWAITINGGAMESTATE");
                break;
            case CA_SENDINGSTATS:
                if ( displayConnectionInfo )
                    s = UI_SafeTranslateString("EXE_UPLOADINGSTATS");
                break;
            case CA_LOADING:
            case CA_PRIMED:
                if ( displayConnectionInfo )
                    s = UI_SafeTranslateString("EXE_AWAITINGHOST");
                break;
            default:
                break;
        }
        if ( Demo_IsPlaying() )
        {
            if ( Demo_IsPlaybackInited() )
            {
                s = UI_SafeTranslateString("EXE_SYNCHRONIZING");
            }
            else if ( Demo_IsClipPlaying() )
            {
                s = UI_SafeTranslateString("EXE_LOADING_CLIP");
            }
            else
            {
                s = UI_SafeTranslateString("EXE_LOADING_FILM");
            }
        }
        if ( s )
            Dvar_SetStringByName("ls_status", (char *)s);
        else
            Dvar_SetStringByName("ls_status", (char *)"");
    }
    CG_DrawInformation(localClientNum);
}

