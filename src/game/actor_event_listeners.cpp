#include "actor_event_listeners.h"
#include <game_mp/g_main_mp.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/cscr_stringlist.h>
#include <game_mp/g_spawn_mp.h>
#include <game_mp/g_misc_mp.h>
#include <clientscript/scr_const.h>

unsigned __int16 *g_AIEV_scrConst_table[28] =
{
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  &scr_const.explode,
  &scr_const.grenadedanger,
  &scr_const.grenadedanger,
  &scr_const.gunshot,
  &scr_const.silenced_shot,
  NULL,
  NULL,
  &scr_const.bulletwhizby,
  &scr_const.bulletwhizby,
  NULL,
  &scr_const.projectile_impact,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};



int g_listenerCount;
AIEventListener g_AIEVlisteners[32];

int __cdecl Actor_FindEventFromString(unsigned __int16 eventString)
{
    for ( int event = 0; event < 28; ++event )
    {
        if ( g_AIEV_scrConst_table[event] && *g_AIEV_scrConst_table[event] == eventString )
            return event;
    }

    Scr_Error(
        va("Unable to find AI event for [%s]", SL_ConvertToString(eventString, SCRIPTINSTANCE_SERVER)),
        SCRIPTINSTANCE_SERVER);
    return 0;
}

void __cdecl Actor_EventListener_Add(int entIndex, unsigned __int16 eventString)
{
    const int event = Actor_FindEventFromString(eventString);
    if ( !event )
        return;

    for ( int listenerIndex = 0; listenerIndex < g_listenerCount; ++listenerIndex )
    {
        if ( g_AIEVlisteners[listenerIndex].entIndex == entIndex )
        {
            Com_BitSetAssert(&g_AIEVlisteners[listenerIndex].events, event, 0xFFFFFFF);
            return;
        }
    }

    if ( g_listenerCount >= 32 )
    {
        Scr_Error(va("Max listeners exceeded; entity id: %d\n", entIndex), SCRIPTINSTANCE_SERVER);
        return;
    }

    AIEventListener &listener = g_AIEVlisteners[g_listenerCount++];
    listener.entIndex = entIndex;
    listener.events = 0;
    Com_BitSetAssert(&listener.events, event, 0xFFFFFFF);
}

void __cdecl RemoveSwapWithLast(unsigned int listenerIndex)
{
    int removeIndex; // [esp+0h] [ebp-4h]

    if ( listenerIndex >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_event_listeners.cpp",
                    176,
                    0,
                    "listenerIndex doesn't index MAX_AI_EV_LISTENERS\n\t%i not in [0, %i)",
                    listenerIndex,
                    32) )
    {
        __debugbreak();
    }
    removeIndex = listenerIndex;
    if ( (int)listenerIndex < --g_listenerCount )
    {
        g_AIEVlisteners[listenerIndex] = g_AIEVlisteners[g_listenerCount];
        removeIndex = g_listenerCount;
    }
    g_AIEVlisteners[removeIndex].entIndex = 1023;
    g_AIEVlisteners[removeIndex].events = 0;
}

void __cdecl Actor_EventListener_RemoveEntity(int entIndex)
{
    signed int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < g_listenerCount; ++i )
    {
        if ( g_AIEVlisteners[i].entIndex == entIndex )
        {
            RemoveSwapWithLast(i);
            return;
        }
    }
}

int __cdecl Actor_EventListener_First(ai_event_t event, int teamFlags)
{
    return Actor_EventListener_Next(-1, event, teamFlags);
}

int __cdecl Actor_EventListener_Next(int index, ai_event_t event, int teamFlags)
{
    int i; // [esp+0h] [ebp-8h]
    sentient_s *sentient; // [esp+4h] [ebp-4h]

    for ( i = index + 1; i < g_listenerCount; ++i )
    {
        if (Com_BitCheckAssert(&g_AIEVlisteners[i].events, event, 0xFFFFFFF))
        {
            sentient = g_entities[g_AIEVlisteners[i].entIndex].sentient;
            if ( !sentient || (teamFlags & (1 << sentient->eTeam)) != 0 )
                return i;
        }
    }
    return -1;
}

gentity_s *__cdecl Actor_EventListener_GetEntity(unsigned int index)
{
    if ( index >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_event_listeners.cpp",
                    287,
                    0,
                    "index doesn't index MAX_AI_EV_LISTENERS\n\t%i not in [0, %i)",
                    index,
                    32) )
    {
        __debugbreak();
    }
    return &g_entities[g_AIEVlisteners[index].entIndex];
}

void __cdecl Actor_EventListener_NotifyToListener(
                gentity_s *listener,
                gentity_s *originator,
                ai_event_t event,
                float *position)
{
    if ( !listener
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_event_listeners.cpp", 302, 0, "%s", "listener") )
    {
        __debugbreak();
    }
    if ( listener->r.inuse )
    {
        Scr_AddVector(position, SCRIPTINSTANCE_SERVER);
        if ( originator )
            Scr_AddEntity(originator, SCRIPTINSTANCE_SERVER);
        else
            Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
        if ( !g_AIEV_scrConst_table[event]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_event_listeners.cpp",
                        314,
                        0,
                        "%s",
                        "g_AIEV_scrConst_table[event]") )
        {
            __debugbreak();
        }
        Scr_Notify(listener, *g_AIEV_scrConst_table[event], 2u);
        ScrNotify_FaceEvent(listener, *g_AIEV_scrConst_table[event]);
    }
}

