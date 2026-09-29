#pragma once
#include <ddl/ddl_api.h>

#if defined(KISAK_DEDICATED) && defined(OPENBLOPS_NO_STEAM_AUTH)
void SV_OfflineStatsResetAll();
void SV_OfflineStatsReset(unsigned int slot);
bool SV_OfflineStatsBindSchema();
bool SV_OfflineStatsConnect(unsigned int slot);
bool SV_OfflineStatsReady(unsigned int slot);
char *SV_OfflineStatsBuffer(unsigned int slot, const ddlState_t *state);
char *SV_OfflineStatsString(unsigned int slot, const ddlState_t *state);
void SV_OfflineStatsSetString(unsigned int slot, const ddlState_t *state, const char *value);
#endif
