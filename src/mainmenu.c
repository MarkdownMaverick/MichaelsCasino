#include "mainmenu.h"
#include "useraccount.h"
#include "gamepad_sdl.h"
#include <strings.h>
#include <stdlib.h>

void ShowAccountStatus(LobbyState *g, const char *msg)
{
    strncpy(g->account_status_message, msg, sizeof(g->account_status_message) - 1);
    g->account_status_timer = GetTime();
}

// Helper to switch states and reset selection
static void SwitchState(LobbyState *g, UIState newState)
{
    g->state = newState;
    g->menu_selection = 0; // Reset highlight to top
}

// ============================================================================
// MAIN MENU
// ============================================================================

void DrawMainMenu(const LobbyState *g)
{
    DrawText("MICHAEL'S CASINO", (int)(CENTER_X - 380.0f), 100, 90, GOLD);
    DrawText("Coming Soon: Poker, Roulette & More!", (int)(CENTER_X - 280), 190, 25, LIGHTGRAY);

    Vector2 mouse = GetMousePosition();

    Rectangle rects[5];
    rects[0] = (Rectangle){CENTER_X - 150, 250, 300, 70}; // Lobby
    rects[1] = (Rectangle){CENTER_X - 150, 340, 300, 70}; // Accounts
    rects[2] = (Rectangle){CENTER_X - 150, 430, 300, 70}; // Shop
    rects[3] = (Rectangle){CENTER_X - 150, 520, 300, 70}; // Settings
    rects[4] = (Rectangle){CENTER_X - 150, 610, 300, 70}; // Leaderboard

    const char *labels[] = {"LOBBY", "ACCOUNTS", "SHOP", "SETTINGS", "LEADERBOARD"};
    Color colors[] = {LIME, SKYBLUE, GOLD, PURPLE, YELLOW};
    Color altColors[] = {GREEN, BLUE, ORANGE, DARKPURPLE, GOLD};

    for (int i = 0; i < 5; i++)
    {
        bool selected = (g->menu_selection == i);
        bool hovered = CheckCollisionPointRec(mouse, rects[i]);

        DrawRectangleRec(rects[i], (selected || hovered) ? colors[i] : altColors[i]);

        if (selected)
            DrawRectangleLinesEx(rects[i], 3, WHITE);

        DrawText(labels[i], (int)(rects[i].x + 20), (int)(rects[i].y + 20), 35, (selected || hovered) ? BLACK : WHITE);
    }
}

void UpdateMainMenu(LobbyState *g, Vector2 mouse)
{
    Rectangle rects[5];
    rects[0] = (Rectangle){CENTER_X - 150, 250, 300, 70};
    rects[1] = (Rectangle){CENTER_X - 150, 340, 300, 70};
    rects[2] = (Rectangle){CENTER_X - 150, 430, 300, 70};
    rects[3] = (Rectangle){CENTER_X - 150, 520, 300, 70};
    rects[4] = (Rectangle){CENTER_X - 150, 610, 300, 70};

    int gamepad = GetActiveGamepad();
    
    // === KEYBOARD INPUT ===
    if (IsKeyPressed(KEY_UP)) {
        g->menu_selection--;
        if (g->menu_selection < 0) g->menu_selection = 4;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        g->menu_selection++;
        if (g->menu_selection > 4) g->menu_selection = 0;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        switch (g->menu_selection) {
            case 0: SwitchState(g, STATE_LOBBY); break;
            case 1: SwitchState(g, STATE_ACCOUNTS_MANAGER); break;
            case 2: SwitchState(g, STATE_SHOP); break;
            case 3: SwitchState(g, STATE_SETTINGS); break;
            case 4: SwitchState(g, STATE_LEADERBOARD); break;
        }
    }
    
    // === GAMEPAD INPUT ===
    if (gamepad >= 0) {
        if (IsGamepadButtonPressedSDL(gamepad, 11)) { // D-Pad Up
            g->menu_selection--;
            if (g->menu_selection < 0) g->menu_selection = 4;
        }
        
        if (IsGamepadButtonPressedSDL(gamepad, 12)) { // D-Pad Down
            g->menu_selection++;
            if (g->menu_selection > 4) g->menu_selection = 0;
        }
        
        if (IsGamepadButtonPressedSDL(gamepad, 0)) { // A button
            switch (g->menu_selection) {
                case 0: SwitchState(g, STATE_LOBBY); break;
                case 1: SwitchState(g, STATE_ACCOUNTS_MANAGER); break;
                case 2: SwitchState(g, STATE_SHOP); break;
                case 3: SwitchState(g, STATE_SETTINGS); break;
                case 4: SwitchState(g, STATE_LEADERBOARD); break;
            }
        }
    }
    
    // === MOUSE INPUT ===
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (CheckCollisionPointRec(mouse, rects[0]))
            SwitchState(g, STATE_LOBBY);
        else if (CheckCollisionPointRec(mouse, rects[1]))
            SwitchState(g, STATE_ACCOUNTS_MANAGER);
        else if (CheckCollisionPointRec(mouse, rects[2]))
            SwitchState(g, STATE_SHOP);
        else if (CheckCollisionPointRec(mouse, rects[3]))
            SwitchState(g, STATE_SETTINGS);
        else if (CheckCollisionPointRec(mouse, rects[4]))
            SwitchState(g, STATE_LEADERBOARD);
    }
}

// ============================================================================
// LOBBY
// ============================================================================

void DrawLobby(const LobbyState *g)
{
    DrawText("SELECT YOUR GAME", (int)(CENTER_X - 300.0f), 100, 70, GOLD);

    Rectangle rects[3];
    rects[0] = (Rectangle){CENTER_X - 450, 300, 300, 120};
    rects[1] = (Rectangle){CENTER_X - 150, 300, 300, 120};
    rects[2] = (Rectangle){CENTER_X + 150, 300, 300, 120};

    const char *titles[] = {"JOKER'S GAMBIT", "BLACKJACK", "SLOT REELS"};

    for (int i = 0; i < 3; i++)
    {
        bool selected = (g->menu_selection == i);
        DrawRectangleRec(rects[i], selected ? LIME : DARKGREEN);
        DrawRectangleLinesEx(rects[i], 4, selected ? WHITE : GOLD);
        DrawText(titles[i], (int)(rects[i].x + 20), (int)(rects[i].y + 40), 30, WHITE);
    }

    DrawText("All games use the same account balance!", (int)(CENTER_X - 280), 500, 28, YELLOW);
    DrawText("(Press 'A' to Select - Coming Soon)", (int)(CENTER_X - 220), 600, 30, GRAY);
    DrawText("BACK (B)", (int)(CENTER_X - 70), (int)SCREEN_H - 150, 35, WHITE);
}

void UpdateLobby(LobbyState *g, Vector2 mouse)
{
    int gamepad = GetActiveGamepad();
    
    // === KEYBOARD INPUT ===
    if (IsKeyPressed(KEY_LEFT)) {
        g->menu_selection--;
        if (g->menu_selection < 0) g->menu_selection = 2;
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        g->menu_selection++;
        if (g->menu_selection > 2) g->menu_selection = 0;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        SwitchState(g, STATE_MAIN_MENU);
    }
    
    // === GAMEPAD INPUT ===
    if (gamepad >= 0) {
        if (IsGamepadButtonPressedSDL(gamepad, 13)) { // D-Pad Left
            g->menu_selection--;
            if (g->menu_selection < 0) g->menu_selection = 2;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 14)) { // D-Pad Right
            g->menu_selection++;
            if (g->menu_selection > 2) g->menu_selection = 0;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 1)) { // B Button
            SwitchState(g, STATE_MAIN_MENU);
        }
    }
    
    // === MOUSE INPUT ===
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        Rectangle back_rect = {CENTER_X - 150, SCREEN_H - 150, 300, 80};
        if (CheckCollisionPointRec(mouse, back_rect))
            SwitchState(g, STATE_MAIN_MENU);
    }
}

// ============================================================================
// SETTINGS
// ============================================================================

void DrawSettings(const LobbyState *g)
{
    DrawText("SETTINGS", (int)(CENTER_X - 200.0f), 100, 80, GOLD);

    bool sel0 = (g->menu_selection == 0);
    bool sel1 = (g->menu_selection == 1);

    // Privacy Setting
    DrawText("P2 Card Privacy:", (int)(CENTER_X - 300.0f), 300, 40, WHITE);
    Rectangle cover_btn = {CENTER_X - 150, 380, 300, 80};
    DrawRectangleRec(cover_btn, g->cover_p2_cards ? LIME : RED);
    if (sel0)
        DrawRectangleLinesEx(cover_btn, 4, WHITE);
    DrawText(g->cover_p2_cards ? "COVERED" : "UNCOVERED", (int)(cover_btn.x + 60), (int)(cover_btn.y + 25), 30, BLACK);

    // AI Speed Setting
    DrawText("AI Move Speed:", (int)(CENTER_X - 300.0f), 550, 40, WHITE);
    Rectangle speed_btn = {CENTER_X - 150, 620, 300, 80};
    const char *speed_text = (fabsf(g->ai_move_delay - 0.5f) < 0.001f) ? "FAST" : (fabsf(g->ai_move_delay - 2.0f) < 0.001f) ? "MEDIUM" : "SLOW";
    Color speed_col = (fabsf(g->ai_move_delay - 0.5f) < 0.001f) ? LIME : (fabsf(g->ai_move_delay - 2.0f) < 0.001f) ? ORANGE : RED;

    DrawRectangleRec(speed_btn, speed_col);
    if (sel1)
        DrawRectangleLinesEx(speed_btn, 4, WHITE);
    DrawText(speed_text, (int)(speed_btn.x + 80), (int)(speed_btn.y + 25), 35, BLACK);

    DrawText("BACK (B)", (int)(CENTER_X - 70), (int)SCREEN_H - 150, 35, WHITE);
}

void UpdateSettings(LobbyState *g, Vector2 mouse)
{
    Rectangle cover_btn = {CENTER_X - 150, 380, 300, 80};
    Rectangle speed_btn = {CENTER_X - 150, 620, 300, 80};
    Rectangle back_btn = {CENTER_X - 150, SCREEN_H - 180, 300, 80};

    int gamepad = GetActiveGamepad();
    
    // === KEYBOARD INPUT ===
    if (IsKeyPressed(KEY_UP)) {
        g->menu_selection--;
        if (g->menu_selection < 0) g->menu_selection = 1;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        g->menu_selection++;
        if (g->menu_selection > 1) g->menu_selection = 0;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        if (g->menu_selection == 0) {
            g->cover_p2_cards = !g->cover_p2_cards;
        } else if (g->menu_selection == 1) {
            if (fabsf(g->ai_move_delay - 0.5f) < EPSILON)
                g->ai_move_delay = 2.0f;
            else if (fabsf(g->ai_move_delay - 2.0f) < EPSILON)
                g->ai_move_delay = 3.0f;
            else
                g->ai_move_delay = 0.5f;
        }
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        SwitchState(g, STATE_MAIN_MENU);
    }
    
    // === GAMEPAD INPUT ===
    if (gamepad >= 0) {
        if (IsGamepadButtonPressedSDL(gamepad, 11)) { // D-Pad Up
            g->menu_selection--;
            if (g->menu_selection < 0) g->menu_selection = 1;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 12)) { // D-Pad Down
            g->menu_selection++;
            if (g->menu_selection > 1) g->menu_selection = 0;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 0)) { // A Button
            if (g->menu_selection == 0) {
                g->cover_p2_cards = !g->cover_p2_cards;
            } else if (g->menu_selection == 1) {
                if (fabsf(g->ai_move_delay - 0.5f) < EPSILON)
                    g->ai_move_delay = 2.0f;
                else if (fabsf(g->ai_move_delay - 2.0f) < EPSILON)
                    g->ai_move_delay = 3.0f;
                else
                    g->ai_move_delay = 0.5f;
            }
        }
        if (IsGamepadButtonPressedSDL(gamepad, 1)) { // B Button
            SwitchState(g, STATE_MAIN_MENU);
        }
    }

    // === MOUSE INPUT ===
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (CheckCollisionPointRec(mouse, cover_btn)) {
            g->cover_p2_cards = !g->cover_p2_cards;
        }
        else if (CheckCollisionPointRec(mouse, speed_btn)) {
            if (fabsf(g->ai_move_delay - 0.5f) < EPSILON)
                g->ai_move_delay = 2.0f;
            else if (fabsf(g->ai_move_delay - 2.0f) < EPSILON)
                g->ai_move_delay = 3.0f;
            else
                g->ai_move_delay = 0.5f;
        }
        else if (CheckCollisionPointRec(mouse, back_btn)) {
            SwitchState(g, STATE_MAIN_MENU);
        }
    }
}

// ============================================================================
// ACCOUNTS MANAGER
// ============================================================================

void DrawAccountsManager(const LobbyState *g)
{
    ClearBackground(BLACK);
    DrawText("ACCOUNT MANAGER", (int)(CENTER_X - 300.0f), 50, 60, GOLD);

    float start_y = 150.0f;
    float card_h = 100.0f;
    float spacing = 10.0f;

    for (int i = 0; i < g->account_count; i++)
    {
        Rectangle card = {CENTER_X - 450.0f, start_y + (float)i * (card_h + spacing), 900.0f, card_h};
        bool selected = (g->menu_selection == i);

        Color base_color = g->accounts[i].is_ai ? DARKBLUE : DARKGRAY;
        if (selected)
            base_color = MAROON;

        DrawRectangleRec(card, base_color);
        DrawRectangleLinesEx(card, selected ? 4.0f : 2.0f, (selected || g->accounts[i].is_logged_in) ? LIME : GRAY);

        DrawText(g->accounts[i].first_name, (int)card.x + 20, (int)card.y + 20, 30, WHITE);

        if (g->p1_account_index == i)
            DrawText("P1 ACTIVE", (int)card.x + 700, (int)card.y + 35, 25, LIME);
        else if (g->p2_account_index == i)
            DrawText("P2 ACTIVE", (int)card.x + 700, (int)card.y + 35, 25, SKYBLUE);

        DrawText(TextFormat("Credits: $%.2f", g->accounts[i].credits), (int)card.x + 20, (int)card.y + 60, 20, LIGHTGRAY);
        if (g->accounts[i].is_ai)
            DrawText("AI BOT", (int)card.x + 300, (int)card.y + 40, 20, GOLD);
    }

    if (GetTime() < g->account_status_timer + 3.0)
    {
        int text_w = MeasureText(g->account_status_message, 25);
        DrawText(g->account_status_message, (int)(CENTER_X - (float)text_w / 2.0f), 920, 25, YELLOW);
    }

    DrawText("Up/Down to Select - A to Toggle - B to Return", (int)(CENTER_X - 300.0f), 960, 20, GRAY);
}

void UpdateAccountsManager(LobbyState *g, Vector2 mouse)
{
    float start_y = 150.0f;
    float card_h = 100.0f;
    float spacing = 10.0f;

    int gamepad = GetActiveGamepad();
    bool trigger_action = false;

    // === KEYBOARD INPUT ===
    if (IsKeyPressed(KEY_UP)) {
        g->menu_selection--;
        if (g->menu_selection < 0) g->menu_selection = g->account_count - 1;
    }
    if (IsKeyPressed(KEY_DOWN)) {
        g->menu_selection++;
        if (g->menu_selection >= g->account_count) g->menu_selection = 0;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        trigger_action = true;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        SwitchState(g, STATE_MAIN_MENU);
    }
    
    // === GAMEPAD INPUT ===
    if (gamepad >= 0) {
        if (IsGamepadButtonPressedSDL(gamepad, 11)) { // D-Pad Up
            g->menu_selection--;
            if (g->menu_selection < 0) g->menu_selection = g->account_count - 1;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 12)) { // D-Pad Down
            g->menu_selection++;
            if (g->menu_selection >= g->account_count) g->menu_selection = 0;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 0)) { // A Button
            trigger_action = true;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 1)) { // B Button
            SwitchState(g, STATE_MAIN_MENU);
        }
    }

    // === MOUSE INPUT ===
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        for (int i = 0; i < g->account_count; i++)
        {
            Rectangle card = {CENTER_X - 450.0f, start_y + (float)i * (card_h + spacing), 900.0f, card_h};
            if (CheckCollisionPointRec(mouse, card))
            {
                g->menu_selection = i;
                trigger_action = true;
                break;
            }
        }
        if (mouse.y > 900)
            SwitchState(g, STATE_MAIN_MENU);
    }

    // === LOGIN/LOGOUT LOGIC ===
    if (trigger_action)
    {
        int i = g->menu_selection;
        if (g->p1_account_index == i)
        {
            LogoutAccount(g, 1);
            ShowAccountStatus(g, "P1 Logged Out");
        }
        else if (g->p2_account_index == i)
        {
            LogoutAccount(g, 2);
            ShowAccountStatus(g, "P2 Logged Out");
        }
        else
        {
            if (g->accounts[i].is_ai)
            {
                LoginAccount(g, i, 2);
                ShowAccountStatus(g, TextFormat("AI %s logged into P2", g->accounts[i].first_name));
            }
            else
            {
                if (g->p1_account_index == -1)
                {
                    LoginAccount(g, i, 1);
                    ShowAccountStatus(g, TextFormat("%s logged into P1", g->accounts[i].first_name));
                }
                else
                {
                    LoginAccount(g, i, 2);
                    ShowAccountStatus(g, TextFormat("%s logged into P2", g->accounts[i].first_name));
                }
            }
        }
        SaveAllAccounts(g);
    }
}

// ============================================================================
// SHOP
// ============================================================================

void DrawShop(const LobbyState *g)
{
    DrawText("TOKEN SHOP", (int)(CENTER_X - 250.0f), 100, 80, GOLD);

    const char *p1 = (g->p1_account_index == -1) ? "Not Logged In" : GetPlayerName(g, 1);
    DrawText("P1 Active Account:", 150, 220, 35, LIME);
    DrawText(p1, 500, 220, 35, WHITE);

    if (g->p1_account_index >= 0)
    {
        DrawText(TextFormat("Credits: $%.2f", g->accounts[g->p1_account_index].credits), 150, 270, 30, LIME);
        DrawText(TextFormat("Tokens: %.1f", g->accounts[g->p1_account_index].tokens), 150, 310, 30, GOLD);
    }

    DrawText("AVAILABLE PURCHASES:", (int)(CENTER_X - 200), 380, 40, SKYBLUE);

    Rectangle p1_buy_rect = {CENTER_X - 200, 500, 400, 150};
    bool selected = (g->menu_selection == 0);

    DrawRectangleRec(p1_buy_rect, selected ? LIME : DARKGREEN);
    if (selected)
        DrawRectangleLinesEx(p1_buy_rect, 4, WHITE);

    DrawText("BUY 1 TOKEN", (int)(p1_buy_rect.x + 110), (int)(p1_buy_rect.y + 30), 30, BLACK);
    DrawText(TextFormat("Cost: $%.2f", TOKEN_PRICE), (int)(p1_buy_rect.x + 120), (int)(p1_buy_rect.y + 75), 25, YELLOW);

    DrawText("BACK (B)", (int)(CENTER_X - 50), (int)SCREEN_H - 120, 30, WHITE);
}

void UpdateShop(LobbyState *g, Vector2 mouse)
{
    Rectangle p1_buy_rect = {CENTER_X - 200, 500, 400, 150};
    Rectangle back_rect = {CENTER_X - 150, SCREEN_H - 150, 300, 80};

    int gamepad = GetActiveGamepad();
    bool trigger = false;

    // === KEYBOARD INPUT ===
    if (IsKeyPressed(KEY_ENTER)) {
        trigger = true;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        SwitchState(g, STATE_MAIN_MENU);
    }
    
    // === GAMEPAD INPUT ===
    if (gamepad >= 0) {
        if (IsGamepadButtonPressedSDL(gamepad, 0)) { // A Button
            trigger = true;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 1)) { // B Button
            SwitchState(g, STATE_MAIN_MENU);
        }
    }

    // === MOUSE INPUT ===
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (CheckCollisionPointRec(mouse, p1_buy_rect))
            trigger = true;
        if (CheckCollisionPointRec(mouse, back_rect))
            SwitchState(g, STATE_MAIN_MENU);
    }

    // === PURCHASE LOGIC ===
    if (trigger && g->p1_account_index >= 0)
    {
        bool is_ai = g->accounts[g->p1_account_index].is_ai;
        if (!is_ai && g->accounts[g->p1_account_index].credits >= TOKEN_PRICE)
        {
            g->accounts[g->p1_account_index].credits -= TOKEN_PRICE;
            g->accounts[g->p1_account_index].tokens += 1.0;
            SaveAllAccounts(g);
        }
    }
}

// ============================================================================
// LEADERBOARD
// ============================================================================

static int CompareCash(const void *a, const void *b)
{
    const LeaderboardEntry *entryA = (const LeaderboardEntry *)a;
    const LeaderboardEntry *entryB = (const LeaderboardEntry *)b;
    int result = 0;
    if (entryB->total_winnings > entryA->total_winnings)
        result = 1;
    else if (entryB->total_winnings < entryA->total_winnings)
        result = -1;
    return result;
}

static int CompareMoves(const void *a, const void *b)
{
    const LeaderboardEntry *entryA = (const LeaderboardEntry *)a;
    const LeaderboardEntry *entryB = (const LeaderboardEntry *)b;
    return (entryA->moves_made - entryB->moves_made);
}

static int CompareDate(const void *a, const void *b)
{
    const LeaderboardEntry *entryA = (const LeaderboardEntry *)a;
    const LeaderboardEntry *entryB = (const LeaderboardEntry *)b;
    return strcmp(entryB->timestamp, entryA->timestamp);
}

static bool g_current_asc = false;
static int WrapCash(const void *a, const void *b) { return CompareCash(a, b) * (g_current_asc ? -1 : 1); }
static int WrapMoves(const void *a, const void *b) { return CompareMoves(a, b) * (g_current_asc ? -1 : 1); }
static int WrapDate(const void *a, const void *b) { return CompareDate(a, b) * (g_current_asc ? -1 : 1); }

void SortLeaderboard(LobbyState *g)
{
    if (g->leaderboard_count <= 1)
        return;
    size_t count = (size_t)g->leaderboard_count;
    g_current_asc = g->leaderboard_sort_ascending;
    switch (g->leaderboard_sort_mode)
    {
    case 0:
        qsort(g->leaderboard, count, sizeof(LeaderboardEntry), WrapCash);
        break;
    case 1:
        qsort(g->leaderboard, count, sizeof(LeaderboardEntry), WrapMoves);
        break;
    case 2:
        qsort(g->leaderboard, count, sizeof(LeaderboardEntry), WrapDate);
        break;
    default:
        break;
    }
}

void DrawLeaderboard(const LobbyState *g)
{
    ClearBackground(BLACK);
    DrawText("LEADERBOARD", (int)(CENTER_X - 250), 50, 60, GOLD);

    Rectangle rects[4];
    rects[0] = (Rectangle){CENTER_X - 600, 150, 250, 50};
    rects[1] = (Rectangle){CENTER_X - 320, 150, 200, 50};
    rects[2] = (Rectangle){CENTER_X - 100, 150, 200, 50};
    rects[3] = (Rectangle){CENTER_X + 120, 150, 200, 50};

    bool sel0 = (g->menu_selection == 0);
    bool sel1 = (g->menu_selection == 1);
    bool sel2 = (g->menu_selection == 2);
    bool sel3 = (g->menu_selection == 3);

    const char *game_names[] = {"Joker's Gambit", "Blackjack", "Slots"};
    DrawRectangleRec(rects[0], sel0 ? BLUE : DARKBLUE);
    if (sel0)
        DrawRectangleLinesEx(rects[0], 3, WHITE);
    DrawText(TextFormat("Game: %s", game_names[g->leaderboard_game_filter]), (int)rects[0].x + 10, (int)rects[0].y + 15, 20, WHITE);

    const char *dir = g->leaderboard_sort_ascending ? " ▲" : " ▼";

    DrawRectangleRec(rects[1], (g->leaderboard_sort_mode == 0) ? LIME : GRAY);
    if (sel1)
        DrawRectangleLinesEx(rects[1], 3, WHITE);
    DrawText(TextFormat("Cash%s", (g->leaderboard_sort_mode == 0 ? dir : "")), (int)rects[1].x + 40, (int)rects[1].y + 15, 20, BLACK);

    DrawRectangleRec(rects[2], (g->leaderboard_sort_mode == 1) ? ORANGE : GRAY);
    if (sel2)
        DrawRectangleLinesEx(rects[2], 3, WHITE);
    DrawText(TextFormat("Moves%s", (g->leaderboard_sort_mode == 1 ? dir : "")), (int)rects[2].x + 40, (int)rects[2].y + 15, 20, BLACK);

    DrawRectangleRec(rects[3], (g->leaderboard_sort_mode == 2) ? SKYBLUE : GRAY);
    if (sel3)
        DrawRectangleLinesEx(rects[3], 3, WHITE);
    DrawText(TextFormat("Date%s", (g->leaderboard_sort_mode == 2 ? dir : "")), (int)rects[3].x + 40, (int)rects[3].y + 15, 20, BLACK);

    int y = 230;
    DrawText("PLAYER", (int)CENTER_X - 600, y, 25, YELLOW);
    DrawText("WINNINGS", (int)CENTER_X - 100, y, 25, YELLOW);

    y += 40;
    int displayed = 0;
    for (int i = 0; i < g->leaderboard_count && displayed < 15; i++)
    {
        const LeaderboardEntry *e = &g->leaderboard[i];
        if (e->game_played != g->leaderboard_game_filter)
            continue;
        y += 45;
        DrawText(e->entry_name, (int)CENTER_X - 600, y, 22, WHITE);
        DrawText(TextFormat("$%.2f", e->total_winnings), (int)CENTER_X - 100, y, 22, LIME);
        displayed++;
    }

    DrawText("Back (B)", (int)(CENTER_X - 50), 980, 24, DARKGRAY);
}

void UpdateLeaderboard(LobbyState *g, Vector2 mouse)
{
    Rectangle rects[4];
    rects[0] = (Rectangle){CENTER_X - 600, 150, 250, 50};
    rects[1] = (Rectangle){CENTER_X - 320, 150, 200, 50};
    rects[2] = (Rectangle){CENTER_X - 100, 150, 200, 50};
    rects[3] = (Rectangle){CENTER_X + 120, 150, 200, 50};

    int gamepad = GetActiveGamepad();
    bool trigger = false;

    // === KEYBOARD INPUT ===
    if (IsKeyPressed(KEY_LEFT)) {
        g->menu_selection--;
        if (g->menu_selection < 0) g->menu_selection = 3;
    }
    if (IsKeyPressed(KEY_RIGHT)) {
        g->menu_selection++;
        if (g->menu_selection > 3) g->menu_selection = 0;
    }
    if (IsKeyPressed(KEY_ENTER)) {
        trigger = true;
    }
    if (IsKeyPressed(KEY_ESCAPE)) {
        SwitchState(g, STATE_MAIN_MENU);
    }
    
    // === GAMEPAD INPUT ===
    if (gamepad >= 0) {
        if (IsGamepadButtonPressedSDL(gamepad, 13)) { // D-Pad Left
            g->menu_selection--;
            if (g->menu_selection < 0) g->menu_selection = 3;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 14)) { // D-Pad Right
            g->menu_selection++;
            if (g->menu_selection > 3) g->menu_selection = 0;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 0)) { // A Button
            trigger = true;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 1)) { // B Button
            SwitchState(g, STATE_MAIN_MENU);
        }
    }

    // === MOUSE INPUT ===
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        for (int i = 0; i < 4; i++)
        {
            if (CheckCollisionPointRec(mouse, rects[i]))
            {
                g->menu_selection = i;
                trigger = true;
                break;
            }
        }
        Rectangle back = {CENTER_X - 150, 950, 300, 80};
        if (CheckCollisionPointRec(mouse, back))
            SwitchState(g, STATE_MAIN_MENU);
    }

    // === FILTER/SORT LOGIC ===
    if (trigger)
    {
        if (g->menu_selection == 0)
        {
            g->leaderboard_game_filter = (GameType)((g->leaderboard_game_filter + 1) % 3);
        }
        else
        {
            int mode = g->menu_selection - 1;
            if (g->leaderboard_sort_mode == mode)
            {
                g->leaderboard_sort_ascending = !g->leaderboard_sort_ascending;
            }
            else
            {
                g->leaderboard_sort_mode = mode;
                g->leaderboard_sort_ascending = (mode == 1);
            }
        }
        SortLeaderboard(g);
    }
}