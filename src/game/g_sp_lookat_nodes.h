#pragma once

#ifdef KISAK_SP
struct gentity_s;

void G_SPRegisterLookAtNodes();
void G_SPResetLookAtNodes();
void G_SPResetClientLookAtNodes(int clientNum);
void G_SPUpdateLookAtNodes(gentity_s *player, const float *start,
    const float *forward, float traceFraction);
void G_SPUpdatePlayerNodeClaim(gentity_s *player, gentity_s *previousTarget);
#endif
