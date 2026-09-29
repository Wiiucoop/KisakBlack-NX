#pragma once
#ifdef KISAK_SP
void CG_RegisterLookAtDvars_SP();
void CG_DrawLookAtText_SP(int localClientNum);
#endif
#include <bgame/bg_local.h>
#include "cg_weapons.h"

enum rankTableColumns_t : __int32
{                                       // XREF: ?CL_GetRankData@@YAPBDHW4rankTableColumns_t@@@Z/r
    MP_RANKTABLE_RANKID       = 0x0,
    MP_RANKTABLE_RANK         = 0x1,
    MP_RANKTABLE_MINXP        = 0x2,
    MP_RANKTABLE_XPTONEXT     = 0x3,
    MP_RANKTABLE_SHORTRANK    = 0x4,
    MP_RANKTABLE_FULLRANK     = 0x5,
    MP_RANKTABLE_ICON         = 0x6,
    MP_RANKTABLE_MAXXP        = 0x7,
    MP_RANKTABLE_WEAPUNLOCK   = 0x8,
    MP_RANKTABLE_PERKUNLOCK   = 0x9,
    MP_RANKTABLE_CHALLENGE    = 0xA,
    MP_RANKTABLE_CAMO         = 0xB,
    MP_RANKTABLE_ATTACHMENT   = 0xC,
    MP_RANKTABLE_LEVEL        = 0xD,
    MP_RANKTABLE_DISPLAYLEVEL = 0xE,
    MP_RANKTABLE_FEATURE      = 0xF,
    MP_RANKTABLE_INGAMERANK   = 0x10,
    MP_RANKTABLE_CP           = 0x11,
    MP_RANKTABLE_COUNT        = 0x12,
};

struct __declspec(align(4)) OverheadFade // sizeof=0x10
{                                       // XREF: .data:overheadFade/r
    int nextCheckTime;
    int lastTime;                       // XREF: CG_DrawFriendlyNames(int)+1D1/w
                                        // CG_DrawFriendlyNames(int)+1F1/r ...
    int startTime;                      // XREF: CG_DrawFriendlyNames(int)+1AB/w
                                        // CG_DrawFriendlyNames(int)+1FE/r ...
    bool visible;                       // XREF: CG_DrawFriendlyNames(int)+175/w
                                        // CG_DrawFriendlyNames(int)+184/r ...
    // padding byte
    // padding byte
    // padding byte
};

struct DrawNameEntity // sizeof=0x10
{                                       // XREF: .data:drawNameEntities/r
    int entnum;                         // XREF: CG_DrawNames(int)+54/r
                                        // CG_AddDrawName+75/w
    float alpha;                        // XREF: CG_DrawNames(int)+70/r
                                        // CG_AddDrawName+89/w
    int teamIndicator;                  // XREF: CG_DrawNames(int)+83/r
                                        // CG_AddDrawName+9D/w
    float dist;
};

struct PlayerDetails // sizeof=0x68
{                                       // XREF: .data:PlayerDetails (* playerDetails)[32]/r
    char fullDisplayName[38];
    char clanDisplayName[7];
    char gamerTag[32];
    // padding byte
    // padding byte
    // padding byte
    unsigned int fullDisplayNameLength;
    unsigned int clanDisplayLength;
    unsigned int gamerTagDisplayLength;
    bool inView;
    // padding byte
    // padding byte
    // padding byte
    int inViewStartTime;
    int inViewLastTime;
};

void __cdecl CG_ClearOverheadFade();
void __cdecl CG_ClearPlayerDetails();
void __cdecl CG_InitPlayerDetails(int localClientNum, int targetClientNum);
void __cdecl CG_DrawOverheadNames(int localClientNum, const centity_s *cent, float alpha, int mode);
char __cdecl CG_CalcNamePosition(int localClientNum, float *origin, float *xOut, float *yOut);
void __cdecl CG_GetViewAxisProjections(const refdef_s *refdef, const float *worldPoint, float *projections);
void __cdecl CG_CalcVehicleNamePosition(int localClientNum, const centity_s *cent, float (*origin)[3]);
double __cdecl CG_FadeCrosshairNameAlpha(int time, int startMsec, int lastMsec, int fadeInMsec, int fadeOutMsec);
char __cdecl CG_CanSeeFriendlyHead(int localClientNum, const centity_s *cent);
bool __cdecl CG_TraceEntIsEntity(int traceEnt, const centity_s *cent);
void __cdecl CG_ScanForCrosshairEntity(int localClientNum);
void __cdecl CG_ScanForCrosshairEntityInternal(int localClientNum);
bool __cdecl CG_CheckIfDrivingRemoteControlVehicle(int localClientNum, unsigned __int16 EntId);
void __cdecl CG_DrawNames(int localClientNum);
int __cdecl compareEntityDist(float *v1, float *v2);
int __cdecl CG_GetTeamIndicator();
bool __cdecl ShouldDrawCrosshairNames(const cg_s *cgameGlob);
void __cdecl CG_DrawCrosshairNames(int localClientNum);
void __cdecl CG_AddDrawName(int localClientNum, int entnum, float alpha, int teamIndicator);
char __cdecl CG_IsValidCrosshairEntity(int localClientNum, const cg_s *cgameGlob, const centity_s *cent, int entNum);
bool __cdecl ShouldDrawFriendlyName(int localClientNum, const centity_s *cent, int entnum, int team);
int __cdecl GetVehicleDriverEntNum(const cg_s *cgameGlob, const centity_s *cent, int entnum);
void __cdecl CG_DrawFriendlyNames(int localClientNum);
void __cdecl CG_DrawVisibleNames(int localClientNum);
