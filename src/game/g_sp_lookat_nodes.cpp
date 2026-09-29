#include "g_sp_lookat_nodes.h"
#ifdef KISAK_SP
#include "actor_events.h"
#include "sentient.h"
#include <game_mp/actor_mp.h>
#include <game_mp/g_main_mp.h>
#include <qcommon/common.h>
#include <universal/com_math.h>
#include <cmath>
#include <cstring>
#include <climits>

namespace
{
const dvar_t *losRange;
const dvar_t *losHalfWidth;
const dvar_t *losMinTime;
struct LookAtNodeState
{
    float origin[2];
    float direction[2];
    int time;
    bool moved;
};
LookAtNodeState states[32];

float DistanceSquaredXY(const float *a, const float *b)
{
    const float x = a[0] - b[0], y = a[1] - b[1];
    return x * x + y * y;
}

// Retail 0x008189C0. The unsquared width comparison and unnormalized
// direction smoothing are intentional instruction-level behavior.
bool StableLookDirection(gentity_s *player, const float *forward)
{
    LookAtNodeState &state = states[player->s.number];
    float length = sqrtf(forward[0] * forward[0] + forward[1] * forward[1]);
    if (length <= 0.0f) length = 1.0f;
    const float direction[2] = { forward[0] / length, forward[1] / length };
    if (DistanceSquaredXY(player->r.currentOrigin, state.origin) > losHalfWidth->current.value
        || direction[0] * state.direction[0] + direction[1] * state.direction[1] < 0.9848f)
    {
        for (int i = 0; i < 2; ++i)
        {
            state.origin[i] = player->r.currentOrigin[i];
            state.direction[i] = direction[i];
        }
        state.time = level.time;
        return false;
    }
    for (int i = 0; i < 2; ++i)
    {
        state.origin[i] += (player->r.currentOrigin[i] - state.origin[i]) * 0.25f;
        state.direction[i] += (direction[i] - state.direction[i]) * 0.25f;
    }
    if (static_cast<long long>(state.time) + losMinTime->current.integer > level.time)
        return false;
    state.time = level.time;
    return true;
}

// Retail 0x00818C00; source-compatible path nodes and sentient handles.
void PublishNodes(gentity_s *player, const float *start, const float *forward, float length)
{
    float end[3], middle[3];
    Vec3Mad(start, length, forward, end);
    for (int i = 0; i < 3; ++i) middle[i] = (start[i] + end[i]) * 0.5f;
    const float horizontalLengthSq = DistanceSquaredXY(start, end);
    pathsort_t nodes[4];
    const int count = Path_NodesInCylinder(middle, sqrtf(horizontalLengthSq) * 0.5f,
        80.0f, nodes, 4, 0x83FFC);
    const float widthSq = losHalfWidth->current.value * losHalfWidth->current.value;
    for (int i = 0; i < count; ++i)
    {
        pathnode_t *node = nodes[i].node;
        // Retail's degenerate XY line produces an unordered distance and does
        // not reject a node. Avoid the source math helper's zero-length assert.
        if (horizontalLengthSq > 0.0f
            && PointToLineDistSq2D(node->constant.vOrigin, start, end) > widthSq)
            continue;
        if (node->dynamic.pOwner.isDefined())
        {
            sentient_s *owner = node->dynamic.pOwner.sentient();
            if (owner->pClaimedNode == node)
            {
                if (Actor_PointNearNode(owner->ent->r.currentOrigin, node)) continue;
                actor_s *actor = owner->ent->actor;
                if (!actor || owner->eTeam != player->sentient->eTeam
                    || Actor_KeepClaimedNode(actor)) continue;
                // Retail actor+0x1AA4. The reconstructed cover search must
                // maintain this existing member for the >1 branch to execute.
                if (actor->numCoverNodesInGoal > 1)
                {
                    Path_RelinquishNodeSoon(owner);
                    node->dynamic.pOwner.setSentient(nullptr);
                }
            }
        }
        node->dynamic.inPlayerLOSTime = level.time + losMinTime->current.integer;
    }
}
}

void G_SPRegisterLookAtNodes()
{
    losRange = _Dvar_RegisterFloat("ai_playerLOSRange", 150.0f, 0.0f, 500.0f, 0x2080, "");
    losHalfWidth = _Dvar_RegisterFloat("ai_playerLOSHalfWidth", 15.0f, 0.0f, 100.0f, 0x2080, "");
    losMinTime = _Dvar_RegisterInt("ai_playerLOSMinTime", 1500, 0, INT_MAX, 0x2080, "");
}

void G_SPResetLookAtNodes() { memset(states, 0, sizeof(states)); }
void G_SPResetClientLookAtNodes(int clientNum)
{
    if (unsigned(clientNum) < 32) memset(&states[clientNum], 0, sizeof(states[clientNum]));
}

void G_SPUpdateLookAtNodes(gentity_s *player, const float *start,
    const float *forward, float traceFraction)
{
    // Retail LOS publication does not depend on the look-at entity. Call after
    // the first trace and before either the null-hit return or trigger retrace.
    if (!player || !player->client || !player->sentient || unsigned(player->s.number) >= 32)
        return;
    if (StableLookDirection(player, forward))
    {
        const float distance = traceFraction * 15000.0f;
        PublishNodes(player, start, forward,
            distance < losRange->current.value ? distance : losRange->current.value);
    }
    if (player->client->ps.weapFlags & 2)
    {
        float end[3];
        Vec3Mad(start, losRange->current.value, forward, end);
        Actor_BroadcastLineEvent(player, nullptr, AI_EV_BLOCK_FRIENDLIES,
            1 << (unsigned(player->sentient->eTeam) & 31), start, end, 0.0f);
    }
}

void G_SPUpdatePlayerNodeClaim(gentity_s *player, gentity_s *previousTarget)
{
    // Retail 0x004634A0 runs in G_RunFrame's early client loop, so the target
    // belongs to the previous frame, before ClientEndFrame replaces the handle.
    if (!player || !player->r.inuse || !player->client || !player->sentient
        || unsigned(player->s.number) >= 32) return;
    sentient_s *self = player->sentient;
    const bool moved = DistanceSquaredXY(self->oldOrigin, player->r.currentOrigin) >= 0.01f;
    states[player->s.number].moved = moved;
    Vec3Copy(player->r.currentOrigin, self->oldOrigin);
    if (Sentient_NearestNodeDirty(self, moved)) Sentient_InvalidateNearestNode(self);

    // Retail 0x007D4770.
    pathnode_t *nearest = Sentient_NearestNode(self);
    float origin[3];
    Sentient_GetOrigin(self, origin);
    if (nearest && DistanceSquaredXY(origin, nearest->constant.vOrigin) > 1024.0f) nearest = nullptr;
    if (nearest != self->pClaimedNode)
    {
        if (self->pClaimedNode)
        {
            if (!previousTarget || !previousTarget->actor || !previousTarget->sentient
                || previousTarget->sentient->pClaimedNode)
                Path_RelinquishNodeNow(self);
            else
                Path_RelinquishNodeSoon(self);
        }
        if (nearest)
        {
            if (!nearest->dynamic.pOwner.isDefined()) Path_ForceClaimNode(nearest, self);
            else
            {
                sentient_s *owner = nearest->dynamic.pOwner.sentient();
                if (owner != self)
                {
                    Sentient_GetOrigin(owner, origin);
                    if (DistanceSquaredXY(origin, nearest->constant.vOrigin) >= 225.0f)
                        Path_ForceClaimNode(nearest, self);
                }
            }
        }
    }
    Sentient_BanNearNodes(self);
}
#endif
