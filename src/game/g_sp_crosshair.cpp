#include "g_sp_crosshair.h"
#ifdef KISAK_SP
#include "actor.h"
#include "g_sp_lookat_nodes.h"
#include "actor_events.h"
#include "bullet.h"
#include "g_weapon.h"
#include "g_scr_vehicle.h"
#include <game_mp/g_main_mp.h>
#include <game_mp/g_spawn_mp.h>
#include <game_mp/g_utils_mp.h>
#include <game_mp/g_combat_mp.h>
#include <game_mp/g_trigger_mp.h>
#include <client_mp/g_client_mp.h>
#include <server_mp/sv_init_mp.h>
#include <server/sv_world.h>
#include <bgame/bg_misc.h>
#include <bgame/bg_weapons.h>
#include <bgame/bg_weapons_def.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/cscr_stringlist.h>
#include <clientscript/scr_const.h>
#include <qcommon/common.h>
#include <universal/com_math.h>
#include <EffectsCore/fx_system.h>
#include <xanim/dobj.h>
#include <cstring>

namespace
{
constexpr int CLIENT_COUNT = 32;
constexpr int ENTITY_COUNT = 1024;
constexpr int LOOK_FLAGS = 0x8 | 0x10 | 0x200000;
constexpr float LOOK_RANGE = 15000.0f;

// Retail client+0x1C80 and entity+0x280/282 cannot be placed at those offsets
// in the MP-derived structures. Keep their ownership, without changing the
// serialized player/entity layouts or the hard-coded gclient allocation stride.
struct PlayerLookAt
{
    EntHandle target;
};
struct EntityLookAt { unsigned short text[2]; unsigned short actorName; };
PlayerLookAt players[CLIENT_COUNT];
EntityLookAt entities[ENTITY_COUNT];

bool ValidClient(int number) { return unsigned(number) < CLIENT_COUNT; }
bool ValidEntity(int number) { return unsigned(number) < ENTITY_COUNT; }

void SetText(int clientNum, int part, unsigned short value)
{
    const char *text = value ? SL_ConvertToString(value, SCRIPTINSTANCE_SERVER) : "none";
    SV_SetConfigstring(CS_SP_LOOKAT_TEXT + clientNum * 2 + part,
        value && part == 0 ? va("\x15%s", text) : const_cast<char *>(text));
}

gentity_s *TraceLookAt(trace_t *trace, const float *start, const float *end,
    int passEntity, int mask, unsigned char *priority, const float *forward)
{
    // Retail 0x00818900: a single locational trace, then FX transmittance.
    G_LocationalTrace(trace, start, end, passEntity, mask, priority, 0);
    const unsigned int hit = Trace_GetEntityHitId(trace);
    if (hit >= ENTITY_COUNT - 2)
        return nullptr;
    float contact[3];
    Vec3Mad(start, trace->fraction * LOOK_RANGE, forward, contact);
    // Retail SP has a local FX visibility system. A headless extension has no
    // rendered smoke blockers; do not dereference its unallocated FX pool.
    if (FX_GetSystem(0) && FX_GetClientVisibility(0, start, contact) < 0.0001f)
        return nullptr;
    return &g_entities[hit];
}
}

void G_SPInitLookAt()
{
    // Called only immediately after EntHandle::Init, never setEnt on old handles
    // after the handle registry has been reset.
    memset(players, 0, sizeof(players));
    memset(entities, 0, sizeof(entities));
    G_SPRegisterLookAtNodes();
    G_SPResetLookAtNodes();
}

void G_SPClearClientLookAt(gclient_s *client)
{
    const int index = int(client - level.clients);
    if (!ValidClient(index)) return;
    players[index].target.setEnt(nullptr);
    memset(&players[index], 0, sizeof(players[index]));
    client->ps.weapFlags &= ~LOOK_FLAGS;
    G_SPResetClientLookAtNodes(index);
    SetText(index, 0, 0);
    SetText(index, 1, 0);
}

void G_SPFreeEntityLookAt(gentity_s *ent)
{
    const int index = int(ent - g_entities);
    if (!ValidEntity(index)) return;
    Scr_SetString(&entities[index].text[0], 0, SCRIPTINSTANCE_SERVER);
    Scr_SetString(&entities[index].text[1], 0, SCRIPTINSTANCE_SERVER);
    G_SPFreeActorName(ent);
    if (ent->client) G_SPClearClientLookAt(ent->client);
}

void G_SPShutdownLookAt()
{
    // Before G_FreeEntities and script shutdown: release registered handles and
    // string references while both registries are alive.
    for (auto &player : players) player.target.setEnt(nullptr);
    for (auto &entity : entities)
    {
        for (auto &text : entity.text) Scr_SetString(&text, 0, SCRIPTINSTANCE_SERVER);
        Scr_SetString(&entity.actorName, 0, SCRIPTINSTANCE_SERVER);
    }
}

void G_SPFreeActorName(gentity_s *ent)
{
    if (ValidEntity(ent->s.number))
        Scr_SetString(&entities[ent->s.number].actorName, 0, SCRIPTINSTANCE_SERVER);
}

void G_SPSetActorName(gentity_s *ent)
{
    Scr_SetString(&entities[ent->s.number].actorName,
        Scr_GetConstStringIncludeNull(0, SCRIPTINSTANCE_SERVER), SCRIPTINSTANCE_SERVER);
}

void G_SPGetActorName(gentity_s *ent)
{
    const unsigned short name = entities[ent->s.number].actorName;
    if (name) Scr_AddConstString(name, SCRIPTINSTANCE_SERVER);
}

void G_SPSetLookAtText(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    EntityLookAt &state = entities[ent->s.number];
    Scr_SetString(&state.text[0], Scr_GetConstString(0, SCRIPTINSTANCE_SERVER), SCRIPTINSTANCE_SERVER);
    // 0x0048CD50 tests count != 0, NOT count > 1. A single argument still
    // performs retail's parameter-1 validation rather than silently clearing it.
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_SetString(&state.text[1], Scr_GetConstIString(1, SCRIPTINSTANCE_SERVER), SCRIPTINSTANCE_SERVER);
}

void G_SPIsLookingAt(scr_entref_t entref)
{
    gentity_s *player = GetEntity(entref);
    if (!player->client || !ValidClient(player->s.number))
        Scr_ObjectError(va("entity %i is not a player", player->s.number), SCRIPTINSTANCE_SERVER);
    EntHandle &target = players[player->s.number].target;
    // Retail short circuits argument evaluation when no look-at handle exists.
    Scr_AddInt(target.isDefined() && target.ent() == Scr_GetEntity(0), SCRIPTINSTANCE_SERVER);
}

void G_SPUpdateLookAtClaim(gentity_s *player)
{
    if (!ValidClient(player->s.number)) return;
    EntHandle &look = players[player->s.number].target;
    G_SPUpdatePlayerNodeClaim(player, look.isDefined() ? look.ent() : nullptr);
}

void G_SPUpdateLookAt(gentity_s *player)
{
    // Behavior reconstructed from retail BlackOps.exe 0x00418D00. This name is
    // descriptive, not an original source-symbol claim. All fields use our types.
    if (!player->client || !player->sentient || !ValidClient(player->s.number)) return;
    playerState_s *ps = &player->client->ps;
    EntHandle &look = players[player->s.number].target;
    ps->weapFlags &= ~LOOK_FLAGS;
    if (player_forceRedCrosshair->current.enabled) ps->weapFlags |= 0x10;
    look.setEnt(nullptr);
    float start[3], forward[3], end[3];
    G_GetPlayerViewOrigin(ps, start);
    G_GetPlayerViewDirection(player, forward, nullptr, nullptr);
    const WeaponDef *weapon = BG_GetWeaponDef(G_GetPlayerWeapon(ps, 0));
    if ((ps->eFlags & 0x4000) && ps->vehiclePos >= 1 && ps->vehiclePos <= 4
        && unsigned(ps->viewlocked_entNum) < ENTITY_COUNT - 2)
    {
        gentity_s *vehicle = &g_entities[ps->viewlocked_entNum];
        if (vehicle->scr_vehicle)
        {
            const int gunner = ps->vehiclePos - 1;
            DObjTrace_s muzzleTrace = {};
            G_TraceBulletPathForVehTurret(vehicle, &muzzleTrace, gunner);
            if (muzzleTrace.fraction != 1.0f) { look.setEnt(vehicle); return; }
            float matrix[4][3];
            G_DObjGetWorldBoneIndexMatrix(vehicle, vehicle->scr_vehicle->boneIndex.gunnerTags[gunner].flash, matrix);
            Vec3Copy(matrix[3], start);
            Vec3Copy(matrix[0], forward);
        }
    }
    extern unsigned char riflePriorityMap[19];
    unsigned char *priority = ps->weapon && weapon->bRifleBullet ? riflePriorityMap : bulletPriorityMap;
    Vec3Mad(start, bg_gunXOffset->current.value, forward, start);
    Vec3Mad(start, LOOK_RANGE, forward, end);
    trace_t trace = {};
    gentity_s *hit = TraceLookAt(&trace, start, end, player->s.number, 0x2280E803, priority, forward);
    G_SPUpdateLookAtNodes(player, start, forward, trace.fraction);
    if (!hit) return;
    if (hit->classname == scr_const.trigger_lookat)
    {
        look.setEnt(hit);
        G_Trigger(hit, player);
        hit = TraceLookAt(&trace, start, end, player->s.number, 0x0280E803, priority, forward);
        if (!hit) return;
    }
    const float distanceSq = Vec3DistanceSq(start, hit->r.currentOrigin);
    const float enemyRangeSq = weapon->enemyCrosshairRange * weapon->enemyCrosshairRange;
    const float nameRangeSq = g_friendlyNameDist->current.value * g_friendlyNameDist->current.value;
    const float friendlyRangeSq = g_friendlyfireDist->current.value * g_friendlyfireDist->current.value;
    // Source actors use CONTENTS_CORPSE (0x8000) even while alive. Preserve
    // their collision contract while mapping that role to retail's actor path.
    const bool sourceActor = hit->actor && hit->sentient && (hit->r.contents & 0x8000);
    if (!(hit->r.contents & 0x02004000) && !sourceActor)
    {
        // Retail ET_VEHICLE=13; this reconstruction's ET_VEHICLE is 14.
        if (hit->s.eType == ET_VEHICLE && !look.isDefined())
        {
            if (hit->health < 0) return;
            if (distanceSq < nameRangeSq) look.setEnt(hit);
            if (!(ps->eFlags & 0x4000) || distanceSq >= enemyRangeSq) return;
            if (hit->scr_vehicle && hit->scr_vehicle->team == Sentient_EnemyTeam(player->sentient->eTeam))
                ps->weapFlags |= 0x10;
            else ps->weapFlags |= 8;
        }
        else if (entities[hit->s.number].text[0] && !look.isDefined())
        {
            if (distanceSq < nameRangeSq) look.setEnt(hit);
            if (hit->s.eType == ET_SCRIPTMOVER && distanceSq < friendlyRangeSq) ps->weapFlags |= 8;
        }
        return;
    }
    if ((trace.cflags & 0x10) || (hit->s.lerp.eFlags & 0x20)
        || zombietron->current.enabled || (hit->actor && !hit->actor->bActivateCrosshair)
        || !hit->sentient) return;

    bool friendly;
    if (hit->client)
        friendly = player->client->sess.cs.team && player->client->sess.cs.team == hit->client->sess.cs.team;
    else
    {
        friendly = hit->sentient->eTeam != Sentient_EnemyTeam(player->sentient->eTeam);
        if (friendly)
        {
            ps->eFlags2 &= ~0x20000000;
            if (hit->sentient->bIgnoreForFriendlyFire) ps->eFlags2 |= 0x20000000;
        }
    }
    if (zombiemode->current.enabled && hit->client)
    {
        const char *name = BG_WeaponName(ps->weapon);
        const int mode = hit->client->ps.pm_type;
        const bool reviveKnife = (mode == 6 || mode == 7)
            && (!I_strcmp(name, "knife_ballistic_upgraded_zm")
                || !I_strcmp(name, "knife_ballistic_bowie_upgraded_zm")
                || !I_strcmp(name, "knife_ballistic_sickle_upgraded_zm"));
        if (!I_strncmp(name, "humangun_", 9) || reviveKnife)
        {
            if (distanceSq < enemyRangeSq)
            {
                if (!look.isDefined()) look.setEnt(hit);
                ps->weapFlags |= 0x200000;
            }
            return;
        }
    }
    if (friendly)
    {
        if (distanceSq < nameRangeSq && !look.isDefined()) look.setEnt(hit);
        if (distanceSq < friendlyRangeSq)
        {
            ps->weapFlags |= 8;
            if (hit->actor && (hit->actor->bDontAvoidPlayer || !(hit->actor->Physics.iTraceMask & 0x02000000)))
                ps->weapFlags |= 0x200000;
        }
    }
    else if (distanceSq < enemyRangeSq)
    {
        if (!look.isDefined()) look.setEnt(hit);
        ps->weapFlags |= 0x10;
    }
}

void G_SPPublishLookAt(gentity_s *player)
{
    // Retail 0x008185E0; the SP configstring pair is relocated to avoid the
    // reconstruction's COD-info range and to support all allocated client slots.
    if (!ValidClient(player->s.number) || !player->client) return;
    const int clientNum = player->client->ps.clientNum;
    if (!ValidClient(clientNum)) return;
    EntHandle &look = players[player->s.number].target;
    if (!look.isDefined()) { SetText(clientNum, 0, 0); return; }
    gentity_s *hit = look.ent();
    if (hit->actor && entities[hit->s.number].actorName)
    {
        if (!player->sentient || !hit->sentient
            || player->sentient->eTeam != hit->sentient->eTeam) return;
        SetText(clientNum, 0, entities[hit->s.number].actorName);
        const unsigned int weapon = G_GetWeaponIndexForName(
            SL_ConvertToString(hit->actor->weaponName, SCRIPTINSTANCE_SERVER));
        SV_SetConfigstring(CS_SP_LOOKAT_TEXT + clientNum * 2 + 1,
            const_cast<char *>(BG_GetWeaponDef(weapon)->szOverlayName));
        return;
    }
    const unsigned short *text = entities[hit->s.number].text;
    if (hit->s.eType == ET_VEHICLE && hit->scr_vehicle) text = &hit->scr_vehicle->lookAtText0;
    SetText(clientNum, 0, text[0]);
    if (text[0]) SetText(clientNum, 1, text[1]);
}
#endif
