#pragma once

#include <universal/dvar.h>
#include <ui/ui_main.h>

void __cdecl UI_Project_RegisterDvars();
#ifdef KISAK_MP
void UI_PrepareCustomBotMenu();
bool UI_IsCustomBotMenu(const menuDef_t *menu);
void UI_AddCustomBotSettings(int localClientNum, UiContext *dc, menuDef_t *menu);
#endif
void __cdecl UI_Project_AssetCache();
void __cdecl UI_Project_Refresh(int localClientNum);
void __cdecl UI_Project_Shutdown(int localClientNum);
void __cdecl UI_UpdateListboxPos_f();
void __cdecl UI_KeyClearStates_f();
void __cdecl UI_SetMap(char *mapname, const char *gametype);
void __cdecl UI_DrawLineGraphSegment(int contextIndex, float *p1, float *p2, rectDef_s *graphRect);
void __cdecl UI_GetGameTypeOnMapName(char *outputString, const char *inputMapName, const char *inputGameType);
char *__cdecl UI_GetMapName(const char *inputMapName, bool returnStringRef);
char *__cdecl UI_GetGameTypeName(const char *inputGameType, bool returnStringRef);
unsigned int __cdecl UI_GetOffsetForTextureCoords(
                unsigned int xPos,
                unsigned int yPos,
                GfxImage *img,
                unsigned int bytesPerPixel);
void __cdecl UI_GenerateHeatMapTextureCallback(GfxImage *param);
void __cdecl UI_GenerateHeatMapTexture(int controllerIndex);
void __cdecl UI_GenerateHeatMapTextureInternal(
                GfxImage *img,
                unsigned __int8 *heatmap,
                int heatmapSize,
                unsigned __int64 xuid,
                const dvar_s *dvarColor);
void __cdecl UI_DrawStatsMilestonesFeederProgressBar(
                int controllerIndex,
                int contextIndex,
                const rectDef_s *rect,
                unsigned int index,
                int type);
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
                const char *dvarName);
void __cdecl UI_DrawBlurMaterial(int contextIndex, rectDef_s *rect, float *color);
void __cdecl UI_DrawCombatRecordPieChart(int contextIndex, rectDef_s *rect, float *color, const char *dvarName);
void __cdecl UI_DrawCombatRecordLineGraph(int contextIndex, rectDef_s *rect, float *color, const char *dvarName);
void __cdecl UI_DrawCombatRecordHistogram(
                int contextIndex,
                rectDef_s *rect,
                float *color,
                const char *dvarName,
                float *samples);
void __cdecl UI_DrawCombatRecordBarGraph(int contextIndex, rectDef_s *rect);
void __cdecl UI_DrawCombatRecordHorizontalBarGraph(
                int localClientNum,
                int contextIndex,
                rectDef_s *rect,
                rectDef_s *parentRect,
                float *color,
                const char *dvarName);
void __cdecl UI_DrawCombatRecordHitLocHeatMap(int contextIndex, rectDef_s *rect);
void __cdecl UI_DrawHeatMap(int contextIndex, const rectDef_s *rect, const float *color);
const char *__cdecl UI_GetOverlayMapNameFromIndex(int mapIndex);
void __cdecl UI_DrawAttributeBar(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                rectDef_s *rect,
                const char *dvarName);
void __cdecl UI_DrawReticlePreview(int localClientNum, int contextIndex, itemDef_s *item, const rectDef_s *rect);
void __cdecl UI_DrawLensPreview(
                int localClientNum,
                int contextIndex,
                itemDef_s *item,
                const rectDef_s *rect,
                const float *color);
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
                char textAlignMode);
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
                char textAlignMode);
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
                char textAlignMode);
void __cdecl UI_DrawGameTypeOnMapName(
                int localClientNum,
                rectDef_s *rect,
                Font_s *font,
                float *color,
                float textScale,
                int style);
void __cdecl UI_DrawWagerTier(
                int localClientNum,
                rectDef_s *rect,
                Font_s *font,
                float *color,
                float textScale,
                int style,
                float text_x,
                float text_y,
                char textAlignMode);
void __cdecl UI_DrawCombatRecordLineGraphGameTypes(
                int contextIndex,
                rectDef_s *rect,
                Font_s *font,
                float *color,
                float textScale,
                int style);
void __cdecl UI_DrawCombatRecordLineGraphValues(
                int contextIndex,
                rectDef_s *rect,
                Font_s *font,
                float *color,
                float textScale,
                int style);
void __cdecl UI_Project_RunMenuScript(
                int localClientNum,
                int contextIndex,
                const char *name,
                const char **args,
                const char *actualScript);
void __cdecl UI_RunMenuScript_StartListenServer();
void __cdecl UI_RunMenuScript_StartServer(int localClientNum);
void __cdecl UI_RunMenuScript_ResetServerSettings();
void __cdecl UI_RunMenuScript_WagerWarning(int localClientNum, int contextIndex);
void __cdecl UI_RunMenuScript_JoinServer(int localClientNum, int contextIndex);
void __cdecl UI_RunMenuScript_RefreshServer(int localClientNum, int contextIndex);
void __cdecl UI_RunMenuScript_CreateFavorites(int localClientNum, int contextIndex);
void __cdecl UI_RunMenuScript_CreateFavoriteInGame(int localClientNum);
void __cdecl UI_Project_InitOnceForAllClients();
#ifdef KISAK_SP
// SP's uiMenuCommand_t is a DIFFERENT, larger enum than MP's (ui_shared.h). These are the SP
// values, transcribed from the retail SP UI_SetActiveMenu switch (Ghidra 0x005852c0) - see the
// case-by-case evidence in ui_main_mp.cpp. They are NOT interchangeable with UIMENU_*.
enum uiMenuCommandSp_t : __int32
{
    UISP_NONE             = 0x0,
    UISP_FULLSCREEN_ERROR = 0x1,   // no MP counterpart
    UISP_MAIN             = 0x2,   // MP's UIMENU_MAIN is 1
    UISP_PAUSEDMENU       = 0x3,   // no MP menu of this name
    UISP_PREGAME          = 0x4,
    UISP_ENDOFGAME        = 0x5,   // MP's UIMENU_ENDOFGAME is 0xA
    UISP_QUICKMESSAGE     = 0x6,   // MP's UIMENU_WM_QUICKMESSAGE is 5
    UISP_BRIEFING         = 0x8,   // SP campaign-only
    UISP_VICTORYSCREEN    = 0x9,   // SP campaign-only
    UISP_SAVEGAMELOADING  = 0xB,   // SP campaign-only
    UISP_SAVEGAMESAVING   = 0xC,   // SP campaign-only
    UISP_SCOREBOARD       = 0xD,   // MP's UIMENU_SCOREBOARD is 7
    UISP_MENU_PLAYERCARD  = 0xE,   // MP's UIMENU_GAMERCARD is 8
    UISP_MAIN_SYSTEMLINK  = 0x11,
    UISP_XBOXLIVE_LOBBY           = 0x13,
    UISP_XBOXLIVE_PRIVATE_LOBBY   = 0x14,
    UISP_SYSTEMLINK_LOBBY         = 0x15,
    UISP_MAIN_ONLINE              = 0x16,
    UISP_INVALID          = 0x7F,  // reaches the switch default -> returns 0
};

// SP-native entry point. Callers that know they are on the SP path and have an SP value (e.g.
// Com_Init's error path, transcribed from the SP binary) must call this. The MP-signature
// UI_SetActiveMenu below translates UIMENU_* -> UISP_* and forwards here, so shared MP-enum
// call sites keep working unmodified.
int __cdecl UI_SetActiveMenuSp(int localClientNum, uiMenuCommandSp_t menu);
uiMenuCommandSp_t __cdecl UI_SpMenuFromMpMenu(uiMenuCommand_t menu);
#endif

int __cdecl UI_SetActiveMenu(int localClientNum, uiMenuCommand_t menu);
char *__cdecl UI_TranslateIntegerToOrdinal(int integer);
int __cdecl UI_Popup(int localClientNum, const char *menu);
bool __cdecl UI_ShouldDrawCrosshair();
char *__cdecl UI_GetGameTypeDisplayNameCaps(const char *pszGameType);
char *__cdecl UI_GetMapDisplayNameCaps(const char *pszMap);
void __cdecl UI_DrawConnectScreen(int localClientNum);


extern const dvar_t *ui_ignoreMousePos;
extern const dvar_t *ui_prevTextEntryBox;
extern const dvar_t *ui_blurAmount;
extern const dvar_t *ui_blurDarkenAmount;
extern const dvar_t *ui_mapCount;
extern const dvar_t *ui_browserHardcore;
extern const dvar_t *ui_showEndOfGame;
extern const dvar_t *ui_serverBrowserMenu;
extern const dvar_t *ui_showAllContracts;
extern const dvar_t *ui_hud_hardcore;
extern const dvar_t *ui_radar_client;
extern const dvar_t *ui_allow_classchange;
extern const dvar_t *ui_allow_teamchange;
extern const dvar_t *ui_map_killstreak;
extern const dvar_t *ui_hud_visible;
extern const dvar_t *ui_party_download_bar_height;
extern const dvar_t *ui_party_download_bar_color;
extern const dvar_t *ui_closeAfterPurchase;
extern const dvar_t *ui_classesCurrentItemEquippedIn;
extern const dvar_t *ui_heatMapColor;
extern const dvar_t *ui_heatMapColorForPlayer;

extern char g_mapname[64];
extern bool g_showLoadingScreenMenu;
