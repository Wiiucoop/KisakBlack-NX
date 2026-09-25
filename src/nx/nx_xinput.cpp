// nx_xinput.cpp -- XInput over libnx HID. The game's existing gamepad layer
// (win_gamepad.cpp GPad_*) drives Switch controllers through this.
#include <XInput.h>
#include <switch.h>

#define NX_ERROR_DEVICE_NOT_CONNECTED 1167

static PadState s_pad;
static bool s_padInit;
static u32 s_packet;

// Reads HID and reports whether a controller is attached. padInitializeDefault
// merges player 1 and handheld mode, so docked pads and the attached Joy-Cons
// both count. GPad_Check (via XInputGetCapabilities) runs before
// XInputGetState every frame, so each entry point polls on its own; only
// padGetButtons is used, which is unaffected by polling twice.
static bool nxPadPoll(void)
{
    if (!s_padInit) {
        padConfigureInput(1, HidNpadStyleSet_NpadStandard);
        padInitializeDefault(&s_pad);
        s_padInit = true;
    }
    padUpdate(&s_pad);
    return padIsConnected(&s_pad);
}

static SHORT nxStick(s32 v)
{
    if (v > 32767) v = 32767;
    if (v < -32768) v = -32768;
    return (SHORT)v;
}

DWORD XInputGetState(DWORD userIndex, XINPUT_STATE *state)
{
    if (userIndex > 0 || !state || !nxPadPoll())
        return NX_ERROR_DEVICE_NOT_CONNECTED;

    memset(state, 0, sizeof(*state));
    XINPUT_GAMEPAD *gp = &state->Gamepad;
    u64 keys = padGetButtons(&s_pad);

    // Face buttons are mapped by POSITION, not by label. The game is written
    // for an Xbox pad, where A (bottom) confirms and B (right) cancels, and
    // Nintendo swaps both pairs: bottom is B, right is A, left is Y, top is X.
    // Mapping by position keeps "bottom confirms, right cancels" and every
    // other face-button binding under the thumb where the game expects it.
    // Consequence: on-screen prompts still show Xbox glyphs, so a prompt that
    // says "A" means the Switch B button and "X" means the Switch Y button.
    WORD buttons = 0;
    if (keys & HidNpadButton_B) buttons |= XINPUT_GAMEPAD_A; // bottom
    if (keys & HidNpadButton_A) buttons |= XINPUT_GAMEPAD_B; // right
    if (keys & HidNpadButton_Y) buttons |= XINPUT_GAMEPAD_X; // left
    if (keys & HidNpadButton_X) buttons |= XINPUT_GAMEPAD_Y; // top
    if (keys & HidNpadButton_Plus) buttons |= XINPUT_GAMEPAD_START;
    if (keys & HidNpadButton_Minus) buttons |= XINPUT_GAMEPAD_BACK;
    if (keys & HidNpadButton_L) buttons |= XINPUT_GAMEPAD_LEFT_SHOULDER;
    if (keys & HidNpadButton_R) buttons |= XINPUT_GAMEPAD_RIGHT_SHOULDER;
    if (keys & HidNpadButton_StickL) buttons |= XINPUT_GAMEPAD_LEFT_THUMB;
    if (keys & HidNpadButton_StickR) buttons |= XINPUT_GAMEPAD_RIGHT_THUMB;
    if (keys & HidNpadButton_Up) buttons |= XINPUT_GAMEPAD_DPAD_UP;
    if (keys & HidNpadButton_Down) buttons |= XINPUT_GAMEPAD_DPAD_DOWN;
    if (keys & HidNpadButton_Left) buttons |= XINPUT_GAMEPAD_DPAD_LEFT;
    if (keys & HidNpadButton_Right) buttons |= XINPUT_GAMEPAD_DPAD_RIGHT;
    gp->wButtons = buttons;

    // ZL/ZR are digital on every Switch controller: fully released or fully
    // pulled, never in between.
    gp->bLeftTrigger = (keys & HidNpadButton_ZL) ? 255 : 0;
    gp->bRightTrigger = (keys & HidNpadButton_ZR) ? 255 : 0;

    // HID sticks span +-32767 with +Y up, the same convention as XInput.
    HidAnalogStickState l = padGetStickPos(&s_pad, 0);
    HidAnalogStickState r = padGetStickPos(&s_pad, 1);
    gp->sThumbLX = nxStick(l.x);
    gp->sThumbLY = nxStick(l.y);
    gp->sThumbRX = nxStick(r.x);
    gp->sThumbRY = nxStick(r.y);

    state->dwPacketNumber = ++s_packet;
    return ERROR_SUCCESS;
}

DWORD XInputSetState(DWORD userIndex, XINPUT_VIBRATION *vibration)
{
    (void)userIndex;
    (void)vibration; // TODO: HidVibration
    return ERROR_SUCCESS;
}

DWORD XInputGetCapabilities(DWORD userIndex, DWORD, XINPUT_CAPABILITIES *caps)
{
    // GPad_Check derives GamePad::enabled from this return value, which in
    // turn fires the game's inserted/removed callbacks.
    if (userIndex > 0 || !nxPadPoll())
        return NX_ERROR_DEVICE_NOT_CONNECTED;
    if (caps) {
        memset(caps, 0, sizeof(*caps));
        caps->Type = 1;    // XINPUT_DEVTYPE_GAMEPAD
        caps->SubType = 1; // XINPUT_DEVSUBTYPE_GAMEPAD
        caps->Gamepad.wButtons = 0xFFFF;
        caps->Gamepad.bLeftTrigger = 255;
        caps->Gamepad.bRightTrigger = 255;
        caps->Gamepad.sThumbLX = 32767;
        caps->Gamepad.sThumbLY = 32767;
        caps->Gamepad.sThumbRX = 32767;
        caps->Gamepad.sThumbRY = 32767;
        caps->Vibration.wLeftMotorSpeed = 65535;
        caps->Vibration.wRightMotorSpeed = 65535;
    }
    return ERROR_SUCCESS;
}

void XInputEnable(BOOL) {}

// The first finger on the touchscreen, in screen pixels -- 1280x720, the same
// space as the game window -- for the menu cursor (IN_Frame, win_input.cpp).
// False when nothing touches it, which is always the case docked.
extern "C" bool NX_TouchPoint(int *x, int *y)
{
    static bool s_touchInit;
    if (!s_touchInit) {
        hidInitializeTouchScreen();
        s_touchInit = true;
    }
    HidTouchScreenState state = {};
    if (!hidGetTouchScreenStates(&state, 1) || state.count <= 0)
        return false;
    *x = (int)state.touches[0].x;
    *y = (int)state.touches[0].y;
    return true;
}
