#pragma once

#ifdef KISAK_SP
struct gentity_s;
struct gclient_s;
struct scr_entref_t;

// Reconstruction names for the retail SP look-at subsystem; not recovered symbols.
void G_SPInitLookAt();
void G_SPFreeActorName(gentity_s *ent);
void G_SPSetActorName(gentity_s *ent);
void G_SPGetActorName(gentity_s *ent);
void G_SPShutdownLookAt();
void G_SPClearClientLookAt(gclient_s *client);
void G_SPFreeEntityLookAt(gentity_s *ent);
void G_SPUpdateLookAtClaim(gentity_s *player);
void G_SPUpdateLookAt(gentity_s *player);
void G_SPPublishLookAt(gentity_s *player);
void G_SPSetLookAtText(scr_entref_t entref);
void G_SPIsLookingAt(scr_entref_t entref);
#endif
