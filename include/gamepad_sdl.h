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
    bool button_state[15];      // Current button states
    bool button_prev[15];       // Previous button states
} GamepadSDL;

// Function prototypes
void InitGamepadSDL(void);
bool IsGamepadAvailableSDL(int gamepad);
const char* GetGamepadNameSDL(int gamepad);
bool IsGamepadButtonDownSDL(int gamepad, int button);
bool XboxBtnPressed(int gamepad, int button);
float GetGamepadAxisSDL(int gamepad, int axis);
void UpdateGamepadSDL(void);

#endif // GAMEPAD_SDL_H