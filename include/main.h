#ifndef MAIN_H
#define MAIN_H
#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
// Forward declare GameState to avoid circular dependency
typedef struct GameState GameState;
// UI Layout constants - Now dynamic
extern float SCREEN_W;
extern float SCREEN_H;
extern float CENTER_X;
#define EPSILON 0.0001f
// Default resolutions
#define DEFAULT_SCREEN_W 1920.0f
#define DEFAULT_SCREEN_H 1080.0f
// File paths
#define ACCOUNTS_FILE "save/accounts.json"
#define LEADERBOARD_FILE "save/leaderboard.json"
#define SETTINGS_FILE "save/settings.json"
// Account limits
#define MAX_ACCOUNTS 5
#define MAX_ACCOUNT_NAME_LEN 12
#define MAX_LEADERBOARD_ENTRIES 100
#define MAX_LEADERBOARD_ENTRY_NAME_LEN 64
#define MAX_LEADERBOARD_WINNER_NAME_LEN 32
#define MAX_LEADERBOARD_TIMESTAMP_LEN 32
// Achievement Constants
#define MAX_ACHIEVEMENTS 50
#define ACHIEVEMENT_NAME_LEN 32
#define ACHIEVEMENT_DESC_LEN 64
// Shop constants
#define TOKEN_PRICE 1000.0
#ifndef CLAMP
#define CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))
#endif
// --- NETWORK STRUCTURES ---
typedef enum
{
    NET_NONE,
    NET_HOST,
    NET_CLIENT
} NetworkRole;

typedef enum
{
    PKT_HANDSHAKE,
    PKT_SEED,    // Host sends RNG seed to client
    PKT_DISCARD, // Send discard index
    PKT_PLACE,   // Send placement hand index
    PKT_PASS,    // Pass turn
    PKT_RESTART
} PacketType;

typedef struct
{
    PacketType type;
    int data; // Generic data (card index, seed, etc.)
} NetPacket;
// Window scale options
typedef enum
{
    SCALE_100 = 0,
    SCALE_125 = 1,
    SCALE_150 = 2,
    SCALE_75 = 3
} WindowScale;
// AI Types
typedef enum
{
    AI_BOB = 0,
    AI_THEA = 1,
    AI_FLINT = 2
} AIType;
typedef enum
{
    AI_DELAY_FAST = 0, // 0.5s
    AI_DELAY_NORMAL,   // 1.0s
    AI_DELAY_SLOW      // 2.0s
} AIDelayMode;
// Member status struct
typedef enum
{
    MEMBER = 0,
    SUPPORTER,
    ADVOCATE,
    PROMOTER,
    CHAMPION,
    VIP,
    ELITE,
    LEGEND,
    HONORARY,
    PRESIDENTIAL
} MEMBERSTATUS;
typedef struct
{
    MEMBERSTATUS status;
    int credits;
    int tokens;
    int win_streak;
    int total_wins;
} PLAYER_STATUS;
// Game Types
typedef enum
{
    GAME_JOKERS_GAMBIT = 0,
    GAME_BLACKJACK = 1,
    GAME_SLOT_REELS = 2
} GameType;
// UI States
typedef enum
{
    STATE_MAIN_MENU,
    STATE_LOBBY,
    STATE_ACCOUNTS_MANAGER,
    STATE_SETTINGS,
    STATE_LEADERBOARD,
    STATE_SHOP,
    STATE_ACHIEVEMENTS,
    STATE_MODE_SELECTION, // NEW
    STATE_AI_SELECTION,   // NEW
    STATE_AI_P2_SELECTION,
    STATE_JOKERS_GAMBIT, // NEW
    STATE_BETTING_SETUP,
    STATE_SLOT_REELS,
    STATE_PVP_SETUP_P1,
    STATE_PVP_SETUP_P2,
    MULTIPLAYER,
    STATE_ONLINE_CHOICE
} UIState;
typedef struct
{
    int total_games_played;
    int total_wins;
    int total_losses;
    double total_winnings_cash; // Cumulative cash won
    double highest_single_win;  // Biggest win in one go
    // Game Specific
    bool jg_matched_jokers;        // Set when both jokers matched in a game
    int jg_fastest_win;            // Lowest round count to win (-1 if none)
    bool jg_filled_all_five_ranks; // True if player ever filled all 5 ranks
    bool cs_had_flush;
    bool cs_had_four_kind;
    bool cs_had_full_house;
    bool cs_had_straight_flush;
    bool cs_had_royal_flush;
    int bj_hands_played;
    int bj_wins;
    int bj_losses;
    int jg_hands_played; // Joker's Gambit
    int jg_wins;
    int slots_spins;
    int slots_wins;
    int current_win_streak;
    int best_win_streak;
} PlayerStats;
// 3. Define the Achievement Structure
typedef struct
{
    char name[ACHIEVEMENT_NAME_LEN];
    char description[ACHIEVEMENT_DESC_LEN];
    bool unlocked;
    // We don't strictly need a 'target' variable here if we hardcode logic,
    // but it keeps the struct clean.
} Achievement;
typedef struct
{
    char name[ACHIEVEMENT_NAME_LEN];
    char description[ACHIEVEMENT_DESC_LEN];
    int target;
} AchievementDef;
typedef struct
{
    char first_name[MAX_ACCOUNT_NAME_LEN + 1];
    char last_name[MAX_ACCOUNT_NAME_LEN + 1];
    double credits;
    double tokens;
    int wins;
    int losses;
    PlayerStats stats;
    Achievement achievements[MAX_ACHIEVEMENTS];
    bool is_ai;
    AIType ai_type;
    bool is_active;
    bool is_logged_in;
    MEMBERSTATUS member_status;
    bool has_insurance;
    bool active_bet;
} Account;
// Leaderboard Entry
typedef struct
{
    float total_winnings;
    float final_credits;
    float bonus;
    int total_rounds;
    GameType game_played;
    char entry_name[MAX_LEADERBOARD_ENTRY_NAME_LEN];
    char winner_name[MAX_LEADERBOARD_WINNER_NAME_LEN];
    char timestamp[MAX_LEADERBOARD_TIMESTAMP_LEN];
} LeaderboardEntry;
typedef struct BettingState
{
    double p1_bet_amount;
    double p2_bet_amount;
    int selected_matchup; // 0=FLINT vs THEA, 1=BOB vs FLINT, 2=THEA vs BOB, 3=RANDOM
    bool bet_placed;
    double payout_multiplier;
    double net_profit;
    bool player_won_bet;
    int bet_on_player; // 1 or 2
    int original_player;
} BettingState;
// Main Game State
typedef struct
{
    UIState state;
    // Navigation
    int menu_selection;
    // Accounts
    Account accounts[MAX_ACCOUNTS];
    int account_count;
    int p1_account_index;
    int p2_account_index;
    bool editing_name;
    char edit_first_name[MAX_ACCOUNT_NAME_LEN];
    char edit_last_name[MAX_ACCOUNT_NAME_LEN];
    int edit_cursor_pos; // For blinking cursor
    double edit_timer;
    // Account status message
    char account_status_message[256];
    double account_status_timer;
    // Settings
    bool cover_p2_cards;
    AIDelayMode ai_delay_mode;
    bool music_enabled;
    WindowScale window_scale;
    bool is_fullscreen;
    // Leaderboard
    LeaderboardEntry leaderboard[MAX_LEADERBOARD_ENTRIES];
    int leaderboard_count;
    bool leaderboard_loaded;
    GameType leaderboard_game_filter;
    int leaderboard_sort_mode; // 0=cash, 1=moves, 2=date
    bool leaderboard_sort_ascending;
    // Achievements
    int achievement_scroll_offset;
    int achievement_view_mode; // 0 = P1, 1 = P2
    int achievement_cursor_col;
    int achievement_cursor_row;
    int achievement_scroll_row;
    bool debug_force_check_achievements;
    // NOTIFICATION SYSTEM
    char notification_text[64];    // Text to display
    char notification_subtext[64]; // Optional subtext (e.g. "Achievement Unlocked!")
    double notification_timer;     // How long to show it
    float notification_y_offset;   // For sliding animation
    // Control account login/out for ai
    AIType selected_p1_ai;
    AIType selected_p2_ai;
    GameState *game_state; // Embedded game state for Joker's Gambit
    BettingState betting;
    bool pvp_dual_window; // Set in mode selection if 2 pads detected + user chooses
    int p1_input_device;  // 0 = keyboard/mouse, 1 = controller 0
    int p2_input_device;  // 0 = keyboard/mouse, 1 = controller 1
    bool pvp_multiplayer; // false = single, true = dual
    bool is_host;
    // Network State
    NetworkRole net_role;
    bool net_connected;
    int net_socket;        // Socket handle
    unsigned int rng_seed; // Seed to sync decks
    double net_timeout;
} LobbyState;
MEMBERSTATUS CalculateMemberStatus(double credits, double tokens);
int GetActiveGamepad(void);
void DebugControllerInput(void);
void UpdateScreenDimensions(void);
void ApplyWindowScale(LobbyState *g);
void ToggleAppFullscreen(LobbyState *g);
void DrawAIP2Selection(const LobbyState *g);
void UpdateAIP2Selection(LobbyState *g, Vector2 mouse);
extern Texture2D g_achievements_atlas;
extern AchievementDef g_achievement_defs[MAX_ACHIEVEMENTS];
extern Sound g_menu_navigate_sound;
#endif // MAIN_H