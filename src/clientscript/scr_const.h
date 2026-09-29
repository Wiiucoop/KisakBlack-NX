#pragma once

struct scr_const_t // sizeof=0x40C
{                                                                             // XREF: .data:scr_const_t scr_const/r
        unsigned __int16 _;                                 // XREF: ScrCmd_attach:loc_645162/r
                                                                                // ScrCmd_detach:loc_6453F2/r ...
        unsigned __int16 active;                        // XREF: ObjectiveStateIndexFromString(objectiveState_t *,uint):loc_64B008/r
                                                                                // .text:0069CFA3/w
        unsigned __int16 spawn;                         // XREF: .text:0069CFB6/w
                                                                                // Path_CheckSpawnExitNodesConnectivity(void):loc_924A57/r
        unsigned __int16 exit;                            // XREF: .text:0069CFC9/w
                                                                                // Path_CheckSpawnExitNodesConnectivity(void):loc_924AA6/r
        unsigned __int16 aim_bone;                    // XREF: AimAssist_CalcAimPos+71/r
                                                                                // .text:0069CFDC/w
        unsigned __int16 aim_highest_bone;    // XREF: AimTarget_GetTargetBounds+A1/r
                                                                                // AimTarget_GetTargetBounds+2BE/r ...
        unsigned __int16 aim_vis_bone;            // XREF: AimTarget_ProcessEntityInternal(int,centity_s const *):loc_406D33/r
                                                                                // CachedTag_GetTagPos(centity_s const *,uint,float * const,int,bool)+B/r ...
        unsigned __int16 all;                             // XREF: .text:0069D015/w
                                                                                // ActorCmd_SetTalkToSpecies+4C/r
        unsigned __int16 allies;                        // XREF: ClientScr_SetSessionTeam+62/r
                                                                                // ClientScr_SetFFATeam+5F/r ...
        unsigned __int16 angles;                        // XREF: PlayerCmd_CanPlayerPlaceTurret+13D/r
                                                                                // .text:0069D03B/w
        unsigned __int16 axis;                            // XREF: ClientScr_SetSessionTeam+41/r
                                                                                // ClientScr_SetFFATeam+41/r ...
        unsigned __int16 bad_path;                    // XREF: .text:0069D061/w
                                                                                // Bot_UpdateScriptGoal+173/r ...
        unsigned __int16 begin_firing;            // XREF: G_ClientDoPerFrameNotifies+160/r
                                                                                // .text:0069D074/w
        unsigned __int16 unknown_location;    // XREF: .text:0069D087/w
        unsigned __int16 cancel_location;     // XREF: ClientThink_real(gentity_s *,usercmd_s *)+653/r
                                                                                // .text:0069D09A/w
        unsigned __int16 confirm_location;    // XREF: ClientThink_real(gentity_s *,usercmd_s *)+623/r
                                                                                // .text:0069D0AD/w
        unsigned __int16 deployed_turret;     // XREF: DeployWeapon(gentity_s *,int,float)+13FD/r
                                                                                // DeployWeapon(gentity_s *,int,float)+143C/r ...
        unsigned __int16 crouch;                        // XREF: ScrCmd_GetStance+65/r
                                                                                // .text:0069D0C0/w ...
        unsigned __int16 current;                     // XREF: ObjectiveStateIndexFromString(objectiveState_t *,uint):loc_64AFF1/r
                                                                                // .text:0069D0D3/w
        unsigned __int16 damage;                        // XREF: PlayerCmd_finishPlayerDamage+99E/r
                                                                                // G_Damage(gentity_s *,gentity_s *,gentity_s *,float const * const,float const * const,int,int,int,int,hitLocation_t,uint,uint,int)+357/r ...
        unsigned __int16 damage_afterdeath; // XREF: .text:0069D0F9/w
        unsigned __int16 dead;                            // XREF: ClientScr_SetSessionState+97/r
                                                                                // ClientScr_GetSessionState+94/r ...
        unsigned __int16 death;                         // XREF: player_die(gentity_s *,gentity_s *,gentity_s *,int,int,int,float const * const,hitLocation_t,int):loc_613970/r
                                                                                // player_die(gentity_s *,gentity_s *,gentity_s *,int,int,int,float const * const,hitLocation_t,int)+3A8/r ...
        unsigned __int16 disconnect;                // XREF: .text:0069D132/w
                                                                                // VM_Notify:loc_8D2C29/r
        unsigned __int16 death_or_disconnect; // XREF: .text:0069D145/w
                                                                                // VM_Notify+1F/r ...
        unsigned __int16 detonate;                    // XREF: HandleClientEvent(gclient_s *,gentity_s *,int,int)+466/r
                                                                                // .text:0069D158/w
        unsigned __int16 direct;                        // XREF: Scr_Target_SetAttackMode(void):loc_68E5FB/r
                                                                                // .text:0069D16B/w
        unsigned __int16 dlight;                        // XREF: .text:0069D17E/w
        unsigned __int16 done;                            // XREF: G_InitGame(int,int,int,int,int)+18B/r
                                                                                // .text:0069D191/w
        unsigned __int16 dropweapon;                // XREF: .text:0069D1A4/w
        unsigned __int16 empty;                         // XREF: ObjectiveStateIndexFromString(objectiveState_t *,uint)+3/r
                                                                                // .text:0069D1B7/w
        unsigned __int16 end_firing;                // XREF: G_ClientDoPerFrameNotifies:loc_62BFF8/r
                                                                                // .text:0069D1CA/w
        unsigned __int16 enter_vehicle;         // XREF: VEH_LinkPlayer(gentity_s *,gentity_s *,int,bool)+5E8/r
                                                                                // VEH_LinkPlayer(gentity_s *,gentity_s *,int,bool)+61A/r ...
        unsigned __int16 entity;                        // XREF: Scr_BulletTrace+22C/r
                                                                                // Scr_PhysicsTrace+2F5/r ...
        unsigned __int16 exit_vehicle;            // XREF: VEH_UnlinkPlayer(gentity_s *,bool,char *)+66F/r
                                                                                // VEH_UnlinkPlayer(gentity_s *,bool,char *)+6A1/r ...
        unsigned __int16 change_seat;             // XREF: VEH_SwitchClientToSeat+E1/r
                                                                                // VEH_SwitchClientToSeat+121/r ...
        unsigned __int16 vehicle_death;         // XREF: PlayerCmd_finishPlayerDamage+DCC/r
                                                                                // Scr_Vehicle_Die(gentity_s *,gentity_s *,gentity_s *,int,int,int,float const * const,hitLocation_t,int)+42/r ...
        unsigned __int16 explode;                     // XREF: G_ExplodeMissile(gentity_s *)+41A/r
                                                                                // .text:0069D23C/w ...
        unsigned __int16 failed;                        // XREF: .text:0069D24F/w
        unsigned __int16 free;                            // XREF: AddScrTeamName+1C/r
                                                                                // GScr_ReadTeamForSpawnPoints(int)+4C/r ...
        unsigned __int16 fraction;                    // XREF: Scr_BulletTrace+191/r
                                                                                // Scr_PhysicsTrace+275/r ...
        unsigned __int16 goal;                            // XREF: VEH_UpdateMove_CheckGoalReached+1AE/r
                                                                                // VEH_UpdateMove_CheckGoalReached+20B/r ...
        unsigned __int16 goal_changed;            // XREF: .text:0069D29B/w
                                                                                // ActorCmd_SetOverrideRunToPos+95/r
        unsigned __int16 goal_yaw;                    // XREF: VEH_UpdateYawAndNotify+28B/r
                                                                                // .text:0069D2AE/w
        unsigned __int16 grenade;                     // XREF: G_FireGrenade(gentity_s *,float * const,float * const,int,uchar,int,int)+77/r
                                                                                // .text:0069D2C1/w
        unsigned __int16 grenadedanger;         // XREF: CheckGrenadeDanger+1C4/r
                                                                                // .text:0069D2D4/w ...
        unsigned __int16 grenade_flee;            // XREF: .text:0069D2E7/w
        unsigned __int16 grenade_fire;            // XREF: G_FireGrenade(gentity_s *,float * const,float * const,int,uchar,int,int)+2A9/r
                                                                                // .text:0069D2FA/w
        unsigned __int16 grenade_launcher_fire;
                                                                                // XREF: G_FireGrenade(gentity_s *,float * const,float * const,int,uchar,int,int)+2C1/r
                                                                                // .text:0069D30D/w
        unsigned __int16 grenade_pullback;    // XREF: NotifyGrenadePullback(gentity_s *,uint)+A6/r
                                                                                // .text:0069D320/w
        unsigned __int16 grenade_throwback; // XREF: G_InitGrenadeEntity(gentity_s *,gentity_s *)+25E/r
                                                                                // .text:0069D333/w
        unsigned __int16 stationary;                // XREF: MissileImpact+5ED/r
                                                                                // BounceMissile+F46/r ...
        unsigned __int16 grenade_stuck;         // XREF: MissileImpact+5BB/r
                                                                                // AttachBoltGrenade+1EA/r ...
        unsigned __int16 grenade_bounce;        // XREF: MissileImpact+621/r
                                                                                // MissileImpact+655/r ...
        unsigned __int16 hero;                            // XREF: .text:0069D37F/w
        unsigned __int16 info_notnull;            // XREF: .text:0069D392/w
        unsigned __int16 invisible;                 // XREF: ObjectiveStateIndexFromString(objectiveState_t *,uint):loc_64AFDA/r
                                                                                // .text:0069D3A5/w
        unsigned __int16 key1;                            // XREF: .text:0069D3B8/w
        unsigned __int16 key2;                            // XREF: .text:0069D3CB/w
        unsigned __int16 killanimscript;        // XREF: .text:0069D3DE/w
                                                                                // Actor_KillAnimScript(actor_s *)+59/r
        unsigned __int16 left;                            // XREF: .text:0069D3F1/w
        unsigned __int16 left_tread;                // XREF: CG_Vehicle+2B6/r
                                                                                // VEH_UpdateHealth(gentity_s *)+A4/r ...
        unsigned __int16 light;                         // XREF: ScriptEntCmd_MoveTo(scr_entref_t)+A0/r
                                                                                // ScriptEntCmd_GravityMove(scr_entref_t)+A0/r ...
        unsigned __int16 missile_fire;            // XREF: G_FireRocket(gentity_s *,uint,float * const,float * const,float const * const,gentity_s *,float const * const)+C1E/r
                                                                                // .text:0069D42A/w
        unsigned __int16 movedone;                    // XREF: Reached_ScriptMover(gentity_s *)+C1/r
                                                                                // .text:0069D43D/w
        unsigned __int16 multikill;                 // XREF: .text:0069D450/w
        unsigned __int16 noclass;                     // XREF: G_InitGentity(gentity_s *)+1C/r
                                                                                // .text:0069D463/w
        unsigned __int16 none;                            // XREF: ClientScr_SetSessionTeam+9E/r
                                                                                // ClientScr_SetFFATeam+7D/r ...
        unsigned __int16 normal;                        // XREF: Scr_BulletTrace+25A/r
                                                                                // Scr_BulletTrace+304/r ...
        unsigned __int16 origin;                        // XREF: PlayerCmd_CanPlayerPlaceTurret+11D/r
                                                                                // .text:0069D49C/w
        unsigned __int16 player;                        // XREF: ClientSpawn(gentity_s *,float const * const,float const * const)+2AB/r
                                                                                // G_GetFreePlayerCorpseIndex(void)+1A/r ...
        unsigned __int16 point_relative;        // XREF: .text:0069D4C2/w
                                                                                // ActorCmd_AnimMode:loc_9175A8/r
        unsigned __int16 position;                    // XREF: Scr_BulletTrace+1CC/r
                                                                                // Scr_PhysicsTrace+295/r ...
        unsigned __int16 projectile_impact; // XREF: MissileImpact+16B3/r
                                                                                // .text:0069D4E8/w ...
        unsigned __int16 prone;                         // XREF: ScrCmd_GetStance+40/r
                                                                                // .text:0069D4FB/w ...
        unsigned __int16 result;                        // XREF: PlayerCmd_CanPlayerPlaceTurret+FD/r
                                                                                // .text:0069D50E/w
        unsigned __int16 right;                         // XREF: .text:0069D521/w
        unsigned __int16 right_tread;             // XREF: VEH_UpdateHealth(gentity_s *)+C8/r
                                                                                // CMD_VEH_GetTreadHealth(scr_entref_t):loc_682CD1/r ...
        unsigned __int16 tank_armor;                // XREF: .text:0069D547/w
        unsigned __int16 reload;                        // XREF: HandleClientEvent(gclient_s *,gentity_s *,int,int)+419/r
                                                                                // .text:0069D55A/w
        unsigned __int16 reload_start;            // XREF: HandleClientEvent(gclient_s *,gentity_s *,int,int)+3CC/r
                                                                                // .text:0069D56D/w
        unsigned __int16 rocket;                        // XREF: G_FireRocket(gentity_s *,uint,float * const,float * const,float const * const,gentity_s *,float const * const)+197/r
                                                                                // G_DropBomb(gentity_s *,uint,float * const,float * const,float const * const,gentity_s *,float const * const)+12E/r ...
        unsigned __int16 rotatedone;                // XREF: Reached_ScriptMover(gentity_s *)+1E8/r
                                                                                // .text:0069D593/w
        unsigned __int16 script_brushmodel; // XREF: G_CreatePhysicsObject+F5/r
                                                                                // GScr_DisconnectPaths+3B/r ...
        unsigned __int16 script_model;            // XREF: DrawEntityCounts+702/r
                                                                                // CanDamage(gentity_s *,gentity_s *,float const * const,float,float * const,int)+704/r ...
        unsigned __int16 script_origin;         // XREF: ScriptEntCmd_MoveTo(scr_entref_t)+8B/r
                                                                                // ScriptEntCmd_GravityMove(scr_entref_t)+8B/r ...
        unsigned __int16 snd_enveffectsprio_level; // XREF: .text:0069D5DF/w
        unsigned __int16 snd_enveffectsprio_shellshock;
                                                                                // XREF: .text:0069D5F2/w
        unsigned __int16 snd_busvolprio_holdbreath; // XREF: .text:0069D605/w
        unsigned __int16 snd_busvolprio_pain; // XREF: .text:0069D618/w
        unsigned __int16 snd_busvolprio_shellshock; // XREF: .text:0069D62B/w
        unsigned __int16 stand;                         // XREF: ScrCmd_GetStance+79/r
                                                                                // .text:0069D63E/w ...
        unsigned __int16 suppression;             // XREF: .text:0069D651/w
        unsigned __int16 suppression_end;     // XREF: .text:0069D664/w
        unsigned __int16 surfacetype;             // XREF: Scr_BulletTrace+295/r
                                                                                // Scr_BulletTrace+328/r ...
        unsigned __int16 tag_aim_pitch;         // XREF: CG_mg42_PreControllers(DObj *,centity_s *)+2B5/r
                                                                                // .text:0069D69D/w ...
        unsigned __int16 tag_aim;                     // XREF: CG_mg42_PreControllers(DObj *,centity_s *)+296/r
                                                                                // Player_GetUseList+F4C/r ...
        unsigned __int16 tag_aim_animated;    // XREF: CG_mg42_PreControllers(DObj *,centity_s *)+2D4/r
                                                                                // .text:0069D6B0/w ...
        unsigned __int16 tag_brass;                 // XREF: Weapon_DrawTagAxis(int,int,int,int)+50/r
                                                                                // CG_EjectWeaponBrass(int,entityState_s const *,int,int):loc_5325A4/r ...
        unsigned __int16 tag_brass1;                // XREF: Weapon_DrawTagAxis(int,int,int,int)+12B/r
                                                                                // CG_EjectWeaponBrass(int,entityState_s const *,int,int):loc_53261E/r ...
        unsigned __int16 tag_brass_2;             // XREF: CG_EjectWeaponBrass(int,entityState_s const *,int,int)+188/r
                                                                                // .text:0069D6E9/w
        unsigned __int16 tag_butt;                    // XREF: .text:0069D6FC/w
                                                                                // turret_RestoreDefaultDropPitch(gentity_s *):loc_6A243F/r
        unsigned __int16 tag_clip;                    // XREF: ChangeViewmodelDobj+303/r
                                                                                // .text:0069D70F/w
        unsigned __int16 tag_flash;                 // XREF: CG_mg42_OverheatUpdate(int,DObj const *,centity_s *)+C9/r
                                                                                // CG_mg42_PreControllers(DObj *,centity_s *)+2F3/r ...
        unsigned __int16 tag_explosive;         // XREF: .text:0069DA79/w
        unsigned __int16 tag_flash_11;            // XREF: CG_EntityEvent(int,centity_s *,int)+10EA/r
                                                                                // .text:0069D735/w ...
        unsigned __int16 tag_flash_2;             // XREF: CG_EntityEvent(int,centity_s *,int)+1119/r
                                                                                // CG_EntityEvent(int,centity_s *,int)+11D7/r ...
        unsigned __int16 tag_flash_22;            // XREF: CG_EntityEvent(int,centity_s *,int)+1143/r
                                                                                // .text:0069D75B/w ...
        unsigned __int16 tag_flash_3;             // XREF: .text:0069D76E/w
                                                                                // .data:00E0A2CC/o
        unsigned __int16 tag_flash1;                // XREF: CG_EntityEvent(int,centity_s *,int)+16FD/r
                                                                                // CG_AddPlayerWeapon(int,GfxScaledPlacement const *,playerState_s const *,centity_s *,int):loc_52CEF4/r ...
        unsigned __int16 tag_flash_alt;         // XREF: CG_AddPlayerWeapon(int,GfxScaledPlacement const *,playerState_s const *,centity_s *,int)+62D/r
                                                                                // .text:0069D794/w ...
        unsigned __int16 tag_fx;                        // XREF: CG_Missile+285/r
                                                                                // CG_Missile+38A/r ...
        unsigned __int16 tag_inhand;                // XREF: CG_GetWeaponAttachBone(clientInfo_t *,weapType_t,weapInventoryType_t)+1A/r
                                                                                // CG_AttachWeapon+3BD/r ...
        unsigned __int16 tag_knife_attach;    // XREF: ChangeViewmodelDobj+1F5/r
                                                                                // .text:0069D9F4/w
        unsigned __int16 tag_knife_fx;            // XREF: CG_MeleeBloodEvent(int,centity_s const *,int)+125/r
                                                                                // CG_MeleeBloodEvent(int,centity_s const *,int):loc_538206/r ...
        unsigned __int16 tag_bayonet;             // XREF: CG_MeleeBloodEvent(int,centity_s const *,int)+186/r
                                                                                // CG_MeleeBloodEvent(int,centity_s const *,int):loc_538267/r ...
        unsigned __int16 tag_laser;                 // XREF: CG_Laser_Add(centity_s *,DObj *,cpose_t *,float const * const,LaserOwnerEnum)+10/r
                                                                                // .text:0069DA2D/w
        unsigned __int16 tag_origin;                // XREF: CG_Missile+2F4/r
                                                                                // CG_Missile+3E7/r ...
        unsigned __int16 tag_weapon;                // XREF: CG_mg42_PreControllers(DObj *,centity_s *)+312/r
                                                                                // CG_PlayerTurretPositionAndBlend:loc_4D99ED/r ...
        unsigned __int16 tag_weapon1;             // XREF: ChangeViewmodelDobj+278/r
                                                                                // .text:0069DA66/w
        unsigned __int16 tag_player;                // XREF: CG_CalcVehicleViewValues(int)+128/r
                                                                                // CG_CalcTurretViewValues+E8/r ...
        unsigned __int16 tag_camera;                // XREF: CG_ApplyViewAnimation(int)+1C4/r
                                                                                // CG_CreateWeaponViewModelXAnim:loc_52925E/r ...
        unsigned __int16 tag_weapon_right;    // XREF: Anim_DrawTags(int,int,int,cpose_t *,int)+CA/r
                                                                                // PlayerCmd_dropItem:loc_607808/r ...
        unsigned __int16 tag_weapon_left;     // XREF: CG_AttachWeapon+239/r
                                                                                // CG_AttachWeapon+2FB/r ...
        unsigned __int16 tag_stowed_back;     // XREF: Anim_DrawTags(int,int,int,cpose_t *,int)+1D2/r
                                                                                // .text:0069DAEB/w
        unsigned __int16 tag_weapon_chest;    // XREF: Anim_DrawTags(int,int,int,cpose_t *,int)+256/r
                                                                                // .text:0069DAD8/w
        unsigned __int16 tag_gasmask;             // XREF: ChangeViewmodelDobj+2CD/r
                                                                                // .text:0069DAFE/w
        unsigned __int16 tag_gasmask2;            // XREF: ChangeViewmodelDobj+2BD/r
                                                                                // .text:0069DB11/w
        unsigned __int16 tag_sync;                    // XREF: .text:0069DB24/w
        unsigned __int16 tag_wake;                    // XREF: .text:0069DB37/w
        unsigned __int16 target_script_trigger; // XREF: .text:0069DB4A/w
        unsigned __int16 tempEntity;                // XREF: G_TempEntity(float const * const,int)+29/r
                                                                                // .text:0069DB5D/w
        unsigned __int16 top;                             // XREF: Scr_Target_SetAttackMode(void)+89/r
                                                                                // .text:0069DB70/w
        unsigned __int16 touch;                         // XREF: ClientImpacts(gentity_s *,pmove_t *)+B4/r
                                                                                // ClientImpacts(gentity_s *,pmove_t *)+D8/r ...
        unsigned __int16 trigger;                     // XREF: WeaponPickup_Notifies+B7/r
                                                                                // G_RunFrame(int)+50F/r ...
        unsigned __int16 trigger_use;             // XREF: GScr_SetCursorHint+71/r
                                                                                // GScr_SetCursorHint+141/r ...
        unsigned __int16 trigger_use_touch; // XREF: GScr_SetCursorHint+86/r
                                                                                // GScr_SetCursorHint+156/r ...
        unsigned __int16 trigger_damage;        // XREF: G_Damage(gentity_s *,gentity_s *,gentity_s *,float const * const,float const * const,int,int,int,int,hitLocation_t,uint,uint,int)+306/r
                                                                                // GScr_EnableGrenadeTouchDamage(scr_entref_t)+2B/r ...
        unsigned __int16 trigger_lookat;        // XREF: .text:0069DBE2/w
        unsigned __int16 trigger_radius;        // XREF: GScr_SetCursorHint+9B/r
                                                                                // GScr_SetCursorHint+16B/r ...
        unsigned __int16 trigger_radius_use; // XREF: GScr_SetCursorHint+B0/r
                                                                                // GScr_SetCursorHint+180/r ...
        unsigned __int16 truck_cam;                 // XREF: .text:0069DC1B/w
        unsigned __int16 turret_return_default_pos; // XREF: .text:0069DC41/w
        unsigned __int16 turret_scan_start; // XREF: .text:0069DC2E/w
                                                                                // Turret_ScanStart+1ED/r
        unsigned __int16 turret_scan_stop;    // XREF: .text:0069DC54/w
                                                                                // Turret_ScanStop+7E/r
        unsigned __int16 turret_tow_fire;     // XREF: .text:0069DC67/w
                                                                                // .text:0069DC7A/w
        unsigned __int16 turret_tow_unlink;
        unsigned __int16 weapon_change_on_turret; // XREF: .text:0069DC8D/w
        unsigned __int16 weapon_change;         // XREF: G_ClientDoPerFrameNotifies+D4/r
                                                                                // .text:0069DCA0/w
        unsigned __int16 weapon_change_complete; // XREF: .text:0069DCB3/w
        unsigned __int16 weapon_fired;            // XREF: CMD_VEH_FireWeapon(scr_entref_t)+C20/r
                                                                                // FireWeapon(gentity_s *,int,int)+50/r ...
        unsigned __int16 worldspawn;                // XREF: SP_worldspawn(SpawnVar *)+22A/r
                                                                                // .text:0069DCD9/w
        unsigned __int16 flashbang;                 // XREF: FlashbangBlastEnt+1CA/r
                                                                                // .text:0069DCEC/w
        unsigned __int16 flash;                         // XREF: PlayerCmd_getOffhandSecondaryClass(scr_entref_t)+A6/r
                                                                                // .text:0069DCFF/w
        unsigned __int16 smoke;                         // XREF: PlayerCmd_getOffhandSecondaryClass(scr_entref_t)+F2/r
                                                                                // .text:0069DD12/w
        unsigned __int16 night_vision_on;     // XREF: G_ClientDoPerFrameNotifies+195/r
                                                                                // .text:0069DD25/w
        unsigned __int16 night_vision_off;    // XREF: G_ClientDoPerFrameNotifies+18D/r
                                                                                // .text:0069DD38/w
        unsigned __int16 back;                            // XREF: .text:0069DD4B/w
        unsigned __int16 back_low;                    // XREF: .text:0069DD5E/w
                                                                                // .data:ushort * * controller_names/o
        unsigned __int16 back_mid;                    // XREF: .text:0069DD71/w
                                                                                // .data:00DFD924/o
        unsigned __int16 back_up;                     // XREF: .text:0069DD84/w
                                                                                // .data:00DFD928/o
        unsigned __int16 head;                            // XREF: .text:0069DD97/w
                                                                                // .data:00DFD930/o
        unsigned __int16 j_mainroot;                // XREF: CG_OffsetThirdPersonView+1DF/r
                                                                                // .text:0069DDAA/w
        unsigned __int16 neck;                            // XREF: .text:0069DDBD/w
                                                                                // .data:00DFD92C/o
        unsigned __int16 pelvis;                        // XREF: .text:0069DDD0/w
                                                                                // .data:00DFD934/o
        unsigned __int16 j_head;                        // XREF: CG_DrawOverheadNames(int,centity_s const *,float,int)+17C/r
                                                                                // CG_CanSeeFriendlyHead(int,centity_s const *)+C0/r ...
        unsigned __int16 mod_unknown;             // XREF: .text:0069DDF6/w
                                                                                // .data:ushort * * modNames/o
        unsigned __int16 mod_pistol_bullet; // XREF: .text:0069DE09/w
                                                                                // .data:00E07A0C/o
        unsigned __int16 mod_rifle_bullet;    // XREF: .text:0069DE1C/w
                                                                                // .data:00E07A10/o
        unsigned __int16 mod_grenade;             // XREF: .text:0069DE2F/w
                                                                                // .data:00E07A14/o
        unsigned __int16 mod_grenade_splash; // XREF: .text:0069DE42/w
                                                                                // .data:00E07A18/o
        unsigned __int16 mod_projectile;        // XREF: .text:0069DE55/w
                                                                                // .data:00E07A1C/o
        unsigned __int16 mod_projectile_splash; // XREF: .text:0069DE68/w
                                                                                // .data:00E07A20/o
        unsigned __int16 mod_melee;                 // XREF: .text:0069DE7B/w
                                                                                // .data:00E07A24/o
        unsigned __int16 mod_bayonet;             // XREF: .text:0069DE8E/w
                                                                                // .data:00E07A28/o
        unsigned __int16 mod_head_shot;         // XREF: .text:0069DEA1/w
                                                                                // .data:00E07A2C/o
        unsigned __int16 mod_crush;                 // XREF: .text:0069DEB4/w
                                                                                // .data:00E07A30/o
        unsigned __int16 mod_telefrag;            // XREF: .text:0069DEC7/w
                                                                                // .data:00E07A34/o
        unsigned __int16 mod_falling;             // XREF: .text:0069DEDA/w
                                                                                // .data:00E07A38/o
        unsigned __int16 mod_suicide;             // XREF: .text:0069DEED/w
                                                                                // .data:00E07A3C/o
        unsigned __int16 mod_trigger_hurt;    // XREF: .text:0069DF00/w
                                                                                // .data:00E07A40/o
        unsigned __int16 mod_explosive;         // XREF: .text:0069DF13/w
                                                                                // .data:00E07A44/o
        unsigned __int16 mod_impact;                // XREF: .text:0069DF26/w
                                                                                // .data:00E07A48/o
        unsigned __int16 mod_burned;                // XREF: .text:0069DF39/w
                                                                                // .data:00E07A4C/o
        unsigned __int16 mod_hit_by_object; // XREF: .text:0069DF4C/w
                                                                                // .data:00E07A50/o
        unsigned __int16 mod_drown;                 // XREF: .text:0069DF5F/w
                                                                                // .data:00E07A54/o
        unsigned __int16 mod_gas;                     // XREF: .text:0069DF72/w
                                                                                // .data:00E07A58/o
        unsigned __int16 face;                            // XREF: .text:0069DF85/w
        unsigned __int16 face_casual;             // XREF: ScrCmd_SendFaceEvent+37/r
                                                                                // .text:0069DFAB/w
        unsigned __int16 face_alert;                // XREF: ScrCmd_SendFaceEvent+5E/r
                                                                                // .text:0069DF98/w
        unsigned __int16 face_cqb;                    // XREF: ScrCmd_SendFaceEvent+85/r
                                                                                // .text:0069DFBE/w
        unsigned __int16 face_running;            // XREF: ScrCmd_SendFaceEvent+AC/r
                                                                                // .text:0069DFD1/w
        unsigned __int16 face_shoot_single; // XREF: ScrCmd_SendFaceEvent+D3/r
                                                                                // .text:0069DFE4/w
        unsigned __int16 face_shoot_burst;    // XREF: ScrCmd_SendFaceEvent+FA/r
                                                                                // .text:0069DFF7/w
        unsigned __int16 face_melee;                // XREF: ScrCmd_SendFaceEvent+1DB/r
                                                                                // .text:0069E056/w
        unsigned __int16 face_react;                // XREF: ScrCmd_SendFaceEvent+121/r
                                                                                // .text:0069E00A/w
        unsigned __int16 face_talk;                 // XREF: ScrCmd_SendFaceEvent+148/r
                                                                                // .text:0069E01D/w
        unsigned __int16 face_talk_long;        // XREF: ScrCmd_SendFaceEvent+16F/r
                                                                                // .text:0069E030/w
        unsigned __int16 face_pain;                 // XREF: ScrCmd_SendFaceEvent+193/r
                                                                                // .text:0069E043/w
        unsigned __int16 face_death;                // XREF: ScrCmd_SendFaceEvent+1B7/r
                                                                                // .text:0069E069/w
        unsigned __int16 melee;
        unsigned __int16 script_vehicle;        // XREF: GScr_SpawnVehicle+5D/r
                                                                                // GScr_SpawnHelicopter+78/r ...
        unsigned __int16 script_vehicle_collision;
                                                                                // XREF: VEH_MoveTrace+1F2/r
                                                                                // .text:0069E08F/w ...
        unsigned __int16 script_vehicle_collmap; // XREF: VEH_GetCollMap+5A/r
                                                                                // .text:0069E0A2/w ...
        unsigned __int16 script_vehicle_corpse;
                                                                                // XREF: CMD_VEH_FreeVehicle(scr_entref_t)+7E/r
                                                                                // .text:0069E0B5/w ...
        unsigned __int16 turret_fire;             // XREF: VEH_UpdateDriverWeapons+26F/r
                                                                                // .text:0069E0C8/w
        unsigned __int16 turret_on_target;    // XREF: VEH_UpdateAim+853/r
                                                                                // .text:0069E0DB/w
        unsigned __int16 gunner_turret_on_target;
                                                                                // XREF: VEH_UpdateGunnerAim+C75/r
                                                                                // .text:0069E0EE/w
        unsigned __int16 turret_not_on_target; // XREF: VEH_UpdateAim+91A/r
                                                                                // .text:0069E101/w
        unsigned __int16 turret_on_vistarget; // XREF: VEH_UpdateAim+8D2/r
                                                                                // .text:0069E114/w
        unsigned __int16 turret_no_vis;         // XREF: VEH_UpdateAim+8EA/r
                                                                                // VEH_UpdateAim+902/r ...
        unsigned __int16 turret_rotate_stopped; // XREF: VEH_UpdateAim+7D0/r
                                                                                // VEH_UpdateGunnerAim+BDF/r ...
        unsigned __int16 turret_deactivate; // XREF: .text:0069E14D/w
                                                                                // G_DeactivateTurret(gentity_s *)+50/r
        unsigned __int16 turretstatechange; // XREF: .text:0069E160/w
                                                                                // turret_SetState(gentity_s *,int)+5D/r ...
        unsigned __int16 turretownerchange; // XREF: .text:0069E173/w
                                                                                // G_ClientStopUsingTurret(gentity_s *)+216/r ...
        unsigned __int16 reached_node;            // XREF: VEH_UpdatePath+370/r
                                                                                // VEH_UpdatePath+A39/r ...
        unsigned __int16 reached_end_node;    // XREF: VEH_UpdatePath+9E6/r
                                                                                // .text:0069E199/w
        unsigned __int16 reached_wait_node; // XREF: VEH_UpdatePath+9C1/r
                                                                                // .text:0069E1AC/w
        unsigned __int16 reached_wait_speed; // XREF: VEH_UpdatePath+C06/r
                                                                                // VEH_UpdatePath+C49/r ...
        unsigned __int16 near_goal;                 // XREF: VEH_UpdateMove_CheckNearGoal+67/r
                                                                                // .text:0069E1D2/w
        unsigned __int16 veh_collision;         // XREF: VEH_CheckForCrash(gentity_s *,float * const)+16E/r
                                                                                // VEH_UpdateClientChopper(gentity_s *)+F56/r ...
        unsigned __int16 veh_predictedcollision;
                                                                                // XREF: VEH_CheckForPredictedCrash(gentity_s *)+1F3/r
                                                                                // .text:0069E1F8/w
        unsigned __int16 veh_inair;                 // XREF: VEH_UpdateNitrousPosition(gentity_s *)+6E8/r
                                                                                // .text:0069E20B/w
        unsigned __int16 veh_landed;                // XREF: VEH_UpdateNitrousPosition(gentity_s *)+706/r
                                                                                // .text:0069E21E/w
        unsigned __int16 veh_suspension_limit_activated;
                                                                                // XREF: VEH_UpdateNitrousPosition(gentity_s *)+724/r
                                                                                // .text:0069E231/w
        unsigned __int16 veh_engine_stutter;
                                                                                // XREF: VEH_UpdateNitrousPosition(gentity_s *)+742/r
                                                                                // .text:0069E244/w
        unsigned __int16 veh_ejectoccupants; // XREF: VEH_UpdateClient+462/r
                                                                                // .text:0069E257/w
        unsigned __int16 veh_usespecialability;
                                                                                // XREF: VEH_UpdateDriverActions+95/r
                                                                                // .text:0069E26A/w
        unsigned __int16 veh_usespecialabilityend;
                                                                                // XREF: VEH_UpdateDriverActions+D4/r
                                                                                // .text:0069E27D/w
        unsigned __int16 veh_firepickup;        // XREF: VEH_UpdateDriverActions+122/r
                                                                                // .text:0069E290/w
        unsigned __int16 veh_firepickupend; // XREF: VEH_UpdateDriverActions+161/r
                                                                                // .text:0069E2A3/w
        unsigned __int16 veh_swappickup;        // XREF: VEH_UpdateDriverActions+1AF/r
                                                                                // .text:0069E2B6/w
        unsigned __int16 veh_swappickupend; // XREF: VEH_UpdateDriverActions+1EE/r
                                                                                // .text:0069E2C9/w
        unsigned __int16 veh_dropdeployable;
                                                                                // XREF: VEH_UpdateDriverActions+23C/r
                                                                                // .text:0069E2DC/w
        unsigned __int16 veh_dropdeployableend;
                                                                                // XREF: VEH_UpdateDriverActions+27B/r
                                                                                // .text:0069E2EF/w
        unsigned __int16 veh_handbreak;         // XREF: VEH_UpdateDriverActions+2C9/r
                                                                                // .text:0069E302/w
        unsigned __int16 veh_boost;                 // XREF: VEH_UpdateDriverActions+30A/r
                                                                                // .text:0069E315/w
        unsigned __int16 veh_boost_regen;     // XREF: VEH_UpdateDriverActions+383/r
                                                                                // .text:0069E328/w
        unsigned __int16 script_camera;         // XREF: .text:0069E34E/w
        unsigned __int16 begin;                         // XREF: ClientBegin(int)+6B/r
                                                                                // .text:0069E33B/w
        unsigned __int16 jumpcut;                     // XREF: .text:0069E361/w
                                                                                // XAnimCalcParts_unsigned_char_+263/r ...
        unsigned __int16 curve_nodehit;         // XREF: .text:0069E374/w
        unsigned __int16 curve_start;             // XREF: .text:0069E387/w
        unsigned __int16 curve_end;                 // XREF: VEH_UpdatePlaneOnCurve+5FA/r
                                                                                // .text:0069E39A/w
        unsigned __int16 asphalt;                     // XREF: .text:0069E3AD/w
        unsigned __int16 bark;                            // XREF: .text:0069E3C0/w
        unsigned __int16 brick;                         // XREF: .text:0069E3D3/w
        unsigned __int16 carpet;                        // XREF: .text:0069E3E6/w
        unsigned __int16 ceramic;                     // XREF: .text:0069E3F9/w
        unsigned __int16 cloth;                         // XREF: .text:0069E40C/w
        unsigned __int16 concrete;                    // XREF: .text:0069E41F/w
        unsigned __int16 cushion;                     // XREF: .text:0069E432/w
        unsigned __int16 dirt;                            // XREF: .text:0069E445/w
        unsigned __int16 flesh;                         // XREF: .text:0069E458/w
        unsigned __int16 foliage;                     // XREF: .text:0069E46B/w
        unsigned __int16 fruit;                         // XREF: .text:0069E47E/w
        unsigned __int16 glass;                         // XREF: .text:0069E491/w
                                                                                // create_gjk_geom(gentity_s *,gjk_collision_visitor *,bool,uint,bool)+162/r
        unsigned __int16 grass;                         // XREF: .text:0069E4A4/w
        unsigned __int16 gravel;                        // XREF: .text:0069E4B7/w
        unsigned __int16 ice;                             // XREF: .text:0069E4CA/w
        unsigned __int16 metal;                         // XREF: .text:0069E4DD/w
        unsigned __int16 mud;                             // XREF: .text:0069E4F0/w
        unsigned __int16 paintedmetal;            // XREF: .text:0069E503/w
        unsigned __int16 paper;                         // XREF: .text:0069E516/w
        unsigned __int16 plaster;                     // XREF: .text:0069E529/w
        unsigned __int16 plastic;                     // XREF: .text:0069E53C/w
        unsigned __int16 rock;                            // XREF: .text:0069E54F/w
        unsigned __int16 rubber;                        // XREF: .text:0069E562/w
        unsigned __int16 sand;                            // XREF: .text:0069E575/w
        unsigned __int16 snow;                            // XREF: .text:0069E588/w
        unsigned __int16 water;                         // XREF: .text:0069E59B/w
        unsigned __int16 wood;                            // XREF: .text:0069E5AE/w
        unsigned __int16 tag_enter_driver;    // XREF: .text:0069E6B8/w
                                                                                // .data:ushort const * * s_entryPointOldTags/o
        unsigned __int16 tag_enter_gunner1; // XREF: .text:0069E717/w
                                                                                // .data:00E0A354/o
        unsigned __int16 tag_enter_gunner2; // XREF: .text:0069E72A/w
                                                                                // .data:00E0A358/o
        unsigned __int16 tag_enter_gunner3; // XREF: .text:0069E73D/w
                                                                                // .data:00E0A348/o ...
        unsigned __int16 tag_enter_gunner4; // XREF: .text:0069E750/w
                                                                                // .data:00E0A34C/o ...
        unsigned __int16 tag_enter_passenger1; // XREF: .text:0069E6CB/w
        unsigned __int16 tag_enter_passenger2; // XREF: .text:0069E6DE/w
        unsigned __int16 tag_enter_passenger3; // XREF: .text:0069E6F1/w
        unsigned __int16 tag_enter_passenger4; // XREF: .text:0069E704/w
        unsigned __int16 tag_driver;                // XREF: .text:0069E5E7/w
                                                                                // .data:ushort const * * s_seatTags/o
        unsigned __int16 tag_passenger1;        // XREF: .text:0069E5FA/w
                                                                                // .data:00E0A324/o
        unsigned __int16 tag_passenger2;        // XREF: .text:0069E60D/w
                                                                                // .data:00E0A328/o
        unsigned __int16 tag_passenger3;        // XREF: .text:0069E620/w
                                                                                // .data:00E0A32C/o
        unsigned __int16 tag_passenger4;        // XREF: .text:0069E633/w
                                                                                // .data:00E0A330/o
        unsigned __int16 tag_passenger5;        // XREF: .text:0069E646/w
                                                                                // .data:00E0A334/o
        unsigned __int16 tag_passenger6;        // XREF: .text:0069E659/w
                                                                                // .data:00E0A338/o
        unsigned __int16 tag_gunner1;             // XREF: CG_OffsetVehicleGunner(int,cg_s *)+25/o
                                                                                // CG_PlayerTurretPositionAndBlend+39C/r ...
        unsigned __int16 tag_gunner2;             // XREF: CG_OffsetVehicleGunner(int,cg_s *)+2C/o
                                                                                // CG_PlayerTurretPositionAndBlend+3AA/r ...
        unsigned __int16 tag_gunner3;             // XREF: CG_OffsetVehicleGunner(int,cg_s *)+33/o
                                                                                // CG_PlayerTurretPositionAndBlend+3B8/r ...
        unsigned __int16 tag_gunner4;             // XREF: CG_OffsetVehicleGunner(int,cg_s *)+3A/o
                                                                                // CG_PlayerTurretPositionAndBlend+3C5/r ...
        unsigned __int16 tag_gunner_barrel1;
                                                                                // XREF: CG_OffsetVehicleGunner(int,cg_s *)+9/o
                                                                                // CG_PlayerTurretPositionAndBlend+58D/r ...
        unsigned __int16 tag_gunner_barrel2;
                                                                                // XREF: CG_OffsetVehicleGunner(int,cg_s *)+10/o
                                                                                // CScrCmd_GetLocalGunnerAngles(scr_entref_t)+3E/o ...
        unsigned __int16 tag_gunner_barrel3;
                                                                                // XREF: CG_OffsetVehicleGunner(int,cg_s *)+17/o
                                                                                // CScrCmd_GetLocalGunnerAngles(scr_entref_t)+45/o ...
        unsigned __int16 tag_gunner_barrel4;
                                                                                // XREF: CG_OffsetVehicleGunner(int,cg_s *)+1E/o
                                                                                // CScrCmd_GetLocalGunnerAngles(scr_entref_t)+4C/o ...
        unsigned __int16 tag_gunner_turret1; // XREF: .text:0069D7F3/w
                                                                                // .data:g_gunnerTurretTags/o ...
        unsigned __int16 tag_gunner_turret2; // XREF: .text:0069D806/w
                                                                                // .data:g_gunnerTurretTags/o ...
        unsigned __int16 tag_gunner_turret3; // XREF: .text:0069D819/w
                                                                                // .data:g_gunnerTurretTags/o ...
        unsigned __int16 tag_gunner_turret4;
                                                                                // XREF: CG_UpdateTurretAndOverheatSounds+6A8/r
                                                                                // .text:0069D82C/w ...
        unsigned __int16 tag_flash_gunner[8];
                                                                                // XREF: CG_EntityEvent(int,centity_s *,int)+1430/r
                                                                                // .text:0069D83F/w ...
        unsigned __int16 tag_gunner_brass1; // XREF: CG_EjectWeaponBrass(int,entityState_s const *,int,int)+137/r
                                                                                // .text:0069D8D7/w
        unsigned __int16 tag_gunner_brass1a;
                                                                                // XREF: CG_EjectWeaponBrass(int,entityState_s const *,int,int)+14C/r
                                                                                // .text:0069D8EA/w
        unsigned __int16 tag_gunner_brass2; // XREF: CG_EjectWeaponBrass(int,entityState_s const *,int,int)+161/r
                                                                                // .text:0069D8FD/w
        unsigned __int16 tag_gunner_brass2a;
                                                                                // XREF: CG_EjectWeaponBrass(int,entityState_s const *,int,int)+176/r
                                                                                // .text:0069D910/w
        unsigned __int16 tag_gunner_hands1; // XREF: .text:0069D923/w
        unsigned __int16 tag_wheel_front_left;
                                                                                // XREF: CG_Vehicle_PreControllers(int,DObj const *,centity_s *)+1963/r
                                                                                // .text:0069D936/w ...
        unsigned __int16 tag_wheel_front_right;
                                                                                // XREF: CG_Vehicle_PreControllers(int,DObj const *,centity_s *)+199D/r
                                                                                // .text:0069D949/w ...
        unsigned __int16 tag_wheel_back_left; // XREF: .text:0069D95C/w
                                                                                // NitrousVehicle::destructible_damage(char const *,int)+44/r ...
        unsigned __int16 tag_wheel_back_right; // XREF: .text:0069D96F/w
                                                                                // NitrousVehicle::destructible_damage(char const *,int)+6A/r ...
        unsigned __int16 tag_wheel_middle_left; // XREF: .text:0069D982/w
                                                                                // NitrousVehicle::destructible_damage(char const *,int)+37/r ...
        unsigned __int16 tag_wheel_middle_right; // XREF: .text:0069D995/w
                                                                                // NitrousVehicle::destructible_damage(char const *,int)+5D/r ...
        unsigned __int16 tag_wheel_front;     // XREF: CG_Vehicle_PreControllers(int,DObj const *,centity_s *)+1BA9/r
                                                                                // .text:0069D9A8/w
        unsigned __int16 tag_wheel_back;        // XREF: CG_Vehicle_PreControllers(int,DObj const *,centity_s *)+1C74/r
                                                                                // .text:0069D9BB/w
        unsigned __int16 freelook;                    // XREF: PlayerCmd_AllowSpectateTeam+F7/r
                                                                                // .text:0069E5C1/w
        unsigned __int16 localplayers;            // XREF: PlayerCmd_AllowSpectateTeam+10F/r
                                                                                // .text:0069E5D4/w
        unsigned __int16 tag_enter_right;     // XREF: .text:0069E763/w
                                                                                // .data:00E0A340/o
        unsigned __int16 tag_enter_left;        // XREF: .text:0069E776/w
                                                                                // .data:ushort const * * s_entryPointTags/o
        unsigned __int16 tag_enter_back;        // XREF: .text:0069E789/w
                                                                                // .data:00E0A344/o
        unsigned __int16 tag_popout;                // XREF: G_UpdateVehicleTags(gentity_s *)+8D/r
                                                                                // .text:0069E7D5/w
        unsigned __int16 tag_body;                    // XREF: CG_CalcVehicleViewValues(int)+B8/r
                                                                                // CG_Vehicle_PreControllers(int,DObj const *,centity_s *)+15D3/r ...
        unsigned __int16 tag_turret;                // XREF: draw_tank_turret_component+F0/r
                                                                                // CG_Vehicle_PreControllers(int,DObj const *,centity_s *)+15F2/r ...
        unsigned __int16 tag_turret_base;     // XREF: G_UpdateVehicleTags(gentity_s *)+101/r
                                                                                // .text:0069E80E/w
        unsigned __int16 tag_barrel;                // XREF: CG_CalcVehicleViewValues(int)+F6/r
                                                                                // CG_Vehicle_PreControllers(int,DObj const *,centity_s *)+1611/r ...
        unsigned __int16 tag_barrel_recoil; // XREF: CG_Vehicle_PreControllers(int,DObj const *,centity_s *)+163D/r
                                                                                // .text:0069E834/w
        unsigned __int16 tag_engine_left;     // XREF: .text:0069E847/w
        unsigned __int16 tag_engine_right;    // XREF: .text:0069E85A/w
        unsigned __int16 tag_minigun_right_barrel;
                                                                                // XREF: CG_Vehicle_PreControllers(int,DObj const *,centity_s *)+17B3/r
                                                                                // .text:0069E86D/w
        unsigned __int16 front_left;                // XREF: CMD_VEH_GetWheelSurface(scr_entref_t):loc_67EC8A/r
                                                                                // .text:0069E880/w ...
        unsigned __int16 front_right;             // XREF: CMD_VEH_GetWheelSurface(scr_entref_t):loc_67EC9F/r
                                                                                // .text:0069E893/w ...
        unsigned __int16 back_left;                 // XREF: CMD_VEH_GetWheelSurface(scr_entref_t):loc_67ECB4/r
                                                                                // .text:0069E8A6/w
        unsigned __int16 back_right;                // XREF: CMD_VEH_GetWheelSurface(scr_entref_t):loc_67ECC9/r
                                                                                // .text:0069E8B9/w
        unsigned __int16 middle_left;             // XREF: CMD_VEH_GetWheelSurface(scr_entref_t):loc_67ECDE/r
                                                                                // .text:0069E8CC/w
        unsigned __int16 middle_right;            // XREF: CMD_VEH_GetWheelSurface(scr_entref_t):loc_67ECF3/r
                                                                                // .text:0069E8DF/w
        unsigned __int16 stuck_to_player;     // XREF: AttachBoltGrenade+118/r
                                                                                // StickMissile+153/r ...
        unsigned __int16 auto_ai;                     // XREF: GScr_SetMode+A4/r
                                                                                // .text:0069E905/w
        unsigned __int16 auto_nonai;                // XREF: GScr_SetMode:loc_64948C/r
                                                                                // .text:0069E918/w
        unsigned __int16 manual;                        // XREF: GScr_SetMode:loc_649443/r
                                                                                // .text:0069E92B/w
        unsigned __int16 manual_ai;                 // XREF: GScr_SetMode:loc_649460/r
                                                                                // .text:0069E93E/w
        unsigned __int16 auto_change;             // XREF: GScr_SetClientNameMode+18/r
                                                                                // .text:0069E951/w
        unsigned __int16 call_vote;                 // XREF: Scr_VoteCalled(gentity_s *,char *,char *,char *)+2F/r
                                                                                // .text:0069E964/w
        unsigned __int16 intermission;            // XREF: ClientScr_SetSessionState+D6/r
                                                                                // ClientScr_GetSessionState+BC/r ...
        unsigned __int16 manual_change;         // XREF: GScr_SetClientNameMode+33/r
                                                                                // .text:0069E98A/w
        unsigned __int16 menuresponse;            // XREF: ClientDisconnect(int)+C4/r
                                                                                // Cmd_MenuResponse_f(gentity_s *)+107/r ...
        unsigned __int16 pistol;                        // XREF: .text:0069E9B0/w
        unsigned __int16 plane_waypoint;        // XREF: .text:0069E9C3/w
        unsigned __int16 playing;                     // XREF: ClientScr_SetSessionState+76/r
                                                                                // ClientScr_GetSessionState+80/r ...
        unsigned __int16 spectator;                 // XREF: ClientScr_SetSessionTeam+80/r
                                                                                // ClientScr_SetFFATeam+8C/r ...
        unsigned __int16 spectator_cycle;     // XREF: Cmd_FollowCycle_f(gentity_s *,int)+231/r
                                                                                // .text:0069E9FC/w
        unsigned __int16 vote;                            // XREF: Scr_PlayerVote(gentity_s *,char *)+13/r
                                                                                // .text:0069EA0F/w
        unsigned __int16 sprint_begin;            // XREF: G_ClientDoPerFrameNotifies+1D5/r
                                                                                // .text:0069EA22/w
        unsigned __int16 sprint_end;                // XREF: G_ClientDoPerFrameNotifies+1CD/r
                                                                                // .text:0069EA35/w
        unsigned __int16 tag_gunner_pov;        // XREF: .text:0069EA48/w
        unsigned __int16 flared_or_tabuned_death;
                                                                                // XREF: player_die(gentity_s *,gentity_s *,gentity_s *,int,int,int,float const * const,hitLocation_t,int)+362/r
                                                                                // .text:0069EA5B/w
        unsigned __int16 destroyed_vehicle; // XREF: Scr_Vehicle_Die(gentity_s *,gentity_s *,gentity_s *,int,int,int,float const * const,hitLocation_t,int)+28E/r
                                                                                // .text:0069EA6E/w
        unsigned __int16 vehicle_driver;        // XREF: VEH_SwitchClientToSeat+13D/r
                                                                                // Scr_Vehicle_Use(gentity_s *,gentity_s *,gentity_s *)+BC/r ...
        unsigned __int16 died_in_turret;        // XREF: .text:0069EA94/w
        unsigned __int16 force_crouch;            // XREF: HandleClientEvent(gclient_s *,gentity_s *,int,int)+4B3/r
                                                                                // .text:0069EAA7/w
        unsigned __int16 scavenger;                 // XREF: Touch_Item(gentity_s *,gentity_s *,int)+20F/r
                                                                                // .text:0069EABA/w
        unsigned __int16 trigger_use_doubletap;
                                                                                // XREF: Player_ActivateDoubleTapCmd+1DC/r
                                                                                // .text:0069EACD/w
        unsigned __int16 doubletap_detonate;
                                                                                // XREF: ClientThink_real(gentity_s *,usercmd_s *)+44A/r
                                                                                // .text:0069EAE0/w
        unsigned __int16 dtp_through_glass; // XREF: .text:0069EAF3/w
                                                                                // GlassSv_Touch(uint,gentity_s *,int)+C9/r
        unsigned __int16 dtp_end;                     // XREF: G_ClientDoPerFrameNotifies+119/r
                                                                                // .text:0069EB06/w
        unsigned __int16 _custom;                     // XREF: .text:0069EB19/w
                                                                                // ScrCmd_animcustom+58/r
        unsigned __int16 angle_deltas;            // XREF: .text:0069EB2C/w
                                                                                // ActorCmd_AnimMode:loc_91751B/r
        unsigned __int16 bulletwhizby;            // XREF: .text:0069EB3F/w
                                                                                // .data:g_AIEV_scrConst_table/o
        unsigned __int16 dog;                             // XREF: .text:0069EB52/w
                                                                                // .data:ushort const * * g_AISpeciesNames/o
        unsigned __int16 enemy;                         // XREF: .text:0069EB65/w
                                                                                // Sentient_SetEnemy(sentient_s *,gentity_s *,int)+1BE/r
        unsigned __int16 enemy_visible;         // XREF: .text:0069EB78/w
                                                                                // Actor_UpdateVisCache(actor_s *,gentity_s const *,sentient_info_t *,bool)+FB/r
        unsigned __int16 face_angle;                // XREF: .text:0069EB8B/w
                                                                                // ActorCmd_OrientMode+2F/r
        unsigned __int16 face_current;            // XREF: .text:0069EB9E/w
                                                                                // ActorCmd_OrientMode:loc_917660/r
        unsigned __int16 face_default;            // XREF: .text:0069EBB1/w
                                                                                // ActorCmd_OrientMode:loc_91781E/r
        unsigned __int16 face_direction;        // XREF: .text:0069EBC4/w
                                                                                // ActorCmd_OrientMode:loc_917690/r
        unsigned __int16 face_enemy;                // XREF: .text:0069EBD7/w
                                                                                // ActorCmd_OrientMode:loc_917702/r
        unsigned __int16 face_enemy_or_motion; // XREF: .text:0069EBEA/w
                                                                                // ActorCmd_OrientMode:loc_917720/r
        unsigned __int16 face_goal;                 // XREF: .text:0069EBFD/w
                                                                                // ActorCmd_OrientMode:loc_91773E/r
        unsigned __int16 face_motion;             // XREF: .text:0069EC10/w
                                                                                // ActorCmd_OrientMode:loc_91775C/r
        unsigned __int16 face_point;                // XREF: .text:0069EC23/w
                                                                                // ActorCmd_OrientMode:loc_91777A/r
        unsigned __int16 gravity;                     // XREF: .text:0069EC5C/w
                                                                                // ActorCmd_TraverseMode+2F/r ...
        unsigned __int16 groundEntChanged;    // XREF: .text:0069EC6F/w
                                                                                // Actor_DoMove(actor_s *)+A19/r
        unsigned __int16 gunshot;                     // XREF: .text:0069EC82/w
                                                                                // .data:g_AIEV_scrConst_table/o
        unsigned __int16 obstacle;                    // XREF: .text:0069EC95/w
                                                                                // G_NotifyScriptsOfSelectedScriptStruct(uint)+32/r ...
        unsigned __int16 movemode;                    // XREF: .text:0069ECA8/w
                                                                                // Actor_SetAnimScript(actor_s *,scr_animscript_t *,uchar,ai_animmode_t,scriptAnimAIFunctionTypes_t)+FB/r
        unsigned __int16 node_out_of_range; // XREF: .text:0069ECCE/w
        unsigned __int16 node_relinquished; // XREF: .text:0069ECE1/w
                                                                                // Path_RelinquishNode(sentient_s *,int)+11D/r
        unsigned __int16 node_taken;                // XREF: .text:0069ECF4/w
        unsigned __int16 node_not_safe;         // XREF: .text:0069ED07/w
        unsigned __int16 noclip;                        // XREF: .text:0069ECBB/w
                                                                                // ActorCmd_TraverseMode:loc_9173D5/r ...
        unsigned __int16 nogravity;                 // XREF: .text:0069ED1A/w
                                                                                // ActorCmd_TraverseMode:loc_9173BA/r ...
        unsigned __int16 nophysics;                 // XREF: .text:0069ED2D/w
                                                                                // ActorCmd_AnimMode:loc_91748A/r
        unsigned __int16 pain;                            // XREF: .text:0069ED40/w
                                                                                // .text:0069F155/w ...
        unsigned __int16 react;                         // XREF: .text:0069ED53/w
        unsigned __int16 pos_deltas;                // XREF: .text:0069ED66/w
                                                                                // ActorCmd_AnimMode:loc_917539/r
        unsigned __int16 run;                             // XREF: .text:0069ED79/w
                                                                                // ActorScr_GetMoveMode+118/r ...
        unsigned __int16 runto_arrived;         // XREF: .text:0069ED8C/w
                                                                                // Actor_CheckNotify+E5/r
        unsigned __int16 silenced_shot;         // XREF: .text:0069ED9F/w
                                                                                // .data:g_AIEV_scrConst_table/o
        unsigned __int16 spawned;                     // XREF: .text:0069EDB2/w
                                                                                // SpawnActor(gentity_s *,uint,enumForceSpawn,int)+3D0/r
        unsigned __int16 start_move;                // XREF: .text:0069EDC5/w
        unsigned __int16 stop;                            // XREF: .text:0069EDD8/w
                                                                                // .text:0069F11C/w ...
        unsigned __int16 stop_soon;                 // XREF: .text:0069EDEB/w
                                                                                // ActorScr_GetMoveMode+F0/r ...
        unsigned __int16 tag_eye;                     // XREF: .text:0069EDFE/w
                                                                                // Actor_CanSeeEntityEx(actor_s *,gentity_s const *,float,float)+224/r ...
        unsigned __int16 walk;                            // XREF: .text:0069EE11/w
                                                                                // ActorScr_GetMoveMode+104/r
        unsigned __int16 world;                         // XREF: .text:0069EE24/w
                                                                                // Actor_GetEntType:loc_917DEB/r
        unsigned __int16 zonly_physics;         // XREF: .text:0069EE37/w
                                                                                // ActorCmd_AnimMode:loc_917557/r
        unsigned __int16 j_ankle_le;                // XREF: CG_Player(int,centity_s *)+307/r
                                                                                // .text:0069EE4A/w
        unsigned __int16 j_ankle_ri;                // XREF: CG_Player(int,centity_s *)+330/r
                                                                                // .text:0069EE5D/w
        unsigned __int16 j_ball_le;                 // XREF: .text:0069EE70/w
        unsigned __int16 j_ball_ri;                 // XREF: .text:0069EE83/w
                                                                                // .data:footTags/o
        unsigned __int16 j_palm_le;                 // XREF: .text:0069EE96/w
                                                                                // .data:footTags/o
        unsigned __int16 j_palm_ri;                 // XREF: .text:0069EEA9/w
                                                                                // .data:footTags/o
        unsigned __int16 j_elbow_le;                // XREF: CG_EntityEvent(int,centity_s *,int)+48B3/r
                                                                                // CG_EntityEvent(int,centity_s *,int):loc_4B65C5/r ...
        unsigned __int16 j_wrist_le;                // XREF: CG_EntityEvent(int,centity_s *,int)+48DB/r
                                                                                // .text:0069EECF/w
        unsigned __int16 broken;                        // XREF: .text:0069EEE2/w
                                                                                // DamagePiece(gentity_s *,uchar,int,float const * const,float const * const,int,bool,int,gentity_s *,int)+49D/r
        unsigned __int16 destructible;            // XREF: .text:0069EEF5/w
        unsigned __int16 destructible_base_piece_death;
                                                                                // XREF: .text:0069EF08/w
                                                                                // DamagePiece(gentity_s *,uchar,int,float const * const,float const * const,int,bool,int,gentity_s *,int)+5F0/r
        unsigned __int16 breakafter;                // XREF: .text:0069EF1B/w
                                                                                // DamagePiece(gentity_s *,uchar,int,float const * const,float const * const,int,bool,int,gentity_s *,int)+573/r
        unsigned __int16 snapacknowledged;    // XREF: .text:0069EF2E/w
        unsigned __int16 j_barrel;                    // XREF: CG_UpdateMinigunTags(cg_s *,centity_s *,DObj const *,int * const)+39/r
                                                                                // .text:0069EF41/w
        unsigned __int16 j_counter;                 // XREF: CG_UpdateViewModelStackCounter(cg_s *,playerState_s const *,DObj const *,WeaponDef const *)+19/r
                                                                                // .text:0069EF54/w
        unsigned __int16 j_spinnerRF;             // XREF: .text:0069EF67/w
        unsigned __int16 j_spinnerLF;             // XREF: .text:0069EF7A/w
        unsigned __int16 j_spinnerR1;             // XREF: .text:0069EF8D/w
        unsigned __int16 j_spinnerL1;             // XREF: .text:0069EFA0/w
        unsigned __int16 j_spinnerR2;             // XREF: .text:0069EFB3/w
        unsigned __int16 j_spinnerL2;             // XREF: .text:0069EFC6/w
        unsigned __int16 j_spinnerR3;             // XREF: .text:0069EFD9/w
        unsigned __int16 j_spinnerL3;             // XREF: .text:0069EFEC/w
        unsigned __int16 ca_disconnected;     // XREF: .text:0069EFFF/w
        unsigned __int16 ca_cinematic;            // XREF: .text:0069F012/w
        unsigned __int16 ca_uicinematic;        // XREF: .text:0069F025/w
        unsigned __int16 ca_logo;                     // XREF: .text:0069F038/w
        unsigned __int16 ca_connecting;         // XREF: .text:0069F04B/w
        unsigned __int16 ca_challenging;        // XREF: .text:0069F05E/w
        unsigned __int16 ca_connected;            // XREF: .text:0069F071/w
        unsigned __int16 ca_sendingstats;     // XREF: .text:0069F084/w
        unsigned __int16 ca_loading;                // XREF: .text:0069F097/w
        unsigned __int16 ca_primed;                 // XREF: .text:0069F0AA/w
        unsigned __int16 ca_active;                 // XREF: .text:0069F0BD/w
        unsigned __int16 ca_map_restart;        // XREF: .text:0069F0D0/w
        unsigned __int16 orientdone;                // XREF: .text:0069F0E3/w
                                                                                // Actor_UpdateBodyAngle(actor_s *)+98/r
        unsigned __int16 menuchanged;             // XREF: .text:0069F0F6/w
        unsigned __int16 init;                            // XREF: .text:0069F109/w
                                                                                // .data:s_animStateCategories/o
        unsigned __int16 move;                            // XREF: .text:0069F12F/w
                                                                                // .data:s_animStateCategories/o
        unsigned __int16 combat;                        // XREF: .text:0069F142/w
                                                                                // .data:s_animStateCategories/o
        unsigned __int16 traverse;                    // XREF: .text:0069F17B/w
                                                                                // .data:s_animStateCategories/o
        unsigned __int16 idle;                            // XREF: .text:0069F18E/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 move_run;                    // XREF: .text:0069F1A1/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 move_start;                // XREF: .text:0069F1B4/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 move_stop;                 // XREF: .text:0069F1C7/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 move_walk;                 // XREF: .text:0069F1DA/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 move_turn_left;        // XREF: .text:0069F1ED/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 move_turn_right;     // XREF: .text:0069F200/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 move_run_turn_left; // XREF: .text:0069F213/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 move_run_turn_right; // XREF: .text:0069F226/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 move_turn_around_left; // XREF: .text:0069F239/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 move_turn_around_right; // XREF: .text:0069F24C/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 move_run_turn_around_left; // XREF: .text:0069F25F/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 move_run_turn_around_right; // XREF: .text:0069F272/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 move_attack;             // XREF: .text:0069F285/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 stop_idle;                 // XREF: .text:0069F298/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 stop_attackidle;     // XREF: .text:0069F2AB/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 stop_attackidle_bark; // XREF: .text:0069F2BE/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 stop_attackidle_growl; // XREF: .text:0069F2D1/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 pain_main;                 // XREF: .text:0069F2E4/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 pain_front;                // XREF: .text:0069F2F7/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 pain_back;                 // XREF: .text:0069F30A/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 pain_left;                 // XREF: .text:0069F31D/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 pain_right;                // XREF: .text:0069F330/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 pain_run_front;        // XREF: .text:0069F343/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 pain_run_back;         // XREF: .text:0069F356/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 pain_run_left;         // XREF: .text:0069F369/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 pain_run_right;        // XREF: .text:0069F37C/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 death_front;             // XREF: .text:0069F38F/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 death_back;                // XREF: .text:0069F3A2/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 death_left;                // XREF: .text:0069F3B5/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 death_right;             // XREF: .text:0069F3C8/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 flashed;                     // XREF: .text:0069F3DB/w
                                                                                // .data:s_animStateCategories/o ...
        unsigned __int16 combat_attackidle; // XREF: .text:0069F3EE/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 combat_attackidle_bark; // XREF: .text:0069F401/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 combat_attackidle_growl; // XREF: .text:0069F414/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 combat_attack_run; // XREF: .text:0069F427/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 combat_attack_player; // XREF: .text:0069F43A/w
        unsigned __int16 combat_attack_player_early; // XREF: .text:0069F44D/w
        unsigned __int16 combat_attack_player_lunge; // XREF: .text:0069F460/w
        unsigned __int16 combat_attack_player_close_range;
                                                                                // XREF: .text:0069F473/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 combat_attack_miss; // XREF: .text:0069F486/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 combat_attack_miss_left; // XREF: .text:0069F499/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 combat_attack_miss_right; // XREF: .text:0069F4AC/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 combat_player_neck_snap; // XREF: .text:0069F4BF/w
        unsigned __int16 combat_player_neck_miss; // XREF: .text:0069F4D2/w
        unsigned __int16 traverse_jump_up_40; // XREF: .text:0069F4E5/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 traverse_jump_up_80; // XREF: .text:0069F4F8/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 traverse_jump_down_40; // XREF: .text:0069F50B/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 traverse_jump_down_80; // XREF: .text:0069F51E/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 traverse_wallhop;    // XREF: .text:0069F531/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 traverse_window;     // XREF: .text:0069F544/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 traverse_through_hole_42; // XREF: .text:0069F557/w
                                                                                // .data:s_animStateNames/o
        unsigned __int16 swimming_begin;        // XREF: .text:0069F56A/w
        unsigned __int16 swimming_end;            // XREF: .text:0069F57D/w
        unsigned __int16 snd_boat_engine;     // XREF: .text:0069F590/w
        unsigned __int16 snd_boat_bow_l;        // XREF: .text:0069F5A3/w
        unsigned __int16 snd_boat_bow_r;        // XREF: .text:0069F5B6/w
        unsigned __int16 gv_event;                    // XREF: .text:0069F5C9/w
                                                                                // Scr_GameVariants_GetRule(void)+73/r
        unsigned __int16 gv_action;                 // XREF: .text:0069F5DC/w
                                                                                // Scr_GameVariants_GetRule(void)+A0/r
        unsigned __int16 gv_target;                 // XREF: .text:0069F5EF/w
                                                                                // Scr_GameVariants_GetRule(void)+D0/r
        unsigned __int16 gv_parameter;            // XREF: .text:0069F602/w
                                                                                // Scr_GameVariants_AddParams(GVRule &)+F1/r
        unsigned __int16 gv_condlhs;                // XREF: .text:0069F615/w
                                                                                // Scr_GameVariants_AddConditional(GVRule &)+41/r
        unsigned __int16 gv_condop;                 // XREF: .text:0069F628/w
                                                                                // Scr_GameVariants_AddConditional(GVRule &)+6F/r
        unsigned __int16 gv_condrhs;                // XREF: .text:0069F63B/w
                                                                                // Scr_GameVariants_AddConditional(GVRule &)+156/r
        unsigned __int16 freeze;                        // XREF: .text:0069F64E/w
        unsigned __int16 up;                                // XREF: .text:0069F661/w
#ifdef KISAK_SP
        // SP-only species names. MP ships a single AI species (dog) so `dog` above is the
        // whole of g_AISpeciesNames there; SP has four. Appended at the END of the struct so
        // no existing member moves -- the MP path must compile byte-identically, and every
        // member here is assigned individually by name in Scr_LoadConsts, so order carries no
        // meaning of its own. Spellings are the ones ActorScr_SetSpecies already reports in
        // its Scr_Error text and the ones aitype/*.gsc assign to self.type (verified against
        // aitype/hudson_int_silhoutte.gsc, which sets self.type = "human").
        unsigned __int16 human;
        unsigned __int16 zombie;
        unsigned __int16 zombie_dog;
        // SP-only arrival notify sent by Actor_MoveAlongPathWithTeam (retail 0x023A586C).
        unsigned __int16 corner_approach;
#endif
};

struct cscr_const_t // sizeof=0xD8
{                                       // XREF: .data:cscr_const_t cscr_const/r
    unsigned __int16 none;              // XREF: CG_EntityEvent(int,centity_s *,int):def_4B68E5/r
                                        // CScr_SetClientScripts(ScriptFunctions *)+14/w ...
    unsigned __int16 asphalt;           // XREF: CScr_SetClientScripts(ScriptFunctions *)+2B/w
                                        // CScr_GetVehicleTreadFxArray+CF/r
    unsigned __int16 bark;              // XREF: CScr_SetClientScripts(ScriptFunctions *)+42/w
                                        // CScr_GetVehicleTreadFxArray+FA/r
    unsigned __int16 brick;             // XREF: CScr_SetClientScripts(ScriptFunctions *)+59/w
                                        // CScr_GetVehicleTreadFxArray+125/r
    unsigned __int16 carpet;            // XREF: CScr_SetClientScripts(ScriptFunctions *)+70/w
                                        // CScr_GetVehicleTreadFxArray+150/r
    unsigned __int16 ceramic;           // XREF: CScr_SetClientScripts(ScriptFunctions *)+87/w
                                        // CScr_GetVehicleTreadFxArray+17B/r
    unsigned __int16 cloth;             // XREF: CScr_SetClientScripts(ScriptFunctions *)+9E/w
                                        // CScr_GetVehicleTreadFxArray+1A6/r
    unsigned __int16 concrete;          // XREF: CScr_SetClientScripts(ScriptFunctions *)+B5/w
                                        // CScr_GetVehicleTreadFxArray+1D1/r
    unsigned __int16 cushion;           // XREF: CScr_SetClientScripts(ScriptFunctions *)+CC/w
                                        // CScr_GetVehicleTreadFxArray+1FC/r
    unsigned __int16 dirt;              // XREF: CScr_SetClientScripts(ScriptFunctions *)+E3/w
                                        // CScr_GetVehicleTreadFxArray+252/r
    unsigned __int16 flesh;             // XREF: CScr_SetClientScripts(ScriptFunctions *)+FA/w
                                        // CScr_GetVehicleTreadFxArray+27D/r
    unsigned __int16 foliage;           // XREF: CScr_SetClientScripts(ScriptFunctions *)+111/w
                                        // CScr_GetVehicleTreadFxArray+2A8/r
    unsigned __int16 fruit;             // XREF: CScr_SetClientScripts(ScriptFunctions *)+128/w
                                        // CScr_GetVehicleTreadFxArray+2D3/r
    unsigned __int16 glass;             // XREF: CScr_SetClientScripts(ScriptFunctions *)+13F/w
                                        // CScr_GetVehicleTreadFxArray+2FE/r
    unsigned __int16 grass;             // XREF: CScr_SetClientScripts(ScriptFunctions *)+156/w
                                        // CScr_GetVehicleTreadFxArray+329/r
    unsigned __int16 gravel;            // XREF: CScr_SetClientScripts(ScriptFunctions *)+16D/w
                                        // CScr_GetVehicleTreadFxArray+354/r
    unsigned __int16 ice;               // XREF: CScr_SetClientScripts(ScriptFunctions *)+184/w
                                        // CScr_GetVehicleTreadFxArray+37F/r
    unsigned __int16 metal;             // XREF: CScr_SetClientScripts(ScriptFunctions *)+19B/w
                                        // CScr_GetVehicleTreadFxArray+3AA/r
    unsigned __int16 mud;               // XREF: CScr_SetClientScripts(ScriptFunctions *)+1B2/w
                                        // CScr_GetVehicleTreadFxArray+3D5/r
    unsigned __int16 paintedmetal;      // XREF: CScr_SetClientScripts(ScriptFunctions *)+1C9/w
                                        // CScr_GetVehicleTreadFxArray+400/r
    unsigned __int16 paper;             // XREF: CScr_SetClientScripts(ScriptFunctions *)+1E0/w
                                        // CScr_GetVehicleTreadFxArray+42B/r
    unsigned __int16 plaster;           // XREF: CScr_SetClientScripts(ScriptFunctions *)+1F7/w
                                        // CScr_GetVehicleTreadFxArray+456/r
    unsigned __int16 plastic;           // XREF: CScr_SetClientScripts(ScriptFunctions *)+20E/w
                                        // CScr_GetVehicleTreadFxArray+481/r
    unsigned __int16 rock;              // XREF: CScr_SetClientScripts(ScriptFunctions *)+225/w
                                        // CScr_GetVehicleTreadFxArray+4AC/r
    unsigned __int16 rubber;            // XREF: CScr_SetClientScripts(ScriptFunctions *)+23C/w
                                        // CScr_GetVehicleTreadFxArray+4D7/r
    unsigned __int16 sand;              // XREF: CScr_SetClientScripts(ScriptFunctions *)+253/w
                                        // CScr_GetVehicleTreadFxArray+502/r
    unsigned __int16 snow;              // XREF: CScr_SetClientScripts(ScriptFunctions *)+26A/w
                                        // CScr_GetVehicleTreadFxArray+52D/r
    unsigned __int16 water;             // XREF: CScr_SetClientScripts(ScriptFunctions *)+281/w
                                        // CScr_GetVehicleTreadFxArray+558/r
    unsigned __int16 wood;              // XREF: CScr_SetClientScripts(ScriptFunctions *)+298/w
                                        // CScr_GetVehicleTreadFxArray+583/r
    unsigned __int16 movedone;          // XREF: CG_InterpolateEntityPosition+11D/r
                                        // CScr_SetClientScripts(ScriptFunctions *)+2AF/w
    unsigned __int16 rotatedone;        // XREF: CG_InterpolateEntityPosition+242/r
                                        // CScr_SetClientScripts(ScriptFunctions *)+2C6/w
    unsigned __int16 entityshutdown;    // XREF: CScr_SetClientScripts(ScriptFunctions *)+2DD/w
                                        // CScrCmd_Delete+D2/r ...
    unsigned __int16 front_left;        // XREF: CScr_SetClientScripts(ScriptFunctions *)+2F4/w
                                        // CScrCmd_GetWheelSurface(scr_entref_t)+132/r ...
    unsigned __int16 front_right;       // XREF: CScr_SetClientScripts(ScriptFunctions *)+30B/w
                                        // CScrCmd_GetWheelSurface(scr_entref_t):loc_4EF597/r ...
    unsigned __int16 back_left;         // XREF: CScr_SetClientScripts(ScriptFunctions *)+322/w
                                        // CScrCmd_GetWheelSurface(scr_entref_t):loc_4EF5AC/r ...
    unsigned __int16 back_right;        // XREF: CScr_SetClientScripts(ScriptFunctions *)+339/w
                                        // CScrCmd_GetWheelSurface(scr_entref_t):loc_4EF5C1/r ...
    unsigned __int16 middle_left;       // XREF: CScr_SetClientScripts(ScriptFunctions *)+350/w
                                        // CScrCmd_GetWheelSurface(scr_entref_t):loc_4EF5D6/r ...
    unsigned __int16 middle_right;      // XREF: CScr_SetClientScripts(ScriptFunctions *)+367/w
                                        // CScrCmd_GetWheelSurface(scr_entref_t):loc_4EF5EB/r ...
    unsigned __int16 fraction;          // XREF: CScr_SetClientScripts(ScriptFunctions *)+37E/w
                                        // CScr_BulletTrace+1F0/r ...
    unsigned __int16 position;          // XREF: CScr_SetClientScripts(ScriptFunctions *)+395/w
                                        // CScr_BulletTrace+237/r ...
    unsigned __int16 entity;            // XREF: CScr_SetClientScripts(ScriptFunctions *)+3AC/w
                                        // CScr_BulletTrace:loc_4F19E3/r
    unsigned __int16 enemy;             // XREF: CG_ActorProcessSnapshot(int,centity_s *)+9A/r
                                        // CScr_SetClientScripts(ScriptFunctions *)+3F1/w
    unsigned __int16 normal;            // XREF: CScr_SetClientScripts(ScriptFunctions *)+3C3/w
                                        // CScr_BulletTrace+2E1/r ...
    unsigned __int16 surfacetype;       // XREF: CScr_SetClientScripts(ScriptFunctions *)+3DA/w
                                        // CScr_BulletTrace+333/r ...
    unsigned __int16 weapon_fired;      // XREF: CScr_SetClientScripts(ScriptFunctions *)+408/w
                                        // CG_FireWeapon_VehicleFire(int,centity_s *,playerState_s const *,int,ushort)+2A/r
    unsigned __int16 death;             // XREF: CScr_SetClientScripts(ScriptFunctions *)+436/w
                                        // CScrCmd_Delete+B5/r ...
    unsigned __int16 trigger;           // XREF: CG_NotifyTriggers(void)+11B/r
                                        // CG_Trigger(centity_s *,centity_s *)+84/r ...
    unsigned __int16 touch;             // XREF: CScr_SetClientScripts(ScriptFunctions *)+44D/w
                                        // CG_DoTouchTriggers(centity_s *,int)+464/r ...
    unsigned __int16 broken;            // XREF: CScr_SetClientScripts(ScriptFunctions *)+41F/w
                                        // CG_NotifyDestructibleScripts+23/r
    unsigned __int16 swimming_begin;    // XREF: CScr_SetClientScripts(ScriptFunctions *)+464/w
                                        // CG_SendSwimNotify(int,int,int)+A/r
    unsigned __int16 swimming_end;      // XREF: CScr_SetClientScripts(ScriptFunctions *)+47B/w
                                        // CG_SendSwimNotify(int,int,int):loc_4FF536/r
    unsigned __int16 breakafter;        // XREF: CScr_SetClientScripts(ScriptFunctions *)+5A6/w
                                        // CG_DestructibleBreakPiece(centity_s *,DestructiblePiece *,int,int,int,float const * const,float const * const,int)+916/r
    unsigned __int16 prone;             // XREF: CScr_SetClientScripts(ScriptFunctions *)+4A9/w
                                        // GetFootstepString(int):$LN2_27/r ...
    unsigned __int16 stand;             // XREF: CScr_SetClientScripts(ScriptFunctions *)+4C0/w
                                        // CScr_GetStance+152/r
    unsigned __int16 crouch;            // XREF: CScr_SetClientScripts(ScriptFunctions *)+4D7/w
                                        // CScr_GetStance+13E/r
    unsigned __int16 sprint;            // XREF: CScr_SetClientScripts(ScriptFunctions *)+4EE/w
                                        // GetFootstepString(int):$LN7_21/r
    unsigned __int16 run;               // XREF: CScr_SetClientScripts(ScriptFunctions *)+505/w
                                        // GetFootstepString(int):$LN6_25/r ...
    unsigned __int16 walk;              // XREF: CScr_SetClientScripts(ScriptFunctions *)+51C/w
                                        // GetFootstepString(int):$LN5_24/r
    unsigned __int16 crouchrun;         // XREF: CScr_SetClientScripts(ScriptFunctions *)+533/w
                                        // GetFootstepString(int):$LN4_26/r
    unsigned __int16 crouchwalk;        // XREF: CScr_SetClientScripts(ScriptFunctions *)+54A/w
                                        // GetFootstepString(int):$LN3_31/r
    unsigned __int16 explode;           // XREF: CG_ExplosionEvent(int,float const * const,float,float,float,int)+105/r
                                        // CScr_SetClientScripts(ScriptFunctions *)+561/w
    unsigned __int16 dtp_land;          // XREF: CG_EntityEvent(int,centity_s *,int)+4936/r
                                        // CScr_SetClientScripts(ScriptFunctions *)+578/w
    unsigned __int16 respawn;           // XREF: CG_Respawn(int,int)+402/r
                                        // CScr_SetClientScripts(ScriptFunctions *)+58F/w
    unsigned __int16 fake_fire;         // XREF: CScr_SetClientScripts(ScriptFunctions *)+5BD/w
                                        // CG_SndWeaponFakeFireNotify+27/r ...
    unsigned __int16 hidden;            // XREF: CG_Vehicle+6A/r
                                        // CScr_SetClientScripts(ScriptFunctions *)+5D4/w
    unsigned __int16 mod_unknown;       // XREF: CScr_SetClientScripts(ScriptFunctions *)+5EB/w
                                        // .data:ushort * * cg_modNames/o
    unsigned __int16 mod_pistol_bullet; // XREF: CScr_SetClientScripts(ScriptFunctions *)+602/w
                                        // .data:00E0385C/o
    unsigned __int16 mod_rifle_bullet;  // XREF: CScr_SetClientScripts(ScriptFunctions *)+619/w
                                        // .data:00E03860/o
    unsigned __int16 mod_grenade;       // XREF: CScr_SetClientScripts(ScriptFunctions *)+630/w
                                        // .data:00E03864/o
    unsigned __int16 mod_grenade_splash;
                                        // XREF: CScr_SetClientScripts(ScriptFunctions *)+647/w
                                        // .data:00E03868/o
    unsigned __int16 mod_projectile;    // XREF: CScr_SetClientScripts(ScriptFunctions *)+65E/w
                                        // .data:00E0386C/o
    unsigned __int16 mod_projectile_splash;
                                        // XREF: CScr_SetClientScripts(ScriptFunctions *)+675/w
                                        // .data:00E03870/o
    unsigned __int16 mod_melee;         // XREF: CScr_SetClientScripts(ScriptFunctions *)+68C/w
                                        // .data:00E03874/o
    unsigned __int16 mod_bayonet;       // XREF: CScr_SetClientScripts(ScriptFunctions *)+6A3/w
                                        // .data:00E03878/o
    unsigned __int16 mod_head_shot;     // XREF: CScr_SetClientScripts(ScriptFunctions *)+6BA/w
                                        // .data:00E0387C/o
    unsigned __int16 mod_crush;         // XREF: CScr_SetClientScripts(ScriptFunctions *)+6D1/w
                                        // .data:00E03880/o
    unsigned __int16 mod_telefrag;      // XREF: CScr_SetClientScripts(ScriptFunctions *)+6E8/w
                                        // .data:00E03884/o
    unsigned __int16 mod_falling;       // XREF: CScr_SetClientScripts(ScriptFunctions *)+6FF/w
                                        // .data:00E03888/o
    unsigned __int16 mod_suicide;       // XREF: CScr_SetClientScripts(ScriptFunctions *)+716/w
                                        // .data:00E0388C/o
    unsigned __int16 mod_trigger_hurt;  // XREF: CScr_SetClientScripts(ScriptFunctions *)+72D/w
                                        // .data:00E03890/o
    unsigned __int16 mod_explosive;     // XREF: CScr_SetClientScripts(ScriptFunctions *)+744/w
                                        // .data:00E03894/o
    unsigned __int16 mod_impact;        // XREF: CScr_SetClientScripts(ScriptFunctions *)+75B/w
                                        // .data:00E03898/o
    unsigned __int16 mod_burned;        // XREF: CScr_SetClientScripts(ScriptFunctions *)+772/w
                                        // .data:00E0389C/o
    unsigned __int16 mod_hit_by_object; // XREF: CScr_SetClientScripts(ScriptFunctions *)+789/w
                                        // .data:00E038A0/o
    unsigned __int16 mod_drown;         // XREF: CScr_SetClientScripts(ScriptFunctions *)+7A0/w
                                        // .data:00E038A4/o
    unsigned __int16 mod_gas;           // XREF: CScr_SetClientScripts(ScriptFunctions *)+7B7/w
                                        // .data:00E038A8/o
    unsigned __int16 face;              // XREF: CG_EntityEvent(int,centity_s *,int)+15EF/r
                                        // CG_EntityEvent(int,centity_s *,int)+16A2/r ...
    unsigned __int16 face_casual;       // XREF: CG_EntityEvent(int,centity_s *,int):$LN17_6/r
                                        // CScr_SetClientScripts(ScriptFunctions *)+7E5/w
    unsigned __int16 face_alert;        // XREF: CG_EntityEvent(int,centity_s *,int):$LN16_7/r
                                        // CScr_SetClientScripts(ScriptFunctions *)+7FC/w
    unsigned __int16 face_cqb;          // XREF: CG_EntityEvent(int,centity_s *,int):$LN15_7/r
                                        // CScr_SetClientScripts(ScriptFunctions *)+813/w
    unsigned __int16 face_running;      // XREF: CG_EntityEvent(int,centity_s *,int):$LN14_6/r
                                        // CScr_SetClientScripts(ScriptFunctions *)+82A/w
    unsigned __int16 face_shoot_single; // XREF: CG_EntityEvent(int,centity_s *,int)+12BD/r
                                        // CG_EntityEvent(int,centity_s *,int):loc_4B328F/r ...
    unsigned __int16 face_shoot_burst;  // XREF: CG_EntityEvent(int,centity_s *,int):$LN12_6/r
                                        // CScr_SetClientScripts(ScriptFunctions *)+858/w
    unsigned __int16 face_melee;        // XREF: CG_EntityEvent(int,centity_s *,int):loc_4B34C9/r
                                        // CG_EntityEvent(int,centity_s *,int)+1843/r ...
    unsigned __int16 face_react;        // XREF: CG_EntityEvent(int,centity_s *,int):$LN10_8/r
                                        // CScr_SetClientScripts(ScriptFunctions *)+886/w
    unsigned __int16 face_talk;         // XREF: CG_EntityEvent(int,centity_s *,int):$LN9_12/r
                                        // CScr_SetClientScripts(ScriptFunctions *)+89D/w
    unsigned __int16 face_talk_long;    // XREF: CG_EntityEvent(int,centity_s *,int):$LN8_9/r
                                        // CScr_SetClientScripts(ScriptFunctions *)+8B4/w
    unsigned __int16 face_pain;         // XREF: CG_EntityEvent(int,centity_s *,int):$LN7_9/r
                                        // CScr_SetClientScripts(ScriptFunctions *)+8CB/w ...
    unsigned __int16 face_death;        // XREF: CG_EntityEvent(int,centity_s *,int):$LN6_9/r
                                        // CScr_SetClientScripts(ScriptFunctions *)+8E2/w
    unsigned __int16 veh_collision;     // XREF: CScr_SetClientScripts(ScriptFunctions *)+8F9/w
                                        // CG_UpdateVehicleNotifies(int,centity_s *)+F3/r
    unsigned __int16 veh_inair;         // XREF: CScr_SetClientScripts(ScriptFunctions *)+910/w
                                        // CG_UpdateVehicleNotifies(int,centity_s *)+115/r
    unsigned __int16 veh_landed;        // XREF: CScr_SetClientScripts(ScriptFunctions *)+927/w
                                        // CG_UpdateVehicleNotifies(int,centity_s *)+137/r
    unsigned __int16 veh_suspension_limit_activated;
                                        // XREF: CScr_SetClientScripts(ScriptFunctions *)+93E/w
                                        // CG_UpdateVehicleNotifies(int,centity_s *)+159/r
    unsigned __int16 veh_engine_stutter;
                                        // XREF: CScr_SetClientScripts(ScriptFunctions *)+955/w
                                        // CG_UpdateVehicleNotifies(int,centity_s *)+17B/r
    unsigned __int16 liveupdate;        // XREF: CScr_SetClientScripts(ScriptFunctions *)+96C/w
                                        // CG_NotifyScriptsOfSelectedScriptStruct(uint):loc_6ECB90/r
    unsigned __int16 disconnect;        // XREF: CScr_SetClientScripts(ScriptFunctions *)+983/w
    unsigned __int16 freeze;            // XREF: CScr_SetClientScripts(ScriptFunctions *)+99A/w
    unsigned __int16 up;                // XREF: CScr_SetClientScripts(ScriptFunctions *)+9B1/w
};

void __cdecl    GScr_LoadConsts();

extern scr_const_t scr_const;
extern cscr_const_t cscr_const;