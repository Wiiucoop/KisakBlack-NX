#pragma once
#include "demo_common.h"
#include <client/screen_placement.h>

struct Material;

const char *__cdecl Demo_GetKeyboardTitle();
const char *__cdecl Demo_GetKeyboardDefaultText();
int __cdecl Demo_GetKeyboardTextSize();
void __cdecl Demo_KeyboardComplete(int localClientNum, char *string, bool errorReported, char *errorString);
void __cdecl Demo_GetStartAndEndTime(int *startTime, int *endTime);
void __cdecl Demo_GetDuration(char *outputText, int duration);
char *__cdecl Demo_GetAuthor();
char *__cdecl Demo_GetTitleName();
char *__cdecl Demo_GetTitleDescription();
char *__cdecl Demo_GetTimeInfo();
char *__cdecl Demo_GetDuration();
char *__cdecl Demo_GetSaveScreenName(const char *type);
char *__cdecl Demo_GetSaveScreenDescription(const char *type);
void __cdecl Demo_GetConnectionTimeInfoForPlayer(int localClientNum, int *connectTime, int *disconnectTime);
int __cdecl Demo_GetPlayerIndexForClientNum(int clientNum, int serverTime);
int __cdecl Demo_GetNextDefaultBookmarkForPlayer(
                int localClientNum,
                int index,
                int *type,
                int *time,
                float (*color)[4]);
int __cdecl Demo_GetClientNumForXuid(int localClientNum, unsigned __int64 xuid);
char __cdecl Demo_GetActivatedColorForButton(int demoControlButtonType, float (*color)[4]);
void __cdecl Demo_DrawVerticalLine(
                const ScreenPlacement *scrPlace,
                float x,
                float y,
                float w,
                float h,
                int horzAlign,
                int vertAlign,
                float s1,
                float t1,
                float s2,
                float t2,
                const float *color,
                Material *material);
int __cdecl Demo_GetSegmentCount();
const char *__cdecl Demo_GetSegmentName(int index);
char *__cdecl Demo_GetSegmentTransition(int index);
const char *__cdecl Demo_GetSegmentInformation(int index, const char *s);
char __cdecl Demo_IsPauseMenuOpen(int localClientNum);
void __cdecl Demo_OpenSavePopUp(int localClientNum, int type);
bool __cdecl Demo_IsSaveDialogOpen(int localClientNum);
void __cdecl Demo_Error(int localClientNum, const char *errorTitle, const char *errorMessage);
bool __cdecl Demo_IsErrorDialogOpen(int localClientNum);
