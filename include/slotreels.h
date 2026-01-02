#ifndef SLOTREELS_H
#define SLOTREELS_H

#include "raylib.h"
#include "jokersgambit.h"     // For Rank, Suit, GetAtlasSourceRect, CARD_W_SCALED, etc.
#include "gamepad_sdl.h"      // For XboxBtnPressed
// Slot Reels Constants
#define REELS_COUNT         5
#define VISIBLE_SYMBOLS     3
#define SYMBOL_SIZE         (CARD_H_SCALED * 1.1f)
#define REEL_WIDTH          (CARD_W_SCALED * 1.1f)
#define REEL_GAP            20.0f
#define REEL_START_X        (CENTER_X - (REELS_COUNT * REEL_WIDTH + (REELS_COUNT - 1) * REEL_GAP) / 2.0f)
#define REEL_START_Y        (SCREEN_H * 0.15f)

#define SPIN_DURATION       2.5f
#define STAGGER_DELAY       0.15f

// Bet levels
#define BET_1_TOKENS        1
#define BET_2_TOKENS        2
#define BET_3_TOKENS        3

typedef enum {
    SLOT_STATE_IDLE,
    SLOT_STATE_SPINNING,
    SLOT_STATE_SHOW_WIN
} SlotState;

typedef struct {
    Rank symbols[REELS_COUNT][VISIBLE_SYMBOLS + 2];
    float offset_y[REELS_COUNT];
    float target_offset[REELS_COUNT];
    float velocity[REELS_COUNT];
    bool stopped[REELS_COUNT];

    int bet_level;
    double total_win_this_spin;

    SlotState state;
    float spin_timer;
    float win_timer;

    Rank final_grid[REELS_COUNT][VISIBLE_SYMBOLS];
    bool win_lines[3];
} SlotReelsState;

typedef enum {
    HAND_HIGH_CARD,
    HAND_JACKS_OR_BETTER,
    HAND_TWO_PAIR,
    HAND_THREE_OF_A_KIND,
    HAND_STRAIGHT,
    HAND_FLUSH,
    HAND_FULL_HOUSE,
    HAND_FOUR_OF_A_KIND,
    HAND_STRAIGHT_FLUSH,
    HAND_ROYAL_FLUSH
} PokerHand;

// Function prototypes
void InitSlotReels(SlotReelsState *slot);
void UpdateSlotReels(LobbyState *core, SlotReelsState *slot);
void DrawSlotReels(const LobbyState *core, const SlotReelsState *slot);

#endif // SLOTREELS_H