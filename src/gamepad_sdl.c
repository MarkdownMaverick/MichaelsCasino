#include "gamepad_sdl.h"
#define P1_PAD 0
#define P2_PAD 1


static GamepadSDL gamepads[MAX_GAMEPADS] = {0};
static bool sdl_initialized = false;
// Initialize SDL gamepad system
void InitGamepadSDL(void)
{
    if (!sdl_initialized)
    {
        SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_JOYSTICK);
        sdl_initialized = true;
        // Scan for connected controllers
        for (int i = 0; i < SDL_NumJoysticks() && i < MAX_GAMEPADS; i++)
        {
            if (SDL_IsGameController(i))
            {
                gamepads[i].controller = SDL_GameControllerOpen(i);
                if (gamepads[i].controller)
                {
                    gamepads[i].connected = true;
                    const char *name = SDL_GameControllerName(gamepads[i].controller);
                    snprintf(gamepads[i].name, sizeof(gamepads[i].name), "%s", name ? name : "Unknown");
                }
            }
        }
    }
}
// Check if gamepad is available
bool IsGamepadAvailableSDL(int gamepad)
{
    if (gamepad < 0 || gamepad >= MAX_GAMEPADS)
        return false;
    return gamepads[gamepad].connected;
}
// Get gamepad name
const char *GetGamepadNameSDL(int gamepad)
{
    if (gamepad < 0 || gamepad >= MAX_GAMEPADS || !gamepads[gamepad].connected)
        return "No Controller";
    return gamepads[gamepad].name;
}
// Button checks (current state)
bool IsGamepadButtonDownSDL(int gamepad, int button)
{
    if (!IsGamepadAvailableSDL(gamepad))
        return false;
    SDL_GameControllerButton sdl_button;
    switch (button)
    {
    case 0:
        sdl_button = SDL_CONTROLLER_BUTTON_A;
        break;
    case 1:
        sdl_button = SDL_CONTROLLER_BUTTON_B;
        break;
    case 2:
        sdl_button = SDL_CONTROLLER_BUTTON_X;
        break;
    case 3:
        sdl_button = SDL_CONTROLLER_BUTTON_Y;
        break;
    case 4:
        sdl_button = SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
        break;
    case 5:
        sdl_button = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
        break;
    case 6:
        sdl_button = SDL_CONTROLLER_BUTTON_BACK;
        break;
    case 7:
        sdl_button = SDL_CONTROLLER_BUTTON_START;
        break;
    case 8:
        sdl_button = SDL_CONTROLLER_BUTTON_GUIDE;
        break;
    case 9:
        sdl_button = SDL_CONTROLLER_BUTTON_LEFTSTICK;
        break;
    case 10:
        sdl_button = SDL_CONTROLLER_BUTTON_RIGHTSTICK;
        break;
    case 11:
        sdl_button = SDL_CONTROLLER_BUTTON_DPAD_UP;
        break;
    case 12:
        sdl_button = SDL_CONTROLLER_BUTTON_DPAD_DOWN;
        break;
    case 13:
        sdl_button = SDL_CONTROLLER_BUTTON_DPAD_LEFT;
        break;
    case 14:
        sdl_button = SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
        break;
    default:
        return false;
    }
    return SDL_GameControllerGetButton(gamepads[gamepad].controller, sdl_button) != 0;
}
// Button pressed (edge detection - true only on the frame the button was pressed)
bool XboxBtnPressed(int gamepad, int button)
{
    if (!IsGamepadAvailableSDL(gamepad))
        return false;
    if (button < 0 || button >= 15)
        return false;
    return gamepads[gamepad].button_state[button] && !gamepads[gamepad].button_prev[button];
}
// Axis value (normalized to -1.0 to 1.0)
float GetGamepadAxisSDL(int gamepad, int axis)
{
    if (!IsGamepadAvailableSDL(gamepad))
        return 0.0f;
    SDL_GameControllerAxis sdl_axis;
    switch (axis)
    {
    case 0:
        sdl_axis = SDL_CONTROLLER_AXIS_LEFTX;
        break;
    case 1:
        sdl_axis = SDL_CONTROLLER_AXIS_LEFTY;
        break;
    case 2:
        sdl_axis = SDL_CONTROLLER_AXIS_RIGHTX;
        break;
    case 3:
        sdl_axis = SDL_CONTROLLER_AXIS_RIGHTY;
        break;
    case 4:
        sdl_axis = SDL_CONTROLLER_AXIS_TRIGGERLEFT;
        break;
    case 5:
        sdl_axis = SDL_CONTROLLER_AXIS_TRIGGERRIGHT;
        break;
    default:
        return 0.0f;
    }
    return (float)SDL_GameControllerGetAxis(gamepads[gamepad].controller, sdl_axis) / 32767.0f;
}
// Update gamepad connections (call once per frame)
void UpdateGamepadSDL(void)
{
    // Save previous button states
    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        if (gamepads[i].connected)
        {
            for (int b = 0; b < 15; b++)
            {
                gamepads[i].button_prev[b] = gamepads[i].button_state[b];
            }
        }
    }
    // Handle SDL events
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_CONTROLLERDEVICEADDED)
        {
            int idx = event.cdevice.which;
            if (idx < MAX_GAMEPADS && !gamepads[idx].connected)
            {
                gamepads[idx].controller = SDL_GameControllerOpen(idx);
                if (gamepads[idx].controller)
                {
                    gamepads[idx].connected = true;
                    const char *name = SDL_GameControllerName(gamepads[idx].controller);
                    snprintf(gamepads[idx].name, sizeof(gamepads[idx].name), "%s", name ? name : "Unknown");
                }
            }
        }
        else if (event.type == SDL_CONTROLLERDEVICEREMOVED)
        {
            for (int i = 0; i < MAX_GAMEPADS; i++)
            {
                if (gamepads[i].connected &&
                    SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamepads[i].controller)) == event.cdevice.which)
                {
                    SDL_GameControllerClose(gamepads[i].controller);
                    gamepads[i].connected = false;
                    gamepads[i].controller = NULL;
                }
            }
        }
    }
    // Update current button states
    for (int i = 0; i < MAX_GAMEPADS; i++)
    {
        if (gamepads[i].connected)
        {
            for (int b = 0; b < 15; b++)
            {
                gamepads[i].button_state[b] = IsGamepadButtonDownSDL(i, b);
            }
        }
    }
}