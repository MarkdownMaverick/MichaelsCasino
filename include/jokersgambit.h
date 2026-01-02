#ifndef JOKERSGAMBIT_H
#define JOKERSGAMBIT_H

#include "main.h"
#include "useraccount.h"
#include <stdint.h>
// UI Layout constants
#define KEYCARD_COL_X (SCREEN_W * 0.469f) // 900/1920 ≈ 0.469
#define GRID_START_Y (SCREEN_H * 0.0185f) // 20/1080 ≈ 0.0185
#define ROW_SPACING (SCREEN_H * 0.157f)   // 180/1080 ≈ 0.157
#define UI_Y (SCREEN_H * 0.6f)
#define BUTTON_W 80
#define BUTTON_H 80
// Game constants / card ui layout
#define HAND_SIZE 5
#define KEYCARDS 5
#define TOTAL_DECK_CARDS 49
#define CARD_SCALE 0.53f
#define CARD_W_SCALED (200 * CARD_SCALE)
#define CARD_H_SCALED (300 * CARD_SCALE)
// UI Spacing Constants
#define UI_PADDING_SMALL 5
#define UI_PADDING_MEDIUM 10
#define UI_PADDING_LARGE 20
// Game constants for economy
#define DEFAULT_CREDITS 10000.00f
#define COST_DISCARD 1.00f
#define COST_PER_ROUND 5.00f
#define REWARD_MATCH 10.00f
#define REWARD_DOUBLE_JOKER 50.00f
#define REWARD_PLACEMENT 2.00f
#define REWARD_COMPLETION 20.00f
#define JOKER_DISCARD 5.00f
#define P1_TEMP_CREDITS_START 100.00f
#define P2_TEMP_CREDITS_START 100.00f
#define LOSER_PENALTY 1000.00f
#define TOKEN_PRICE 1000.0
#define TOKEN_REWARD_GAME_COMPLETE 1.0
#define CREDIT_COST_RESTART 100.00f
#define CREDIT_COST_QUIT_EARLY 100.00f
// Atlas layout constants
#define ATLAS_COLS 13
#define ATLAS_ROWS 5
#define ATLAS_CARD_WIDTH 200
#define ATLAS_CARD_HEIGHT 300
#define ATLAS_TOTAL_WIDTH 2600
#define ATLAS_TOTAL_HEIGHT 1500
#define EPSILON 0.0001f

typedef struct
{
    int row;
    int col;
} AtlasPosition;
typedef enum
{
    RANK_2 = 0,
    RANK_3,
    RANK_4,
    RANK_5,
    RANK_6,
    RANK_7,
    RANK_8,
    RANK_9,
    RANK_10,
    RANK_JACK,
    RANK_QUEEN,
    RANK_KING,
    RANK_ACE,
    RANK_JOKER,
    RANK_BACK,
    RANK_NONE = -1
} Rank;
typedef enum
{
    SUIT_CLUBS,
    SUIT_DIAMONDS,
    SUIT_HEARTS,
    SUIT_SPADES,
    SUIT_NONE
} Suit;
typedef struct
{
    Rank rank;
    Suit suit;
    bool is_valid;
    Texture2D texture;
    char filename[16];
} Card;
typedef enum
{
    STATE_ROUND_START,
    STATE_P1_SELECT_DISCARD,
    STATE_P2_SELECT_DISCARD,
    STATE_REVEAL_AND_RESOLVE,
    STATE_WAIT_FOR_TURN,
    STATE_HAND_RESHUFFLE,
    STATE_COVER_ANIMATION,
    STATE_ROUNDS_COMPLETED,
    STATE_CHECK_WIN,
    STATE_GAME_OVER
} GameStateEnum;
typedef enum
{
    MODE_PVP,
    MODE_PVAI,
    MODE_AIVAI,
    MODE_BETTING
} GameMode;

struct GameState
{
    Account accounts[MAX_ACCOUNTS];
    float ai_move_delay;
    Card deck[TOTAL_DECK_CARDS];
    int top_card_index;
    int current_deck_size;
    Card player1_hand[HAND_SIZE];
    Card player2_hand[HAND_SIZE];
    int p1_hand_size;
    int p2_hand_size;
    Card keycards[KEYCARDS];
    Card p1_slots[KEYCARDS][3];
    Card p2_slots[KEYCARDS][3];
    int p1_completed_ranks;
    int p2_completed_ranks;
    bool p1_done_placing;
    bool p2_done_placing;
    int p1_discard_idx;
    bool p1_selected;
    int p2_discard_idx;
    bool p2_selected;
    Card revealed_p1;
    Card revealed_p2;
    Card delay_discard_p1;
    Card delay_discard_p2;
    bool discards_pending;
    bool p1_discard_ready;
    bool p2_discard_ready;
    float Reshuffle_cover_timer;
    GameStateEnum state;
    GameMode mode;
    float ai_timer;
    bool p1_ai_done_placing_rounds;
    float p1_credits;
    float p2_credits;
    float p1_temp_credits;
    float p2_temp_credits;
    float p1_game_winnings;
    float p2_game_winnings;
    int total_rounds;
    int placement_phases_count;
    int account_count;
    int p1_account_index;
    int p2_account_index;
    AIType selected_opponent_ai;
    bool game_over;
    int winner;
    float final_score_p1;
    float final_score_p2;
    double win_timer_start;
    char account_status_message[128];
    double account_status_timer;
    bool cover_p1_cards; // For P2 window (symmetric to cover_p2_cards)
    bool cover_p2_cards;
    bool confirm_restart;
    bool confirm_menu;
    int p1_discard_cursor; // 0-4 for hand navigation
    int p2_discard_cursor;
};
typedef struct
{
    uint8_t player; //
    uint8_t type;   // 0=left, 1=right, 2=action(A/Space), 3=pass(B/C), 4=back
    uint8_t cursor; // 0-4 for hand/discard
} InputEvent;
extern Texture2D g_background_texture;
extern Texture2D g_ui_frame_texture;
extern Texture2D g_button_texture;
extern Sound g_discard_sound;
extern Sound g_place_sound;
extern Sound g_filled_rank_sound;
extern Sound g_reveal_sound;
extern Sound g_win_sound;
extern Sound g_joker_sound;
extern Sound g_matching_jokers_sound;
extern Sound g_matching_cards_sound;
extern Sound g_continue_sound;
extern Sound g_coin_sound;
extern Sound g_shuffle_sound;
extern Texture2D g_atlas_texture;
Rectangle HandRect(int player, int idx);
Rectangle ButtonRect(int player, int idx);
Rectangle SlotRect(int player, int key_idx, int slot_idx);
Rectangle KeyCardRect(int key_idx);
Rectangle DiscardPileRect(int player);
Rectangle ContinueButtonRect(int player);
Rectangle GetCardSourceRect(Card c);
float GetRankY(int key_idx);
// Main game loop functions
void UpdateJokersGambit(LobbyState *core, Vector2 mouse);
void DrawJokersGambit(const LobbyState *core);
// Game state functions
void UpdateDiscardSelection(GameState *g, Vector2 mouse);
void UpdatePlacementPhase(GameState *g, Vector2 mouse);
void CheckWinCondition(GameState *g, LobbyState *core);
// Drawing functions
void DrawGameLayout(LobbyState *core, const GameState *g);
void DrawGameOver(LobbyState *core, GameState *g);
void DrawPlayerUI(LobbyState *core, const GameState *g, int player);
// Helper functions
bool IsPlayerAI(const GameState *g, int player);
void InitGame(LobbyState *core);
void RestartGameKeepingAccounts(LobbyState *core);
void LoadCardAtlas(void);
void UnloadCardAtlas(void);
Rectangle GetAtlasSourceRect(Rank rank, Suit suit);
Rectangle GetAtlasBackCard(void);
Rectangle GetAtlasTempCover(void);
Rectangle GetAtlasJoker(int joker_index);
Rectangle GetDealingAnimationRect(void);
Rectangle AtlasRect(int row, int col);
void DrawCard(Card c, Rectangle rect, Color tint);
void DrawButton(Rectangle rect, bool is_hovered, bool is_enabled, const char *text);
void DrawContinueButtons(const GameState *g, Vector2 mouse);
Card DrawFromDeck(GameState *g);
void CheckRankCompletionBonus(GameState *g, int player, int key_idx, int cards_before);
float GetRewardMultiplier(int completed_ranks);
Card BlankCard(void);
float CalculateFinalScore(float current_credits, float temp_credits, int total_rounds, bool is_winner, bool is_loser);
void ProcessPendingDiscards(GameState *g);
void ResolveDiscards(GameState *g);
void RefreshHands(GameState *g);
void UpdateWinStats(GameState *g);
void ReturnToDeck(GameState *g, Card c);
void AddLeaderboardEntry(LobbyState *core, int winner);
void UpdateScale(void);
void StartPVPGame(LobbyState *g);
// Network multiplayer
#endif