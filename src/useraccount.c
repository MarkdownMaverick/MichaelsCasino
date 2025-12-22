#include "useraccount.h"
#include <cjson/cJSON.h>

bool IsAlpha(int c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool IsNameValid(const char *name) {
    if (name == NULL || strlen(name) == 0)
        return false;
    if (strlen(name) > MAX_ACCOUNT_NAME_LEN)
        return false;

    for (size_t i = 0; i < strlen(name); i++) {
        if (!IsAlpha(name[i]))
            return false;
    }
    return true;
}

void InitAiAccounts(LobbyState *g) {
    // AI Account 1: BOB
    strcpy(g->accounts[0].first_name, "BOB");
    strcpy(g->accounts[0].last_name, "");
    g->accounts[0].credits = 1000000.0;
    g->accounts[0].tokens = 0.0;
    g->accounts[0].wins = 0;
    g->accounts[0].losses = 0;
    g->accounts[0].is_ai = true;
    g->accounts[0].ai_type = AI_BOB;
    g->accounts[0].is_active = true;
    g->accounts[0].is_logged_in = false;

    // AI Account 2: THEA
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

    // AI Account 3: FLINT
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
}

void InitPlayerAccounts(LobbyState *g) {
    // Player Account 1
    strcpy(g->accounts[3].first_name, "Player");
    strcpy(g->accounts[3].last_name, "One");
    g->accounts[3].credits = 10000.0;
    g->accounts[3].tokens = 0.0;
    g->accounts[3].wins = 0;
    g->accounts[3].losses = 0;
    g->accounts[3].is_ai = false;
    g->accounts[3].is_active = true;
    g->accounts[3].is_logged_in = false;

    // Player Account 2
    strcpy(g->accounts[4].first_name, "Player");
    strcpy(g->accounts[4].last_name, "Two");
    g->accounts[4].credits = 10000.0;
    g->accounts[4].tokens = 0.0;
    g->accounts[4].wins = 0;
    g->accounts[4].losses = 0;
    g->accounts[4].is_ai = false;
    g->accounts[4].is_active = true;
    g->accounts[4].is_logged_in = false;

    g->account_count = 5;
}

void LoadAllAccounts(LobbyState *g) {
    FILE *fp = fopen(ACCOUNTS_FILE, "r");
    if (!fp) {
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
    if (!buffer) {
        printf("ERROR: Failed to allocate memory for accounts file\n");
        fclose(fp);
        return;
    }

    size_t read_size = fread(buffer, 1, (size_t)fsize, fp);
    buffer[read_size] = '\0';
    fclose(fp);

    cJSON *root = cJSON_Parse(buffer);
    if (!root) {
        printf("ERROR: Failed to parse accounts JSON\n");
        free(buffer);
        return;
    }

    cJSON *accounts_array = cJSON_GetObjectItem(root, "accounts");
    if (!cJSON_IsArray(accounts_array)) {
        printf("ERROR: accounts is not an array\n");
        cJSON_Delete(root);
        free(buffer);
        return;
    }

    int count = 0;
    cJSON *account = NULL;
    cJSON_ArrayForEach(account, accounts_array) {
        if (count >= MAX_ACCOUNTS) break;

        cJSON *first = cJSON_GetObjectItem(account, "first_name");
        cJSON *last = cJSON_GetObjectItem(account, "last_name");
        cJSON *bal = cJSON_GetObjectItem(account, "credits");
        cJSON *wins = cJSON_GetObjectItem(account, "wins");
        cJSON *losses = cJSON_GetObjectItem(account, "losses");
        cJSON *is_ai = cJSON_GetObjectItem(account, "is_ai");
        cJSON *ai_type = cJSON_GetObjectItem(account, "ai_type");
        cJSON *is_active = cJSON_GetObjectItem(account, "is_active");
        cJSON *tokens = cJSON_GetObjectItem(account, "tokens");

        if (cJSON_IsNumber(tokens)) g->accounts[count].tokens = tokens->valuedouble;
        else g->accounts[count].tokens = 0.0;

        if (cJSON_IsString(first)) strncpy(g->accounts[count].first_name, first->valuestring, MAX_ACCOUNT_NAME_LEN);
        if (cJSON_IsString(last)) strncpy(g->accounts[count].last_name, last->valuestring, MAX_ACCOUNT_NAME_LEN);
        if (cJSON_IsNumber(bal)) g->accounts[count].credits = bal->valuedouble;
        if (cJSON_IsNumber(wins)) g->accounts[count].wins = wins->valueint;
        if (cJSON_IsNumber(losses)) g->accounts[count].losses = losses->valueint;
        if (cJSON_IsBool(is_ai)) g->accounts[count].is_ai = cJSON_IsTrue(is_ai);
        if (cJSON_IsNumber(ai_type)) g->accounts[count].ai_type = (AIType)ai_type->valueint;
        if (cJSON_IsBool(is_active)) g->accounts[count].is_active = cJSON_IsTrue(is_active);

        g->accounts[count].is_logged_in = false;
        count++;
    }

    g->account_count = count;
    cJSON_Delete(root);
    free(buffer);
}

void SaveAllAccounts(const LobbyState *g) {
    cJSON *root = cJSON_CreateObject();
    if (!root) return;

    cJSON *accounts_array = cJSON_CreateArray();
    if (!accounts_array) {
        cJSON_Delete(root);
        return;
    }

    cJSON_AddItemToObject(root, "accounts", accounts_array);

    for (int i = 0; i < g->account_count; i++) {
        cJSON *acc = cJSON_CreateObject();
        if (!acc) continue;

        cJSON_AddStringToObject(acc, "first_name", g->accounts[i].first_name);
        cJSON_AddStringToObject(acc, "last_name", g->accounts[i].last_name);
        cJSON_AddNumberToObject(acc, "credits", g->accounts[i].credits);
        cJSON_AddNumberToObject(acc, "wins", g->accounts[i].wins);
        cJSON_AddNumberToObject(acc, "losses", g->accounts[i].losses);
        cJSON_AddBoolToObject(acc, "is_ai", g->accounts[i].is_ai);
        cJSON_AddNumberToObject(acc, "ai_type", (int)g->accounts[i].ai_type);
        cJSON_AddBoolToObject(acc, "is_active", g->accounts[i].is_active);
        cJSON_AddNumberToObject(acc, "tokens", g->accounts[i].tokens);
        cJSON_AddItemToArray(accounts_array, acc);
    }

    char *json_string = cJSON_Print(root);
    FILE *fp = fopen(ACCOUNTS_FILE, "w");
    if (fp) {
        fprintf(fp, "%s", json_string);
        fclose(fp);
    }

    free(json_string);
    cJSON_Delete(root);
}

void CreateDefaultLeaderboard(LobbyState *g) {
    printf("Creating default leaderboard with sample data\n");
    
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    // Create sample entries for each game type
    const char *sample_names[] = {"BOB", "THEA", "FLINT", "Player One", "Player Two"};
    
    g->leaderboard_count = 0;
    
    // Sample entries for Joker's Gambit
    for (int i = 0; i < 5; i++) {
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
    
    // Sample entries for Blackjack
    for (int i = 0; i < 5; i++) {
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
    
    // Sample entries for Slot Reels
    for (int i = 0; i < 5; i++) {
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

void LoadLeaderboard(LobbyState *g) {
    FILE *fp = fopen(LEADERBOARD_FILE, "r");
    if (!fp) {
        CreateDefaultLeaderboard(g);
        g->leaderboard_loaded = true;
        return;
    }

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *buffer = (char *)malloc((size_t)fsize + 1);
    if (!buffer) {
        fclose(fp);
        return;
    }

    size_t read_size = fread(buffer, 1, (size_t)fsize, fp);
    buffer[read_size] = '\0';
    fclose(fp);

    cJSON *root = cJSON_Parse(buffer);
    if (!root) {
        free(buffer);
        return;
    }

    cJSON *entries = cJSON_GetObjectItem(root, "entries");
    int count = 0;
    if (cJSON_IsArray(entries)) {
        cJSON *entry = NULL;
        cJSON_ArrayForEach(entry, entries) {
            if (count >= MAX_LEADERBOARD_ENTRIES) break;
            
            cJSON *winnings = cJSON_GetObjectItem(entry, "total_winnings");
            cJSON *credits = cJSON_GetObjectItem(entry, "final_credits");
            cJSON *bonus = cJSON_GetObjectItem(entry, "bonus");
            cJSON *rounds = cJSON_GetObjectItem(entry, "total_rounds");
            cJSON *moves = cJSON_GetObjectItem(entry, "moves_made");
            cJSON *game = cJSON_GetObjectItem(entry, "game_played");
            cJSON *entry_name = cJSON_GetObjectItem(entry, "entry_name");
            cJSON *winner = cJSON_GetObjectItem(entry, "winner_name");
            cJSON *timestamp = cJSON_GetObjectItem(entry, "timestamp");

            if (cJSON_IsNumber(winnings)) g->leaderboard[count].total_winnings = (float)winnings->valuedouble;
            if (cJSON_IsNumber(credits)) g->leaderboard[count].final_credits = (float)credits->valuedouble;
            if (cJSON_IsNumber(bonus)) g->leaderboard[count].bonus = (float)bonus->valuedouble;
            if (cJSON_IsNumber(rounds)) g->leaderboard[count].total_rounds = rounds->valueint;
            if (cJSON_IsNumber(moves)) g->leaderboard[count].moves_made = moves->valueint;
            if (cJSON_IsNumber(game)) g->leaderboard[count].game_played = (GameType)game->valueint;
            if (cJSON_IsString(entry_name)) strncpy(g->leaderboard[count].entry_name, entry_name->valuestring, MAX_LEADERBOARD_ENTRY_NAME_LEN - 1);
            if (cJSON_IsString(winner)) strncpy(g->leaderboard[count].winner_name, winner->valuestring, MAX_LEADERBOARD_WINNER_NAME_LEN - 1);
            if (cJSON_IsString(timestamp)) strncpy(g->leaderboard[count].timestamp, timestamp->valuestring, MAX_LEADERBOARD_TIMESTAMP_LEN - 1);
            count++;
        }
    }
    g->leaderboard_count = count;
    g->leaderboard_loaded = true;
    cJSON_Delete(root);
    free(buffer);
}

void SaveLeaderboard(const LobbyState *g) {
    cJSON *root = cJSON_CreateObject();
    cJSON *entries = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "entries", entries);

    for (int i = 0; i < g->leaderboard_count; i++) {
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
    if (fp) {
        fprintf(fp, "%s", json_string);
        fclose(fp);
    }
    free(json_string);
    cJSON_Delete(root);
}

void LogoutAccount(LobbyState *g, int player) {
    if (player == 1) {
        if (g->p1_account_index >= 0)
            g->accounts[g->p1_account_index].is_logged_in = false;
        g->p1_account_index = -1;
    } else if (player == 2) {
        if (g->p2_account_index >= 0)
            g->accounts[g->p2_account_index].is_logged_in = false;
        g->p2_account_index = -1;
    }
}

void LoginAccount(LobbyState *g, int index, int player) {
    if (index < 0 || index >= g->account_count) return;
    if (player == 1 && g->p2_account_index == index) return;
    if (player == 2 && g->p1_account_index == index) return;

    if (player == 1) {
        LogoutAccount(g, 1);
        g->p1_account_index = index;
        g->accounts[index].is_logged_in = true;
    } else if (player == 2) {
        LogoutAccount(g, 2);
        g->p2_account_index = index;
        g->accounts[index].is_logged_in = true;
    }
}

const char *GetPlayerName(const LobbyState *g, int player) {
    static char name_buffer[MAX_ACCOUNT_NAME_LEN * 2 + 2];
    int idx = (player == 1) ? g->p1_account_index : g->p2_account_index;
    if (idx < 0 || idx >= g->account_count) return "Unknown";

    if (strlen(g->accounts[idx].last_name) > 0)
        snprintf(name_buffer, sizeof(name_buffer), "%s %s",
                 g->accounts[idx].first_name, g->accounts[idx].last_name);
    else
        snprintf(name_buffer, sizeof(name_buffer), "%s",
                 g->accounts[idx].first_name);
    return name_buffer;
}

void UpdateAccountCredits(LobbyState *g) {
    SaveAllAccounts(g);
}