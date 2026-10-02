// nx_clock.h -- the GPU clock in handheld mode (see nx_clock.cpp).
#pragma once

void NX_ClockInit();
// Call often (it checks every couple of seconds): applies nx_gpuclock in
// handheld mode, MHz, 0 for the system's clock.
void NX_ClockUpdate(int handheldMhz);
void NX_ClockExit();
