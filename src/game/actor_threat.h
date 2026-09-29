#pragma once
#include "sentient.h"

enum ThreatDebugStringCategory : __int32
{                                       // XREF: DebugSetThreatString/r
                                        // DebugSetThreatStringFromString/r
    TDS_TOTAL_THREAT     = 0x0,
    TDS_FLASHED          = 0x1,
    TDS_SUPPRESSED       = 0x2,
    TDS_THREATBIAS       = 0x3,
    TDS_THREATBIAS_GROUP = 0x4,
    TDS_ATTACKER_COUNT   = 0x5,
    TDS_CUR_ENEMY_BONUS  = 0x6,
    TDS_AWARENESS        = 0x7,
    TDS_DIST_THREAT      = 0x8,
    TDS_SCARINESS_THREAT = 0x9,
    NUM_TDS              = 0xA,
};

struct potential_threat_t // sizeof=0xC
{                                       // XREF: actor_s/r
    bool isEnabled;
    // padding byte
    // padding byte
    // padding byte
    float direction[2];
};

struct threat_bias_t // sizeof=0x424
{                                       // XREF: .data:threat_bias_t g_threatBias/r
    unsigned __int16 groupName[16];     // XREF: Actor_InitThreatBiasGroups(void)+5/w
                                        // Actor_InitThreatBiasGroups(void)+A/w ...
    int threatTable[16][16];            // XREF: Actor_InitThreatBiasGroups(void)+34/o
                                        // Actor_GetThreatBias(int,int)+83/r
    int threatGroupCount;               // XREF: Actor_InitThreatBiasGroups(void)+41/w
                                        // Actor_GetThreatBias(int,int)+15/r ...
};

extern threat_bias_t g_threatBias;

bool __fastcall Actor_CheckIgnore(sentient_s *self, sentient_s *enemy);
void __cdecl Actor_InitThreatBiasGroups();
int __fastcall Actor_GetThreatBias(int groupSelf, int groupEnemy);
void __fastcall Actor_FlagEnemyUnattackable(actor_s *self);
bool __fastcall Actor_CaresAboutInfo(actor_s *self, sentient_s *pOther);
double __cdecl Sentient_GetScarinessForDistance(sentient_s *self, sentient_s *enemy, float fDist);
void __fastcall Actor_InitThreatUpdateInterval(actor_s *self);
void __fastcall Actor_UpdateThreat(actor_s *self);
void __cdecl DebugThreatStringSimple(const actor_s *self, gentity_s *enemy, char *string, const float *color);
int __fastcall Actor_UpdateSingleThreat(actor_s *self, sentient_s *enemy);
void __cdecl DebugResetThreatStrings(const actor_s *self);
void __cdecl DebugSetThreatString(ThreatDebugStringCategory category, int threat);
void __cdecl DebugSetThreatStringFromString(ThreatDebugStringCategory category, const char *string);
void __cdecl DebugThreatStringAll(const actor_s *self, sentient_s *enemy, int threat);
int __fastcall Actor_ThreatFromScariness(float fScariness);
int __fastcall Actor_ThreatFromDistance(float fDistance);
bool __fastcall Actor_CanBePathedTo(actor_s *self, sentient_s *enemy, int isCurrentEnemy);
bool __fastcall Actor_IsFullyAware(actor_s *self, sentient_s *enemy, int isCurrentEnemy);
void __cdecl DebugThreatNodes(
                const actor_s *self,
                sentient_s *enemy,
                pathnode_t *selfNode,
                pathnode_t *enemyNode,
                const float *color);
int __fastcall Actor_ThreatFromVisibilityAndAwareness(int isVisible, int isFullyAware, int friendlyTimingOut);
int __fastcall Actor_ThreatFromAttackerCount(actor_s *self, sentient_s *enemy, int isCurrentEnemy);
bool __fastcall Actor_ShouldIgnoreFromAttackerCount(actor_s *self, sentient_s *enemy, int isCurrentEnemy);
int __fastcall Actor_ThreatBonusForCurrentEnemy(
                int isCurrentEnemy,
                int isFullyAware,
                int friendlyTimingOut,
                int isPlayer,
                int isDamaged);
int __fastcall Actor_ThreatCoveringFire(actor_s *self, sentient_s *enemy);
int __fastcall Actor_ThreatFlashed(sentient_s *enemy);
void __fastcall Actor_IncrementThreatTime(actor_s *self);
void __fastcall Actor_SetPotentialThreat(potential_threat_t *self, float yaw);
void __fastcall Actor_ClearPotentialThreat(potential_threat_t *self);
void __cdecl Actor_PotentialThreat_Debug(actor_s *self);
