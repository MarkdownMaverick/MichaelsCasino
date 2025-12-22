#ifndef MAIN_H
#define MAIN_H

#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
// UI Layout constants
#define SCREEN_W 1900.0f
#define SCREEN_H 1080.0f
#define CENTER_X (SCREEN_W / 2.0f)
#define EPSILON 0.0001f

// File paths
#define ACCOUNTS_FILE "accounts.json"
#define LEADERBOARD_FILE "leaderboard.json"

// Account limits
#define MAX_ACCOUNTS 5
#define MAX_ACCOUNT_NAME_LEN 12
#define MAX_LEADERBOARD_ENTRIES 100
#define MAX_LEADERBOARD_ENTRY_NAME_LEN 64
#define MAX_LEADERBOARD_WINNER_NAME_LEN 32
#define MAX_LEADERBOARD_TIMESTAMP_LEN 32

// Shop constants
#define TOKEN_PRICE 10.0

// AI Types
typedef enum {
    AI_BOB = 0,
    AI_THEA = 1,
    AI_FLINT = 2
} AIType;

// Game Types
typedef enum {
    GAME_JOKERS_GAMBIT = 0,
    GAME_BLACKJACK = 1,
    GAME_SLOT_REELS = 2
} GameType;

// UI States
typedef enum {
    STATE_MAIN_MENU,
    STATE_LOBBY,
    STATE_ACCOUNTS_MANAGER,
    STATE_SETTINGS,
    STATE_LEADERBOARD,
    STATE_SHOP
} UIState;

// Account structure
typedef struct {
    char first_name[MAX_ACCOUNT_NAME_LEN + 1];
    char last_name[MAX_ACCOUNT_NAME_LEN + 1];
    double credits;
    double tokens;
    int wins;
    int losses;
    bool is_ai;
    AIType ai_type;
    bool is_active;
    bool is_logged_in;
} Account;

// Leaderboard Entry
typedef struct {
    float total_winnings;
    float final_credits;
    float bonus;
    int total_rounds;
    int moves_made;
    GameType game_played;
    char entry_name[MAX_LEADERBOARD_ENTRY_NAME_LEN];
    char winner_name[MAX_LEADERBOARD_WINNER_NAME_LEN];
    char timestamp[MAX_LEADERBOARD_TIMESTAMP_LEN];
} LeaderboardEntry;

// Main Game State
typedef struct {
    UIState state;
    
    // Navigation
    int menu_selection; // Tracks which button is highlighted (0 = top/left)

    // Accounts
    Account accounts[MAX_ACCOUNTS];
    int account_count;
    int p1_account_index;
    int p2_account_index;
    
    // Account status message
    char account_status_message[256];
    double account_status_timer;
    
    // Settings
    bool cover_p2_cards;
    float ai_move_delay;
    
    // Leaderboard
    LeaderboardEntry leaderboard[MAX_LEADERBOARD_ENTRIES];
    int leaderboard_count;
    bool leaderboard_loaded;
    GameType leaderboard_game_filter;
    int leaderboard_sort_mode; // 0=cash, 1=moves, 2=date
    bool leaderboard_sort_ascending;
} LobbyState;

// Global sound (declare extern if defined elsewhere)
extern Music g_background_music;
int GetActiveGamepad(void);
void DebugControllerInput(void);
#endif // MAIN_H