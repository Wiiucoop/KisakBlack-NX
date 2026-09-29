#pragma once
#include <bgame/bg_public.h>
#include "g_main_mp.h"
#include <cgame/cg_weapons.h>
#include <cgame_mp/cg_predict_mp.h>

struct viewState_t;

void __cdecl P_DamageFeedback(gentity_s *player);
void __cdecl ClientImpacts(gentity_s *ent, pmove_t *pm);
void __cdecl G_DoTouchTriggers(gentity_s *ent);
void __cdecl SpectatorThink(gentity_s *ent, usercmd_s *ucmd);
int __cdecl ClientInactivityTimer(gclient_s *client);
void __cdecl ClientIntermissionThink(gentity_s *ent);
void __cdecl NotifyGrenadePullback(gentity_s *ent, unsigned int weaponIndex);
void __cdecl HandleClientEvent(gclient_s *client, gentity_s *ent, int event, int eventParm);
void __cdecl AttemptLiveGrenadePickup(gentity_s *clientEnt);
bool __cdecl IsLiveGrenade(gentity_s *ent);
void __cdecl ClientEvents(gentity_s *ent, __int16 oldEventSequence);
void __cdecl G_SetLastServerTime(int clientNum, int lastServerTime);
void __cdecl G_SetClientContents(gentity_s *pEnt);
void __cdecl ClientVehicleInteraction(gentity_s *ent);
void __cdecl ClientVehicleJumpOff(gentity_s *ent);
void __cdecl G_PlayerVehiclePositionAndBlend(gentity_s *ent, gentity_s *pTurretEnt);
void __cdecl ClientThink_real(gentity_s *ent, usercmd_s *ucmd);
void __cdecl Player_WaterUpdate(gentity_s *ent);
void __cdecl G_PlayerStateToEntityStateExtrapolate(playerState_s *ps, entityState_s *s, int time, int snap);
void __cdecl G_AddPlayerMantleBlockage(float *endPos, int duration, pmove_t *pm);
void __cdecl G_DrawServerAiming(const gclient_s *client);
void __cdecl G_BuildWeaponState(gclient_s *client, const viewState_t *vs, weaponState_t *ws);
void __cdecl G_SaveWeaponState(const weaponState_t *ws, gclient_s *client);
void __cdecl ClientThink(int clientNum);
void __cdecl G_RunClient(gentity_s *ent);
void __cdecl IntermissionClientEndFrame(gentity_s *ent);
bool __cdecl G_ClientCanSpectateTeam(gclient_s *client, team_t team);
bool __cdecl G_ClientCanSpectateTeamOrLocalPlayer(gclient_s *client, clientState_s *cs);
void __cdecl SpectatorClientEndFrame(gentity_s *ent);
int __cdecl GetFollowPlayerState(unsigned int clientNum, playerState_s *ps);
int __cdecl StuckInClient(gentity_s *self);
void __cdecl CM_CheckForTraps(gentity_s *ent);
void __cdecl G_PlayerController(const gentity_s *self, int *partBits);
int __cdecl G_UpdateClientInfo(gentity_s *ent);
void __cdecl ClientEndFrame(gentity_s *ent);
void __cdecl G_AddClientKnife(clientInfo_t *ci, playerState_s *ps);
gentity_s *__cdecl G_GetPlayer(unsigned int clientNum);
void __cdecl G_PlayerEvent(int clientNum, int event);

#ifdef KISAK_SP
void G_SetGrenadeSuicideDisabled_SP(bool disabled);
bool G_IsGrenadeSuicideDisabled_SP();
#endif

extern pmove_t g_pmove[32];
