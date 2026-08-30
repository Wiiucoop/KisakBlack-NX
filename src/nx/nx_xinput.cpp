// nx_xinput.cpp -- XInput over libnx HID. The game's existing gamepad layer
// (win_gamepad.cpp GPad_*) drives Switch controllers through this.
#include <XInput.h>
#include <switch.h>

static PadState s_pad;
static bool s_padInit;
static u32 s_packet;

static void nxPadEnsureInit(void)
{
    if (!s_padInit) {
        padConfigureInput(1, HidNpadStyleSet_NpadStandard);
        padInitializeDefault(&s_pad);
        s_padInit = true;
    }
}

static SHORT nxStick(float v)
{
    if (v > 32767.0f) v = 32767.0f;
    if (v < -32768.0f) v = -32768.0f;
    return (SHORT)v;
}

DWORD XInputGetState(DWORD userIndex, XINPUT_STATE *state)
{
    if (userIndex > 0 || !state)
        return 1167; // ERROR_DEVICE_NOT_CONNECTED

    nxPadEnsureInit();
    padUpdate(&s_pad);

    // While the applet loses focus (HOME menu), report the pad as idle.
    u64 keys = padGetButtons(&s_pad);

    XINPUT_GAMEPAD *gp = &state->Gamepad;
    memset(state, 0, sizeof(*state));

    WORD buttons = 0;
    if (keys & HidNpadButton_A) buttons |= XINPUT_GAMEPAD_A; // note: Switch A = east
    if (keys & HidNpadButton_B) buttons |= XINPUT_GAMEPAD_B;
    if (keys & HidNpadButton_X) buttons |= XINPUT_GAMEPAD_X;
    if (keys & HidNpadButton_Y) buttons |= XINPUT_GAMEPAD_Y;
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

    gp->bLeftTrigger = (keys & HidNpadButton_ZL) ? 255 : 0;
    gp->bRightTrigger = (keys & HidNpadButton_ZR) ? 255 : 0;

    HidAnalogStickState l = padGetStickPos(&s_pad, 0);
    HidAnalogStickState r = padGetStickPos(&s_pad, 1);
    gp->sThumbLX = nxStick((float)l.x);
    gp->sThumbLY = nxStick((float)l.y);
    gp->sThumbRX = nxStick((float)r.x);
    gp->sThumbRY = nxStick((float)r.y);

    state->dwPacketNumber = ++s_packet;
    return 0; // ERROR_SUCCESS
}

DWORD XInputSetState(DWORD userIndex, XINPUT_VIBRATION *vibration)
{
    (void)userIndex;
    (void)vibration; // TODO: HidVibration
    return 0;
}

DWORD XInputGetCapabilities(DWORD userIndex, DWORD, XINPUT_CAPABILITIES *caps)
{
    if (userIndex > 0)
        return 1167;
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
    return 0;
}

void XInputEnable(BOOL) {}
