#ifdef KISAK_MP
#include "ui_main_mp.h"
#include <ui/ui_shared.h>
#include <ui/ui_utils.h>
#include <qcommon/common.h>
#include <list>
#include <vector>

namespace
{
    menuDef_t botMenu;
    itemDef_s botItems[8];
    itemDef_s *botItemPointers[8];
    textDef_s botText[8];
    focusItemDef_s botFocus[8];
    multiDef_s botCount;
    multiDef_s botDifficulty;
    multiDef_s friendlyCount;
    multiDef_s enemyCount;
    char friendlyLabels[18][32];
    char enemyLabels[18][32];
    char botCountLabels[18][24];
    GenericEventScript closeScript;
    GenericEventHandler closeEvent;
    GenericEventScript openScript;
    GenericEventHandler openEvent;
    GenericEventScript focusScript;
    GenericEventHandler focusEvent;
    GenericEventScript leaveFocusScript;
    GenericEventHandler leaveFocusEvent;
    GenericEventScript menuCloseScript;
    GenericEventHandler menuCloseEvent;
    ItemKeyHandler escapeKey;
    bool initialized;

    struct LobbyButton
    {
        itemDef_s item{};
        textDef_s text{};
        focusItemDef_s focus{};
        GenericEventScript script{};
        GenericEventHandler event{};
        std::vector<itemDef_s *> items;
    };
    // Asset-owned menus retain these pointers across UI restarts. Base and
    // patch assets need separate stable storage, released at process shutdown.
    std::list<LobbyButton> lobbyButtons;

    void InitBotMenu(const itemDef_s *button)
    {
        if (initialized)
            return;
        initialized = true;
        botMenu.window.name = "openblops_bot_settings";
        botMenu.window.rect = {-200.0f, -142.0f, 400.0f, 284.0f, 2, 2};
        botMenu.window.rectClient = botMenu.window.rect;
        botMenu.initialRectInfo = botMenu.window.rect;
        botMenu.ui3dWindowId = -1;
        botMenu.hideBits = ~0ULL;
        botMenu.window.style = 1;
        botMenu.window.modal = 1;
        botMenu.window.backColor[0] = 0.06f;
        botMenu.window.backColor[1] = 0.06f;
        botMenu.window.backColor[2] = 0.06f;
        botMenu.window.backColor[3] = 0.92f;
        botMenu.window.border = 1;
        botMenu.window.borderSize = 1.0f;
        for (int c = 0; c < 3; ++c)
            botMenu.window.borderColor[c] = 0.5f;
        botMenu.window.borderColor[3] = 0.55f;
        botMenu.openSlideSpeed = botMenu.closeSlideSpeed = -1;
        botMenu.openSlideDirection = botMenu.closeSlideDirection = -1;
        botMenu.openFadingTime = botMenu.closeFadingTime = -1;
        botMenu.fadeTimeCounter = botMenu.slideTimeCounter = -1;
        for (int c = 0; c < 4; ++c)
            botMenu.window.foreColor[c] = 1.0f;
        botMenu.fadeClamp = 1.0f;
        botMenu.focusColor[0] = 1.0f;
        botMenu.focusColor[1] = 0.65f;
        botMenu.focusColor[2] = 0.2f;
        botMenu.focusColor[3] = 1.0f;
        botMenu.itemCount = ARRAY_COUNT(botItems);
        botMenu.items = botItemPointers;
        botMenu.cursorItem[0] = -1;
        botMenu.imageTrack = button->imageTrack;

        const char *names[] = {"bots_title", "bots_count", "bots_difficulty", "bots_hint", "bots_done", "bots_enemies", "bots_accent", "bots_divider"};
        const char *labels[] = {"BOT SETTINGS", "", "", "Bots join after you choose a team. Max 17 bots.", "DONE", "", "", ""};
        const float positions[] = {16.0f, 68.0f, 148.0f, 190.0f, 238.0f, 108.0f, 52.0f, 226.0f};
        for (int i = 0; i < ARRAY_COUNT(botItems); ++i)
        {
            itemDef_s &item = botItems[i];
            item.window.name = names[i];
            item.window.rectClient = {20.0f, positions[i], 360.0f, 30.0f, 2, 2};
            item.window.rect = item.window.rectClient;
            item.window.dynamicFlags[0] = 4;
            for (int c = 0; c < 4; ++c)
                item.window.foreColor[c] = 1.0f;
            item.type = item.dataType = (i == 1 || i == 2 || i == 5) ? 10 : (i == 4 ? 3 : (i >= 6 ? 0 : 1));
            item.imageTrack = button->imageTrack;
            item.parent = &botMenu;
            item.ui3dWindowId = -1;
            item.hideBits = ~0ULL;
            if (i == 0 || i == 3 || i >= 6)
                item.window.staticFlags |= 0x100000;
            item.typeData.textDef = &botText[i];
            botText[i] = *button->typeData.textDef;
            botText[i].text = labels[i];
            botText[i].textExpData = nullptr;
            botText[i].textalignx = (i == 1 || i == 2 || i == 4 || i == 5) ? 10.0f : 0.0f;
            botText[i].textscale = i == 3 ? 0.3f : 0.4f;
            botText[i].textTypeData.focusItemDef = &botFocus[i];
            if (i == 1 || i == 2 || i == 4 || i == 5)
            {
                item.window.style = 1;
                item.window.border = 1;
                item.window.borderSize = 1.0f;
                item.window.backColor[3] = 0.3f;
                for (int c = 0; c < 3; ++c)
                    item.window.borderColor[c] = 0.8f;
                item.window.borderColor[3] = 0.2f;
            }
            if (i >= 6)
            {
                item.window.style = 1;
                item.window.rectClient.h = item.window.rect.h = i == 6 ? 2.0f : 1.0f;
                item.window.backColor[0] = 0.953f;
                item.window.backColor[1] = 0.722f;
                item.window.backColor[2] = 0.212f;
                item.window.backColor[3] = i == 6 ? 1.0f : 0.35f;
            }
            botItemPointers[i] = &item;
        }
        // Keep keyboard navigation in the same order as the visual rows.
        botItemPointers[2] = &botItems[5];
        botItemPointers[3] = &botItems[2];
        botItemPointers[4] = &botItems[3];
        botItemPointers[5] = &botItems[4];
        botItems[1].dvar = "scr_num_bots";
        // Text painting uses foreColor even when focused; focusColor alone has no effect.
        focusScript.action = "play uin_navigation_over ; setcolor forecolor .953 .722 .212 1 ; setcolor bordercolor .953 .722 .212 .9 ; setcolor backcolor .18 .15 .08 .85 ;";
        leaveFocusScript.action = "setcolor forecolor 1 1 1 1 ; setcolor bordercolor .8 .8 .8 .2 ; setcolor backcolor 0 0 0 .3 ;";
        leaveFocusEvent.name = "leaveFocus";
        leaveFocusEvent.eventScript = &leaveFocusScript;
        focusEvent.name = "onFocus";
        focusEvent.eventScript = &focusScript;
        focusEvent.next = &leaveFocusEvent;
        botItems[1].onEvent = &focusEvent;
        botItems[2].onEvent = &focusEvent;
        botItems[5].onEvent = &focusEvent;
        botItems[5].dvar = "scr_num_bots_enemy";
        botFocus[5].focusTypeData.multi = &enemyCount;
        friendlyCount.count = 9; // Eight bots plus the host fill a nine-player team.
        enemyCount.count = 10;
        botFocus[1].focusTypeData.multi = &botCount;
        botCount.count = 18;
        for (int i = 0; i < 18; ++i)
        {
            Com_sprintf(botCountLabels[i], sizeof(botCountLabels[i]), i ? "< BOTS: %d >" : "< BOTS: OFF >", i);
            botCount.dvarList[i] = botCountLabels[i];
            botCount.dvarValue[i] = (float)i;
            Com_sprintf(friendlyLabels[i], sizeof(friendlyLabels[i]), "< FRIENDLY BOTS: %d >", i);
            Com_sprintf(enemyLabels[i], sizeof(enemyLabels[i]), "< ENEMY BOTS: %d >", i);
            friendlyCount.dvarList[i] = friendlyLabels[i];
            enemyCount.dvarList[i] = enemyLabels[i];
            friendlyCount.dvarValue[i] = enemyCount.dvarValue[i] = (float)i;
        }
        botItems[2].dvar = "scr_bot_difficulty";
        botFocus[2].focusTypeData.multi = &botDifficulty;
        botDifficulty.count = 4;
        botDifficulty.strDef = 1;
        const char *values[] = {"easy", "normal", "hard", "fu"};
        const char *labelsByDifficulty[] = {"< DIFFICULTY: RECRUIT >", "< DIFFICULTY: REGULAR >", "< DIFFICULTY: HARDENED >", "< DIFFICULTY: VETERAN >"};
        for (int i = 0; i < 4; ++i)
        {
            botDifficulty.dvarStr[i] = values[i];
            botDifficulty.dvarList[i] = labelsByDifficulty[i];
        }
        closeScript.action = "play uin_navigation_menu_lg_close ; close self ;";
        closeEvent.name = "action";
        closeEvent.eventScript = &closeScript;
        closeEvent.next = &focusEvent;
        botItems[4].onEvent = &closeEvent;
        openScript.action = "activateBlur ; play uin_navigation_menu_lg_open ; setfocus bots_count ;";
        openEvent.name = "onOpen";
        openEvent.eventScript = &openScript;
        menuCloseScript.action = "deactivateBlur ;";
        menuCloseEvent.name = "onClose";
        menuCloseEvent.eventScript = &menuCloseScript;
        openEvent.next = &menuCloseEvent;
        botMenu.onEvent = &openEvent;
        escapeKey.key = 27;
        escapeKey.keyScript = &closeScript;
        botMenu.onKey = &escapeKey;
    }
}

void UI_PrepareCustomBotMenu()
{
    const char *gametype = Dvar_GetString("ui_gametype");
    if (!I_strnicmp(gametype, "hc", 2))
        gametype += 2;
    // Stock free-for-all and party modes have no friendly team. The script
    // manager uses level.teambased as the authoritative in-match decision.
    const bool teams = I_stricmp(gametype, "dm") && I_stricmp(gametype, "gun")
        && I_stricmp(gametype, "oic") && I_stricmp(gametype, "hlnd") && I_stricmp(gametype, "shrp");
    botItems[1].dvar = teams ? "scr_num_bots_friendly" : "scr_num_bots";
    botFocus[1].focusTypeData.multi = teams ? &friendlyCount : &botCount;
    botItems[5].window.dynamicFlags[0] = teams ? 4 : 0;
    botMenu.window.rect.h = teams ? 284.0f : 244.0f;
    botMenu.window.rect.y = -botMenu.window.rect.h * 0.5f;
    botMenu.window.rectClient = botMenu.initialRectInfo = botMenu.window.rect;
    const float offset = teams ? 0.0f : 40.0f;
    botItems[2].window.rectClient.y = 148.0f - offset;
    botItems[3].window.rectClient.y = 190.0f - offset;
    botItems[4].window.rectClient.y = 238.0f - offset;
    botItems[7].window.rectClient.y = 226.0f - offset;
    botText[3].text = teams ? "Bots join after you choose a team. Max 17 bots." : "Bots join after you enter the match.";
    for (int i = 0; i < ARRAY_COUNT(botItems); ++i)
    {
        botItems[i].window.rect = botItems[i].window.rectClient;
        memset(botText[i].textRect, 0, sizeof(botText[i].textRect));
    }
}

bool UI_IsCustomBotMenu(const menuDef_t *menu)
{
    return menu == &botMenu;
}

void UI_AddCustomBotSettings(int localClientNum, UiContext *dc, menuDef_t *menu)
{
    if (I_stricmp(menu->window.name, "menu_xboxlive_privatelobby"))
        return;
    _Dvar_RegisterInt("scr_num_bots", 0, 0, 17, 1u, "Number of bots in custom matches (zero disables bots)");
    _Dvar_RegisterInt("scr_num_bots_friendly", 0, 0, 8, 1u, "Friendly bots in team custom matches");
    _Dvar_RegisterInt("scr_num_bots_enemy", 0, 0, 9, 1u, "Enemy bots in team custom matches");
    if (Menu_FindItemByName(menu, "openblops_bot_settings_button"))
    {
        UI_AddMenu(localClientNum, dc, &botMenu, 1);
        return;
    }
    for (int i = 0; i < menu->itemCount; ++i)
    {
        const itemDef_s *source = menu->items[i];
        if (source->type != 3 || !source->typeData.textDef)
            continue;
        for (GenericEventHandler *event = source->onEvent; event; event = event->next)
        {
            if (I_stricmp(event->name, "action"))
                continue;
            for (GenericEventScript *script = event->eventScript; script; script = script->next)
            {
                if (!script->action || !strstr(script->action, "open server_settings"))
                    continue;
                InitBotMenu(source);
                lobbyButtons.emplace_back();
                LobbyButton &entry = lobbyButtons.back();
                entry.item = *source;
                entry.item.window.name = "openblops_bot_settings_button";
                // Retail leaves this row between Killstreaks and Playercard.
                entry.item.window.rect.y = entry.item.window.rectClient.y = 174.0f;
                entry.item.rectExpData = nullptr;
                entry.item.animInfo = nullptr;
                entry.item.window.style = 1;
                entry.item.window.border = 1;
                entry.item.window.borderSize = 1.0f;
                entry.item.window.borderColor[3] = 0.0f;
                entry.text = *source->typeData.textDef;
                entry.text.text = "BOT SETTINGS";
                entry.text.textExpData = nullptr;
                entry.text.textTypeData.focusItemDef = &entry.focus;
                entry.item.typeData.textDef = &entry.text;
                entry.script.action = "play uin_navigation_click ; open openblops_bot_settings ;";
                entry.event.name = "action";
                entry.event.eventScript = &entry.script;
                entry.event.next = &focusEvent;
                entry.item.onEvent = &entry.event;
                entry.items.assign(menu->items, menu->items + menu->itemCount);
                entry.items.push_back(&entry.item);
                menu->items = entry.items.data();
                ++menu->itemCount;
                UI_AddMenu(localClientNum, dc, &botMenu, 1);
                return;
            }
        }
    }
    Com_PrintWarning(13, "Custom bots: private lobby has no Server Settings button to extend.\n");
}

#endif
