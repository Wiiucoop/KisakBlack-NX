#pragma once

struct windowDef_t;
struct menuDef_t;
struct itemDef_s;
struct rectDef_s;
struct editFieldDef_s;
struct multiDef_s;
struct enumDvarDef_s;
struct gameMsgDef_s;
struct listBoxDef_s;
struct animParamsDef_t;
struct focusItemDef_s;
struct textDef_s;
struct ownerDrawDef_s;

struct stringDef_s // sizeof=0x8
{                                       // XREF: stringDef_t/r
    stringDef_s *next;
    const char *str;
};

void __cdecl Window_SetDynamicFlags(int contextIndex, windowDef_t *w, int flags);
void __cdecl Window_AddDynamicFlags(int contextIndex, windowDef_t *w, int newFlags);
void __cdecl Window_RemoveDynamicFlags(int contextIndex, windowDef_t *w, int newFlags);
void __cdecl Window_SetStaticFlags(windowDef_t *w, int flags);
void __cdecl Menu_SetCursorItem(int contextIndex, menuDef_t *menu, int cursorItem);
int __cdecl Item_IsVisible(int localClientNum, int contextIndex, itemDef_s *item);
bool __cdecl Item_EnableShowViaDvar(const itemDef_s *item, int flag);
#ifdef KISAK_NX
void Item_NxReportDvarGate(const itemDef_s *item, int flag);
#endif
void __cdecl Item_SetTextRect(int contextIndex, itemDef_s *item, const rectDef_s *textRect);
int __cdecl Item_GetCursorPosOffset(int contextIndex, itemDef_s *item, const char *text, int delta);
bool __cdecl ListBox_HasValidCursorPos(int contextIndex, itemDef_s *item);
void __cdecl Menu_UpdatePosition(int contextIndex, menuDef_t *menu);
void __cdecl Item_SetScreenCoords(int contextIndex, itemDef_s *item, float x, float y, int horzAlign, int vertAlign);
int __cdecl Item_IsEditFieldDef(itemDef_s *item);
int __cdecl Item_IsTextDefType(itemDef_s *item);
int __cdecl Item_IsFocusDefType(itemDef_s *item);
bool __cdecl Item_IsOwnerDrawDefType(itemDef_s *item);
editFieldDef_s *__cdecl Item_GetEditFieldDef(itemDef_s *item);
multiDef_s *__cdecl Item_GetMultiDef(itemDef_s *item);
ownerDrawDef_s *__cdecl Item_GetOwnerDrawDef(itemDef_s *item);
textDef_s *__cdecl Item_GetTextDef(itemDef_s *item);
focusItemDef_s *__cdecl Item_GetFocusItemDef(itemDef_s *item);
listBoxDef_s *__cdecl Item_GetListBoxDef(itemDef_s *item);
double __cdecl Item_GetListBoxFeederID(const itemDef_s *item);
enumDvarDef_s *__cdecl Item_GetEnumDvarName(itemDef_s *item);
gameMsgDef_s *__cdecl Item_GetGameMsgDef(itemDef_s *item);
void __cdecl Item_PropertiesToAnimState(itemDef_s *item, animParamsDef_t *animParams);
void __cdecl Item_AnimStateToProperties(animParamsDef_t *animParams, itemDef_s *item);
unsigned __int8 *__cdecl UI_Alloc(unsigned int size, int alignment);
void __cdecl String_Init();
const char *__cdecl String_Alloc(const char *p);
int __cdecl hashForString(const char *str);
int __cdecl Int_Parse(const char **p, int *i);
int __cdecl Float_Parse(const char **p, float *f);
void __cdecl UI_GetGameTypesList();
void UI_GetGameTypesList_LoadObj();
void UI_GetGameTypesList_FastFile();
const char *__cdecl UI_GetBusyDotsIndicator();
void __cdecl UI_ListMenus_f();
void __cdecl CL_SelectStringTableEntryInDvar_f();
bool __cdecl UI_VerifyString(const char *str);
