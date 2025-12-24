#include "useraccount.h"
#include <cjson/cJSON.h>
AchievementDef g_achievement_defs[MAX_ACHIEVEMENTS] = {0};
const char *GetPlayerNameByIndex(const LobbyState *g, int idx)
{
    static char name_buffer[MAX_ACCOUNT_NAME_LEN * 2 + 2];
    if (idx < 0 || idx >= g->account_count)
        return "Unknown";
    if (strlen(g->accounts[idx].last_name) > 0)
        snprintf(name_buffer, sizeof(name_buffer), "%s %s",
                 g->accounts[idx].first_name, g->accounts[idx].last_name);
    else
        snprintf(name_buffer, sizeof(name_buffer), "%s",
                 g->accounts[idx].first_name);
    return name_buffer;
}
void InitAchievements(Account *acc)
{
    // Zero out stats if this is a fresh init
    // Note: We don't zero stats here if loading from file, handled in LoadAllAccounts
    // Define the 50 Achievements
    const char *names[MAX_ACHIEVEMENTS] = {
        "Novice Player",      // Play 10 Games
        "Regular",            // Play 100 Games
        "Veteran",            // Play 500 Games
        "Beginner Luck",      // Win 10 Games
        "Champion",           // Win 100 Games
        "Master",             // Win 500 Games
        "Grandmaster",        // Win 1000 Games
        "Flint's Nemesis",    // Bankrupt Flint
        "Thea's Downfall",    // Bankrupt Thea
        "Bob's Ruin",         // Bankrupt Bob
        "Ouch",               // Lose 10 Games
        "Bad Day",            // Lose 50 Games
        "Rough Patch",        // Lose 100 Games
        "Joker Jackpot",      // Match Jokers
        "Joker Pro",          // Win in Less Than 20 Rounds
        "Quick Win",          // Win 1,000 in One Go
        "Big Score",          // Win 5,000 in One Go
        "Huge Payout",        // Win 10,000 in One Go
        "Massive Win",        // Win 50,000 in One Go
        "Jackpot Hunter",     // Win 100,000 in One Go
        "Piggy Bank",         // Reach 100 Tokens
        "Deep Pockets",       // Reach 500 Tokens
        "High Roller",        // Reach 1,000 Tokens
        "Millionaire",        // Reach 10,000 Tokens
        "Elite",              // Reach 100k Tokens
        "Spare Change",       // Reach 50k Credits
        "Mega Rich",          // Reach 100k Credits
        "Tycoon",             // Reach 500k Credits
        "Credit King",        // Reach 1M Credits
        "Credit Emperor",     // Reach 10M Credits
        "BJ Rookie",          // Win 10 BJ Hands
        "BJ Pro",             // Win 50 BJ Hands
        "BJ Expert",          // Win 100 BJ Hands
        "Card Counter",       // Win 500 BJ Hands
        "21 Master",          // Win 1,000 BJ Hands
        "Heating Up",         // Win 3 in a Row
        "On Fire",            // Win 5 in a Row
        "Unstoppable",        // Win 10 in a Row
        "Godlike",            // Win 15 in a Row
        "Invincible",         // Win 20 in a Row
        "Flush Finder",       // Get a Flush
        "Four of a Kind",     // Get a 4 of a Kind
        "Full House Hero",    // Get a Full House
        "Straight Flush Pro", // Get a Straight Flush
        "Slot King",          // Get a Royal Flush
        "Lucky Charm",        // Spin 100 Times
        "High Variance",      // Spin 1,000 Times
        "Slot Marathon",      // Spin 5,000 Times
        "Its A Secret",       // Secret achievement for jokers gambit , to win you need 3 out of 5 ranks filled , if you fill 5 ranks to end the game this achievement will be given
        "Completionist"       // Unlock 49 Achievements
    };
    const char *descs[MAX_ACHIEVEMENTS] = {
        "Play 10 Games", "Play 100 Games", "Play 500 Games", "Win 10 Games", "Win 100 Games",             // Joker's Gambit achievements
        "Win 500 Games", "Win 1000 Games", "Bankrupt Flint", "Bankrupt Thea", "Bankrupt Bob",             // Joker's Gambit achievements
        "Lose 10 Games", "Lose 50 Games", "Lose 100 Games", "Match Jokers", "Win in Less Than 20 Rounds", // Joker's Gambit achievements
        "Win 1,000 in One Go", "Win 5,000 in One Go", "Win 10,000 in One Go", "Win 50,000 in One Go", "Win 100,000 in One Go",
        "Reach 100 Tokens", "Reach 500 Tokens", "Reach 1,000 Tokens", "Reach 10,000 Tokens", "Reach 100,000 Tokens",
        "Reach 50k Credits", "Reach 100k Credits", "Reach 500k Credits", "Reach 1M Credits", "Reach 10M Credits",
        "Win 10 BJ Hands", "Win 50 BJ Hands", "Win 100 BJ Hands", "Win 500 BJ Hands", "Win 1,000 BJ Hands",  // Blackjack achievements
        "Win 3 in a Row", "Win 5 in a Row", "Win 10 in a Row", "Win 15 in a Row", "Win 20 in a Row",         // Blackjack achievements
        "Get a Flush", "Get a 4 of a Kind", "Get a Full House", "Get a Straight Flush", "Get a Royal Flush", // Card Slots Jacks or Better achievements
        "Spin 100 Times", "Spin 1,000 Times", "Spin 5,000 Times",                                            // Card Slots Jacks or Better achievements
        "Its a secret", "Unlock 49 Achievements"};
    for (int i = 0; i < MAX_ACHIEVEMENTS; i++)
    {
        strncpy(acc->achievements[i].name, names[i], ACHIEVEMENT_NAME_LEN);
        strncpy(acc->achievements[i].description, descs[i], ACHIEVEMENT_DESC_LEN);
        // Do not reset 'unlocked' here if it was loaded from file,
        // but for safety in InitPlayerAccounts, you should set unlocked = false manually.
    }
}
// Call this whenever a game ends
void UpdateGameStats(Account *acc, GameType game, double win_amount)
{
    if (!acc)
        return;
    acc->stats.total_games_played++;
    if (win_amount > 0)
    {
        acc->stats.total_wins++;
        acc->stats.total_winnings_cash += win_amount;
        acc->stats.current_win_streak++;
        if (acc->stats.current_win_streak > acc->stats.best_win_streak)
        {
            acc->stats.best_win_streak = acc->stats.current_win_streak;
        }
        if (win_amount > acc->stats.highest_single_win)
        {
            acc->stats.highest_single_win = win_amount;
        }
    }
    else
    {
        acc->stats.total_losses++;
        acc->stats.current_win_streak = 0;
    }
    switch (game)
    {
    case GAME_BLACKJACK:
        acc->stats.bj_hands_played++;
        if (win_amount > 0)
            acc->stats.bj_wins++;
        else
            acc->stats.bj_losses++;
        break;
    case GAME_JOKERS_GAMBIT:
        acc->stats.jg_hands_played++;
        if (win_amount > 0)
            acc->stats.jg_wins++;
        break;
    case GAME_SLOT_REELS:
        acc->stats.slots_spins++;
        if (win_amount > 0)
            acc->stats.slots_wins++;
        break;
    default:
        break;
    }
}

// Change the signature to accept LobbyState for access to AI accounts
void CheckAchievements(Account *acc, const LobbyState *g)
{
    // Helper macro to unlock (only if not already unlocked)
#define CHECK(idx, condition) \
    if (!acc->achievements[idx].unlocked && (condition)) \
        acc->achievements[idx].unlocked = true;

    // General Play
    CHECK(0,  acc->stats.total_games_played >= 10);
    CHECK(1,  acc->stats.total_games_played >= 100);
    CHECK(2,  acc->stats.total_games_played >= 500);
    CHECK(3,  acc->stats.total_wins >= 10);
    CHECK(4,  acc->stats.total_wins >= 100);
    CHECK(5,  acc->stats.total_wins >= 500);
    CHECK(6,  acc->stats.total_wins >= 1000);

    // Bankrupt AI opponents
    if (g != NULL) {
        for (int i = 0; i < g->account_count; i++) {
            if (g->accounts[i].is_ai) {
                if (strcmp(g->accounts[i].first_name, "FLINT") == 0 && g->accounts[i].credits <= 0)
                    CHECK(7, true);
                if (strcmp(g->accounts[i].first_name, "THEA") == 0 && g->accounts[i].credits <= 0)
                    CHECK(8, true);
                if (strcmp(g->accounts[i].first_name, "BOB") == 0 && g->accounts[i].credits <= 0)
                    CHECK(9, true);
            }
        }
    }

    CHECK(10, acc->stats.total_losses >= 10);
    CHECK(11, acc->stats.total_losses >= 50);
    CHECK(12, acc->stats.total_losses >= 100);

    // Jokers Gambit specific
    CHECK(13, acc->stats.jg_matched_jokers);                    // Set this flag when player matches both jokers
    CHECK(14, acc->stats.jg_fastest_win > 0 && acc->stats.jg_fastest_win <= 20);  // Rounds to win

    // Big single-game payouts (any game)
    CHECK(15, acc->stats.highest_single_win >= 1000);
    CHECK(16, acc->stats.highest_single_win >= 5000);
    CHECK(17, acc->stats.highest_single_win >= 10000);
    CHECK(18, acc->stats.highest_single_win >= 50000);
    CHECK(19, acc->stats.highest_single_win >= 100000);

    // Tokens
    CHECK(20, acc->tokens >= 100);
    CHECK(21, acc->tokens >= 500);
    CHECK(22, acc->tokens >= 1000);
    CHECK(23, acc->tokens >= 10000);
    CHECK(24, acc->tokens >= 100000);

    // Credits
    CHECK(25, acc->credits >= 50000);
    CHECK(26, acc->credits >= 100000);
    CHECK(27, acc->credits >= 500000);
    CHECK(28, acc->credits >= 1000000);
    CHECK(29, acc->credits >= 10000000);

    // Blackjack wins
    CHECK(30, acc->stats.bj_wins >= 10);
    CHECK(31, acc->stats.bj_wins >= 50);
    CHECK(32, acc->stats.bj_wins >= 100);
    CHECK(33, acc->stats.bj_wins >= 500);
    CHECK(34, acc->stats.bj_wins >= 1000);

    // Best win streak (any game)
    CHECK(35, acc->stats.best_win_streak >= 3);
    CHECK(36, acc->stats.best_win_streak >= 5);
    CHECK(37, acc->stats.best_win_streak >= 10);
    CHECK(38, acc->stats.best_win_streak >= 15);
    CHECK(39, acc->stats.best_win_streak >= 20);

    // Card Slots (assuming you have a game state or recent hand result)
    CHECK(40, acc->stats.cs_had_flush);
    CHECK(41, acc->stats.cs_had_four_kind);
    CHECK(42, acc->stats.cs_had_full_house);
    CHECK(43, acc->stats.cs_had_straight_flush);
    CHECK(44, acc->stats.cs_had_royal_flush);

    // Slot spins
    CHECK(45, acc->stats.slots_spins >= 100);
    CHECK(46, acc->stats.slots_spins >= 1000);
    CHECK(47, acc->stats.slots_spins >= 5000);

    // Jokers Gambit: Fill all 5 ranks to win (secret achievement)
    CHECK(48, acc->stats.jg_filled_all_five_ranks);

    // Completionist: Unlock 25 or more achievements
    int unlocked_count = 0;
    for (int i = 0; i < MAX_ACHIEVEMENTS - 1; i++) {  // Exclude self (49)
        if (acc->achievements[i].unlocked)
            unlocked_count++;
    }
    CHECK(49, unlocked_count >= 25);

#undef CHECK
}

bool IsAlpha(int c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}
bool IsNameValid(const char *name)
{
    if (name == NULL || strlen(name) == 0)
        return false;
    if (strlen(name) > MAX_ACCOUNT_NAME_LEN)
        return false;
    for (size_t i = 0; i < strlen(name); i++)
    {
        if (!IsAlpha(name[i]))
            return false;
    }
    return true;
}
const char *GetMemberStatusString(MEMBERSTATUS status)
{
    switch (status)
    {
    case MEMBER:
        return "MEMBER";
    case SUPPORTER:
        return "SUPPORTER";
    case ADVOCATE:
        return "ADVOCATE";
    case PROMOTER:
        return "PROMOTER";
    case CHAMPION:
        return "CHAMPION";
    case VIP:
        return "VIP";
    case ELITE:
        return "ELITE";
    case LEGEND:
        return "LEGEND";
    case HONORARY:
        return "HONORARY";
    case PRESIDENTIAL:
        return "PRESIDENTIAL";
    default:
        return "UNKNOWN";
    }
}
MEMBERSTATUS CalculateMemberStatus(double credits, double tokens)
{
    if (credits >= 10000000 && tokens >= 10000)
        return PRESIDENTIAL;
    if (credits >= 5000000 && tokens >= 5000)
        return HONORARY;
    if (credits >= 2500000 && tokens >= 2500)
        return LEGEND;
    if (credits >= 1000000 && tokens >= 1000)
        return ELITE;
    if (credits >= 500000 && tokens >= 500)
        return VIP;
    if (credits >= 250000 && tokens >= 250)
        return CHAMPION;
    if (credits >= 100000 && tokens >= 100)
        return PROMOTER;
    if (credits >= 50000 && tokens >= 50)
        return ADVOCATE;
    if (credits >= 10000 && tokens >= 10)
        return SUPPORTER;
    return MEMBER;
}
// ---------------------------
void InitAiAccounts(LobbyState *g)
{
    strcpy(g->accounts[0].first_name, "BOB"); // AI Accounts 1 to 3
    strcpy(g->accounts[0].last_name, "");
    g->accounts[0].credits = 1000000.0;
    g->accounts[0].tokens = 0.0;
    g->accounts[0].wins = 0;
    g->accounts[0].losses = 0;
    g->accounts[0].is_ai = true;
    g->accounts[0].ai_type = AI_BOB;
    g->accounts[0].is_active = true;
    g->accounts[0].is_logged_in = false;
    g->accounts[0].member_status = CalculateMemberStatus(g->accounts[0].credits, g->accounts[0].tokens);
    strcpy(g->accounts[1].first_name, "THEA");
    strcpy(g->accounts[1].last_name, "");
    g->accounts[1].credits = 1000000.0;
    g->accounts[1].tokens = 0.0;
    g->accounts[1].wins = 0;
    g->accounts[1].losses = 0;
    g->accounts[1].is_ai = true;
    g->accounts[1].ai_type = AI_THEA;
    g->accounts[1].is_active = true;
    g->accounts[1].is_logged_in = false;
    g->accounts[1].member_status = CalculateMemberStatus(g->accounts[1].credits, g->accounts[1].tokens);
    strcpy(g->accounts[2].first_name, "FLINT");
    strcpy(g->accounts[2].last_name, "");
    g->accounts[2].credits = 1000000.0;
    g->accounts[2].tokens = 0.0;
    g->accounts[2].wins = 0;
    g->accounts[2].losses = 0;
    g->accounts[2].is_ai = true;
    g->accounts[2].ai_type = AI_FLINT;
    g->accounts[2].is_active = true;
    g->accounts[2].is_logged_in = false;
    g->accounts[2].member_status = CalculateMemberStatus(g->accounts[2].credits, g->accounts[2].tokens);
}
void InitPlayerAccounts(LobbyState *g) // UPDATE InitPlayerAccounts to zero the stats and init achievements
{
    strcpy(g->accounts[3].first_name, "Player1"); // Player Account 1
    strcpy(g->accounts[3].last_name, "One");
    g->accounts[3].credits = 10000.0;
    g->accounts[3].tokens = 0.0;
    g->accounts[3].wins = 0;
    g->accounts[3].losses = 0;
    g->accounts[3].is_ai = false;
    g->accounts[3].is_active = true;
    g->accounts[3].is_logged_in = false;
    g->accounts[3].member_status = CalculateMemberStatus(g->accounts[3].credits, g->accounts[3].tokens);
    memset(&g->accounts[3].stats, 0, sizeof(PlayerStats));
    memset(g->accounts[3].achievements, 0, sizeof(g->accounts[3].achievements));
    InitAchievements(&g->accounts[3]);
    strcpy(g->accounts[4].first_name, "Player2"); // Player Account 2
    strcpy(g->accounts[4].last_name, "Two");
    g->accounts[4].credits = 10000.0;
    g->accounts[4].tokens = 0.0;
    g->accounts[4].wins = 0;
    g->accounts[4].losses = 0;
    g->accounts[4].is_ai = false;
    g->accounts[4].is_active = true;
    g->accounts[4].is_logged_in = false;
    g->accounts[4].member_status = CalculateMemberStatus(g->accounts[4].credits, g->accounts[4].tokens);
    memset(&g->accounts[4].stats, 0, sizeof(PlayerStats));
    memset(g->accounts[4].achievements, 0, sizeof(g->accounts[4].achievements));
    InitAchievements(&g->accounts[4]);
    g->account_count = 5;
}
void LoadAllAccounts(LobbyState *g)
{
    FILE *fp = fopen(ACCOUNTS_FILE, "r");
    if (!fp)
    {
        printf("No accounts file found, creating default accounts\n");
        InitAiAccounts(g);
        InitPlayerAccounts(g);
        SaveAllAccounts(g);
        return;
    }
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *buffer = (char *)malloc((size_t)fsize + 1);
    if (!buffer)
    {
        printf("ERROR: Failed to allocate memory for accounts file\n");
        fclose(fp);
        return;
    }
    size_t read_size = fread(buffer, 1, (size_t)fsize, fp);
    buffer[read_size] = '\0';
    fclose(fp);
    cJSON *root = cJSON_Parse(buffer);
    if (!root)
    {
        printf("ERROR: Failed to parse accounts JSON\n");
        free(buffer);
        return;
    }
    cJSON *accounts_array = cJSON_GetObjectItem(root, "accounts");
    if (!cJSON_IsArray(accounts_array))
    {
        printf("ERROR: accounts is not an array\n");
        cJSON_Delete(root);
        free(buffer);
        return;
    }
    int count = 0;
    cJSON *account = NULL;
    cJSON_ArrayForEach(account, accounts_array)
    {
        if (count >= MAX_ACCOUNTS)
            break;
        cJSON *first = cJSON_GetObjectItem(account, "first_name");
        cJSON *last = cJSON_GetObjectItem(account, "last_name");
        cJSON *bal = cJSON_GetObjectItem(account, "credits");
        cJSON *wins = cJSON_GetObjectItem(account, "wins");
        cJSON *losses = cJSON_GetObjectItem(account, "losses");
        cJSON *is_ai = cJSON_GetObjectItem(account, "is_ai");
        cJSON *ai_type = cJSON_GetObjectItem(account, "ai_type");
        cJSON *is_active = cJSON_GetObjectItem(account, "is_active");
        cJSON *tokens = cJSON_GetObjectItem(account, "tokens");
        // cJSON *json_status = cJSON_GetObjectItem(account, "Member_Status");
        if (cJSON_IsNumber(tokens))
            g->accounts[count].tokens = tokens->valuedouble;
        else
            g->accounts[count].tokens = 0.0;
        if (cJSON_IsString(first))
            strncpy(g->accounts[count].first_name, first->valuestring, MAX_ACCOUNT_NAME_LEN);
        if (cJSON_IsString(last))
            strncpy(g->accounts[count].last_name, last->valuestring, MAX_ACCOUNT_NAME_LEN);
        if (cJSON_IsNumber(bal))
            g->accounts[count].credits = bal->valuedouble;
        if (cJSON_IsNumber(wins))
            g->accounts[count].wins = wins->valueint;
        if (cJSON_IsNumber(losses))
            g->accounts[count].losses = losses->valueint;
        if (cJSON_IsBool(is_ai))
            g->accounts[count].is_ai = cJSON_IsTrue(is_ai);
        g->accounts[count].member_status = CalculateMemberStatus(g->accounts[count].credits, g->accounts[count].tokens);
        if (cJSON_IsNumber(ai_type))
            g->accounts[count].ai_type = (AIType)ai_type->valueint;
        if (cJSON_IsBool(is_active))
            g->accounts[count].is_active = cJSON_IsTrue(is_active);
        g->accounts[count].is_logged_in = false;
        count++;
    }
    g->account_count = count;
    cJSON_Delete(root);
    free(buffer);
}
void SaveAllAccounts(const LobbyState *g)
{
    cJSON *root = cJSON_CreateObject();
    if (!root)
        return;
    cJSON *accounts_array = cJSON_CreateArray();
    if (!accounts_array)
    {
        cJSON_Delete(root);
        return;
    }
    cJSON_AddItemToObject(root, "accounts", accounts_array);
    for (int i = 0; i < g->account_count; i++)
    {
        cJSON *acc = cJSON_CreateObject();
        if (!acc)
            continue;
        cJSON_AddStringToObject(acc, "first_name", g->accounts[i].first_name);
        cJSON_AddStringToObject(acc, "last_name", g->accounts[i].last_name);
        cJSON_AddNumberToObject(acc, "credits", g->accounts[i].credits);
        cJSON_AddNumberToObject(acc, "wins", g->accounts[i].wins);
        cJSON_AddNumberToObject(acc, "losses", g->accounts[i].losses);
        cJSON_AddBoolToObject(acc, "is_ai", g->accounts[i].is_ai);
        cJSON_AddNumberToObject(acc, "ai_type", (int)g->accounts[i].ai_type);
        cJSON_AddBoolToObject(acc, "is_active", g->accounts[i].is_active);
        cJSON_AddNumberToObject(acc, "tokens", g->accounts[i].tokens);
        cJSON_AddNumberToObject(acc, "Member_Status", (int)g->accounts[i].member_status);
        cJSON_AddItemToArray(accounts_array, acc);
    }
    char *json_string = cJSON_Print(root);
    FILE *fp = fopen(ACCOUNTS_FILE, "w");
    if (fp)
    {
        fprintf(fp, "%s", json_string);
        fclose(fp);
    }
    free(json_string);
    cJSON_Delete(root);
}
void CreateDefaultLeaderboard(LobbyState *g)
{
    printf("Creating default leaderboard with sample data\n");
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    const char *sample_names[] = {"BOB", "THEA", "FLINT", "Player One", "Player Two"};
    g->leaderboard_count = 0;
    for (int i = 0; i < 5; i++)
    {
        LeaderboardEntry *e = &g->leaderboard[g->leaderboard_count++];
        e->game_played = GAME_JOKERS_GAMBIT;
        e->total_winnings = 5000.0f - ((float)i * 500.0f);
        e->final_credits = 15000.0f - ((float)i * 1000.0f);
        e->bonus = 500.0f;
        e->total_rounds = 10 + i;
        e->moves_made = 15 + (i * 3);
        strncpy(e->winner_name, sample_names[i], MAX_LEADERBOARD_WINNER_NAME_LEN);
        strncpy(e->entry_name, "Sample Game", MAX_LEADERBOARD_ENTRY_NAME_LEN);
        strftime(e->timestamp, MAX_LEADERBOARD_TIMESTAMP_LEN, "%Y-%m-%d %H:%M", t);
    }
    for (int i = 0; i < 5; i++) // Sample entries for Blackjack
    {
        LeaderboardEntry *e = &g->leaderboard[g->leaderboard_count++];
        e->game_played = GAME_BLACKJACK;
        e->total_winnings = 3000.0f - ((float)i * 300.0f);
        e->final_credits = 13000.0f - ((float)i * 800.0f);
        e->bonus = 300.0f;
        e->total_rounds = 8 + i;
        e->moves_made = 12 + (i * 2);
        strncpy(e->winner_name, sample_names[i], MAX_LEADERBOARD_WINNER_NAME_LEN);
        strncpy(e->entry_name, "Sample Game", MAX_LEADERBOARD_ENTRY_NAME_LEN);
        strftime(e->timestamp, MAX_LEADERBOARD_TIMESTAMP_LEN, "%Y-%m-%d %H:%M", t);
    }
    for (int i = 0; i < 5; i++) // Sample entries for Slot Reels
    {
        LeaderboardEntry *e = &g->leaderboard[g->leaderboard_count++];
        e->game_played = GAME_SLOT_REELS;
        e->total_winnings = 8000.0f - ((float)i * 800.0f);
        e->final_credits = 18000.0f - ((float)i * 1500.0f);
        e->bonus = 1000.0f;
        e->total_rounds = 20 + i;
        e->moves_made = 0; // N/A for slots
        strncpy(e->winner_name, sample_names[i], MAX_LEADERBOARD_WINNER_NAME_LEN);
        strncpy(e->entry_name, "Sample Game", MAX_LEADERBOARD_ENTRY_NAME_LEN);
        strftime(e->timestamp, MAX_LEADERBOARD_TIMESTAMP_LEN, "%Y-%m-%d %H:%M", t);
    }
    SaveLeaderboard(g);
}
void LoadLeaderboard(LobbyState *g)
{
    FILE *fp = fopen(LEADERBOARD_FILE, "r");
    if (!fp)
    {
        CreateDefaultLeaderboard(g);
        g->leaderboard_loaded = true;
        return;
    }
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *buffer = (char *)malloc((size_t)fsize + 1);
    if (!buffer)
    {
        fclose(fp);
        return;
    }
    size_t read_size = fread(buffer, 1, (size_t)fsize, fp);
    buffer[read_size] = '\0';
    fclose(fp);
    cJSON *root = cJSON_Parse(buffer);
    if (!root)
    {
        free(buffer);
        return;
    }
    cJSON *entries = cJSON_GetObjectItem(root, "entries");
    int count = 0;
    if (cJSON_IsArray(entries))
    {
        cJSON *entry = NULL;
        cJSON_ArrayForEach(entry, entries)
        {
            if (count >= MAX_LEADERBOARD_ENTRIES)
                break;
            cJSON *winnings = cJSON_GetObjectItem(entry, "total_winnings");
            cJSON *credits = cJSON_GetObjectItem(entry, "final_credits");
            cJSON *bonus = cJSON_GetObjectItem(entry, "bonus");
            cJSON *rounds = cJSON_GetObjectItem(entry, "total_rounds");
            cJSON *moves = cJSON_GetObjectItem(entry, "moves_made");
            cJSON *game = cJSON_GetObjectItem(entry, "game_played");
            cJSON *entry_name = cJSON_GetObjectItem(entry, "entry_name");
            cJSON *winner = cJSON_GetObjectItem(entry, "winner_name");
            cJSON *timestamp = cJSON_GetObjectItem(entry, "timestamp");
            if (cJSON_IsNumber(winnings))
                g->leaderboard[count].total_winnings = (float)winnings->valuedouble;
            if (cJSON_IsNumber(credits))
                g->leaderboard[count].final_credits = (float)credits->valuedouble;
            if (cJSON_IsNumber(bonus))
                g->leaderboard[count].bonus = (float)bonus->valuedouble;
            if (cJSON_IsNumber(rounds))
                g->leaderboard[count].total_rounds = rounds->valueint;
            if (cJSON_IsNumber(moves))
                g->leaderboard[count].moves_made = moves->valueint;
            if (cJSON_IsNumber(game))
                g->leaderboard[count].game_played = (GameType)game->valueint;
            if (cJSON_IsString(entry_name))
                strncpy(g->leaderboard[count].entry_name, entry_name->valuestring, MAX_LEADERBOARD_ENTRY_NAME_LEN - 1);
            if (cJSON_IsString(winner))
                strncpy(g->leaderboard[count].winner_name, winner->valuestring, MAX_LEADERBOARD_WINNER_NAME_LEN - 1);
            if (cJSON_IsString(timestamp))
                strncpy(g->leaderboard[count].timestamp, timestamp->valuestring, MAX_LEADERBOARD_TIMESTAMP_LEN - 1);
            count++;
        }
    }
    g->leaderboard_count = count;
    g->leaderboard_loaded = true;
    cJSON_Delete(root);
    free(buffer);
}
void SaveLeaderboard(const LobbyState *g)
{
    cJSON *root = cJSON_CreateObject();
    cJSON *entries = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "entries", entries);
    for (int i = 0; i < g->leaderboard_count; i++)
    {
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "total_winnings", g->leaderboard[i].total_winnings);
        cJSON_AddNumberToObject(entry, "final_credits", g->leaderboard[i].final_credits);
        cJSON_AddNumberToObject(entry, "bonus", g->leaderboard[i].bonus);
        cJSON_AddNumberToObject(entry, "total_rounds", g->leaderboard[i].total_rounds);
        cJSON_AddNumberToObject(entry, "moves_made", g->leaderboard[i].moves_made);
        cJSON_AddNumberToObject(entry, "game_played", (int)g->leaderboard[i].game_played);
        cJSON_AddStringToObject(entry, "entry_name", g->leaderboard[i].entry_name);
        cJSON_AddStringToObject(entry, "winner_name", g->leaderboard[i].winner_name);
        cJSON_AddStringToObject(entry, "timestamp", g->leaderboard[i].timestamp);
        cJSON_AddItemToArray(entries, entry);
    }
    char *json_string = cJSON_Print(root);
    FILE *fp = fopen(LEADERBOARD_FILE, "w");
    if (fp)
    {
        fprintf(fp, "%s", json_string);
        fclose(fp);
    }
    free(json_string);
    cJSON_Delete(root);
}
void LogoutAccount(LobbyState *g, int player)
{
    if (player == 1)
    {
        if (g->p1_account_index >= 0)
            g->accounts[g->p1_account_index].is_logged_in = false;
        g->p1_account_index = -1;
    }
    else if (player == 2)
    {
        if (g->p2_account_index >= 0)
            g->accounts[g->p2_account_index].is_logged_in = false;
        g->p2_account_index = -1;
    }
}
void LoginAccount(LobbyState *g, int index, int player)
{
    if (index < 0 || index >= g->account_count)
        return;
    if (player == 1 && g->p2_account_index == index)
        return;
    if (player == 2 && g->p1_account_index == index)
        return;
    if (player == 1)
    {
        LogoutAccount(g, 1);
        g->p1_account_index = index;
        g->accounts[index].is_logged_in = true;
    }
    else if (player == 2)
    {
        LogoutAccount(g, 2);
        g->p2_account_index = index;
        g->accounts[index].is_logged_in = true;
    }
}
const char *GetPlayerName(const LobbyState *g, int player)
{
    static char name_buffer[MAX_ACCOUNT_NAME_LEN * 2 + 2];
    int idx = (player == 1) ? g->p1_account_index : g->p2_account_index;
    if (idx < 0 || idx >= g->account_count)
        return "Unknown";
    if (strlen(g->accounts[idx].last_name) > 0)
        snprintf(name_buffer, sizeof(name_buffer), "%s %s",
                 g->accounts[idx].first_name, g->accounts[idx].last_name);
    else
        snprintf(name_buffer, sizeof(name_buffer), "%s",
                 g->accounts[idx].first_name);
    return name_buffer;
}
void UpdateAccountCredits(LobbyState *g)
{
    for (int i = 0; i < g->account_count; i++) // Important: Update member status before saving!
    {
        g->accounts[i].member_status = CalculateMemberStatus(g->accounts[i].credits, g->accounts[i].tokens);
    }
    SaveAllAccounts(g);
}
void LoadAchievements(LobbyState *g)
{
    FILE *fp = fopen("save/achievements.json", "r");
    if (!fp)
    {
        InitAchievements(&g->accounts[0]); // Fallback to hardcoded if no file, Assume first human
        InitAchievements(&g->accounts[1]); // Second human
        return;
    }
    fseek(fp, 0, SEEK_END);
    long size_long = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    size_t size = (size_t)size_long; // Cast to size_t
    char *json_str = (char *)malloc(size + 1);
    fread(json_str, 1, size, fp);
    json_str[size] = '\0';
    fclose(fp);
    cJSON *root = cJSON_Parse(json_str);
    free(json_str);
    if (!root)
        return;
    cJSON *ach_array = cJSON_GetObjectItem(root, "achievements");
    if (cJSON_IsArray(ach_array) && cJSON_GetArraySize(ach_array) == MAX_ACHIEVEMENTS)
    {
        for (int i = 0; i < MAX_ACHIEVEMENTS; i++)
        {
            cJSON *item = cJSON_GetArrayItem(ach_array, i);
            strncpy(g_achievement_defs[i].name, cJSON_GetObjectItem(item, "name")->valuestring, ACHIEVEMENT_NAME_LEN - 1);
            strncpy(g_achievement_defs[i].description, cJSON_GetObjectItem(item, "description")->valuestring, ACHIEVEMENT_DESC_LEN - 1);
            g_achievement_defs[i].target = cJSON_GetObjectItem(item, "target")->valueint;
        }
    }
    // Load per-human
    cJSON *players = cJSON_GetObjectItem(root, "players");
    if (players)
    {
        int human_idx = 0;
        for (int i = 0; i < g->account_count && human_idx < 2; i++)
        {
            if (g->accounts[i].is_ai)
                continue;
            const char *key = (human_idx == 0) ? "Player1" : "Player2";
            cJSON *player_data = cJSON_GetObjectItem(players, key);
            if (player_data)
            {
                cJSON *unlocked_array = cJSON_GetObjectItem(player_data, "unlocked");
                if (cJSON_IsArray(unlocked_array) && cJSON_GetArraySize(unlocked_array) == MAX_ACHIEVEMENTS)
                {
                    for (int j = 0; j < MAX_ACHIEVEMENTS; j++) // Set name/desc from defs
                    {
                        g->accounts[i].achievements[j].unlocked = cJSON_IsTrue(cJSON_GetArrayItem(unlocked_array, j));
                        strncpy(g->accounts[i].achievements[j].name, g_achievement_defs[j].name, ACHIEVEMENT_NAME_LEN - 1);
                        strncpy(g->accounts[i].achievements[j].description, g_achievement_defs[j].description, ACHIEVEMENT_DESC_LEN - 1);
                    }
                }
            }
            human_idx++;
        }
    }
    cJSON_Delete(root);
}
void SaveAchievements(const LobbyState *g)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "version", 1.0);
    cJSON_AddStringToObject(root, "last_updated", "2025-12-23T00:00:00Z"); // Update with current time in real code
    cJSON *ach_array = cJSON_AddArrayToObject(root, "achievements");
    for (int i = 0; i < MAX_ACHIEVEMENTS; i++)
    {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddStringToObject(item, "name", g_achievement_defs[i].name);
        cJSON_AddStringToObject(item, "description", g_achievement_defs[i].description);
        cJSON_AddNumberToObject(item, "target", g_achievement_defs[i].target);
        cJSON_AddItemToArray(ach_array, item);
    }
    cJSON *players = cJSON_AddObjectToObject(root, "players");
    int human_idx = 0;
    for (int i = 0; i < g->account_count && human_idx < 2; i++)
    {
        if (g->accounts[i].is_ai)
            continue;
        const char *key = (human_idx == 0) ? "Player1" : "Player2";
        cJSON *player_data = cJSON_CreateObject();
        cJSON *unlocked_array = cJSON_AddArrayToObject(player_data, "unlocked");
        for (int j = 0; j < MAX_ACHIEVEMENTS; j++)
        {
            cJSON_AddItemToArray(unlocked_array, cJSON_CreateBool(g->accounts[i].achievements[j].unlocked));
        }
        cJSON_AddItemToObject(players, key, player_data); // Add unlock_dates and progress when implemented
        human_idx++;
    }
    char *json_string = cJSON_Print(root);
    FILE *fp = fopen("save/achievements.json", "w");
    if (fp)
    {
        fprintf(fp, "%s", json_string);
        fclose(fp);
    }
    free(json_string);
    cJSON_Delete(root);
}
void LoadSettings(LobbyState *g)
{
    FILE *fp = fopen(SETTINGS_FILE, "r");
    if (!fp)
        return; // Defaults already set
    fseek(fp, 0, SEEK_END);
    long size_long = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    size_t size = (size_t)size_long;
    char *json_str = (char *)malloc(size + 1);
    fread(json_str, 1, size, fp);
    json_str[size] = '\0';
    fclose(fp);
    cJSON *root = cJSON_Parse(json_str);
    free(json_str);
    if (!root)
        return;
    cJSON *item;
    if ((item = cJSON_GetObjectItem(root, "cover_p2_cards")) != NULL)
        g->cover_p2_cards = cJSON_IsTrue(item);
    if ((item = cJSON_GetObjectItem(root, "ai_move_delay")) != NULL)
        g->ai_move_delay = (float)item->valuedouble;
    if ((item = cJSON_GetObjectItem(root, "music_enabled")) != NULL)
        g->music_enabled = cJSON_IsTrue(item);
    if ((item = cJSON_GetObjectItem(root, "window_scale")) != NULL)
        g->window_scale = item->valueint;
    if ((item = cJSON_GetObjectItem(root, "is_fullscreen")) != NULL)
        g->is_fullscreen = cJSON_IsTrue(item);
    cJSON_Delete(root);
    ApplyWindowScale(g); // Apply loaded settings
    if (g->music_enabled)
    {
        PlayMusicStream(g_background_music);
    }
    else
    {
        StopMusicStream(g_background_music);
    }
}
void SaveSettings(const LobbyState *g)
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "cover_p2_cards", g->cover_p2_cards);
    cJSON_AddNumberToObject(root, "ai_move_delay", g->ai_move_delay);
    cJSON_AddBoolToObject(root, "music_enabled", g->music_enabled);
    cJSON_AddNumberToObject(root, "window_scale", g->window_scale);
    cJSON_AddBoolToObject(root, "is_fullscreen", g->is_fullscreen);
    char *json_string = cJSON_Print(root);
    FILE *fp = fopen(SETTINGS_FILE, "w");
    if (fp)
    {
        fprintf(fp, "%s", json_string);
        fclose(fp);
    }
    free(json_string);
    cJSON_Delete(root);
}