#include "com_expressions.h"
#include "com_memory.h"
#include "q_parse.h"
#include "q_shared.h"
#include <qcommon/common.h>
#include "com_expressions_eval.h"
#include <server_mp/sv_bot_mp.h>
#include <ui/l_precomp.h>
#include "com_loadutils.h"
#include <ui/ui_shared.h>

expressionRpn prog[150];

int s_operatorPrecedence[] =
{
  2147483647,
  0,
  11,
  11,
  11,
  13,
  13,
  9,
  9,
  15,
  15,
  15,
  15,
  16,
  16,
  25,
  25,
  99,
  80,
  17,
  18,
  9,
  14,
  14
};

const char *g_expOperatorNames[] =
{
  "NOOP",
  ")",
  "*",
  "/",
  "%",
  "+",
  "-",
  "-",
  "!",
  "<",
  "<=",
  ">",
  ">=",
  "==",
  "!=",
  "&&",
  "||",
  "(",
  ",",
  "&",
  "|",
  "~",
  "<<",
  ">>"
};

const char *g_expFunctionNames[457] =
{
  "INVALID",
  "sin",
  "cos",
  "min",
  "max",
  "milliseconds",
  "dvar",
  "dvarint",
  "dvarbool",
  "dvarfloat",
  "dvarstring",
  "ui_active",
  "flashbanged",
  "scoped",
  "scoreboard_visible",
  "inkillcam",
  "isdualwield",
  "isfuelweapon",
  "player",
  "selecting_location",
  "team",
  "otherteam",
  "marinesfield",
  "opforfield",
  "menuisopen",
  "writingdata",
  "inlobby",
  "inprivateparty",
  "privatepartyhost",
  "privatepartyhostinlobby",
  "aloneinparty",
  "sameclanmembersinparty",
  "aloneinlobby",
  "adsjavelin",
  "weaplockblink",
  "weapattacktop",
  "weapattackdirect",
  "secondsastime",
  "tablelookup",
  "statstablelookup",
  "getclassbonus",
  "locstring",
  "localvarint",
  "localvarbool",
  "localvarfloat",
  "localvarstring",
  "timeleft",
  "secondsascountdown",
  "gamemsgwndactive",
  "int",
  "string",
  "float",
  "gametypename",
  "gametype",
  "gametypedescription",
  "scoreatrank",
  "friendsonline",
  "spectatingclient",
  "keybinding",
  "actionslotusable",
  "hudfade",
  "maxrecommendedplayers",
  "acceptinginvite",
  "gamehost",
  "isvisibilitybitset",
  "issplitscreen",
  "issplitscreenhost",
  "splitscreennum",
  "iscinematicfinished",
  "isselectinglocationalkillstreak",
  "isselectingairstrike",
  "isselectingartillery",
  "isselectingnapalm",
  "isselectingmortar",
  "isselectingcomlink",
  "hasfriends",
  "haspendingfriends",
  "hasinvites",
  "partyismissingmappack",
  "partymissingmappackerror",
  "anynewmappacks",
  "isintermission",
  "issuperuser",
  "isautojoindevuser",
  "isitemlocked",
  "isitemdualwieldlocked",
  "isitemdualwieldpurchased",
  "isitemnew",
  "getitemref",
  "getitemname",
  "getitemimage",
  "getitemunlocklevel",
  "getitemunlockplevel",
  "getitemdesc",
  "getitemindex",
  "getitemdualwieldindex",
  "getitemdualwieldbaseindex",
  "getItemCost",
  "getitemdualwieldcost",
  "getItemSellPrice",
  "getItemCount",
  "getitemgroup",
  "getstatbyname",
  "isclanowner",
  "isclanadmin",
  "isclanmember",
  "getclanrank",
  "getxuid",
  "getselfgamertag",
  "getrankbyxuid",
  "getprestigebyxuid",
  "getcodpointsbyxuid",
  "getdisplaylevelbyxuid",
  "getclanshortdescription",
  "getclanlongdescription",
  "getclandatefounded",
  "getclanmembercount",
  "getclanonlinemembercount",
  "getclanname",
  "getclanmotd",
  "getclantagandname",
  "getrecommendedplayercount",
  "getMutedStatus",
  "getUIRect",
  "getFeederData",
  "hasFocus",
  "isVisible",
  "getRank",
  "getPrestige",
  "getPlayerCardTitle",
  "getdefaultclassslot",
  "getcacFactionNameWithButtons",
  "getitemattachment",
  "getstatforfriendorself",
  "getsortedstatsforfriendorself",
  "getFloatAsFormattedString",
  "changeSortedStatsPivot",
  "canScrollUpOrDown",
  "getCurrentScrollBarPosition",
  "getCombatRecordInfoBarWidth",
  "getCombatRecordInfoBarText",
  "getCombatRecordInfoBarTagText",
  "getCombatRecordHistogramHeight",
  "getCombatRecordPieChartText",
  "getCombatRecordMinMaxScore",
  "getCombatRecordFailedContracts",
  "getCombatRecordLockedString",
  "getNumWagerMatchesPlayed",
  "getNumWagerMatchesWon",
  "getAfterActionReportAwardsInfo",
  "getNumPersonalBests",
  "getPersonalBestName",
  "getPersonalBestValue",
  "getPersonalBestDelta",
  "getPersonalBestPrefix",
  "getNumWeaponUnlocks",
  "getNumFeatureUnlocks",
  "getUnlockedWeaponItemIndex",
  "getUnlockedFeatureItemIndex",
  "getNumStatsMilestones",
  "getStatsMilestoneValue",
  "getStatsMilestoneName",
  "getCurrentChallengeXpReward",
  "getCurrentChallengeCpReward",
  "getCurrentChallengeProgress",
  "isCurrentChallengeItemClassified",
  "getNumChallenges",
  "getTotalMatchesPlayed",
  "getCopyClassConfirmationText",
  "getCopyClassDialogTitle",
  "isFileshareDataSummaryValid",
  "getFileshareRecentGamesCount",
  "getFileshareGameMap",
  "getFileshareGameMapName",
  "getFileshareGameType",
  "getFileshareGameTypeName",
  "getFileshareGameDate",
  "getFileshareFileName",
  "getFileshareFileId",
  "getFileshareFileSize",
  "getImageWidth",
  "getTextWidth",
  "getTextHeight",
  "getcomposite",
  "iscompositevalid",
  "isSignedIn",
  "isProfileSignedIn",
  "gettoastpopupwidth",
  "gettoastpopupicon",
  "gettoastpopuptitle",
  "gettoastpopupdescription",
  "getNumLives",
  "playersAlive",
  "getPlaylistMaxPartySize",
  "getTime",
  "getBombTime",
  "canSpecCycle",
  "canSpecFree",
  "getDStat",
  "getDStatForPreviousMatch",
  "isStableStatsBufferInitialized",
  "getClientInPlace",
  "getClientName",
  "toOrdinal",
  "getScoreForClient",
  "isDemoPlaying",
  "isDemoClipRecording",
  "isDemoClipPlaying",
  "isDemoMovieRendering",
  "isDemoThirdPersonCamera",
  "isDemoMovieCamera",
  "isDisplayingPartyScoreboard",
  "getDemoTitleName",
  "getDemoTitleDescription",
  "getDemoAuthor",
  "getDemoTimeInfo",
  "getDemoDuration",
  "getDemoSegmentCount",
  "getDemoSegmentInformation",
  "isClipModified",
  "canStartDemoPlayback",
  "getDemoSaveScreenName",
  "getDemoSaveScreenDescription",
  "getTheaterFilmNotSelectedMessage",
  "isItemPurchased",
  "isCurrentItemPurchased",
  "isCurrentItemAttachmentPurchased",
  "isCurrentItemOptionPurchased",
  "isItemAttachmentPointPurchased",
  "getCurrentItemCost",
  "getCurrentItemSellPrice",
  "getCurrentItemAttachmentCost",
  "getCurrentItemName",
  "getCurrentItemIndex",
  "getCurrentItemAttachmentName",
  "getCurrentItemAttachmentDesc",
  "getCurrentItemNumAttachments",
  "getCurrentItemAttachmentNum",
  "getCurrentItemAttachmentPoint",
  "getItemNumAttachmentsEquipped",
  "getItemEquippedAttachment",
  "PlaylistPlayerCount",
  "CategoryPlayerCountForPlaylist",
  "TotalPlayersInPlaylists",
  "GetPlayersRegisteredOnline",
  "getCurrentMapName",
  "getLeaderboardMinReqText",
  "getBaseLbMenuName",
  "getCurrentGameType",
  "getPreviousMapName",
  "getPreviousGameType",
  "isPartyReady",
  "isExtraCamActive",
  "getCurrentWeapon",
  "ShowZombieMap",
  "getattachmentname",
  "getattachmentimage",
  "getattachmentdesc",
  "getweaponoptionimage",
  "getweaponoptionname",
  "isplayerjoinable",
  "isplayerinvitable",
  "isrecommendedplayerinvitable",
  "isextracamstatic",
  "getnumitemattachmentswithattachpoint",
  "getcurrentitemoption",
  "getweaponoptioncost",
  "isitemgroupnew",
  "select",
  "choose",
  "hastacticalmaskoverlay",
  "invehicle",
  "getnumactivecontracts",
  "getcurrentcontractindex",
  "getcontractname",
  "getcontractdesc",
  "getcontractprogress",
  "getcontractrequiredcount",
  "getcontractcost",
  "iscontractactive",
  "getindexforactivecontract",
  "getindexfornthactivecontract",
  "iscontractinprogress",
  "iscontractexpired",
  "iscontractcomplete",
  "getcontractcombattimeleft",
  "menuhasfocus",
  "getcustomclassloadoutitem",
  "getcustomclassmodifier",
  "getcustomclassname",
  "getCACItemIndex",
  "getMachineID",
  "approxequals",
  "lbval",
  "statval",
  "areStatsFetched",
  "isSignedInToLive",
  "getlocalclientnum",
  "isprimarylocalclient",
  "getselltext",
  "getitemprice",
  "isguest",
  "isfriendfromxuid",
  "randomintrange",
  "getselectedemblemlayer",
  "getgroupscount",
  "isgroup",
  "GetWeaponOptionGroupIndex",
  "getDownloadProgress",
  "getUploadProgress",
  "getUploadTimeRemaining",
  "IsCurrentSortedItemEquipped",
  "IsCurrentItemEquippedInAnyCustomClass",
  "IsItemEquipped",
  "getfeedercount",
  "IsCurrentItemClassified",
  "GetCurrentItemClassifiedHintText",
  "arecontractsfetched",
  "istimesynced",
  "iscontractlocked",
  "gettimescontractpurchased",
  "gettimescontractpurchasable",
  "getcontractcooldowntime",
  "getremainingcontractcooldowntime",
  "getcontractunlocklevel",
  "getcontractrewardtext",
  "getcontractexpirationtype",
  "getcontractexpirationdata",
  "isTaskInProgress",
  "getPlaceWithTiesForScore",
  "getissuperuser",
  "isinguidedmissile",
  "isingame",
  "emblemLayerState",
  "emblemLayerName",
  "emblemLayerCanOutline",
  "emblemLayerCanDuplicate",
  "emblemSelectedLayer",
  "emblemLayerCost",
  "emblemLayerUnlockLevel",
  "emblemIconName",
  "emblemIconUnlockDesc",
  "emblemIcon",
  "emblemIconCost",
  "emblemIconState",
  "emblemIsModified",
  "emblemPurchasedLayerCount",
  "emblemBackgroundCount",
  "emblemBackgroundIsLocked",
  "emblemBackgroundIsClassified",
  "emblemBackgroundIsPurchased",
  "emblemBackgroundMaterial",
  "emblemPlayerBackgroundMaterial",
  "emblemSelectedBackground",
  "emblemBackgroundCost",
  "emblemBackgroundName",
  "emblemBackgroundUnlockDesc",
  "emblemStateDisplay",
  "emblemCategoryDisplay",
  "emblemFilterCount",
  "emblemFilterIconID",
  "itemhasdualwield",
  "itemisdualwield",
  "getdemofileid",
  "getfilesharerating",
  "getautojoinlobbystatus",
  "getcountertotal",
  "showbusydotsindicator",
  "canratefilmintheater",
  "getattachmentsformatted",
  "isclantagfeaturelocked",
  "isclantagfeaturepurchased",
  "getCurrentClanTagFeature",
  "GetClanTagFeatureCost",
  "GetClanTagFeatureName",
  "IsItemOptionPurchasedByName",
  "GetItemOptionByName",
  "GetFaceCamoIndex",
  "howmanyreadiesneeded",
  "GetLiveGroupCount",
  "getUserTagFromIndex",
  "getFileShareFilterList",
  "canRenderClip",
  "canShowContentFromUser",
  "isContentRatingAllowed",
  "isDemonwareFetchingDone",
  "GetIndexIntoMatchScoreboard",
  "GetWagerPlaceForMatchScoreboard",
  "GetWagerGametypeNameFromEnum",
  "GetCollectiblesCount",
  "anySignedIn",
  "anySignedInToLive",
  "anySignedInToLiveAndStatsFetched",
  "isProItemVersionUnlocked",
  "isProItemVersion",
  "getFlagCarrierForTeam",
  "getFlagStatusForTeam",
  "getChallengeProgressString",
  "getChallengeName",
  "getName",
  "getProgressString",
  "getXpReward",
  "getCpReward",
  "getDescription",
  "getChallengeDescription",
  "toUpper",
  "getPlaylistName",
  "getTimeUntilNewContracts",
  "needToPerformCommunitySearch",
  "getLbTypeWithButtons",
  "getLBFilter",
  "getLBTypeByDuration",
  "getScoreboardColumnHeader",
  "getProItemVersionCost",
  "getNumSortedItemsEquipped",
  "getGameInvitesCount",
  "getMySlotInfo",
  "isAttachmentAllowedOnItemIndex",
  "isCurrentOrPreviousMapEntryAvailable",
  "getFileShareTotalVotes",
  "GridMove",
  "getPooledFileDetails",
  "GetClanTagFeaturePlevel",
  "GetWeaponOptionUnlockLvl",
  "GetWeaponOptionUnlockPLevel",
  "Add64",
  "Sub64",
  "Div64",
  "Mul64",
  "IsPremiumSubscriber",
  "GetUserFileRating",
  "HostMigrationWaitingForPlayers",
  "IsItemUnlocked",
  "IsWeaponItemUnlocked",
  "IsWeaponItemPurchased",
  "IsWeaponItemClassified",
  "GetWeaponName",
  "IsGuestByXUID",
  "canSwitchToLobby",
  "getMapIndexByName",
  "getGamemodeIndexByName",
  "aloneinpartyignoresplitscreen",
  "aloneinlobbyignoresplitscreen",
  "serverSort",
  "serverSortDirection",
  "isViewportLarge",
  "IsChallengeItemPurchased",
  "GetChallengeAttachmentName",
  "isItemLockedForAll",
  "getServerCounts",
  "GetLowestLocalCP",
  "isWagerServer",
  "getContextHeight",
  "GetCurrentItemMultiText",
  "getCopyCustomGametypeClassConfirmationText",
  "getCopyCustomGametypeClassDialogTitle",
  NULL
};






void __cdecl Expression_Free(ExpressionStatement *statement)
{
    if ( statement->rpn )
        Z_Free((char *)statement->rpn, 36);
    if ( statement->filename )
        Z_Free(statement->filename, 36);
    statement->rpn = 0;
    statement->numRpn = 0;
    statement->filename = 0;
}

expressionRpn *__cdecl MakeRPN(expressionEntry *entry, int *length)
{
    unsigned int idxOper; // [esp+0h] [ebp-260h]
    int idxProg; // [esp+4h] [ebp-25Ch]
    expressionEntry *oper[150]; // [esp+8h] [ebp-258h]

    idxProg = 0;
    idxOper = 0;
    while (entry)
    {
        if (entry->type == 1)
        {
            if ((unsigned int)idxProg >= 0x96
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions.cpp",
                    238,
                    0,
                    "%s",
                    "idxProg < ARRAY_COUNT( prog )"))
            {
                __debugbreak();
            }
            prog[idxProg].type = 0;
            // The op doubles as the operand's dataType: OP_NOOP, OP_RIGHTPAREN
            // and OP_MULTIPLY are VAL_INT, VAL_FLOAT and VAL_STRING. nx-port:
            // the whole internals union is copied, not its int -- the decompile
            // went through intVal, which drops half of a string's pointer here.
            prog[idxProg].data.cmdIdx = entry->data.op;
            prog[idxProg].data.constant.internals = entry->data.operand.internals;
            ++idxProg;
        }
        else
        {
            if (entry->type
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions.cpp",
                    243,
                    0,
                    "%s",
                    "entry->type == OPERATOR"))
            {
                __debugbreak();
            }
            if (entry->data.op == OP_LEFTPAREN)
            {
                if (idxOper >= 0x96
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions.cpp",
                        246,
                        0,
                        "%s",
                        "idxOper < ARRAY_COUNT( oper )"))
                {
                    __debugbreak();
                }
                oper[idxOper++] = entry;
            }
            else if (entry->data.op == OP_RIGHTPAREN)
            {
                if (!idxOper
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions.cpp",
                        252,
                        0,
                        "%s",
                        "idxOper"))
                {
                    __debugbreak();
                }
                while (idxOper
                    && oper[idxOper - 1]->data.op != OP_LEFTPAREN
                    && oper[idxOper - 1]->data.op < NUM_EXPRESSION_OPERATORS)
                {
                    if ((unsigned int)idxProg >= 0x96
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions.cpp",
                            254,
                            0,
                            "%s",
                            "idxProg < ARRAY_COUNT( prog )"))
                    {
                        __debugbreak();
                    }
                    prog[idxProg].type = 1;
                    prog[idxProg++].data.cmdIdx = oper[--idxOper]->data.op;
                }
                if (oper[idxOper - 1]->data.op != OP_LEFTPAREN
                    && oper[idxOper - 1]->data.op < NUM_EXPRESSION_OPERATORS
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions.cpp",
                        258,
                        0,
                        "%s",
                        "oper[idxOper-1]->data.op == OP_LEFTPAREN || oper[idxOper-1]->data.op >= NUM_EXPRESSION_OPERATORS"))
                {
                    __debugbreak();
                }
                if (oper[idxOper - 1]->data.op == OP_LEFTPAREN)
                {
                    --idxOper;
                }
                else
                {
                    if ((unsigned int)idxProg >= 0x96
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions.cpp",
                            262,
                            0,
                            "%s",
                            "idxProg < ARRAY_COUNT( prog )"))
                    {
                        __debugbreak();
                    }
                    prog[idxProg].type = 1;
                    prog[idxProg++].data.cmdIdx = oper[--idxOper]->data.op;
                }
            }
            else if (entry->data.op >= NUM_EXPRESSION_OPERATORS)
            {
                if (idxOper >= 0x96
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions.cpp",
                        282,
                        0,
                        "%s",
                        "idxOper < ARRAY_COUNT( oper )"))
                {
                    __debugbreak();
                }
                oper[idxOper++] = entry;
            }
            else
            {
                while (idxOper
                    && oper[idxOper - 1]->data.op < NUM_EXPRESSION_OPERATORS
                    && s_operatorPrecedence[oper[idxOper - 1]->data.op] <= s_operatorPrecedence[entry->data.op])
                {
                    if ((unsigned int)idxProg >= 0x96
                        && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions.cpp",
                            272,
                            0,
                            "%s",
                            "idxProg < ARRAY_COUNT( prog )"))
                    {
                        __debugbreak();
                    }
                    prog[idxProg].type = 1;
                    prog[idxProg++].data.cmdIdx = oper[--idxOper]->data.op;
                }
                if (idxOper >= 0x96
                    && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions.cpp",
                        276,
                        0,
                        "%s",
                        "idxOper < ARRAY_COUNT( oper )"))
                {
                    __debugbreak();
                }
                oper[idxOper++] = entry;
            }
        }
        entry = entry->next;
    }
    while (idxOper && oper[idxOper - 1]->data.op != OP_LEFTPAREN)
    {
        if ((unsigned int)idxProg >= 0x96
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions.cpp",
                286,
                0,
                "%s",
                "idxProg < ARRAY_COUNT( prog )"))
        {
            __debugbreak();
        }
        prog[idxProg].type = 1;
        prog[idxProg++].data.cmdIdx = oper[--idxOper]->data.op;
    }
    if ((unsigned int)idxProg >= 0x96
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions.cpp",
            290,
            0,
            "%s",
            "idxProg < ARRAY_COUNT( prog )"))
    {
        __debugbreak();
    }
    prog[idxProg++].type = 3;
    *length = idxProg;
    return prog;
}
char __cdecl Expression_Parse(
                const char **text,
                ExpressionStatement *statement,
                void *compileBuffer,
                int compileBufferSize)
{
    const char *OperandAsString; // eax
    expressionEntry *v6; // [esp+8h] [ebp-68h]
    expressionEntry *v7; // [esp+Ch] [ebp-64h]
    expressionEntry *v8; // [esp+24h] [ebp-4Ch]
    float fl; // [esp+28h] [ebp-48h]
    int i; // [esp+2Ch] [ebp-44h]
    expressionRpn *rpn; // [esp+30h] [ebp-40h]
    expOperationEnum op; // [esp+34h] [ebp-3Ch]
    expressionEntry *entriesTail; // [esp+38h] [ebp-38h]
    const char *sourceText; // [esp+3Ch] [ebp-34h]
    int lastType; // [esp+40h] [ebp-30h]
    ParseTokenType tokenType; // [esp+44h] [ebp-2Ch]
    expressionEntry *head; // [esp+48h] [ebp-28h]
    ExpressionAllocState *allocState; // [esp+4Ch] [ebp-24h]
    expOperationEnum lastOp; // [esp+50h] [ebp-20h]
    parseInfo_t *token; // [esp+54h] [ebp-1Ch]
    Operand lastOperand; // [esp+58h] [ebp-18h]
    int numOpenLeftParens; // [esp+60h] [ebp-10h]
    expressionEntry *newExpression; // [esp+64h] [ebp-Ch]
    ExpressionAllocState _allocState; // [esp+68h] [ebp-8h] BYREF

    allocState = 0;
    lastOp = OP_NOOP;
    //lastOperand = 0;
    lastOperand.dataType = VAL_INT;
    lastOperand.internals.intVal = 0;
    lastType = 2;
    if ( compileBuffer )
    {
        _allocState.buffer = compileBuffer;
        _allocState.size = compileBufferSize;
        allocState = &_allocState;
    }
    if ( !text
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions.cpp", 322, 0, "%s", "text") )
    {
        __debugbreak();
    }
    head = 0;
    entriesTail = 0;
    sourceText = *text;
    statement->rpn = 0;
    statement->numRpn = 0;
    Com_BeginParseSession("expression");
    Com_SetSpaceDelimited(0);
    numOpenLeftParens = 0;
    while ( 1 )
    {
        token = Com_ParseOnLine(text);
        if ( !token )
            break;
        tokenType = Com_GetTokenType();
        if ( !I_stricmp(token->token, ";") )
            break;
        if ( tokenType == PARSE_TOKEN_STRING )
            op = OP_NOOP;
        else
            op = (expOperationEnum)Expression_ParseOperatorToken(token->token);
        if ( op == OP_NOOP )
        {
            switch ( tokenType )
            {
                case PARSE_TOKEN_NUMBER:
                    fl = atof(token->token);
                    i = atoi(token->token);
                    // nx-port: sizeof(expressionEntry) in place of the x86
                    // 16 here and below; it is 32 on LP64.
                    if ( fl == (float)i )
                    {
                        v6 = Expression_Alloc(allocState, sizeof(expressionEntry));
                        v6->next = 0;
                        v6->type = 1;
                        v6->data.op = OP_NOOP;
                        v6->data.operand.internals.intVal = i;
                        newExpression = v6;
                    }
                    else
                    {
                        v7 = Expression_Alloc(allocState, sizeof(expressionEntry));
                        v7->next = 0;
                        v7->type = 1;
                        v7->data.op = OP_RIGHTPAREN;
                        v7->data.operand.internals.floatVal = fl;
                        newExpression = v7;
                    }
                    goto LABEL_39;
                case PARSE_TOKEN_STRING:
                case PARSE_TOKEN_NAME:
                    newExpression = Expression_StringOperand(token->token, allocState);
                    goto LABEL_39;
                case PARSE_TOKEN_HASH:
                    newExpression = Expression_HashOperand(token->token, allocState);
LABEL_39:
                    lastType = 1;
                    lastOperand = newExpression->data.operand;
                    goto LABEL_40;
                default:
                    Com_PrintError(
                        16,
                        "%s:%d - Expression Error: Unknown token '%s'\n",
                        statement->filename,
                        statement->line,
                        token->token);
                    Com_PrintError(16, "%s\n", sourceText);
                    Com_EndParseSession();
                    return 0;
            }
        }
        if ( op == OP_SUBTRACT )
        {
            if ( !lastType && lastOp != OP_RIGHTPAREN || lastType == 2 )
                op = OP_NEGATE;
            goto LABEL_31;
        }
        if ( op == OP_LEFTPAREN )
        {
            ++numOpenLeftParens;
            if ( lastType == 2 )
                goto LABEL_31;
            if ( lastType )
            {
                OperandAsString = GetOperandAsString(lastOperand);
                Com_PrintError(16, "%s:%d - Expression Error: %s(...\n", statement->filename, statement->line, OperandAsString);
                Com_PrintError(16, "%s\n", sourceText);
                Com_EndParseSession();
                return 0;
            }
            if ( !Expression_OpIsFunction(lastOp) )
                goto LABEL_31;
            lastOp = OP_LEFTPAREN;
        }
        else
        {
            if ( op == OP_RIGHTPAREN )
            {
                if ( --numOpenLeftParens < 0 )
                {
                    Com_PrintError(
                        16,
                        "%s:%d - Expression Error: Found a right parenthesis that doesn't match any left parenthesis\n",
                        statement->filename,
                        statement->line);
                    Com_PrintError(16, "%s\n", sourceText);
                    Com_EndParseSession();
                    return 0;
                }
                if ( !numOpenLeftParens )
                    break;
            }
LABEL_31:
            v8 = Expression_Alloc(allocState, sizeof(expressionEntry));
            v8->next = 0;
            v8->type = 0;
            v8->data.op = op;
            newExpression = v8;
            lastType = 0;
            lastOp = op;
LABEL_40:
            if ( newExpression )
            {
                if ( entriesTail )
                {
                    entriesTail->next = newExpression;
                    entriesTail = newExpression;
                }
                else
                {
                    entriesTail = newExpression;
                    head = newExpression;
                }
                entriesTail->next = 0;
            }
        }
    }
    Com_EndParseSession();
    rpn = MakeRPN(head, &statement->numRpn);
    // nx-port: sizeof(expressionRpn), which the decompile spelled 12 -- its x86
    // size. It is 24 on LP64, so only half the program was copied and
    // EvaluateExpression ran off into whatever followed it in the buffer.
    statement->rpn = (expressionRpn *)Expression_Alloc(allocState, sizeof(expressionRpn) * statement->numRpn);
    memcpy((unsigned __int8 *)statement->rpn, (unsigned __int8 *)rpn, sizeof(expressionRpn) * statement->numRpn);
    return 1;
}

const char *__cdecl GetOperandAsString(Operand operand)
{
    switch ( operand.dataType )
    {
        case VAL_STRING:
            return va("%s", operand.internals.string);
        case VAL_INT:
            return va("%i", operand.internals.intVal);
        case VAL_FLOAT:
            return va("%f", operand.internals.floatVal);
    }
    return "";
}

expressionEntry *__cdecl Expression_Alloc(ExpressionAllocState *allocState, int size)
{
    expressionEntry *result; // [esp+0h] [ebp-4h]
    expressionEntry *resulta; // [esp+0h] [ebp-4h]
    signed int sizea; // [esp+10h] [ebp+Ch]

    if ( allocState )
    {
        // nx-port: rounded to pointer alignment, not to 4. The entries carved
        // out of this buffer hold pointers, and a string operand's odd length
        // would otherwise leave every entry after it misaligned.
        sizea = (size + (int)sizeof(void *) - 1) & ~((int)sizeof(void *) - 1);
        if ( allocState->size < sizea
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions.cpp",
                        121,
                        0,
                        "%s",
                        "allocState->size >= size") )
        {
            __debugbreak();
        }
        allocState->size -= sizea;
        resulta = (expressionEntry *)allocState->buffer;
        allocState->buffer = (char *)resulta + sizea;
        return resulta;
    }
    else
    {
        result = (expressionEntry *)Z_Malloc(size, "Expression_Alloc", 36);
        if ( !result
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions.cpp", 114, 0, "%s", "result") )
        {
            __debugbreak();
        }
        return result;
    }
}

bool __cdecl Expression_OpIsFunction(expOperationEnum op)
{
    return op >= NUM_EXPRESSION_OPERATORS;
}

int __cdecl Expression_ParseOperatorToken(const char *token)
{
    int index; // [esp+0h] [ebp-8h]
    int opNum; // [esp+4h] [ebp-4h]
    int opNuma; // [esp+4h] [ebp-4h]

    for ( opNum = 0; opNum < 24; ++opNum )
    {
        if ( !I_stricmp(g_expOperatorNames[opNum], token) )
            return opNum;
    }
    for ( opNuma = 0; ; ++opNuma )
    {
        if ( !g_expFunctionNames[opNuma] )
            return 0;
        if ( !I_stricmp(g_expFunctionNames[opNuma], token) )
            break;
    }
    index = Expression_GetIndexForFunction(opNuma);
    if ( index != -1 )
        return index + 24;
    Com_PrintError(16, "Expression function '%s' not found\n", token);
    return 0;
}

expressionEntry *__cdecl Expression_HashOperand(const char *str, ExpressionAllocState *alloc)
{
    expressionEntry *entry; // [esp+Ch] [ebp-4h]

    entry = Expression_Alloc(alloc, sizeof(expressionEntry));   // nx-port: was 16
    entry->next = 0;
    entry->type = 1;
    entry->data.op = OP_NOOP;
    entry->data.operand.internals.intVal = Com_HashString(str, 0);
    return entry;
}

expressionEntry *__cdecl Expression_StringOperand(const char *str, ExpressionAllocState *alloc)
{
    char v2; // cl
    _BYTE *intVal; // [esp+8h] [ebp-1Ch]
    const char *v5; // [esp+Ch] [ebp-18h]
    expressionEntry *entry; // [esp+20h] [ebp-4h]

    // nx-port: the string lives right after the entry. The decompile sized
    // that as strlen + 17 (x86 entry + NUL) and kept its address in intVal,
    // an int: on LP64 the pointer lost its top half and the copy below wrote
    // through what was left.
    entry = Expression_Alloc(alloc, sizeof(expressionEntry) + strlen(str) + 1);
    entry->next = 0;
    entry->type = 1;
    entry->data.op = OP_MULTIPLY;
    entry->data.operand.internals.string = (const char *)&entry[1];
    v5 = str;
    intVal = (_BYTE *)&entry[1];
    do
    {
        v2 = *v5;
        *intVal++ = *v5++;
    }
    while ( v2 );
    return entry;
}

char __cdecl Expression_Read(int handle, ExpressionStatement *statement)
{
    char v3; // [esp+3h] [ebp-C81h]
    char *v4; // [esp+8h] [ebp-C7Ch]
    char *v5; // [esp+Ch] [ebp-C78h]
    const char *expressionPtr; // [esp+20h] [ebp-C64h] BYREF
    char filename[64]; // [esp+24h] [ebp-C60h] BYREF
    pc_token_s token; // [esp+64h] [ebp-C20h] BYREF
    int line; // [esp+474h] [ebp-810h] BYREF
    int numOpenLeftParens; // [esp+478h] [ebp-80Ch]
    char expressionText[2052]; // [esp+47Ch] [ebp-808h] BYREF

    expressionPtr = expressionText;
    PC_SourceFileAndLine(handle, filename, &line);
    expressionText[0] = 0;
    numOpenLeftParens = 0;
    while ( PC_ReadTokenHandle(handle, &token) )
    {
        if ( token.type == 1 )
        {
            I_strncat(expressionText, 2048, "\"");
            I_strncat(expressionText, 2048, token.string);
            I_strncat(expressionText, 2048, "\" ");
        }
        else
        {
            I_strncat(expressionText, 2048, token.string);
            I_strncat(expressionText, 2048, " ");
        }
        if ( !I_strcmp(token.string, ";") )
            break;
        if ( token.type != 1 )
        {
            if ( I_strcmp(token.string, "(") )
            {
                if ( !I_strcmp(token.string, ")") && !--numOpenLeftParens )
                    break;
            }
            else
            {
                ++numOpenLeftParens;
            }
        }
    }
    statement->line = line;
    statement->filename = (char *)Z_Malloc(&filename[strlen(filename) + 1] - &filename[1] + 1, "Expression", 36);
    v5 = filename;
    v4 = statement->filename;
    do
    {
        v3 = *v5;
        *v4++ = *v5++;
    }
    while ( v3 );
    return Expression_Parse(&expressionPtr, statement, 0, 0);
}

void __cdecl Expression_Init()
{
    char *buf; // [esp+0h] [ebp-10h]
    const char *parse; // [esp+4h] [ebp-Ch] BYREF
    int index; // [esp+8h] [ebp-8h]
    const char *token; // [esp+Ch] [ebp-4h]

    buf = Com_LoadRawTextFile("expressions/functions.txt");
    if ( buf )
    {
        parse = buf;
        Com_BeginParseSession("expressions/functions.txt");
        for ( index = 0; ; ++index )
        {
            if ( index >= 1024
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_expressions.cpp",
                            539,
                            0,
                            "%s",
                            "index < EXPRESSION_MAX_FUNCTIONS") )
            {
                __debugbreak();
            }
            token = (const char *)Com_Parse(&parse);
            if ( !token || !*token )
                break;
            Expression_MapIndexToFunction(index, token);
        }
        Com_EndParseSession();
        Com_UnloadRawTextFile(buf);
    }
    else
    {
        Com_Error(ERR_DROP, "Couldn't load file '%s'", "expressions/functions.txt");
    }
}

