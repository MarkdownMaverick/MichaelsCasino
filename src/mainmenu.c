#include "mainmenu.h"
#include "useraccount.h"
#include "gamepad_sdl.h"
#include <strings.h>
#include <stdlib.h>
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif
#ifndef CLAMP
#define CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))
#endif
void ShowAccountStatus(LobbyState *g, const char *msg)
{
    strncpy(g->account_status_message, msg, sizeof(g->account_status_message) - 1);
    g->account_status_timer = GetTime();
}
// Helper to switch states and reset selection
static void SwitchState(LobbyState *g, UIState newState)
{
    PlaySound(g_menu_confirm_sound);
    g->state = newState;
    g->menu_selection = 0;
    if (newState == STATE_ACHIEVEMENTS)
    {
        g->achievement_cursor_col = 0;
        g->achievement_cursor_row = 0;
        g->achievement_scroll_row = 0;
        g->achievement_view_mode = 0; // Default P1
    }
}
// ============================================================================
// MAIN MENU
// ============================================================================
void DrawMainMenu(const LobbyState *g)
{
    DrawText("MICHAEL'S CASINO", (int)(CENTER_X - 380.0f), 100, 90, GOLD);
    DrawText("Coming Soon: Poker, Roulette & More!", (int)(CENTER_X - 280), 190, 25, LIGHTGRAY);
    Vector2 mouse = GetMousePosition();
    Rectangle rects[6]; // Fix: Size to 6
    float start_y = 230;
    float gap = 80;
    for (int i = 0; i < 6; i++)
    {
        rects[i] = (Rectangle){CENTER_X - 150, start_y + ((float)i * gap), 300, 70};
    }
    const char *labels[] = {"LOBBY", "ACCOUNTS", "SHOP", "SETTINGS", "LEADERBOARD", "ACHIEVEMENTS"};
    Color colors[] = {LIME, SKYBLUE, GOLD, PURPLE, YELLOW, ORANGE};
    for (int i = 0; i < 6; i++)
    {
        bool selected = (g->menu_selection == i);
        bool hovered = CheckCollisionPointRec(mouse, rects[i]);
        DrawRectangleRec(rects[i], (selected || hovered) ? colors[i] : GOLD);
        if (selected)
            DrawRectangleLinesEx(rects[i], 3, WHITE);
        DrawText(labels[i], (int)(rects[i].x + 20), (int)(rects[i].y + 20), 35, (selected || hovered) ? BLACK : WHITE);
    }
    DrawText("F11 or Xbox Button = Fullscreen", (int)(CENTER_X - 180), (int)SCREEN_H - 50, 20, GRAY);
}
void UpdateMainMenu(LobbyState *g, Vector2 mouse)
{
    Rectangle rects[6]; // Fix: Size to 6
    float start_y = 230;
    float gap = 80;
    for (int i = 0; i < 6; i++)
    {
        rects[i] = (Rectangle){CENTER_X - 150, start_y + ((float)i * gap), 300, 70};
    }
    int gamepad = GetActiveGamepad();
    // === KEYBOARD INPUT ===
    if (IsKeyPressed(KEY_UP))
    {
        PlaySound(g_menu_navigate_sound);
        g->menu_selection--;
        if (g->menu_selection < 0)
            g->menu_selection = 5; // Fix: Wrap at 5
    }
    if (IsKeyPressed(KEY_DOWN))
    {
        PlaySound(g_menu_navigate_sound);
        g->menu_selection++;
        if (g->menu_selection > 5) // Fix: Wrap at 5
            g->menu_selection = 0;
    }
    if (IsKeyPressed(KEY_ENTER))
    {
        switch (g->menu_selection)
        {
        case 0:
            SwitchState(g, STATE_LOBBY);
            break;
        case 1:
            SwitchState(g, STATE_ACCOUNTS_MANAGER);
            break;
        case 2:
            SwitchState(g, STATE_SHOP);
            break;
        case 3:
            SwitchState(g, STATE_SETTINGS);
            break;
        case 4:
            SwitchState(g, STATE_LEADERBOARD);
            break;
        case 5: // Fix: Add achievements case
            SwitchState(g, STATE_ACHIEVEMENTS);
            break;
        default:
            break;
        }
    }
    // === GAMEPAD INPUT ===
    if (gamepad >= 0)
    {
        if (IsGamepadButtonPressedSDL(gamepad, 11))
        {
            PlaySound(g_menu_navigate_sound);
            g->menu_selection--;
            if (g->menu_selection < 0)
                g->menu_selection = 5; // Fix: Wrap at 5
        }
        if (IsGamepadButtonPressedSDL(gamepad, 12))
        {
            PlaySound(g_menu_navigate_sound);
            g->menu_selection++;
            if (g->menu_selection > 5) // Fix: Wrap at 5
                g->menu_selection = 0;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 0))
        {
            switch (g->menu_selection)
            {
            case 0:
                SwitchState(g, STATE_LOBBY);
                break;
            case 1:
                SwitchState(g, STATE_ACCOUNTS_MANAGER);
                break;
            case 2:
                SwitchState(g, STATE_SHOP);
                break;
            case 3:
                SwitchState(g, STATE_SETTINGS);
                break;
            case 4:
                SwitchState(g, STATE_LEADERBOARD);
                break;
            case 5: // Fix: Add achievements case
                SwitchState(g, STATE_ACHIEVEMENTS);
                break;
            default:
                break;
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
        else if (CheckCollisionPointRec(mouse, rects[5])) // Fix: Add achievements mouse handling
            SwitchState(g, STATE_ACHIEVEMENTS);
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
    if (IsKeyPressed(KEY_LEFT))
    {
        PlaySound(g_menu_navigate_sound);
        g->menu_selection--;
        if (g->menu_selection < 0)
            g->menu_selection = 2;
    }
    if (IsKeyPressed(KEY_RIGHT))
    {
        PlaySound(g_menu_navigate_sound);
        g->menu_selection++;
        if (g->menu_selection > 2)
            g->menu_selection = 0;
    }
    if (IsKeyPressed(KEY_B))
    {
        SwitchState(g, STATE_MAIN_MENU);
    }
    // === GAMEPAD INPUT ===
    if (gamepad >= 0)
    {
        if (IsGamepadButtonPressedSDL(gamepad, 13))
        {
            PlaySound(g_menu_navigate_sound);
            g->menu_selection--;
            if (g->menu_selection < 0)
                g->menu_selection = 2;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 14))
        {
            PlaySound(g_menu_navigate_sound);
            g->menu_selection++;
            if (g->menu_selection > 2)
                g->menu_selection = 0;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 1))
        {
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
    DrawText("SETTINGS", (int)(CENTER_X - 200.0f), 80, 80, GOLD);
    bool sel0 = (g->menu_selection == 0);
    bool sel1 = (g->menu_selection == 1);
    bool sel2 = (g->menu_selection == 2);
    bool sel3 = (g->menu_selection == 3);
    // Privacy Setting
    DrawText("P2 Card Privacy:", (int)(CENTER_X - 300.0f), 200, 35, WHITE);
    Rectangle cover_btn = {CENTER_X - 150, 250, 300, 70};
    DrawRectangleRec(cover_btn, g->cover_p2_cards ? LIME : RED);
    if (sel0)
        DrawRectangleLinesEx(cover_btn, 4, WHITE);
    DrawText(g->cover_p2_cards ? "COVERED" : "UNCOVERED", (int)(cover_btn.x + 60), (int)(cover_btn.y + 20), 28, BLACK);
    // AI Speed Setting
    DrawText("AI Move Speed:", (int)(CENTER_X - 300.0f), 350, 35, WHITE);
    Rectangle speed_btn = {CENTER_X - 150, 400, 300, 70};
    const char *speed_text = (fabsf(g->ai_move_delay - 0.5f) < 0.001f) ? "FAST" : (fabsf(g->ai_move_delay - 2.0f) < 0.001f) ? "MEDIUM"
                                                                                                                            : "SLOW";
    Color speed_col = (fabsf(g->ai_move_delay - 0.5f) < 0.001f) ? LIME : (fabsf(g->ai_move_delay - 2.0f) < 0.001f) ? ORANGE
                                                                                                                   : RED;
    DrawRectangleRec(speed_btn, speed_col);
    if (sel1)
        DrawRectangleLinesEx(speed_btn, 4, WHITE);
    DrawText(speed_text, (int)(speed_btn.x + 80), (int)(speed_btn.y + 20), 30, BLACK);
    // Music Toggle
    DrawText("Background Music:", (int)(CENTER_X - 300.0f), 500, 35, WHITE);
    Rectangle music_btn = {CENTER_X - 150, 550, 300, 70};
    DrawRectangleRec(music_btn, g->music_enabled ? LIME : RED);
    if (sel2)
        DrawRectangleLinesEx(music_btn, 4, WHITE);
    DrawText(g->music_enabled ? "ON" : "OFF", (int)(music_btn.x + 100), (int)(music_btn.y + 20), 35, BLACK);
    // Window Scale
    DrawText("Window Scale:", (int)(CENTER_X - 300.0f), 650, 35, WHITE);
    Rectangle scale_btn = {CENTER_X - 150, 700, 300, 70};
    const char *scale_text[] = {"100%", "125%", "150%", "75%"};
    DrawRectangleRec(scale_btn, SKYBLUE);
    if (sel3)
        DrawRectangleLinesEx(scale_btn, 4, WHITE);
    DrawText(scale_text[g->window_scale], (int)(scale_btn.x + 90), (int)(scale_btn.y + 20), 35, BLACK);
    if (g->is_fullscreen)
        DrawText("(Disabled in Fullscreen)", (int)(CENTER_X - 120), 780, 20, GRAY);
    // Debug button (only visible if you want, or always for dev)
    Rectangle debug_rect = {CENTER_X - 200.0f, 600.0f, 400.0f, 80.0f};
    Vector2 mouse = GetMousePosition();
    bool debug_hovered = CheckCollisionPointRec(mouse, debug_rect);

    Color debug_bg = debug_hovered ? RED : MAROON;
    DrawRectangleRec(debug_rect, debug_bg);
    DrawRectangleLinesEx(debug_rect, 4.0f, debug_hovered ? ORANGE : DARKGRAY);
    DrawText("DEBUG: Force Achievement Check", (int)(debug_rect.x + 20), (int)(debug_rect.y + 25), 30, WHITE);
    DrawText("BACK (B)", (int)(CENTER_X - 70), (int)SCREEN_H - 100, 35, WHITE);
}
void UpdateSettings(LobbyState *g, Vector2 mouse)
{
    Rectangle cover_btn = {CENTER_X - 150, 250, 300, 70};
    Rectangle speed_btn = {CENTER_X - 150, 400, 300, 70};
    Rectangle music_btn = {CENTER_X - 150, 550, 300, 70};
    Rectangle scale_btn = {CENTER_X - 150, 700, 300, 70};
    Rectangle debug_rect = {CENTER_X - 150, 800, 300, 70};

    int gamepad = GetActiveGamepad();
    // === KEYBOARD INPUT ===
    if (IsKeyPressed(KEY_UP))
    {
        PlaySound(g_menu_navigate_sound);
        g->menu_selection--;
        if (g->menu_selection < 0)
            g->menu_selection = 4;
    }
    if (IsKeyPressed(KEY_DOWN))
    {
        PlaySound(g_menu_navigate_sound);
        g->menu_selection++;
        if (g->menu_selection > 4)
            g->menu_selection = 0;
    }
    if (IsKeyPressed(KEY_ENTER))
    {
        PlaySound(g_menu_confirm_sound);
        if (g->menu_selection == 0)
        {
            g->cover_p2_cards = !g->cover_p2_cards;
        }
        else if (g->menu_selection == 1)
        {
            if (fabsf(g->ai_move_delay - 0.5f) < EPSILON)
                g->ai_move_delay = 2.0f;
            else if (fabsf(g->ai_move_delay - 2.0f) < EPSILON)
                g->ai_move_delay = 3.0f;
            else
                g->ai_move_delay = 0.5f;
        }
        else if (g->menu_selection == 2)
        {
            g->music_enabled = !g->music_enabled;
            if (g->music_enabled)
            {
                PlayMusicStream(g_background_music);
            }
            else
            {
                StopMusicStream(g_background_music);
            }
        }
        else if (g->menu_selection == 3 && !g->is_fullscreen)
        {
            g->window_scale = (WindowScale)((g->window_scale + 1) % 4);
            ApplyWindowScale(g);
        }
        else if (g->menu_selection == 4)
        {
    PlaySound(g_menu_confirm_sound);
            SaveAchievements(g);
            SwitchState(g, STATE_MAIN_MENU);
        }
        
    }
    if (IsKeyPressed(KEY_B))
    {
        SwitchState(g, STATE_MAIN_MENU);
        SaveSettings(g);
    }
    // === GAMEPAD INPUT ===
    if (gamepad >= 0)
    {
        if (IsGamepadButtonPressedSDL(gamepad, 11))
        {
            PlaySound(g_menu_navigate_sound);
            g->menu_selection--;
            if (g->menu_selection < 0)
                g->menu_selection = 3;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 12))
        {
            PlaySound(g_menu_navigate_sound);
            g->menu_selection++;
            if (g->menu_selection > 3)
                g->menu_selection = 0;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 0))
        {
            PlaySound(g_menu_confirm_sound);
            if (g->menu_selection == 0)
            {
                g->cover_p2_cards = !g->cover_p2_cards;
            }
            else if (g->menu_selection == 1)
            {
                if (fabsf(g->ai_move_delay - 0.5f) < EPSILON)
                    g->ai_move_delay = 2.0f;
                else if (fabsf(g->ai_move_delay - 2.0f) < EPSILON)
                    g->ai_move_delay = 3.0f;
                else
                    g->ai_move_delay = 0.5f;
            }
            else if (g->menu_selection == 2)
            {
                g->music_enabled = !g->music_enabled;
                if (g->music_enabled)
                {
                    PlayMusicStream(g_background_music);
                }
                else
                {
                    StopMusicStream(g_background_music);
                }
            }
            else if (g->menu_selection == 3 && !g->is_fullscreen)
            {
                g->window_scale = (WindowScale)((g->window_scale + 1) % 4);
                ApplyWindowScale(g);
            }
               else if (g->menu_selection == 4)
            {
                PlaySound(g_menu_confirm_sound);
            SaveAchievements(g);
            SwitchState(g, STATE_MAIN_MENU);
            }
        }
        if (IsKeyPressed(KEY_B) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 1)))
        { // ESC or B
            SwitchState(g, STATE_MAIN_MENU);
            SaveSettings(g);
        }
    }
    // === MOUSE INPUT ===
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (CheckCollisionPointRec(mouse, cover_btn))
        {
            PlaySound(g_menu_confirm_sound);
            g->cover_p2_cards = !g->cover_p2_cards;
        }
        else if (CheckCollisionPointRec(mouse, speed_btn))
        {
            PlaySound(g_menu_confirm_sound);
            if (fabsf(g->ai_move_delay - 0.5f) < EPSILON)
                g->ai_move_delay = 2.0f;
            else if (fabsf(g->ai_move_delay - 2.0f) < EPSILON)
                g->ai_move_delay = 3.0f;
            else
                g->ai_move_delay = 0.5f;
        }
        else if (CheckCollisionPointRec(mouse, music_btn))
        {
            PlaySound(g_menu_confirm_sound);
            g->music_enabled = !g->music_enabled;
            if (g->music_enabled)
            {
                PlayMusicStream(g_background_music);
            }
            else
            {
                StopMusicStream(g_background_music);
            }
        }
        else if (CheckCollisionPointRec(mouse, scale_btn) && !g->is_fullscreen)
        {
            PlaySound(g_menu_confirm_sound);
            g->window_scale = (WindowScale)((g->window_scale + 1) % 4);
            ApplyWindowScale(g);
        }
        else if (CheckCollisionPointRec(mouse, debug_rect))
        {
            PlaySound(g_menu_confirm_sound);
            SaveAchievements(g);
            SwitchState(g, STATE_MAIN_MENU);
        }
        else if (mouse.y > SCREEN_H - 150)
        {
            SwitchState(g, STATE_MAIN_MENU);
            SaveSettings(g);
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
        DrawText(TextFormat("%s %s", g->accounts[i].first_name, g->accounts[i].last_name),
                 (int)card.x + 20, (int)card.y + 20, 30, WHITE);
        if (g->p1_account_index == i)
            DrawText("P1 ACTIVE", (int)card.x + 700, (int)card.y + 35, 25, LIME);
        else if (g->p2_account_index == i)
            DrawText("P2 ACTIVE", (int)card.x + 700, (int)card.y + 35, 25, SKYBLUE);
        DrawText(TextFormat("Credits: $%.2f", g->accounts[i].credits), (int)card.x + 20, (int)card.y + 60, 20, LIGHTGRAY);
        DrawText(TextFormat("Tokens: $%.2f Member Status: %s",
                            g->accounts[i].tokens,
                            GetMemberStatusString(g->accounts[i].member_status)),
                 (int)card.x + 20, (int)card.y + 80, 20, LIGHTGRAY);
        if (g->accounts[i].is_ai)
            DrawText("AI BOT", (int)card.x + 300, (int)card.y + 40, 20, GOLD);
    }
    if (GetTime() < g->account_status_timer + 3.0)
    {
        int text_w = MeasureText(g->account_status_message, 25);
        DrawText(g->account_status_message, (int)(CENTER_X - (float)text_w / 2.0f), (int)(SCREEN_H - 120), 25, YELLOW);
    }
    DrawText("A = Login | B = Menu", (int)(CENTER_X - 250.0f), (int)SCREEN_H - 60, 20, GRAY);
}
void UpdateAccountsManager(LobbyState *g, Vector2 mouse)
{
    float start_y = 150.0f;
    float card_h = 100.0f;
    float spacing = 10.0f;
    int gamepad = GetActiveGamepad();
    bool trigger_action = false;
    // === KEYBOARD INPUT ===
    if (IsKeyPressed(KEY_UP))
    {
        PlaySound(g_menu_navigate_sound);
        g->menu_selection--;
        if (g->menu_selection < 0)
            g->menu_selection = g->account_count - 1;
    }
    if (IsKeyPressed(KEY_DOWN))
    {
        PlaySound(g_menu_navigate_sound);
        g->menu_selection++;
        if (g->menu_selection >= g->account_count)
            g->menu_selection = 0;
    }
    if (IsKeyPressed(KEY_ENTER))
    {
        trigger_action = true;
    }
    if (IsKeyPressed(KEY_B))
    {
        SwitchState(g, STATE_MAIN_MENU);
    }
    // === GAMEPAD INPUT ===
    if (gamepad >= 0)
    {
        if (IsGamepadButtonPressedSDL(gamepad, 11))
        {
            PlaySound(g_menu_navigate_sound);
            g->menu_selection--;
            if (g->menu_selection < 0)
                g->menu_selection = g->account_count - 1;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 12))
        {
            PlaySound(g_menu_navigate_sound);
            g->menu_selection++;
            if (g->menu_selection >= g->account_count)
                g->menu_selection = 0;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 0))
        {
            trigger_action = true;
        }
        if (IsKeyPressed(KEY_B) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 1)))
        { // ESC or B
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
        if (mouse.y > SCREEN_H - 100)
            SwitchState(g, STATE_MAIN_MENU);
    }
    // === LOGIN/LOGOUT LOGIC ===
    if (trigger_action)
    {
        PlaySound(g_menu_confirm_sound);
        int i = g->menu_selection;
        if (g->p1_account_index == i)
        {
            LogoutAccount(g, 1);
            ShowAccountStatus(g, "Logged Out");
        }
        else
        {
            if (g->accounts[i].is_ai)
            {
                ShowAccountStatus(g, TextFormat("Dealer Name: %s", g->accounts[i].first_name));
            }
            else
            {
                if (g->p1_account_index == -1)
                {
                    LoginAccount(g, i, 1);
                    ShowAccountStatus(g, TextFormat("%s Logged In!", g->accounts[i].first_name));
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
    DrawText("Player 1 Active Account:", 150, 220, 35, LIME);
    DrawText(p1, 500, 220, 35, WHITE);
    if (g->p1_account_index >= 0)
    {
        DrawText(TextFormat("Credits: $%.2f", g->accounts[g->p1_account_index].credits), 150, 270, 30, LIME);
        DrawText(TextFormat("Tokens: %.1f", g->accounts[g->p1_account_index].tokens), 150, 310, 30, GOLD);
    }
    DrawText("AVAILABLE PURCHASES :", (int)(CENTER_X - 200), 380, 40, SKYBLUE);
    Rectangle p1_buy_rect = {CENTER_X - 200, 500, 400, 150};
    bool selected = (g->menu_selection == 0);
    DrawRectangleRec(p1_buy_rect, selected ? LIME : DARKGREEN);
    if (selected)
        DrawRectangleLinesEx(p1_buy_rect, 4, WHITE);
    DrawText("BUY TOKENS", (int)(p1_buy_rect.x + 110), (int)(p1_buy_rect.y + 30), 30, BLACK);
    DrawText(TextFormat("Cost: $%.2f", TOKEN_PRICE), (int)(p1_buy_rect.x + 120), (int)(p1_buy_rect.y + 75), 25, YELLOW);
    DrawText("BACK (B)", (int)(CENTER_X - 50), (int)SCREEN_H - 120, 30, WHITE);
}
void UpdateShop(LobbyState *g, Vector2 mouse)
{
    Rectangle p1_buy_rect = {CENTER_X - 200, 500, 400, 150};
    int gamepad = GetActiveGamepad();
    bool trigger = false;
    bool buy_pressed = false;
    // === KEYBOARD INPUT ===
    // Navigation
    if (IsKeyPressed(KEY_LEFT) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 11)))
    {
        g->menu_selection--;
        if (g->menu_selection < 0)
            g->menu_selection = 3; // 4 items total (3 buy + 1 sell)
    }
    if (IsKeyPressed(KEY_RIGHT) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 12)))
    {
        g->menu_selection++;
        if (g->menu_selection > 3)
            g->menu_selection = 0;
    }
    // Back with B button
    if (IsKeyPressed(KEY_B) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 1)))
    {
        SwitchState(g, STATE_MAIN_MENU);
    }
    if (IsKeyPressed(KEY_ENTER))
    {
        trigger = true;
    }
    if (IsKeyPressed(KEY_B))
    {
        SwitchState(g, STATE_MAIN_MENU);
    }
    // === GAMEPAD INPUT ===
    if (gamepad >= 0)
    {
        if (IsGamepadButtonPressedSDL(gamepad, 2))
        { // X button
            buy_pressed = true;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 1))
        {
            SwitchState(g, STATE_MAIN_MENU);
        }
    }
    if (buy_pressed || trigger)
    {
        // Check conditions
        if (g->p1_account_index < 0)
        {
            ShowAccountStatus(g, "Log in first!");
            PlaySound(g_place_sound);
        }
        else
        {
            int idx = g->p1_account_index;
            Account *acc = &g->accounts[idx];
            if (acc->credits < TOKEN_PRICE)
            {
                ShowAccountStatus(g, "Insufficient Credits!");
                PlaySound(g_place_sound);
            }
            else
            {
                // Success
                acc->credits -= TOKEN_PRICE;
                ShowAccountStatus(g, "Purchase successful!");
                PlaySound(g_coin_sound);
                g->accounts[g->p1_account_index].tokens += 1.0;
                UpdateAccountCredits(g);
            }
        }
    }
    // === MOUSE INPUT ===
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (CheckCollisionPointRec(mouse, p1_buy_rect))
            trigger = true;
        if (mouse.y > SCREEN_H - 150)
            SwitchState(g, STATE_MAIN_MENU);
    }
    // === PURCHASE LOGIC ===
    if (trigger && g->p1_account_index >= 0)
    {
        PlaySound(g_menu_confirm_sound);
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
    if (entryB->total_winnings > entryA->total_winnings)
        return 1;
    if (entryB->total_winnings < entryA->total_winnings)
        return -1;
    return 0;
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
    // Filter buttons
    Rectangle rects[4];
    rects[0] = (Rectangle){CENTER_X - 600, 130, 250, 50};
    rects[1] = (Rectangle){CENTER_X - 320, 130, 200, 50};
    rects[2] = (Rectangle){CENTER_X - 100, 130, 200, 50};
    rects[3] = (Rectangle){CENTER_X + 120, 130, 200, 50};
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
    // Header
    int y = 210;
    DrawText("RANK", (int)CENTER_X - 650, y, 25, YELLOW);
    DrawText("PLAYER", (int)CENTER_X - 550, y, 25, YELLOW);
    DrawText("WINNINGS", (int)CENTER_X - 50, y, 25, YELLOW);
    DrawText("ROUNDS", (int)CENTER_X + 150, y, 25, YELLOW);
    DrawText("MOVES", (int)CENTER_X + 300, y, 25, YELLOW);
    DrawText("Date/Time", (int)CENTER_X + 450, y, 25, YELLOW);
    // Entries
    y = 250;
    int displayed = 0;
    int rank = 1;
    for (int i = 0; i < g->leaderboard_count && displayed < 15; i++)
    {
        const LeaderboardEntry *e = &g->leaderboard[i];
        if (e->game_played != g->leaderboard_game_filter)
            continue;
        DrawText(TextFormat("#%d", rank), (int)CENTER_X - 650, y, 22, GOLD);
        DrawText(e->winner_name, (int)CENTER_X - 550, y, 22, WHITE);
        DrawText(TextFormat("$%.2f", e->total_winnings), (int)CENTER_X - 50, y, 22, LIME);
        DrawText(TextFormat("%d", e->total_rounds), (int)CENTER_X + 170, y, 22, SKYBLUE);
        DrawText(TextFormat("%d", e->moves_made), (int)CENTER_X + 320, y, 22, ORANGE);
        DrawText(e->timestamp, 1400, y, 20, LIGHTGRAY); // Right-aligned timestamp
        y += 40;
        displayed++;
        rank++;
    }
    if (displayed == 0)
    {
        DrawText("No entries for this game yet!", (int)(CENTER_X - 180), 400, 30, GRAY);
    }
    DrawText("Back (B)", (int)(CENTER_X - 50), (int)SCREEN_H - 60, 24, DARKGRAY);
}
void UpdateLeaderboard(LobbyState *g, Vector2 mouse)
{
    Rectangle rects[4];
    rects[0] = (Rectangle){CENTER_X - 600, 130, 250, 50};
    rects[1] = (Rectangle){CENTER_X - 320, 130, 200, 50};
    rects[2] = (Rectangle){CENTER_X - 100, 130, 200, 50};
    rects[3] = (Rectangle){CENTER_X + 120, 130, 200, 50};
    int gamepad = GetActiveGamepad();
    bool trigger = false;
    // === KEYBOARD INPUT ===
    if (IsKeyPressed(KEY_LEFT))
    {
        PlaySound(g_menu_navigate_sound);
        g->menu_selection--;
        if (g->menu_selection < 0)
            g->menu_selection = 3;
    }
    if (IsKeyPressed(KEY_RIGHT))
    {
        PlaySound(g_menu_navigate_sound);
        g->menu_selection++;
        if (g->menu_selection > 3)
            g->menu_selection = 0;
    }
    if (IsKeyPressed(KEY_ENTER))
    {
        trigger = true;
    }
    if (IsKeyPressed(KEY_B))
    {
        SwitchState(g, STATE_MAIN_MENU);
    }
    // === GAMEPAD INPUT ===
    if (gamepad >= 0)
    {
        if (IsGamepadButtonPressedSDL(gamepad, 13))
        {
            PlaySound(g_menu_navigate_sound);
            g->menu_selection--;
            if (g->menu_selection < 0)
                g->menu_selection = 3;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 14))
        {
            PlaySound(g_menu_navigate_sound);
            g->menu_selection++;
            if (g->menu_selection > 3)
                g->menu_selection = 0;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 0))
        {
            trigger = true;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 1))
        {
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
        if (mouse.y > SCREEN_H - 100)
            SwitchState(g, STATE_MAIN_MENU);
    }
    // === FILTER/SORT LOGIC ===
    if (trigger)
    {
        PlaySound(g_menu_confirm_sound);
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
// ============================================================================
// Achievements
// ============================================================================
void DrawAchievements(const LobbyState *g)
{
    DrawText("ACHIEVEMENTS", (int)(CENTER_X - 200.0f), 100, 60, ORANGE);
    // Find human account indices
    int human_indices[2] = {-1, -1};
    int human_count = 0;
    for (int i = 0; i < g->account_count && human_count < 2; i++)
    {
        if (!g->accounts[i].is_ai)
        {
            human_indices[human_count++] = i;
        }
    }
    bool p1_valid = (human_count >= 1);
    bool p2_valid = (human_count >= 2);
    int view_mode = g->achievement_view_mode;
    view_mode = CLAMP(view_mode, 0, human_count - 1);
    int idx = human_indices[view_mode];
    const Account *acc = &g->accounts[idx];
    if (g_achievements_atlas.id == 0)
    {
        DrawText("Failed to load atlas/achievements.png", (int)(CENTER_X - 250.0f), 400, 30, RED);
    }
    // Player switch buttons
    Rectangle p1_rect = {CENTER_X - 250.0f, 170.0f, 100.0f, 45.0f};
    Rectangle p2_rect = {CENTER_X + 150.0f, 170.0f, 100.0f, 45.0f};
    Vector2 mouse = GetMousePosition();
    bool p1_hovered = CheckCollisionPointRec(mouse, p1_rect);
    bool p2_hovered = CheckCollisionPointRec(mouse, p2_rect);
    DrawRectangleRec(p1_rect, p1_valid ? (p1_hovered ? SKYBLUE : LIGHTGRAY) : GRAY);
    DrawRectangleRec(p2_rect, p2_valid ? (p2_hovered ? SKYBLUE : LIGHTGRAY) : GRAY);
    DrawText("P1", (int)p1_rect.x + 25, (int)p1_rect.y + 12, 28, BLACK);
    DrawText("P2", (int)p2_rect.x + 30, (int)p2_rect.y + 12, 28, BLACK);
    DrawText(TextFormat("Viewing: %s", GetPlayerNameByIndex(g, idx)), (int)(CENTER_X - 200.0f), 250, 30, WHITE);
    DrawText(TextFormat("(%s)", GetMemberStatusString(acc->member_status)), (int)(CENTER_X - 50.0f), 250, 30, WHITE);
    int unlocked_count = 0;
    for (int i = 0; i < MAX_ACHIEVEMENTS; i++)
    {
        if (acc->achievements[i].unlocked)
            unlocked_count++;
    }
    DrawText(TextFormat("%d / %d Unlocked", unlocked_count, MAX_ACHIEVEMENTS), (int)(CENTER_X - 100.0f), 290, 25, GOLD);
    // Grid setup
    int icon_size = 130;
    int col_spacing = 25;
    int row_height = 165;
    int grid_cols = 5;
    int grid_rows = 10;
    float grid_width = (float)grid_cols * (float)icon_size + (float)(grid_cols - 1) * (float)col_spacing;
    float start_x = CENTER_X - grid_width / 2.0f;
    float start_y = 330.0f;
    int visible_rows = (int)((SCREEN_H - 500.0f) / (float)row_height);
    visible_rows = CLAMP(visible_rows, 3, grid_rows);
    int scroll_row = g->achievement_scroll_row;
    scroll_row = CLAMP(scroll_row, 0, grid_rows - visible_rows);
    float col_x[5];
    for (int c = 0; c < 5; c++)
    {
        col_x[c] = start_x + (float)c * ((float)icon_size + (float)col_spacing);
    }
    // Draw visible grid
    for (int r_rel = 0; r_rel < visible_rows && (scroll_row + r_rel) < grid_rows; r_rel++)
    {
        int grid_row = scroll_row + r_rel;
        float row_y = start_y + (float)r_rel * (float)row_height;
        for (int c = 0; c < grid_cols; c++)
        {
            int ach_id = grid_row * grid_cols + c;
            if (ach_id >= MAX_ACHIEVEMENTS)
                continue;
            Rectangle src = {(float)(c * 150), (float)(grid_row * 150), 150.0f, 150.0f};
            Rectangle dest = {col_x[c], row_y, (float)icon_size, (float)icon_size};
            DrawTexturePro(g_achievements_atlas, src, dest, (Vector2){0, 0}, 0.0f, WHITE);
            bool unlocked = acc->achievements[ach_id].unlocked;
            const char *status = unlocked ? "UNLOCKED" : "LOCKED";
            Color status_color = unlocked ? GREEN : RED;
            Vector2 text_size = MeasureTextEx(GetFontDefault(), status, 22.0f, 1.0f);
            DrawText(status, (int)(dest.x + (float)icon_size / 2.0f - text_size.x / 2.0f),
                     (int)(dest.y + (float)icon_size + 8.0f), 22, status_color);
            // Cursor highlight
            if (c == g->achievement_cursor_col && grid_row == g->achievement_cursor_row)
            {
                DrawRectangleLinesEx(dest, 5.0f, YELLOW);
                DrawRectangleLinesEx(dest, 2.0f, GOLD);
            }
        }
    }
    // Scroll indicators
    if (scroll_row > 0)
    {
        DrawText("↑", (int)(CENTER_X + 350.0f), (int)start_y - 30, 40, WHITE);
    }
    if (scroll_row + visible_rows < grid_rows)
    {
        DrawText("↓", (int)(CENTER_X + 350.0f), (int)(start_y + (float)visible_rows * (float)row_height + 20.0f), 40, WHITE);
    }
    // Info rect at bottom for selected achievement
    int selected_ach_id = g->achievement_cursor_row * grid_cols + g->achievement_cursor_col;
    if (selected_ach_id < MAX_ACHIEVEMENTS)
    {
        Rectangle info_rect = {CENTER_X - 400.0f, SCREEN_H - 150.0f, 800.0f, 120.0f};
        DrawRectangleRec(info_rect, DARKBLUE);
        DrawRectangleLinesEx(info_rect, 2.0f, LIGHTGRAY);
        bool unlocked = acc->achievements[selected_ach_id].unlocked;
        Color name_color = unlocked ? GREEN : GRAY;
        DrawText(acc->achievements[selected_ach_id].name, (int)(info_rect.x + 20.0f), (int)(info_rect.y + 20.0f), 30, name_color);
        DrawText(acc->achievements[selected_ach_id].description, (int)(info_rect.x + 20.0f), (int)(info_rect.y + 60.0f), 20, WHITE);
        DrawText(unlocked ? "Unlocked" : "Locked", (int)(info_rect.x + 20.0f), (int)(info_rect.y + 90.0f), 20, unlocked ? GREEN : RED);
    }
    DrawText("LB/RB, 1/2, Click: Switch Player | D-Pad/WASD/Arrows: Navigate | B/ESC: Back",
             (int)(CENTER_X - 450.0f), (int)(SCREEN_H - 170.0f), 20, DARKGRAY); // Moved up for info rect
}
void UpdateAchievements(LobbyState *g, Vector2 mouse)
{
    int gamepad = GetActiveGamepad();
    // Compute human count
    int human_count = 0;
    for (int i = 0; i < g->account_count; i++)
    {
        if (!g->accounts[i].is_ai)
            human_count++;
    }
    bool p1_valid = (human_count >= 1);
    bool p2_valid = (human_count >= 2);
    int view_mode = g->achievement_view_mode;
    view_mode = CLAMP(view_mode, 0, human_count - 1);
    // Player switch buttons
    Rectangle p1_rect = {CENTER_X - 250.0f, 170.0f, 100.0f, 45.0f};
    Rectangle p2_rect = {CENTER_X + 150.0f, 170.0f, 100.0f, 45.0f};
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
        if (CheckCollisionPointRec(mouse, p1_rect) && p1_valid)
        {
            g->achievement_view_mode = 0;
            PlaySound(g_menu_confirm_sound);
            return;
        }
        if (CheckCollisionPointRec(mouse, p2_rect) && p2_valid)
        {
            g->achievement_view_mode = 1;
            PlaySound(g_menu_confirm_sound);
            return;
        }
    }
    if (IsKeyPressed(KEY_ONE) && p1_valid)
    {
        g->achievement_view_mode = 0;
        PlaySound(g_menu_confirm_sound);
    }
    if (IsKeyPressed(KEY_TWO) && p2_valid)
    {
        g->achievement_view_mode = 1;
        PlaySound(g_menu_confirm_sound);
    }
    if (gamepad >= 0)
    {
        if (IsGamepadButtonPressedSDL(gamepad, 4) && p1_valid)
        { // Left Shoulder
            g->achievement_view_mode = 0;
            PlaySound(g_menu_confirm_sound);
        }
        if (IsGamepadButtonPressedSDL(gamepad, 5) && p2_valid)
        { // Right Shoulder
            g->achievement_view_mode = 1;
            PlaySound(g_menu_confirm_sound);
        }
        if (IsGamepadButtonPressedSDL(gamepad, 1))
        { // B
            SwitchState(g, STATE_MAIN_MENU);
            return;
        }
    }
    if (IsKeyPressed(KEY_B))
    {
        SwitchState(g, STATE_MAIN_MENU);
        return;
    }
    // Grid navigation setup
    int grid_cols = 5;
    int grid_rows = 10;
    int icon_size = 130;
    int col_spacing = 25;
    int row_height = 165;
    int visible_rows = (int)((SCREEN_H - 500.0f) / (float)row_height);
    visible_rows = CLAMP(visible_rows, 3, grid_rows);
    g->achievement_scroll_row = CLAMP(g->achievement_scroll_row, 0, grid_rows - visible_rows);
    // Grid nav inputs (prioritize controller D-pad, then keyboard)
    bool moved = false;
    if (gamepad >= 0)
    {
        if (IsGamepadButtonPressedSDL(gamepad, 11))
        { // D-pad Up
            g->achievement_cursor_row = (g->achievement_cursor_row - 1 + grid_rows) % grid_rows;
            moved = true;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 12))
        { // D-pad Down
            g->achievement_cursor_row = (g->achievement_cursor_row + 1) % grid_rows;
            moved = true;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 13))
        { // D-pad Left
            g->achievement_cursor_col = (g->achievement_cursor_col - 1 + grid_cols) % grid_cols;
            moved = true;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 14))
        { // D-pad Right
            g->achievement_cursor_col = (g->achievement_cursor_col + 1) % grid_cols;
            moved = true;
        }
    }
    // Keyboard: WASD + Arrows
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
    {
        g->achievement_cursor_row = (g->achievement_cursor_row - 1 + grid_rows) % grid_rows;
        moved = true;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
    {
        g->achievement_cursor_row = (g->achievement_cursor_row + 1) % grid_rows;
        moved = true;
    }
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
    {
        g->achievement_cursor_col = (g->achievement_cursor_col - 1 + grid_cols) % grid_cols;
        moved = true;
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
    {
        g->achievement_cursor_col = (g->achievement_cursor_col + 1) % grid_cols;
        moved = true;
    }
    if (moved)
    {
        PlaySound(g_menu_navigate_sound);
    }
    // Auto-scroll to keep cursor visible (center-ish)
    int target_scroll = g->achievement_cursor_row - visible_rows / 2;
    target_scroll = CLAMP(target_scroll, 0, grid_rows - visible_rows);
    g->achievement_scroll_row = target_scroll;
    // Mouse hover snap to grid cell
    float grid_width = (float)grid_cols * (float)icon_size + (float)(grid_cols - 1) * (float)col_spacing;
    float start_x = CENTER_X - grid_width / 2.0f;
    float start_y = 330.0f;
    int scroll_row = g->achievement_scroll_row;
    for (int r_rel = 0; r_rel < visible_rows && (scroll_row + r_rel) < grid_rows; r_rel++)
    {
        int grid_row = scroll_row + r_rel;
        float row_y = start_y + (float)r_rel * (float)row_height;
        for (int c = 0; c < grid_cols; c++)
        {
            float cell_x = start_x + (float)c * ((float)icon_size + (float)col_spacing);
            Rectangle cell_rect = {cell_x, row_y, (float)icon_size, (float)icon_size};
            if (CheckCollisionPointRec(mouse, cell_rect))
            {
                g->achievement_cursor_col = c;
                g->achievement_cursor_row = grid_row;
                int hover_scroll = grid_row - visible_rows / 2;
                hover_scroll = CLAMP(hover_scroll, 0, grid_rows - visible_rows);
                g->achievement_scroll_row = hover_scroll;
                break;
            }
        }
    }
}