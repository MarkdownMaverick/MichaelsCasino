#ifndef MAIN_H
#define MAIN_H
#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
typedef struct GameState GameState;
extern float SCREEN_W;
extern float SCREEN_H;
extern float CENTER_X;
#define EPSILON 0.0001f
#define DEFAULT_SCREEN_W 1920.0f
#define DEFAULT_SCREEN_H 1080.0f
#define ACCOUNTS_FILE "save/accounts.json"
#define LEADERBOARD_FILE "save/leaderboard.json"
#define SETTINGS_FILE "save/settings.json"
#define MAX_ACCOUNTS 5
#define MAX_ACCOUNT_NAME_LEN 12
#define MAX_LEADERBOARD_ENTRIES 100
#define MAX_LEADERBOARD_ENTRY_NAME_LEN 64
#define MAX_LEADERBOARD_WINNER_NAME_LEN 32
#define MAX_LEADERBOARD_TIMESTAMP_LEN 32
#define MAX_ACHIEVEMENTS 50
#define ACHIEVEMENT_NAME_LEN 32
#define ACHIEVEMENT_DESC_LEN 64
#define TOKEN_PRICE 1000.0
#ifndef CLAMP
#define CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))
#endif
typedef enum
{
    NET_NONE,
    NET_HOST,
    NET_CLIENT
} NetworkRole;
typedef enum
{
    PKT_HANDSHAKE,
    PKT_SEED,    
    PKT_P1_DISCARD,      
    PKT_P2_DISCARD,      
    PKT_DISCARD_RESOLVE,   
    PKT_P1_PLACE,        
    PKT_P2_PLACE,        
    PKT_PASS,    
    PKT_RESTART,
    PKT_PLAYER_NAME,
    PKT_CLIENT_INFO,
    PKT_WIN
} PacketType;
typedef struct
{
    PacketType type;
    int data; 
} NetPacket;
typedef enum
{
    SCALE_100 = 0,
    SCALE_125 = 1,
    SCALE_150 = 2,
    SCALE_75 = 3
} WindowScale;
typedef enum
{
    AI_BOB = 0,
    AI_THEA = 1,
    AI_FLINT = 2
} AIType;
typedef enum
{
    AI_DELAY_FAST = 0, 
    AI_DELAY_NORMAL,   
    AI_DELAY_SLOW      
} AIDelayMode;
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
typedef enum
{
    GAME_JOKERS_GAMBIT = 0,
    GAME_BLACKJACK = 1,
    GAME_SLOT_REELS = 2
} GameType;
typedef enum
{
    STATE_MAIN_MENU,
    STATE_LOBBY,
    STATE_ACCOUNTS_MANAGER,
    STATE_SETTINGS,
    STATE_LEADERBOARD,
    STATE_SHOP,
    STATE_ACHIEVEMENTS,
    STATE_MODE_SELECTION, 
    STATE_AI_SELECTION,   
    STATE_AI_P2_SELECTION,
    STATE_JOKERS_GAMBIT, 
    STATE_BETTING_SETUP,
    STATE_SLOT_REELS,
    STATE_PVP_SETUP_P1,
    STATE_PVP_SETUP_P2,
    STATE_MULTIPLAYER,
    STATE_ONLINE_CHOICE,
    STATE_HOSTING_WAITING
} UIState;
typedef struct
{
    int total_games_played;
    int total_wins;
    int total_losses;
    double total_winnings_cash; 
    double highest_single_win;  
    bool jg_matched_jokers;        
    int jg_fastest_win;            
    bool jg_filled_all_five_ranks; 
    bool cs_had_flush;
    bool cs_had_four_kind;
    bool cs_had_full_house;
    bool cs_had_straight_flush;
    bool cs_had_royal_flush;
    int bj_hands_played;
    int bj_wins;
    int bj_losses;
    int jg_hands_played; 
    int jg_wins;
    int slots_spins;
    int slots_wins;
    int current_win_streak;
    int best_win_streak;
} PlayerStats;
typedef struct
{
    char name[ACHIEVEMENT_NAME_LEN];
    char description[ACHIEVEMENT_DESC_LEN];
    bool unlocked;
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
    int selected_matchup; 
    bool bet_placed;
    double payout_multiplier;
    double net_profit;
    bool player_won_bet;
    int bet_on_player; 
    int original_player;
} BettingState;
typedef struct
{
    UIState state;
    int menu_selection;
    Account accounts[MAX_ACCOUNTS];
    int account_count;
    int p1_account_index;
    int p2_account_index;
    bool editing_name;
    char edit_first_name[MAX_ACCOUNT_NAME_LEN];
    char edit_last_name[MAX_ACCOUNT_NAME_LEN];
    int edit_cursor_pos; 
    double edit_timer;
    char account_status_message[256];
    double account_status_timer;
    bool cover_p2_cards;
    AIDelayMode ai_delay_mode;
    bool music_enabled;
    WindowScale window_scale;
    bool is_fullscreen;
    LeaderboardEntry leaderboard[MAX_LEADERBOARD_ENTRIES];
    int leaderboard_count;
    bool leaderboard_loaded;
    GameType leaderboard_game_filter;
    int leaderboard_sort_mode; 
    bool leaderboard_sort_ascending;
    int achievement_scroll_offset;
    int achievement_view_mode; 
    int achievement_cursor_col;
    int achievement_cursor_row;
    int achievement_scroll_row;
    bool debug_force_check_achievements;
    char notification_text[64];    
    char notification_subtext[64]; 
    double notification_timer;     
    float notification_y_offset;   
    AIType selected_p1_ai;
    AIType selected_p2_ai;
    GameState *game_state; 
    BettingState betting;
    bool pvp_dual_window; 
    int p1_input_device;  
    int p2_input_device;  
    bool pvp_multiplayer; 
    bool is_host;
    NetworkRole net_role;
    bool net_connected;
    int net_socket;        
    int net_listen_socket; 
    unsigned int rng_seed; 
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
#endif 