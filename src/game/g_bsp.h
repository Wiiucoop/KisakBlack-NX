#pragma once

#include "pathnode.h"

struct PathData // sizeof=0x28
{                                       // XREF: GameWorldSp/r
    unsigned int nodeCount;             // XREF: Bot_UpdatePath+47B/r
                                        // Bot_UpdatePath+484/r
    pathnode_t *nodes;                  // XREF: Bot_UpdatePath+49E/r
                                        // Bot_UpdateDirection+98D/r
    pathbasenode_t *basenodes;
    unsigned int chainNodeCount;
    unsigned __int16 *chainNodeForNode;
    unsigned __int16 *nodeForChainNode;
    int visBytes;
    unsigned __int8 *pathVis;
    int nodeTreeCount;
    pathnode_tree_t *nodeTree;

    PathData()
    {
        memset(this, 0, sizeof(PathData)); // lwss add
    }
};

struct GameWorldSp // sizeof=0x2C
{
    const char *name;
    PathData path;
};

struct GameWorldMp // sizeof=0x2C
{                                       // XREF: .data:GameWorldMp gameWorldMp/r
    const char *name;
    PathData path;                      // XREF: Bot_UpdatePath+47B/r
};


// gameWorldMp is the shared path-data world used by pathnode.cpp/sv_bot_mp.cpp
// et al regardless of build flavor -- nothing in this codebase actually reads
// or writes SP-specific world data yet, so it stays unconditional rather than
// being hidden behind KISAK_SP (that used to make it flat-out undeclared for
// every KISAK_SP translation unit that references it, since the #else branch
// was the only place it existed). gameWorldSp is declared for future
// SP-specific pathfinding work but is currently unused anywhere.
extern GameWorldMp gameWorldMp;
#ifdef KISAK_SP
extern GameWorldSp gameWorldSp;
#endif

extern GameWorldMp *gameWorldCurrent;
