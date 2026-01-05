#ifndef REELSUI_H
#define REELSUI_H
#include "main.h"       
#define BTN_WIDTH 120.0f
#define BTN_HEIGHT 60.0f
#define BTN_SPACING 15.0f
#define SPIN_BTN_WIDTH 200.0f
#define SPIN_BTN_HEIGHT 100.0f
#define P1_UI_X 10
#define HOLD_BTN_HEIGHT 40.0f
#define BET_BTN_START_X (SCREEN_W * 0.05f)
#define BET_BTN_START_Y (SCREEN_H * 0.5f)
#define LINE_BTN_START_X (SCREEN_W - BET_BTN_START_X - BTN_WIDTH)
#define LINE_BTN_START_Y BET_BTN_START_Y
#define SPIN_BTN_X (CENTER_X - SPIN_BTN_WIDTH / 2.0f)
#define SPIN_BTN_Y (SCREEN_H - SPIN_BTN_HEIGHT - 50.0f)
struct SlotReelsState;
void DrawSlotReels(const LobbyState *core, const struct SlotReelsState *slot);
Rectangle GetHoldButton(int reel_idx);
#endif 