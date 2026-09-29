#include "bg_perks.h"
#include <universal/q_shared.h>

const dvar_t *perk_skillSet;
const dvar_t *perk_weapSpreadMultiplier;
const dvar_t *perk_weapRateMultiplier;
const dvar_t *perk_weapReloadMultiplier;
const dvar_t *perk_weapSwitchMultiplier;
const dvar_t *perk_weapAdsMultiplier;
const dvar_t *perk_weapMeleeMultiplier;
const dvar_t *perk_extraBreath;
const dvar_t *perk_bulletPenetrationMultiplier;
const dvar_t *perk_grenadeDeath;
const dvar_t *perk_sprintMultiplier;
const dvar_t *perk_grenadeTossBackTimer;
const dvar_t *perk_killstreakReduction;
const dvar_t *perk_extraMoneyMultiplier;
const dvar_t *perk_delayExplosiveTime;
const dvar_t *perk_disarmExplosiveTime;
const dvar_t *perk_speedMultiplier;
const dvar_t *perk_deathStreakCountRequired;
const dvar_t *perk_sprintRecoveryMultiplier;
const dvar_t *perk_healthRegenMultiplier;
const dvar_t *perk_damageKickReduction;
const dvar_t *perk_shellShockReduction;
const dvar_t *perk_mantleReduction;
const dvar_t *perk_interactSpeedMultiplier;
const dvar_t *perk_throwbackInnerRadius;
const dvar_t *perk_flakJacket;
const dvar_t *perk_flakJacket_hardcore;
const dvar_t *perk_fireproof;
const dvar_t *perk_armorPiercing;
const dvar_t *perk_dogsAttackGhost;
const dvar_t *perk_blackbirdShowsGpsJammer;


const char *bg_perkNames[52] =
{
  "specialty_armorpiercing",
  "specialty_armorvest",
  "specialty_bulletaccuracy",
  "specialty_bulletdamage",
  "specialty_bulletflinch",
  "specialty_bulletpenetration",
  "specialty_copycat",
  "specialty_delayexplosive",
  "specialty_detectexplosive",
  "specialty_disarmexplosive",
  "specialty_explosivedamage",
  "specialty_extraammo",
  "specialty_extramoney",
  "specialty_fallheight",
  "specialty_fastads",
  "specialty_fastinteract",
  "specialty_fastmantle",
  "specialty_fastmeleerecovery",
  "specialty_fastreload",
  "specialty_fastweaponswitch",
  "specialty_finalstand",
  "specialty_fireproof",
  "specialty_flakjacket",
  "specialty_gambler",
  "specialty_gas_mask",
  "specialty_gpsjammer",
  "specialty_grenadepulldeath",
  "specialty_healthregen",
  "specialty_holdbreath",
  "specialty_killstreak",
  "specialty_longersprint",
  "specialty_loudenemies",
  "specialty_movefaster",
  "specialty_nomotionsensor",
  "specialty_noname",
  "specialty_nottargetedbyai",
  "specialty_pin_back",
  "specialty_pistoldeath",
  "specialty_quieter",
  "specialty_reconnaissance",
  "specialty_rof",
  "specialty_scavenger",
  "specialty_shades",
  "specialty_showenemyequipment",
  "specialty_stunprotection",
  "specialty_shellshock",
  "specialty_sprintrecovery",
  "specialty_showonradar",
  "specialty_twoattach",
  "specialty_twogrenades",
  "specialty_twoprimaries",
  "specialty_unlimitedsprint"
};

#ifdef KISAK_SP
// BlackOps.exe initializes this exact 15-entry table at 0x00BDF2CC from
// the string pointers at 0x00B6DFE8.  SP perks occupy two bits apiece.
const char *bg_spPerkNames[BG_SP_PERK_COUNT] =
{
  "specialty_longersprint",
  "specialty_unlimitedsprint",
  "specialty_scavanger",
  "specialty_fastreload",
  "specialty_bulletdamage",
  "specialty_bulletaccuracy",
  "specialty_flakjacket",
  "specialty_armorvest",
  "specialty_quickrevive",
  "specialty_altmelee",
  "specialty_rof",
  "specialty_extraammo",
  "specialty_endurance",
  "specialty_deadshot",
  "specialty_additionalprimaryweapon"
};
#endif

unsigned int __cdecl BG_GetPerkIndexForName(const char *perkName)
{
    unsigned int idx; // [esp+0h] [ebp-4h]

#ifdef KISAK_SP
    if ( !perkName )
        return BG_SP_PERK_COUNT;
    for ( idx = 0; idx < BG_SP_PERK_COUNT && I_stricmp(perkName, bg_spPerkNames[idx]); ++idx )
        ;
#else
    if ( !perkName )
        return 52;
    for ( idx = 0; idx < 0x34 && I_stricmp(perkName, bg_perkNames[idx]); ++idx )
        ;
#endif
    return idx;
}

const char *__cdecl BG_GetPerkNameForIndex(unsigned int perkIndex)
{
#ifdef KISAK_SP
    if ( perkIndex >= BG_SP_PERK_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_perks.cpp",
                    144,
                    0,
                    "perkIndex doesn't index SP_PERK_COUNT\n\t%i not in [0, %i)",
                    perkIndex,
                    BG_SP_PERK_COUNT) )
    {
        __debugbreak();
    }
    return bg_spPerkNames[perkIndex];
#else
    if ( perkIndex >= 0x34
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_perks.cpp",
                    144,
                    0,
                    "perkIndex doesn't index PERK_COUNT\n\t%i not in [0, %i)",
                    perkIndex,
                    52) )
    {
        __debugbreak();
    }
    return bg_perkNames[perkIndex];
#endif
}

void __cdecl Perks_RegisterDvars()
{
    perk_skillSet = _Dvar_RegisterInt("skills", 0, 0, 0x7FFFFFFF, 0x82u, "SP character skill levels");
    perk_weapSpreadMultiplier = _Dvar_RegisterFloat(
                                                                "perk_weapSpreadMultiplier",
                                                                0.64999998,
                                                                0.0,
                                                                1.0,
                                                                0x80u,
                                                                "Percentage of weapon spread to use");
    perk_weapReloadMultiplier = _Dvar_RegisterFloat(
                                                                "perk_weapReloadMultiplier",
                                                                0.5,
                                                                0.0,
                                                                1.0,
                                                                0x80u,
                                                                "Percentage of weapon reload time to use");
    perk_weapRateMultiplier = _Dvar_RegisterFloat(
                                                            "perk_weapRateMultiplier",
                                                            0.75,
                                                            0.0,
                                                            1.0,
                                                            0x80u,
                                                            "Percentage of weapon firing rate to use");
    perk_weapSwitchMultiplier = _Dvar_RegisterFloat(
                                                                "perk_weapSwitchMultiplier",
                                                                0.5,
                                                                0.0099999998,
                                                                1.0,
                                                                0x80u,
                                                                "Percentage of weapon switching rate to use");
    perk_weapAdsMultiplier = _Dvar_RegisterFloat(
                                                         "perk_weapAdsMultiplier",
                                                         0.5,
                                                         0.0099999998,
                                                         1.0,
                                                         0x80u,
                                                         "Percentage of weapon ADS rate to use");
    perk_weapMeleeMultiplier = _Dvar_RegisterFloat(
                                                             "perk_weapMeleeMultiplier",
                                                             0.5,
                                                             0.0099999998,
                                                             1.0,
                                                             0x80u,
                                                             "Percentage of weapon melee recovery rate to use");
    perk_extraBreath = _Dvar_RegisterFloat(
                                             "perk_extraBreath",
                                             5.0,
                                             0.0,
                                             3.4028235e38,
                                             0x80u,
                                             "Number of extra seconds a player can hold his breath");
    perk_bulletPenetrationMultiplier = _Dvar_RegisterFloat(
                                                                             "perk_bulletPenetrationMultiplier",
                                                                             2.0,
                                                                             0.0,
                                                                             30.0,
                                                                             0x80u,
                                                                             "Multiplier for extra bullet penetration");
    perk_grenadeDeath = _Dvar_RegisterString(
                                                "perk_grenadeDeath",
                                                "frag_grenade_short_mp",
                                                0x80u,
                                                "Name of the grenade weapon to drop");
    perk_sprintMultiplier = _Dvar_RegisterFloat(
                                                        "perk_sprintMultiplier",
                                                        2.0,
                                                        0.0,
                                                        3.4028235e38,
                                                        0x80u,
                                                        "Multiplier for player_sprinttime");
    perk_grenadeTossBackTimer = _Dvar_RegisterInt(
                                                                "perk_grenadeTossBackTimer",
                                                                2500,
                                                                0,
                                                                6000,
                                                                0x80u,
                                                                "Minimum timer for a grenade.    If the time left is less than this it will be bumped up.    Tossback Perk");
    perk_killstreakReduction = _Dvar_RegisterInt(
                                                             "perk_killstreakReduction",
                                                             1,
                                                             0,
                                                             5,
                                                             0x80u,
                                                             "Reduced number of kills required to obtain a killstreak.");
    perk_extraMoneyMultiplier = _Dvar_RegisterFloat(
                                                                "perk_extraMoneyMultiplier",
                                                                2.0,
                                                                0.0,
                                                                10.0,
                                                                0x80u,
                                                                "Multiplier for extra COD points.");
    perk_delayExplosiveTime = _Dvar_RegisterFloat(
                                                            "perk_delayExplosiveTime",
                                                            0.2,
                                                            0.0,
                                                            10.0,
                                                            0x80u,
                                                            "Time in seconds to delay a proximity explosive");
    perk_disarmExplosiveTime = _Dvar_RegisterFloat(
                                                             "perk_disarmExplosiveTime",
                                                             2.0,
                                                             0.0,
                                                             20.0,
                                                             0x80u,
                                                             "Time in seconds to disarm a proximity explosive");
    perk_speedMultiplier = _Dvar_RegisterFloat(
                                                     "perk_speedMultiplier",
                                                     1.0700001,
                                                     0.0,
                                                     5.0,
                                                     0x80u,
                                                     "Player speed multiplier");
    perk_deathStreakCountRequired = _Dvar_RegisterInt(
                                                                        "perk_deathStreakCountRequired",
                                                                        5,
                                                                        1,
                                                                        10,
                                                                        0x80u,
                                                                        "Number of deaths required to obtain a deathstreak.");
    perk_sprintRecoveryMultiplier = _Dvar_RegisterFloat(
                                                                        "perk_sprintRecoveryMultiplier",
                                                                        0.60000002,
                                                                        0.0,
                                                                        1.0,
                                                                        0x80u,
                                                                        "Percent of sprint recovery time to use.");
    perk_healthRegenMultiplier = _Dvar_RegisterFloat(
                                                                 "perk_healthRegenMultiplier",
                                                                 1.5,
                                                                 1.0,
                                                                 5.0,
                                                                 0x80u,
                                                                 "Multiplier to the time which the player starts to regenerate health after damage");
    perk_damageKickReduction = _Dvar_RegisterFloat(
                                                             "perk_damageKickReduction",
                                                             0.2,
                                                             0.0,
                                                             1.0,
                                                             0x80u,
                                                             "The percentage decrease of view kick flinch when the player takes damage");
    perk_shellShockReduction = _Dvar_RegisterFloat(
                                                             "perk_shellShockReduction",
                                                             0.5,
                                                             0.0,
                                                             1.0,
                                                             0x80u,
                                                             "The percentage decrease of shellshock time");
    perk_mantleReduction = _Dvar_RegisterFloat(
                                                     "perk_mantleReduction",
                                                     0.5,
                                                     0.0,
                                                     1.0,
                                                     0x80u,
                                                     "The percentage decrease of mantle up time");
    perk_interactSpeedMultiplier = _Dvar_RegisterFloat(
                                                                     "perk_interactSpeedMultiplier",
                                                                     1.15,
                                                                     0.0,
                                                                     10.0,
                                                                     0x80u,
                                                                     "Multiplier for increasing use timers.");
    perk_throwbackInnerRadius = _Dvar_RegisterFloat(
                                                                "perk_throwbackInnerRadius",
                                                                120.0,
                                                                0.0,
                                                                3.4028235e38,
                                                                0x80u,
                                                                "The radius to a live grenade player must be within initially to do a throwback with the "
                                                                "toss back perk on");
    perk_flakJacket = _Dvar_RegisterFloat(
                                            "perk_flakJacket",
                                            35.0,
                                            0.0,
                                            3.4028235e38,
                                            0x80u,
                                            "Percent of explosive damage done that will apply to player.");
    perk_flakJacket_hardcore = _Dvar_RegisterFloat(
                                                             "perk_flakJacket_hardcore",
                                                             9.0,
                                                             0.0,
                                                             3.4028235e38,
                                                             0x80u,
                                                             "Percent of explosive damage done that will apply to player in hardcore.");
    perk_fireproof = _Dvar_RegisterFloat(
                                         "perk_fireproof",
                                         95.0,
                                         0.0,
                                         3.4028235e38,
                                         0x80u,
                                         "Percent to reduce flame damage done to the player.");
    perk_armorPiercing = _Dvar_RegisterFloat(
                                                 "perk_armorPiercing",
                                                 40.0,
                                                 0.0,
                                                 3.4028235e38,
                                                 0x80u,
                                                 "Percent bullet damage is increased for helicopters, spy planes, and turrets.");
    perk_dogsAttackGhost = _Dvar_RegisterBool(
                                                     "perk_dogsAttackGhost",
                                                     1,
                                                     0x80u,
                                                     "Set to true if the dogs should attack players with 'specialty_nottargetedbyai'");
    perk_blackbirdShowsGpsJammer = _Dvar_RegisterBool(
                                                                     "perk_blackbirdShowsGpsJammer",
                                                                     1,
                                                                     0x80u,
                                                                     "Set to true if the blackbird should show players with 'specialty_gpsjammer'");
}

