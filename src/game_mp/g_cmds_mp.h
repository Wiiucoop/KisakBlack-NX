#pragma once
#include <bgame/bg_public.h>

int __cdecl CheatsOk(gentity_s *ent);
char *__cdecl ConcatArgs(int start);
int __cdecl SV_Cmd_Argc();
void __cdecl G_setfog(char *fogstring);
void __cdecl Cmd_Give_f(gentity_s *ent);
void __cdecl Cmd_Take_f(gentity_s *ent);
void __cdecl Cmd_God_f(gentity_s *ent);
void __cdecl Cmd_DemiGod_f(gentity_s *ent);
void __cdecl Cmd_Notarget_f(gentity_s *ent);
void __cdecl Cmd_Noclip_f(gentity_s *ent);
void __cdecl Cmd_UFO_f(gentity_s *ent);
void __cdecl Cmd_Kill_f(gentity_s *ent);
void __cdecl StopFollowing(gentity_s *ent);
int __cdecl Cmd_FollowCycle_f(gentity_s *ent, int dir);
bool __cdecl G_IsPlaying(gentity_s *ent);
void __cdecl G_Say(gentity_s *ent, gentity_s *target, int mode, const char *chatText);
void __cdecl G_SayTo(
                gentity_s *ent,
                gentity_s *other,
                int mode,
                int color,
                const char *teamString,
                const char *cleanname,
                const char *message);
void __cdecl Cmd_Where_f(gentity_s *ent);
void __cdecl Cmd_CallVote_f(gentity_s *ent);
void __cdecl Cmd_Vote_f(gentity_s *ent);
void __cdecl Cmd_SetViewpos_f(gentity_s *ent);
void __cdecl Cmd_EntityCount_f();
void __cdecl Cmd_MenuResponse_f(gentity_s *pEnt);
#ifdef KISAK_SP
void __cdecl Cmd_MenuLevelMessage_f();   // retail FUN_0054bbb0, dispatched from ClientCommand "mlvl"
#endif
void __cdecl ClientCommand(int clientNum);
void __cdecl Cmd_Say_f(gentity_s *ent, int mode, int arg0);
void Cmd_PrintEntities_f();
void Cmd_VisionSetNaked_f();
const char *__cdecl SV_Cmd_Argv(int argIndex);
void Cmd_VisionSetNight_f();
