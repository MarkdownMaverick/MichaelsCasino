#ifndef GAMEPAD_SDL_H
#define GAMEPAD_SDL_H
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdio.h>
#define MAX_GAMEPADS 4
typedef struct {
    SDL_GameController* controller;
    bool connected;
    char name[128];
    bool button_state[15];      
    bool button_prev[15];       
} GamepadSDL;
typedef enum GamepadButtonIndex {
    BTN_A = 0,
    BTN_B,
    BTN_X,
    BTN_Y,
    BTN_LB,
    BTN_RB,
    BTN_MENU,
    BTN_START,
    BTN_XBOX,
    BTN_LS,
    BTN_RS,
    BTN_UP,
    BTN_DOWN,
    BTN_LEFT,
    BTN_RIGHT
} GamepadButtonIndex;
void InitGamepadSDL(void);
bool IsGamepadAvailableSDL(int gamepad);
const char* GetGamepadNameSDL(int gamepad);
bool IsGamepadButtonDownSDL(int gamepad, int button);
bool XboxBtnPressed(int gamepad, int button);
float GetGamepadAxisSDL(int gamepad, int axis);
void UpdateGamepadSDL(void);
#endif 