#include "sv_offline_stats.h"

#if defined(KISAK_DEDICATED) && defined(OPENBLOPS_NO_STEAM_AUTH)
#include <server/server.h>
#include <live/live_stats.h>
#include <qcommon/common.h>
#include <clientscript/cscr_vm.h>
#include <ddl/ddl_buffer.h>
#include <vector>
#include <cstring>

namespace
{
enum class ProfileState { Empty, Initializing, Ready };
struct Profile
{
    ProfileState state = ProfileState::Empty;
    unsigned int generation = 0;
};
Profile profiles[32];
ddlDef_t *schema;
unsigned long long schemaFingerprint;
bool schemaReady;

unsigned long long Fingerprint(const ddlDef_t *ddl)
{
    unsigned long long hash = 1469598103934665603ULL;
    auto add = [&hash](unsigned int value) { hash = (hash ^ value) * 1099511628211ULL; };
    auto name = [&add](const char *text) { if (text) while (*text) add((unsigned char)*text++); add(0); };
    add(ddl->version); add(ddl->size); add(ddl->structCount); add(ddl->enumCount);
    for (int i = 0; i < ddl->structCount; ++i)
    {
        const auto &structure = ddl->structList[i];
        name(structure.name); add(structure.size); add(structure.memberCount);
        for (int j = 0; j < structure.memberCount; ++j)
        {
            const auto &member = structure.members[j];
            name(member.name); add(member.size); add(member.offset); add(member.type);
            add(member.externalIndex); add(member.min); add(member.max);
            add(member.arraySize); add(member.enumIndex); add(member.permission);
        }
    }
    for (int i = 0; i < ddl->enumCount; ++i)
    {
        const auto &enumeration = ddl->enumList[i];
        name(enumeration.name); add(enumeration.memberCount);
        for (int j = 0; j < enumeration.memberCount; ++j) name(enumeration.members[j]);
    }
    return hash;
}
}

void SV_OfflineStatsResetAll()
{
    for (unsigned int slot = 0; slot < 32; ++slot) SV_OfflineStatsReset(slot);
    schema = nullptr;
    schemaReady = false;
}

void SV_OfflineStatsReset(unsigned int slot)
{
    if (slot >= 32) return;
    profiles[slot].state = ProfileState::Empty;
    ++profiles[slot].generation;
}

bool SV_OfflineStatsBindSchema()
{
    schemaReady = false;
    if (!LiveStats_InitServerSchema()) return false;
    ddlDef_t *next = LiveStats_GetStatsDDL();
    const auto fingerprint = Fingerprint(next);
    for (const auto &profile : profiles)
    {
        if (profile.state == ProfileState::Ready && fingerprint != schemaFingerprint)
        {
            Com_PrintError(15, "Offline stats: schema changed while players are connected; disconnect players before changing schema.\n");
            return false;
        }
    }
    schema = next;
    schemaFingerprint = fingerprint;
    schemaReady = true;
    return true;
}

bool SV_OfflineStatsReady(unsigned int slot)
{
    return schemaReady && svs.clients && slot < 32 && com_maxclients
        && slot < (unsigned int)com_maxclients->current.integer
        && profiles[slot].state == ProfileState::Ready;
}

bool SV_OfflineStatsConnect(unsigned int slot)
{
    if (SV_OfflineStatsReady(slot)) return true; // Map restart/reconnect callback for this occupant.
    if (!schemaReady || !svs.clients || slot >= 32 || !com_maxclients
        || slot >= (unsigned int)com_maxclients->current.integer) return false;
    auto &profile = profiles[slot];
    if (profile.state != ProfileState::Empty) return false;
    profile.state = ProfileState::Initializing;
    auto &client = svs.clients[slot];
    if (!LiveStats_BuildServerDefaults((char *)client.stats, (char *)client.globalStats, client.purchasedItems)
        || !DDL_AssociateBuffer((char *)client.stats, sizeof(client.stats), schema)
        || !DDL_AssociateBuffer((char *)client.globalStats, sizeof(client.globalStats), schema))
    {
        memset(client.stats, 0, sizeof(client.stats));
        memset(client.globalStats, 0, sizeof(client.globalStats));
        memset(client.globalStatsStable, 0, sizeof(client.globalStatsStable));
        memset(client.purchasedItems, 0, sizeof(client.purchasedItems));
        SV_OfflineStatsReset(slot);
        Com_PrintError(15, "Offline stats: slot %u initialization failed (DDL version %d).\n", slot, schema->version);
        return false;
    }
    memcpy(client.globalStatsStable, client.globalStats, sizeof(client.globalStatsStable));
    memset(client.modifiedStatBytes, 0, sizeof(client.modifiedStatBytes));
    client.statsModified = false;
    client.statsValidated = true;
    // This record was created locally; no network stats packets were received.
    client.statPacketsReceived = 0;
    profile.state = ProfileState::Ready;
    Com_Printf(15, "Offline stats: slot %u generation %u READY (DDL version %d).\n", slot, profile.generation, schema->version);
    return true;
}

char *SV_OfflineStatsBuffer(unsigned int slot, const ddlState_t *state)
{
    if (!SV_OfflineStatsReady(slot))
        Scr_Error("dstat: offline player profile is not ready", 0);
    if (!state || state->ddl != schema || !state->member)
        Scr_Error("dstat: invalid schema or missing leaf", 0);
    const auto &member = *state->member;
    if (member.arraySize <= 0 || member.size <= 0 || member.size % member.arraySize
        || state->absoluteOffset < 0 || member.type < 0 || member.type > 5 || member.type == 4
        || (member.arraySize > 1 && (state->arrayIndex < 0 || state->arrayIndex >= member.arraySize)))
        Scr_Error("dstat: invalid or non-leaf member", 0);
    const unsigned int bits = member.size / member.arraySize;
    const unsigned long long end = (unsigned long long)state->absoluteOffset + bits;
    if (end > (unsigned int)schema->size || end + 320 > (unsigned long long)STATS_BUFFER_SIZE * 8
        || (member.type <= 2 && bits > 32) || (member.type == 3 && bits != 64)
        || (member.type == 5 && (bits % 8 || state->absoluteOffset % 8)))
        Scr_Error("dstat: member extends outside its stats buffer or has an invalid width", 0);
    // Session records use permissions consistently, independent of client UI/training dvars.
    return member.permission == 2 ? (char *)svs.clients[slot].globalStats : (char *)svs.clients[slot].stats;
}

char *SV_OfflineStatsString(unsigned int slot, const ddlState_t *state)
{
    char *buffer = SV_OfflineStatsBuffer(slot, state);
    if (state->member->type != 5) Scr_Error("dstat: expected string member", 0);
    const size_t bytes = state->member->size / state->member->arraySize / 8;
    // Avoid the legacy shared 64-byte DDL scratch and its array-element overread.
    static thread_local std::vector<char> result;
    result.resize(bytes + 1);
    memcpy(result.data(), buffer + 40 + state->absoluteOffset / 8, bytes);
    result[bytes] = '\0';
    return result.data(); // Caller immediately copies into script-owned storage.
}

void SV_OfflineStatsSetString(unsigned int slot, const ddlState_t *state, const char *value)
{
    char *buffer = SV_OfflineStatsBuffer(slot, state);
    if (state->member->type != 5 || !value) Scr_Error("dstat: expected string value", 0);
    const size_t bytes = state->member->size / state->member->arraySize / 8;
    const size_t length = strlen(value);
    if (length > bytes) Scr_Error("dstat: string exceeds the DDL element capacity", 0);
    char *destination = buffer + 40 + state->absoluteOffset / 8;
    memset(destination, 0, bytes);
    memcpy(destination, value, length);
    DDL_SetValueChanged(buffer);
}
#endif
