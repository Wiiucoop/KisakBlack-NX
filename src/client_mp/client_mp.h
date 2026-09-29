#pragma once

#include <game/teams.h>
#include <cgame_mp/cg_scoreboard_mp.h>

union $4EE974FEE7F16CD9507A14B5D3A2E3FF // sizeof=0x8
{                                                                             // XREF: clientState_s/r
        unsigned __int64 xuid;
        unsigned int xuid32[2];
};

enum VehicleAnimState : __int32
{                                                                             // XREF: clientState_s/r
        VEHICLEANIMSTATE_IDLE            = 0x0,
        VEHICLEANIMSTATE_ENTRY         = 0x1,
        VEHICLEANIMSTATE_CHANGEPOS = 0x2,
        VEHICLEANIMSTATE_EXIT            = 0x3,
        VEHICLEANIMSTATECOUNT            = 0x4,
};

struct clientState_s // sizeof=0xD0
{                                                                             // XREF: .data:clientState_s g_defaultClientState/r
                                                                                // snapshot_s/r ...
        int clientIndex;                                        // XREF: Demo_SwitchPlayer(int,bool,int,bool)+B9/r
                                                                                // Demo_SwitchPlayer(int,bool,int,bool)+13F/r ...
        team_t team;                                                // XREF: SpectatorClientEndFrame(gentity_s *):loc_5FBB65/r
        ffa_team_t ffaTeam;
        int modelindex;
        int attachModelIndex[6];
        int attachTagIndex[6];
        char name[32];
        float maxSprintTimeMultiplier;
        int rank;
        int prestige;
        int lastDamageTime;
        int lastStandStartTime;
#ifdef KISAK_SP
        // Retail SP client-state field +0x44. Occupies the existing alignment
        // gap at +116 in this MP-derived layout; all later offsets stay fixed.
        int beingRevived;
#else
        // padding byte
        // padding byte
        // padding byte
        // padding byte
#endif
        //$4EE974FEE7F16CD9507A14B5D3A2E3FF ___u12;
        union //$4EE974FEE7F16CD9507A14B5D3A2E3FF // sizeof=0x8
        {                                                                             // XREF: clientState_s/r
                unsigned __int64 xuid;
                unsigned int xuid32[2];
        };

        unsigned int perks[2];
        char clanAbbrev[8];
        int attachedVehEntNum;
        int attachedVehSeat;
        int needsRevive;
        VehicleAnimState vehAnimState;
        score_s score;
        int clientUIVisibilityFlags;
};
