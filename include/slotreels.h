#ifndef SLOTREELS_H
#define SLOTREELS_H
#include "raylib.h"
#include "jokersgambit.h" 
#include "gamepad_sdl.h"  
typedef struct
{
    Rank rank;
    Suit suit;
} Symbol;
#define MAX_DECK_SIZE 52
#define REELS_COUNT 5
#define VISIBLE_SYMBOLS 3
#define SYMBOL_SIZE (CARD_H_SCALED * 1.1f)
#define REEL_WIDTH (CARD_W_SCALED * 1.1f)
#define REEL_GAP 20.0f
#define REEL_START_X (CENTER_X - (REELS_COUNT * REEL_WIDTH + (REELS_COUNT - 1) * REEL_GAP) / 2.0f)
#define REEL_START_Y (SCREEN_H * 0.15f)
#define SPIN_DURATION 2.5f
#define STAGGER_DELAY 0.15f
#define MAX_GAMBLE_STEPS 5
#define BET_1_TOKENS 1
#define BET_2_TOKENS 2
#define BET_3_TOKENS 3
typedef enum
{
    SLOT_STATE_INSERT_TOKENS,
    SLOT_STATE_IDLE,
    SLOT_STATE_SPINNING,
    SLOT_STATE_SHOW_WIN,
    SLOT_STATE_GAMBLE
} SlotState;
typedef enum
{
    BET_AMOUNT_1 = 1,
    BET_AMOUNT_5 = 5,
    BET_AMOUNT_10 = 10
} BetAmount;
typedef enum
{
    PAYLINE_TOP = 0,
    PAYLINE_MIDDLE = 1,
    PAYLINE_BOTTOM = 2,
    PAYLINE_ALL = 3
} PaylineSelection;
typedef struct SlotReelsState {
        Symbol symbols[REELS_COUNT][VISIBLE_SYMBOLS + 2];
    float offset_y[REELS_COUNT];
    float target_offset[REELS_COUNT];
    float velocity[REELS_COUNT];
    bool stopped[REELS_COUNT];
    int bet_level;
    double total_win_this_spin;
    SlotState state;
    float spin_timer;
    float win_timer;
    BetAmount bet_amount;
    PaylineSelection payline_mode;
    Symbol final_grid[3][REELS_COUNT]; 
    bool win_lines[3];
    int selected_button; 
    bool hold_reel[REELS_COUNT];
    bool hold_locked[REELS_COUNT]; 
    double gamble_current;
    Symbol gamble_card;      
    Symbol gamble_next_card; 
    Symbol gamble_deck[5];   
    int gamble_steps;        
    bool gamble_can_gamble;
    bool show_gamble_result;
    bool has_inserted_tokens;
} SlotReelsState;
typedef enum
{
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
void InitSlotReels(SlotReelsState *slot);
void UpdateSlotReels(LobbyState *core, SlotReelsState *slot);
PokerHand EvaluateLine(const Symbol hand[REELS_COUNT]);
int GetActivePaylineCount(PaylineSelection mode);
#endif 