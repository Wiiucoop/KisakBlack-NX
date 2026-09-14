#pragma once
#include <qcommon/cmd.h>
#include <client/screen_placement.h>
#include "ui_shared.h"
#include <universal/dvar.h>

struct Font_s;
struct UiContext;
struct uiInfo_s;
struct UILocalVarContext;

uiInfo_s *__cdecl UI_UIContext_GetInfo(int contextIndex);
uiInfo_s *__cdecl UI_GetInfo(int localClientNum);
UILocalVarContext *__cdecl UI_UIContext_GetLocalVarsContext(int contextIndex);
const char *__cdecl UI_GetMonthAbbrev(unsigned int month);
const char **__cdecl UI_GetServerFilter(const char **filter, int filtera);
bool __cdecl UI_KeysBypassMenu(int localClientNum);
char *__cdecl UI_GetMenuBuffer(char *filename);
char *__cdecl GetMenuBuffer_LoadObj(char *filename);
char *__cdecl GetMenuBuffer_FastFile(const char *filename);
void __cdecl UI_DrawBuildNumber(int contextIndex);
void __cdecl UI_DrawSides(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                float size,
                const float *color);
void __cdecl UI_DrawTopBottom(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                float size,
                const float *color);
void __cdecl UI_DrawRect(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float width,
                float height,
                int horzAlign,
                int vertAlign,
                float size,
                const float *color);
void __cdecl UI_DrawHighlightRect(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                float size,
                const float *hiColor,
                const float *loColor);
int __cdecl UI_TextWidth(const char *text, int maxChars, Font_s *font, float scale);
int __cdecl UI_TextHeight(Font_s *font, float scale);
void __cdecl UI_DrawText(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                int horzAlign,
                int vertAlign,
                float scale,
                const float *color,
                int style);
void __cdecl UI_DrawTextRotated(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                int horzAlign,
                int vertAlign,
                float scale,
                const float *color,
                int style,
                float rotation);
void __cdecl UI_DrawTextWithGlowW(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                float w,
                int horzAlign,
                int vertAlign,
                float scale,
                const float *color,
                int style,
                const float *glowColor,
                bool subtitle,
                bool cinematic);
void __cdecl UI_DrawTextWithGlow(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                int horzAlign,
                int vertAlign,
                float scale,
                const float *color,
                int style,
                const float *glowColor,
                bool subtitle,
                bool cinematic);
void __cdecl UI_DrawTextNoSnap(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                int horzAlign,
                int vertAlign,
                float scale,
                const float *color,
                int style);
void __cdecl UI_DrawTextWithCursor(
                const ScreenPlacement *scrPlace,
                char *text,
                int maxChars,
                Font_s *font,
                float x,
                float y,
                int horzAlign,
                int vertAlign,
                float scale,
                const float *color,
                int style,
                int cursorPos,
                char cursor);
Font_s *__cdecl UI_GetFontHandleDefault(float scale);
Font_s *__cdecl UI_GetFontHandle(const ScreenPlacement *scrPlace, int fontEnum, float scale);
void __cdecl UI_UpdateTime(int localClientNum, int realtime);
void __cdecl UI_Refresh(int localClientNum);
bool __cdecl UI_ShouldDrawBuildNumber(int localClientNum);
bool __cdecl UI_ShouldUpdateFriends();
void __cdecl UI_Shutdown(int localClientNum);
int __cdecl Load_ScriptMenu(int localClientNum, const char *pszMenu, int imageTrack);
MenuList *__cdecl Load_ScriptMenuInternal(const char *pszMenu, int imageTrack);
char *__cdecl UI_GetMapDisplayName(const char *pszMap);
char *__cdecl UI_GetMapDisplayNameFromPartialLoadNameMatch(const char *mapName, int *mapLoadNameLen);
char *__cdecl UI_GetGameTypeDisplayName(const char *pszGameType);
char *__cdecl UI_GetGameTypeDisplayShortName(const char *pszGameType);
void __cdecl UI_Update(const char *name);
void __cdecl UI_VerifyLanguage();
void __cdecl UI_UpdateDisplayServers(int localClientNum, uiInfo_s *uiInfo);
int __cdecl UI_GetClientNumForPlayerListNum(int playerListIndex);
void __cdecl UI_RunMenuScript(int localClientNum, int contextIndex, const char **args, const char *actualScript);
char __cdecl UI_GetOpenOrCloseMenuOnDvarArgs(
                const char **args,
                const char *cmd,
                char *dvarName,
                int dvarNameLen,
                char *testValue,
                int testValueLen,
                char *menuName,
                int menuNameLen);
void __cdecl UI_OpenMenuOnDvar(
                int localClientNum,
                uiInfo_s *uiInfo,
                const char *cmd,
                const char *menuName,
                const char *dvarName,
                const char *testValue);
bool __cdecl UI_DvarValueTest(const char *cmd, const char *dvarName, const char *testValue, bool wantMatch);
void __cdecl UI_CloseMenuOnDvar(
                int localClientNum,
                uiInfo_s *uiInfo,
                const char *cmd,
                const char *menuName,
                const char *dvarName,
                const char *testValue);
void __cdecl UI_Pause(int localClientNum, int b);
void __cdecl UI_MouseEvent(int localClientNum, int x, int y);
uiMenuCommand_t __cdecl UI_GetActiveMenu(int localClientNum);
const char *__cdecl UI_GetTopActiveMenuName(int localClientNum);
int __cdecl UI_IsFullscreen(int localClientNum);
double __cdecl UI_GetBlurRadius(int localClientNum);
char *__cdecl UI_SafeTranslateString(const char *reference);
bool __cdecl UI_AnyMenuVisible(int localClientNum);
char *__cdecl UI_ReplaceConversionString(char *sourceString, const char *replaceString);
char *__cdecl UI_ReplaceConversionStrings(char *sourceString, int numStrings, const char **replaceStrings);
char *__cdecl UI_ReplaceConversionInt(char *sourceString, int replaceInt);
char *__cdecl UI_ReplaceConversionInts(char *sourceString, int numInts, int *replaceInts);
void __cdecl UI_FilterStringForButtonAnimation(char *str, unsigned int strMaxSize);
void __cdecl UI_ReplaceConversions(
                char *sourceString,
                ConversionArguments *arguments,
                char *outputString,
                int outputStringSize);
void __cdecl UI_CloseAll(int localClientNum);
void __cdecl UI_CloseFocusedMenu(int localClientNum);
bool __cdecl Menu_IsMenuOpenAndVisible(int localClientNum, const char *menuName);
int __cdecl UI_PopupScriptMenu(int localClientNum, const char *menuName, bool useMouse);
void __cdecl UI_ClosePopupScriptMenu(int localClientNum, bool allowResponse);
bool __cdecl UI_AllowScriptMenuResponse(int localClientNum);
void __cdecl UI_CloseInGameMenu(int localClientNum);
void __cdecl UI_UpdatePendingPings(uiInfo_s *uiInfo);
int __cdecl UI_CheckExecKey(int localClientNum, int key);
int __cdecl UI_OwnerDrawWidth(int ownerDraw, Font_s *font, float scale);
char __cdecl UI_DrawRecordLevel(int contextIndex, rectDef_s *rect);
int __cdecl UI_GetTalkerClientNum(int localClientNum, int contextIndex, int num);
void __cdecl UI_DrawPrettyLine(
                int contextIndex,
                float *p1,
                float *p2,
                float lineWidth,
                int horzAlign,
                int vertAlign,
                float *color);
void __cdecl UI_OwnerDraw(
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
void __cdecl UI_DrawLocalTalking(int contextIndex, const rectDef_s *rect, const float *color);
void __cdecl UI_DrawPlaylistIcon(int localClientNum, const rectDef_s *rect, const float *color);
void __cdecl UI_DrawCategoryIcon(int localClientNum, rectDef_s *rect, float *color);
void __cdecl UI_DrawControllerIcon(int localClientNum, int contextIndex, const rectDef_s *rect, const float *color);
void __cdecl UI_PrettyLineOwnerdraw(int contextIndex, rectDef_s *rect, float lineWidth, float *color);
void __cdecl UI_OwnerDrawText(
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
                int textAlignMode,
                char *text);
void __cdecl UI_DrawKeyBindStatus(
                int contextIndex,
                rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                int textStyle);
void __cdecl UI_DrawTalkerNum(
                int localClientNum,
                int contextIndex,
                int num,
                rectDef_s *rect,
                Font_s *font,
                float *color,
                float textScale,
                int style);
void __cdecl UI_DrawReservedSlots(
                int contextIndex,
                rectDef_s *rect,
                Font_s *font,
                float *color,
                float textScale,
                int style);
void __cdecl UI_DrawLoggedInUser(
                int localClientNum,
                int contextIndex,
                rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                int textStyle);
double __cdecl UI_GetTextAlignAdj(int alignment, float width, float textWidth);
void __cdecl UI_DrawLoggedInUserName(
                int localClientNum,
                int contextIndex,
                rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                int textStyle);
void __cdecl UI_DrawPlaylistDescription(
                int localClientNum,
                rectDef_s *rect,
                Font_s *font,
                float *color,
                float textScale,
                int style,
                char textAlignment);
char *__cdecl GetXpLockDescription(int localClientNum, int playlistId);
char *__cdecl GetPLevelLockDescription(int localClientNum, int playlistId);
void __cdecl UI_DrawScrollingText(
                int contextIndex,
                rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                int style);
void __cdecl UI_DrawScrollingTextCategory(
                int contextIndex,
                rectDef_s *rect,
                Font_s *font,
                float scale,
                float *color,
                int style);
bool __cdecl UI_OwnerDrawVisible(__int16 flags);
int __cdecl UI_OwnerDrawHandleKey(int ownerDraw, int flags, int key);
void __cdecl UI_InitOnceForAllClients();
void UI_RegisterDvars();
void UI_AssetCache();
void UI_InitUIInfos();
void __cdecl UI_Init(int localClientNum);
char __cdecl UI_KeyEvent_AutoJoinButtonPressed(int localClientNum, UiContext *dc, menuDef_t *menu, int key, int down);
void __cdecl UI_KeyEvent(int localClientNum, int key, int down);
bool __cdecl UI_KeyEvent_CancelButtonPressed(UiContext *dc, menuDef_t *menu, int key, int down);
void __cdecl UI_CloseAllMenus(int localClientNum);
void __cdecl UI_UIContext_OpenMenu(int localClientNum, int contextIndex, const char *menuName);
void __cdecl UI_UIContext_CloseMenu(int localClientNum, int contextIndex, const char *menuName);
void __cdecl UI_UIContext_CloseMenuImmediate(int localClientNum, int contextIndex, const char *menuName);
void __cdecl UI_UIContext_OpenToastPopup(
                int contextIndex,
                const char *toastPopupIconName,
                const char *GLOBAL_EMPTY_STRING,
                const char *toastPopupDesc,
                int toastPopupDuration);
void __cdecl UI_OpenMenu(int localClientNum, const char *menuName);
void __cdecl UI_CloseMenu(int localClientNum, const char *menuName);
void __cdecl UI_CloseMenuImmediate(int localClientNum, const char *menuName);
void __cdecl UI_OpenToastPopup(
                int localClientNum,
                const char *toastPopupIconName,
                const char *GLOBAL_EMPTY_STRING,
                const char *toastPopupDesc,
                int toastPopupDuration);
void __cdecl UI_DrawMapLevelshot(int localClientNum);
void __cdecl UI_LoadIngameMenus(int contextIndex);
unsigned __int8 __cdecl UI_ToUpper_CodePage_1250(unsigned __int8 character);
unsigned __int8 __cdecl UI_ToUpper_CodePage_1251(unsigned __int8 character);
unsigned __int8 __cdecl UI_ToUpper_CodePage_1252(unsigned __int8 character);
unsigned __int8 __cdecl UI_CharToUpper(unsigned __int8 character);
char *__cdecl UI_ToUpper(const char *sourceString);

extern const dvar_t *ui_netGameType;
extern const dvar_t *uiscript_debug;
extern const dvar_t *ui_animSpeedScale;
extern const dvar_t *ui_customModeIndex;
extern const dvar_t *ui_customModeName;
extern const dvar_t *ui_customModeEditName;
extern const dvar_t *ui_customModeEditDesc;
extern const dvar_t *ui_customModeDesc;
extern const dvar_t *ui_customClassName;
extern const dvar_t *g_allowvote;
extern const dvar_t *cg_brass;
extern const dvar_t *fx_marks;
extern const dvar_t *server1;
extern const dvar_t *server2;
extern const dvar_t *server3;
extern const dvar_t *server4;
extern const dvar_t *server5;
extern const dvar_t *server6;
extern const dvar_t *server7;
extern const dvar_t *server8;
extern const dvar_t *server9;
extern const dvar_t *server10;
extern const dvar_t *server11;
extern const dvar_t *server12;
extern const dvar_t *server13;
extern const dvar_t *server14;
extern const dvar_t *server15;
extern const dvar_t *server16;
extern const dvar_t *ui_netSource;
extern const dvar_t *ui_smallFont;
extern const dvar_t *ui_bigFont;
extern const dvar_t *ui_extraBigFont;
extern const dvar_t *ui_currentMap;
extern const dvar_t *ui_gametype;
extern const dvar_t *ui_mapname;
extern const dvar_t *ui_demoname;
extern const dvar_t *ui_currentRecentPlayer;
extern const dvar_t *ui_serverStatusTimeOut;
extern const dvar_t *ui_buildLocation;
extern const dvar_t *ui_buildSize;
extern const dvar_t *ui_showList;
extern const dvar_t *ui_showMenuOnly;
extern const dvar_t *ui_menuLvlNotify;
extern const dvar_t *selectedFriendName;
extern const dvar_t *selectedFriendClanTag;
extern const dvar_t *selectedPlayerClanName;
extern const dvar_t *selectedFriendIndex;
extern const dvar_t *selectedStoreItemIndex;
extern const dvar_t *selectedGroupIndex;
extern const dvar_t *selectedPlayerXuid;
extern const dvar_t *selectedMenuItemIndex;
extern const dvar_t *highlightedPlayerXuid;
extern const dvar_t *highlightedPlayerName;
extern const dvar_t *highlightedPlayerClanTag;
extern const dvar_t *splitscreen_lobbyPlayerCount;
extern const dvar_t *splitscreen_partyPlayerCount;
extern const dvar_t *tickerHeaderWidth;
extern const dvar_t *ui_borderLowLightScale;
extern const dvar_t *ui_cinematicsTimestamp;
extern const dvar_t *ui_connectScreenTextGlowColor;
extern const dvar_t *ui_drawCrosshair;
extern const dvar_t *ui_onCloseArgs;
extern const dvar_t *notice_onEscArg;
extern const dvar_t *statusinfo_onEscArg;
extern const dvar_t *ui_ethernetLinkActive;
extern const dvar_t *ui_listboxIndex;
extern const dvar_t *ui_readingSaveDevice;
extern const dvar_t *ui_signedInToProfile;
extern const dvar_t *emblem_scroll_delay_first;
extern const dvar_t *emblem_scroll_delay_rest;

extern uiInfo_s uiInfoArray[1];
