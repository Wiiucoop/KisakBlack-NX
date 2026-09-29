#pragma once
#include "actor_events.h"

struct AIEventListener // sizeof=0x8
{
    int entIndex;
    unsigned int events;
};

int __cdecl Actor_FindEventFromString(unsigned __int16 eventString);
void __cdecl Actor_EventListener_Add(int entIndex, unsigned __int16 eventString);
void __cdecl RemoveSwapWithLast(unsigned int listenerIndex);
void __cdecl Actor_EventListener_RemoveEntity(int entIndex);
int __cdecl Actor_EventListener_First(ai_event_t event, int teamFlags);
int __cdecl Actor_EventListener_Next(int index, ai_event_t event, int teamFlags);
gentity_s *__cdecl Actor_EventListener_GetEntity(unsigned int index);
void __cdecl Actor_EventListener_NotifyToListener(
                gentity_s *listener,
                gentity_s *originator,
                ai_event_t event,
                float *position);
