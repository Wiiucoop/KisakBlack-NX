#pragma once
#ifdef KISAK_SP
bool Scr_IsClientNameField(unsigned int offset);
#endif
#include <game_mp/g_main_mp.h>

struct client_fields_s // sizeof=0x1C
{                                       // XREF: .rdata:fields/r
                                        // client_fields_t/r
    const char *name;
    int ofs;
    int size[1];
    fieldtype_t type;
    unsigned int whichbits;
    void (__cdecl *setter)(gclient_s *, const client_fields_s *);
    void (__cdecl *getter)(gclient_s *, const client_fields_s *);
};

void __cdecl ClientScr_ReadOnly(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_SetSessionTeam(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetFFATeam(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_GetFFATeam(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_GetName(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_GetGroundType(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_GetSessionTeam(gclient_s *pSelf, const client_fields_s *__formal);
#ifdef KISAK_SP
void __cdecl ClientScr_SetDowns_SP(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_GetDowns_SP(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetRevives_SP(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_GetRevives_SP(gclient_s *pSelf, const client_fields_s *__formal);
#endif
int __cdecl CScr_GetColumnTypeByName(const char *typeName);
const char *__cdecl CScr_GetColumnNameByType(scoreboardColumnType_t columnType);
void __cdecl ClientScr_SetSessionState(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_GetSessionState(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetMaxHealth(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetScore(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetKills(gclient_s *pSelf, const client_fields_s *__formal);
VariableUnion __cdecl ClientScr_SetColumnValue(gclient_s *pSelf, scoreboardColumnType_t columnType);
int __cdecl ClientScr_GetScoreboardColumnIndexByType(scoreboardColumnType_t columnType);
void __cdecl ClientScr_SetDeaths(gclient_s *pSelf, const client_fields_s *__formal);

void __cdecl ClientScr_SetAssists(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetDefends(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetPlants(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetDefuses(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetReturns(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetCaptures(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetDestructions(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetSurvived(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetStabs(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetTomahawks(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetHumiliated(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetX2Score(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetHeadshots(gclient_s *pSelf, const client_fields_s *__formal);

void __cdecl ClientScr_SetSpectatorClient(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetKillCamEntity(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetKillCamTargetEntity(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetStatusIcon(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_GetStatusIcon(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetHeadIcon(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_GetHeadIcon(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetHeadIconTeam(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_GetHeadIconTeam(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetArchiveTime(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_GetArchiveTime(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_SetPSOffsetTime(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl ClientScr_GetEntityFromIndex(gclient_s *pSelf, const client_fields_s *pField);
void __cdecl ClientScr_GetPSOffsetTime(gclient_s *pSelf, const client_fields_s *__formal);
void __cdecl GScr_AddFieldsForClient();
void __cdecl Scr_SetClientField(gclient_s *client, int offset);
void __cdecl Scr_GetClientField(gclient_s *client, int offset);
