#include "mainmenu.h"
#include "useraccount.h"
#include "gamepad_sdl.h"
#include <strings.h>
#include <stdlib.h>
#include "main.h"
#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

void ShowNotification(LobbyState *g, const char *title, const char *subtitle)
{
    strncpy(g->notification_text, title, 63);
    strncpy(g->notification_subtext, subtitle, 63);
    g->notification_timer = GetTime() + 4.0; // Show for 4 seconds
    g->notification_y_offset = -100.0f;      // Start off-screen (top)
}

void DrawNotification(LobbyState *g)
{
    if (GetTime() > g->notification_timer)
        return;

    // Animation: Slide in and out
    float time_left = (float)(g->notification_timer - GetTime());
    float target_y = 20.0f; // Target position at top of screen

    // Slide In
    if (time_left > 3.5f)
    {
        g->notification_y_offset += (target_y - g->notification_y_offset) * 0.1f;
    }
    // Slide Out
    else if (time_left < 0.5f)
    {
        g->notification_y_offset -= 5.0f;
    }
    // Stay
    else
    {
        g->notification_y_offset = target_y;
    }

    // Draw the Toast Box
    float w = 400;
    float h = 80;
    float x = CENTER_X - (w / 2.0f);
    float y = g->notification_y_offset;

    DrawRectangle((int)x, (int)y, (int)w, (int)h, DARKBLUE);
    DrawRectangleLines((int)x, (int)y, (int)w, (int)h, GOLD);

    // Icon (Placeholder or use Atlas)
    DrawRectangle((int)x + 10, (int)y + 10, 60, 60, GOLD);
    DrawText("!", (int)x + 30, (int)y + 20, 40, DARKBLUE);

    // Text
    DrawText(g->notification_text, (int)x + 80, (int)y + 15, 20, GOLD);
    DrawText(g->notification_subtext, (int)x + 80, (int)y + 40, 20, WHITE);
}

void ShowAccountStatus(LobbyState *g, const char *msg)
{
    strncpy(g->account_status_message, msg, sizeof(g->account_status_message) - 1);
    g->account_status_timer = GetTime();
}
// Helper to switch states and reset selection
void SwitchState(LobbyState *g, UIState newState)
{
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
        g->menu_selection--;
        if (g->menu_selection < 0)
            g->menu_selection = 5; // Fix: Wrap at 5
    }
    if (IsKeyPressed(KEY_DOWN))
    {
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
                g->menu_selection--;
            if (g->menu_selection < 0)
                g->menu_selection = 5; // Fix: Wrap at 5
        }
        if (IsGamepadButtonPressedSDL(gamepad, 12))
        {
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
// ============================================================================
//  
// ============================================================================

void UpdateLobby(LobbyState *g, Vector2 mouse)
{
    int gamepad = GetActiveGamepad();

    // === KEYBOARD INPUT ===
    if (IsKeyPressed(KEY_LEFT))
    {
        g->menu_selection--;
        if (g->menu_selection < 0)
            g->menu_selection = 2;
    }
    if (IsKeyPressed(KEY_RIGHT))
    {
        g->menu_selection++;
        if (g->menu_selection > 2)
            g->menu_selection = 0;
    }
    if (IsKeyPressed(KEY_B))
    {
        SwitchState(g, STATE_MAIN_MENU);
        return;
    }

    // === GAMEPAD INPUT ===
    if (gamepad >= 0)
    {
        if (IsGamepadButtonPressedSDL(gamepad, 13))
        {
                g->menu_selection--;
            if (g->menu_selection < 0)
                g->menu_selection = 2;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 14))
        {
                g->menu_selection++;
            if (g->menu_selection > 2)
                g->menu_selection = 0;
        }
        if (IsGamepadButtonPressedSDL(gamepad, 1))
        {
            SwitchState(g, STATE_MAIN_MENU);
            return;
        }
    }

    // === SELECTION ===
    bool trigger = IsKeyPressed(KEY_ENTER) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 0));

    // === MOUSE INPUT ===
    Rectangle game_rects[3];
    game_rects[0] = (Rectangle){CENTER_X - 450, 300, 300, 120};
    game_rects[1] = (Rectangle){CENTER_X - 150, 300, 300, 120};
    game_rects[2] = (Rectangle){CENTER_X + 150, 300, 300, 120};

    for (int i = 0; i < 3; i++)
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, game_rects[i]))
        {
            g->menu_selection = i;
            trigger = true;
        }
    }

    Rectangle back_rect = {CENTER_X - 150, SCREEN_H - 150, 300, 80};
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, back_rect))
    {
        SwitchState(g, STATE_MAIN_MENU);
        return;
    }

    // Handle selection
    if (trigger)
    {
    
        switch (g->menu_selection)
        {
        case 0: // Joker's Gambit
            if (g->p1_account_index < 0)
            {
                ShowNotification(g, "LOGIN REQUIRED", "Log in via Accounts Manager!");
            }
            else
            {
                SwitchState(g, STATE_MODE_SELECTION);
            }
            break;

        case 1: // Blackjack
            ShowNotification(g, "COMING SOON", "Blackjack is under development!");
            break;

        case 2: // Slot Reels
            ShowNotification(g, "COMING SOON", "Slots are under development!");
            break;
        default:
            break;
        }    
        }
    }

    // ============================================================================
    // SETTINGS
    // ============================================================================
    void DrawSettings(const LobbyState *g)
    {
        Vector2 mouse = GetMousePosition();
        DrawText("SETTINGS", (int)(CENTER_X - 200.0f), 100, 60, PURPLE);
        // Music toggle
        Rectangle music_rect = {CENTER_X - 150.0f, 200.0f, 300.0f, 60.0f};
        bool music_hovered = CheckCollisionPointRec(mouse, music_rect);
        bool music_selected = (g->menu_selection == 0);
        DrawRectangleRec(music_rect, music_selected || music_hovered ? LIME : GREEN);
        DrawText(g->music_enabled ? "Music: ON" : "Music: OFF", (int)music_rect.x + 20, (int)music_rect.y + 15, 30, BLACK);
        // Cover P2 cards
        Rectangle cover_rect = {CENTER_X - 150.0f, 280.0f, 300.0f, 60.0f};
        bool cover_hovered = CheckCollisionPointRec(mouse, cover_rect);
        bool cover_selected = (g->menu_selection == 1);
        DrawRectangleRec(cover_rect, cover_selected || cover_hovered ? LIME : GREEN);
        DrawText(g->cover_p2_cards ? "Cover P2 Cards: ON" : "Cover P2 Cards: OFF", (int)cover_rect.x + 20, (int)cover_rect.y + 15, 30, BLACK);
        // AI Delay (example slider)
        Rectangle delay_rect = {CENTER_X - 150.0f, 360.0f, 300.0f, 60.0f};
        bool delay_hovered = CheckCollisionPointRec(mouse, delay_rect);
        bool delay_selected = (g->menu_selection == 2);
        DrawRectangleRec(delay_rect, delay_selected || delay_hovered ? LIME : GREEN);
        const char *delay_names[] = {"Fast (0.5s)", "Normal (1.0s)", "Slow (2.0s)"};
        DrawText(TextFormat("AI Think Delay: %s", delay_names[g->ai_delay_mode]),
                 (int)delay_rect.x + 20, (int)delay_rect.y + 15, 30, BLACK);
        // Window Scale
        Rectangle scale_rect = {CENTER_X - 150.0f, 440.0f, 300.0f, 60.0f};
        bool scale_hovered = CheckCollisionPointRec(mouse, scale_rect);
        bool scale_selected = (g->menu_selection == 3);
        DrawRectangleRec(scale_rect, scale_selected || scale_hovered ? LIME : GREEN);
        const char *scales[] = {"100%", "125%", "150%", "75%"};
        DrawText(TextFormat("Window Scale: %s", scales[g->window_scale]), (int)scale_rect.x + 20, (int)scale_rect.y + 15, 30, BLACK);
        // Fullscreen toggle
        Rectangle fs_rect = {CENTER_X - 150.0f, 520.0f, 300.0f, 60.0f};
        bool fs_hovered = CheckCollisionPointRec(mouse, fs_rect);
        bool fs_selected = (g->menu_selection == 4);
        DrawRectangleRec(fs_rect, fs_selected || fs_hovered ? LIME : GREEN);
        DrawText(g->is_fullscreen ? "Fullscreen: ON" : "Fullscreen: OFF", (int)fs_rect.x + 20, (int)fs_rect.y + 15, 30, BLACK);
        // Debug button
        Rectangle debug_rect = {CENTER_X - 150.0f, 600.0f, 300.0f, 60.0f};
        bool debug_hovered = CheckCollisionPointRec(mouse, debug_rect);
        bool debug_selected = (g->menu_selection == 5);
        DrawRectangleRec(debug_rect, debug_selected || debug_hovered ? RED : MAROON);
        DrawText("DEBUG: Check Achievements", (int)debug_rect.x + 20, (int)debug_rect.y + 15, 30, WHITE);
        // Back button
        Rectangle back_rect = {CENTER_X - 150.0f, SCREEN_H - 120.0f, 300.0f, 60.0f};
        bool back_hovered = CheckCollisionPointRec(mouse, back_rect);
        bool back_selected = (g->menu_selection == 6);
        DrawRectangleRec(back_rect, back_selected || back_hovered ? LIGHTGRAY : GRAY);
        DrawText("Back", (int)back_rect.x + 110, (int)back_rect.y + 15, 30, BLACK);
    }
    void UpdateSettings(LobbyState * g, Vector2 mouse)
    {
        int gamepad = GetActiveGamepad();
        // Navigation (7 items: 0-5 settings + debug, 6 back)
        if (IsKeyPressed(KEY_UP) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 11)))
        {
            g->menu_selection = (g->menu_selection - 1 + 7) % 7;
            }
        if (IsKeyPressed(KEY_DOWN) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 12)))
        {
            g->menu_selection = (g->menu_selection + 1) % 7;
            }
        bool trigger = false;
        if (IsKeyPressed(KEY_ENTER) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 0)))
        {
            trigger = true;
        }
        // Handle selections
        if (trigger)
        {
                    switch (g->menu_selection)
            {
            case 0: // Music
                g->music_enabled = !g->music_enabled;
                if (g->music_enabled)
                    PlayMusicStream(g_background_music);
                else
                    StopMusicStream(g_background_music);
                SaveSettings(g);
                break;
            case 1: // Cover P2
                g->cover_p2_cards = !g->cover_p2_cards;
                SaveSettings(g);
                break;
            case 2:                                                           // AI Delay
                g->ai_delay_mode = (AIDelayMode)((g->ai_delay_mode + 1) % 3); // Cycle through 0-2
                SaveSettings(g);
                break;
            case 3: // Window Scale (cycle enums)
                g->window_scale = (WindowScale)((g->window_scale + 1) % 4);
                ApplyWindowScale(g);
                SaveSettings(g);
                break;
            case 4: // Fullscreen
                ToggleAppFullscreen(g);
                SaveSettings(g);
                break;
            case 5: // Debug Check Achievements
            {
                int human_count = 0;
                for (int i = 0; i < g->account_count && human_count < 2; i++)
                {
                    if (!g->accounts[i].is_ai)
                    {
                        CheckAchievements(&g->accounts[i], g);
                        human_count++;
                    }
                }
                SaveAchievements(g);
                ShowAccountStatus(g, "Achievements Checked!");
                break;
            }
                SaveAchievements(g);
                ShowAccountStatus(g, "Achievements Checked!");
                break;
            case 6: // Back
                SwitchState(g, STATE_MAIN_MENU);
                break;
            default:
                break;
            }
        }
        // Mouse clicks (for each rect, similar to selection)
        Rectangle music_rect = {CENTER_X - 150.0f, 200.0f, 300.0f, 60.0f};
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, music_rect))
        {
            g->music_enabled = !g->music_enabled;
            // ... same as case 0
        }
        // Repeat for other rects, including debug and back
        // ESC / B for back
        if (IsKeyPressed(KEY_ESCAPE) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 1)))
        {
            SwitchState(g, STATE_MAIN_MENU);
        }
    } // ============================================================================
    // ACCOUNTS MANAGER
    // ============================================================================
    void DrawAccountsManager(const LobbyState *g)
    {
        ClearBackground(BLACK);
        DrawText("ACCOUNT MANAGER", (int)(CENTER_X - 300.0f), 50, 60, GOLD);
        float start_y = 150.0f;
        float card_h = 120.0f; // Increased height for insurance button
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
            // Name and status
            DrawText(TextFormat("%s %s", g->accounts[i].first_name, g->accounts[i].last_name),
                     (int)card.x + 20, (int)card.y + 20, 30, WHITE);
            if (g->p1_account_index == i)
                DrawText("P1 ACTIVE", (int)card.x + 700, (int)card.y + 25, 25, LIME);
            // Credits and tokens
            Color credit_color = g->accounts[i].credits < 0 ? RED : LIGHTGRAY;
            DrawText(TextFormat("Credits: $%.2f", g->accounts[i].credits),
                     (int)card.x + 20, (int)card.y + 60, 20, credit_color);
            DrawText(TextFormat("Tokens: %.0f | Status: %s",
                                g->accounts[i].tokens,
                                GetMemberStatusString(g->accounts[i].member_status)),
                     (int)card.x + 20, (int)card.y + 85, 20, LIGHTGRAY);
            // Insurance indicator or button
            if (!g->accounts[i].is_ai)
            {
                if (g->accounts[i].has_insurance)
                {
                    // Show insurance status and button if in trouble
                    if (g->accounts[i].credits <= 0)
                    {
                        Rectangle ins_btn = {card.x + 650, card.y + 55, 220, 50};
                        bool btn_hover = CheckCollisionPointRec(GetMousePosition(), ins_btn);
                        DrawRectangleRec(ins_btn, btn_hover ? GREEN : DARKGREEN);
                        DrawRectangleLinesEx(ins_btn, 3, LIME);
                        DrawText("USE INSURANCE", (int)ins_btn.x + 20, (int)ins_btn.y + 15, 20, WHITE);
                    }
                    else
                    {
                        DrawText("[INSURED]", (int)card.x + 300, (int)card.y + 25, 22, GREEN);
                    }
                }
            }
            if (g->accounts[i].is_ai)
                DrawText("AI BOT", (int)card.x + 720, (int)card.y + 25, 20, GOLD);
        }
        // Status message
        if (GetTime() < g->account_status_timer + 3.0)
        {
            int text_w = MeasureText(g->account_status_message, 25);
            DrawText(g->account_status_message, (int)(CENTER_X - (float)text_w / 2.0f), (int)(SCREEN_H - 120), 25, YELLOW);
        }
        DrawText("A = Login/Select | B = Menu | Click Insurance to Use", (int)(CENTER_X - 320.0f), (int)SCREEN_H - 60, 20, GRAY);
    }
    void UpdateAccountsManager(LobbyState * g, Vector2 mouse)
    {
        float start_y = 150.0f;
        float card_h = 120.0f;
        float spacing = 10.0f;
        int gamepad = GetActiveGamepad();
        bool trigger_action = false;
        // Navigation
        if (IsKeyPressed(KEY_UP) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 11)))
        {
                g->menu_selection--;
            if (g->menu_selection < 0)
                g->menu_selection = g->account_count - 1;
        }
        if (IsKeyPressed(KEY_DOWN) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 12)))
        {
                g->menu_selection++;
            if (g->menu_selection >= g->account_count)
                g->menu_selection = 0;
        }
        // Select action
        if (IsKeyPressed(KEY_ENTER) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 0)))
        {
            trigger_action = true;
        }
        // Back
        if (IsKeyPressed(KEY_B) || IsKeyPressed(KEY_ESCAPE) ||
            (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 1)))
        {
            SwitchState(g, STATE_MAIN_MENU);
            return;
        }
        // Mouse input
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            for (int i = 0; i < g->account_count; i++)
            {
                Rectangle card = {CENTER_X - 450.0f, start_y + (float)i * (card_h + spacing), 900.0f, card_h};
                // Check insurance button
                if (!g->accounts[i].is_ai && g->accounts[i].has_insurance && g->accounts[i].credits <= 0)
                {
                    Rectangle ins_btn = {card.x + 650, card.y + 55, 220, 50};
                    if (CheckCollisionPointRec(mouse, ins_btn))
                    {
                        // Apply insurance immediately
                        g->accounts[i].credits = 10000.0;
                        g->accounts[i].tokens = 10.0;
                        g->accounts[i].has_insurance = false;
                        ShowAccountStatus(g, "Insurance Applied! Credits restored to $10,000");
                        PlaySound(g_coin_sound);
                        SaveAllAccounts(g);
                        return;
                    }
                }
                // Check card click
                if (CheckCollisionPointRec(mouse, card))
                {
                    g->menu_selection = i;
                    trigger_action = true;
                    break;
                }
            }
            if (mouse.y > SCREEN_H - 100)
            {
                SwitchState(g, STATE_MAIN_MENU);
                return;
            }
        }
        // Login/logout logic
        if (trigger_action)
        {
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
                    ShowAccountStatus(g, TextFormat("Dealer: %s", g->accounts[i].first_name));
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
    // SHOP - Replace your existing DrawShop and UpdateShop with these
    // ============================================================================
    void DrawShop(const LobbyState *g)
    {
        ClearBackground(BLACK);
        DrawText("TOKEN SHOP", (int)(CENTER_X - 250.0f), 60, 80, GOLD);
        // Player info at top
        const char *p1 = (g->p1_account_index == -1) ? "Not Logged In" : GetPlayerName(g, 1);
        DrawText("Active Account:", 150, 160, 30, LIME);
        DrawText(p1, 420, 160, 30, WHITE);
        if (g->p1_account_index >= 0)
        {
            DrawText(TextFormat("Credits: $%.2f", g->accounts[g->p1_account_index].credits),
                     150, 200, 28, LIME);
            DrawText(TextFormat("Tokens: %.0f", g->accounts[g->p1_account_index].tokens),
                     550, 200, 28, GOLD);
            // Insurance status
            if (g->accounts[g->p1_account_index].has_insurance)
            {
                DrawText("[INSURED]", 850, 200, 28, GREEN);
            }
        }
        // Section divider
        DrawRectangle(100, 250, (int)SCREEN_W - 200, 3, GOLD);
        // Token Packages
        DrawText("TOKEN PACKAGES", (int)(CENTER_X - 200), 280, 40, SKYBLUE);
        Rectangle token_rects[3];
        float start_x = CENTER_X - 550;
        float card_width = 340;
        float card_height = 180;
        float gap = 30;
        for (int i = 0; i < 3; i++)
        {
            token_rects[i] = (Rectangle){
                start_x + (float)i * (card_width + gap),
                350,
                card_width,
                card_height};
        }
        const char *token_labels[] = {"1 TOKEN", "5 TOKENS", "10 TOKENS"};
        const char *token_prices[] = {"$1,000", "$4,000", "$7,000"};
        const char *token_savings[] = {"", "Save $1,000!", "Save $3,000!"};
        Color token_colors[] = {DARKGREEN, DARKBLUE, PURPLE};
        Vector2 mouse = GetMousePosition();
        for (int i = 0; i < 3; i++)
        {
            bool selected = (g->menu_selection == i);
            bool hovered = CheckCollisionPointRec(mouse, token_rects[i]);
            DrawRectangleRec(token_rects[i], token_colors[i]);
            if (selected || hovered)
                DrawRectangleLinesEx(token_rects[i], 5, LIME);
            else
                DrawRectangleLinesEx(token_rects[i], 3, GOLD);
            DrawText(token_labels[i],
                     (int)(token_rects[i].x + 90),
                     (int)(token_rects[i].y + 30),
                     32, GOLD);
            DrawText(token_prices[i],
                     (int)(token_rects[i].x + 110),
                     (int)(token_rects[i].y + 80),
                     28, WHITE);
            if (strlen(token_savings[i]) > 0)
            {
                DrawText(token_savings[i],
                         (int)(token_rects[i].x + 80),
                         (int)(token_rects[i].y + 130),
                         22, YELLOW);
            }
        }
        // Insurance section
        DrawText("PROTECTION", (int)(CENTER_X - 140), 580, 40, ORANGE);
        Rectangle insurance_rect = {CENTER_X - 400, 650, 800, 180};
        bool ins_selected = (g->menu_selection == 3);
        bool ins_hovered = CheckCollisionPointRec(mouse, insurance_rect);
        bool has_insurance = (g->p1_account_index >= 0 && g->accounts[g->p1_account_index].has_insurance);
        DrawRectangleRec(insurance_rect, has_insurance ? DARKGRAY : MAROON);
        if (ins_selected || ins_hovered)
            DrawRectangleLinesEx(insurance_rect, 5, LIME);
        else
            DrawRectangleLinesEx(insurance_rect, 3, GOLD);
        DrawText("BANKRUPTCY INSURANCE", (int)(insurance_rect.x + 200), (int)(insurance_rect.y + 30), 32, YELLOW);
        DrawText("$50,000", (int)(insurance_rect.x + 330), (int)(insurance_rect.y + 75), 28, WHITE);
        if (has_insurance)
        {
            DrawText("[ACTIVE] - One-Time Use", (int)(insurance_rect.x + 240), (int)(insurance_rect.y + 120), 24, GREEN);
        }
        else
        {
            DrawText("Protects from total reset at -$25,000", (int)(insurance_rect.x + 180), (int)(insurance_rect.y + 120), 22, LIGHTGRAY);
        }
        // Status message
        if (GetTime() < g->account_status_timer + 3.0)
        {
            int text_w = MeasureText(g->account_status_message, 28);
            DrawText(g->account_status_message, (int)(CENTER_X - (float)text_w / 2.0f), (int)(SCREEN_H - 120), 28, YELLOW);
        }
        // Controls
        DrawText("A/Enter = Purchase | B/Esc = Back", (int)(CENTER_X - 250), (int)SCREEN_H - 60, 24, GRAY);
    }
    void UpdateShop(LobbyState * g, Vector2 mouse)
    {
        int gamepad = GetActiveGamepad();
        // Navigation (4 items: 3 token packages + 1 insurance)
        if (IsKeyPressed(KEY_LEFT) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 13)))
        {
                g->menu_selection = (g->menu_selection - 1 + 4) % 4;
        }
        if (IsKeyPressed(KEY_RIGHT) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 14)))
        {
                g->menu_selection = (g->menu_selection + 1) % 4;
        }
        if (IsKeyPressed(KEY_UP) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 11)))
        {
                g->menu_selection = (g->menu_selection < 3) ? 3 : (g->menu_selection - 1);
        }
        if (IsKeyPressed(KEY_DOWN) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 12)))
        {
                g->menu_selection = (g->menu_selection == 3) ? 0 : 3;
        }
        // Back
        if (IsKeyPressed(KEY_B) || IsKeyPressed(KEY_ESCAPE) ||
            (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 1)))
        {
            SwitchState(g, STATE_MAIN_MENU);
            return;
        }
        // Purchase trigger
        bool trigger = IsKeyPressed(KEY_ENTER) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 0));
        // Mouse selection
        Rectangle token_rects[3];
        float start_x = CENTER_X - 550;
        float card_width = 340;
        float card_height = 180;
        float gap = 30;
        for (int i = 0; i < 3; i++)
        {
            token_rects[i] = (Rectangle){start_x + (float)i * (card_width + gap), 350, card_width, card_height};
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, token_rects[i]))
            {
                g->menu_selection = i;
                trigger = true;
            }
        }
        Rectangle insurance_rect = {CENTER_X - 400, 650, 800, 180};
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, insurance_rect))
        {
            g->menu_selection = 3;
            trigger = true;
        }
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && mouse.y > SCREEN_H - 100)
        {
            SwitchState(g, STATE_MAIN_MENU);
            return;
        }
        // Handle purchases
        if (trigger)
        {
            if (g->p1_account_index < 0)
            {
                ShowAccountStatus(g, "Please log in first!");
                PlaySound(g_place_sound);
                return;
            }
            Account *acc = &g->accounts[g->p1_account_index];
            switch (g->menu_selection)
            {
            case 0: // 1 token
                if (acc->credits >= 1000.0)
                {
                    acc->credits -= 1000.0;
                    acc->tokens += 1.0;
                    ShowAccountStatus(g, "Purchased 1 Token!");
                    PlaySound(g_coin_sound);
                    SaveAllAccounts(g);
                }
                else
                {
                    ShowAccountStatus(g, "Insufficient Credits!");
                    PlaySound(g_place_sound);
                }
                break;
            case 1: // 5 tokens
                if (acc->credits >= 4000.0)
                {
                    acc->credits -= 4000.0;
                    acc->tokens += 5.0;
                    ShowAccountStatus(g, "Purchased 5 Tokens! (Saved $1,000)");
                    PlaySound(g_coin_sound);
                    SaveAllAccounts(g);
                }
                else
                {
                    ShowAccountStatus(g, "Insufficient Credits!");
                    PlaySound(g_place_sound);
                }
                break;
            case 2: // 10 tokens
                if (acc->credits >= 7000.0)
                {
                    acc->credits -= 7000.0;
                    acc->tokens += 10.0;
                    ShowAccountStatus(g, "Purchased 10 Tokens! (Saved $3,000)");
                    PlaySound(g_coin_sound);
                    SaveAllAccounts(g);
                }
                else
                {
                    ShowAccountStatus(g, "Insufficient Credits!");
                    PlaySound(g_place_sound);
                }
                break;
            case 3: // Insurance
                if (acc->has_insurance)
                {
                    ShowAccountStatus(g, "You already have insurance!");
                    PlaySound(g_place_sound);
                }
                else if (acc->credits >= 50000.0)
                {
                    acc->credits -= 50000.0;
                    acc->has_insurance = true;
                    ShowAccountStatus(g, "Insurance Purchased! You're protected.");
                    PlaySound(g_coin_sound);
                    SaveAllAccounts(g);
                }
                else
                {
                    ShowAccountStatus(g, "Insufficient Credits! Need $50,000");
                    PlaySound(g_place_sound);
                }
                break;
            default:
                break;
            }
            UpdateAccountCredits(g);
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
    void SortLeaderboard(LobbyState * g)
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
    void UpdateLeaderboard(LobbyState * g, Vector2 mouse)
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
                g->menu_selection--;
            if (g->menu_selection < 0)
                g->menu_selection = 3;
        }
        if (IsKeyPressed(KEY_RIGHT))
        {
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
                        g->menu_selection--;
                if (g->menu_selection < 0)
                    g->menu_selection = 3;
            }
            if (IsGamepadButtonPressedSDL(gamepad, 14))
            {
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
        // 1. ADD BACK BUTTON (Top Left)
        Rectangle back_btn = {20, 20, 120, 50};
        Vector2 mouse = GetMousePosition();
        bool back_hover = CheckCollisionPointRec(mouse, back_btn);
        DrawRectangleRec(back_btn, back_hover ? RED : MAROON);
        DrawRectangleLinesEx(back_btn, 2, WHITE);
        DrawText("BACK", (int)back_btn.x + 25, (int)back_btn.y + 15, 20, WHITE);

        // 2. ADJUST HEADER POSITIONS (Fix Overlap)
        DrawText("ACHIEVEMENTS", (int)(CENTER_X - 240.0f), 50, 60, ORANGE); // Moved up slightly

        // Find human account indices logic...
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

        // Player switch buttons (Moved down slightly to clear header)
        Rectangle p1_rect = {CENTER_X - 260.0f, 130.0f, 100.0f, 45.0f};
        Rectangle p2_rect = {CENTER_X + 150.0f, 130.0f, 100.0f, 45.0f};

        bool p1_hovered = CheckCollisionPointRec(mouse, p1_rect);
        bool p2_hovered = CheckCollisionPointRec(mouse, p2_rect);
        DrawRectangleRec(p1_rect, p1_valid ? (p1_hovered ? SKYBLUE : LIGHTGRAY) : GRAY);
        DrawRectangleRec(p2_rect, p2_valid ? (p2_hovered ? SKYBLUE : LIGHTGRAY) : GRAY);
        DrawText("P1", (int)p1_rect.x + 35, (int)p1_rect.y + 12, 20, BLACK);
        DrawText("P2", (int)p2_rect.x + 35, (int)p2_rect.y + 12, 20, BLACK);

        // 3. SEPARATE NAME AND STATUS TEXT
        // Draw Name centered
        const char *player_name = GetPlayerNameByIndex(g, idx);
        int name_width = MeasureText(player_name, 30);

        // FIX: Added (float) cast to name_width
        DrawText(TextFormat("Viewing: %s", player_name), (int)(CENTER_X - (float)name_width / 2.0f), 200, 30, WHITE);

        // Draw Status below name
        const char *status_str = TextFormat("(%s)", GetMemberStatusString(acc->member_status));
        int status_width = MeasureText(status_str, 24);

        // FIX: Added (float) cast to status_width
        DrawText(status_str, (int)(CENTER_X - (float)status_width / 2.0f), 240, 24, LIGHTGRAY);
        // Progress Bar (Adjusted Y position)
        int unlocked_count = 0;
        for (int i = 0; i < MAX_ACHIEVEMENTS; i++)
        {
            if (acc->achievements[i].unlocked)
                unlocked_count++;
        }

        float progress = (float)unlocked_count / (float)MAX_ACHIEVEMENTS;
        Rectangle prog_bg = {CENTER_X - 250, 280, 500, 25}; // Moved to Y=280
        DrawRectangleRec(prog_bg, DARKGRAY);
        DrawRectangleLinesEx(prog_bg, 2, GRAY);
        Rectangle prog_fill = {CENTER_X - 250, 280, 500 * progress, 25};
        DrawRectangleRec(prog_fill, (progress >= 0.99f) ? GOLD : LIME);

        const char *prog_text = TextFormat("%d / %d (%d%%)", unlocked_count, MAX_ACHIEVEMENTS, (int)(progress * 100));
        int text_w = MeasureText(prog_text, 20);
        DrawText(prog_text, (int)(CENTER_X - (float)text_w / 2.0f), 282, 20, BLACK);

        // Grid setup
        int icon_size = 130;
        int col_spacing = 25;
        int row_height = 165;
        int grid_cols = 5;
        int grid_rows = 10;
        float grid_width = (float)grid_cols * (float)icon_size + (float)(grid_cols - 1) * (float)col_spacing;
        float start_x = CENTER_X - grid_width / 2.0f;
        float start_y = 330.0f; // Kept original grid start Y

        // ... [Rest of Grid Drawing Code Remains Same] ...
        int visible_rows = (int)((SCREEN_H - 500.0f) / (float)row_height);
        visible_rows = CLAMP(visible_rows, 3, grid_rows);
        int scroll_row = g->achievement_scroll_row;
        scroll_row = CLAMP(scroll_row, 0, grid_rows - visible_rows);
        float col_x[5];
        for (int c = 0; c < 5; c++)
            col_x[c] = start_x + (float)c * ((float)icon_size + (float)col_spacing);

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
                if (c == g->achievement_cursor_col && grid_row == g->achievement_cursor_row)
                {
                    DrawRectangleLinesEx(dest, 5.0f, YELLOW);
                    DrawRectangleLinesEx(dest, 2.0f, GOLD);
                }
            }
        }
        // Indicators
        if (scroll_row > 0)
            DrawText("↑", (int)(CENTER_X + 350.0f), (int)start_y - 30, 40, WHITE);
        if (scroll_row + visible_rows < grid_rows)
            DrawText("↓", (int)(CENTER_X + 350.0f), (int)(start_y + (float)visible_rows * (float)row_height + 20.0f), 40, WHITE);

        // Info rect
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
                 (int)(CENTER_X - 450.0f), (int)(SCREEN_H - 170.0f), 20, DARKGRAY);
    }

    void UpdateAchievements(LobbyState * g, Vector2 mouse)
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
        // Back button
        Rectangle back_btn = {20, 20, 120, 50};
        // Player switch buttons
        Rectangle p1_rect = {CENTER_X - 250.0f, 130.0f, 100.0f, 45.0f};
        Rectangle p2_rect = {CENTER_X + 150.0f, 130.0f, 100.0f, 45.0f};
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (CheckCollisionPointRec(mouse, back_btn))
            {
                            SwitchState(g, STATE_MAIN_MENU);
                return;
            }
        }
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            if (CheckCollisionPointRec(mouse, p1_rect) && p1_valid)
            {
                g->achievement_view_mode = 0;
                            return;
            }
            if (CheckCollisionPointRec(mouse, p2_rect) && p2_valid)
            {
                g->achievement_view_mode = 1;
                            return;
            }
        }
        if (IsKeyPressed(KEY_ONE) && p1_valid)
        {
            g->achievement_view_mode = 0;
                }
        if (IsKeyPressed(KEY_TWO) && p2_valid)
        {
            g->achievement_view_mode = 1;
                }
        if (gamepad >= 0)
        {
            if (IsGamepadButtonPressedSDL(gamepad, 4) && p1_valid)
            { // Left Shoulder
                g->achievement_view_mode = 0;
                        }
            if (IsGamepadButtonPressedSDL(gamepad, 5) && p2_valid)
            { // Right Shoulder
                g->achievement_view_mode = 1;
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
void DrawBettingSetup(const LobbyState *g)
{
    ClearBackground(BLACK);
    DrawText("BETTING MODE", (int)(CENTER_X - 200), 60, 60, GOLD);
    
    // Player info
    if (g->p1_account_index >= 0) {
        DrawText(TextFormat("Your Tokens: %.0f", g->accounts[g->p1_account_index].tokens),
                 150, 140, 28, LIME);
    }
    
    // Matchup selection
    DrawText("SELECT MATCHUP", (int)(CENTER_X - 180), 200, 40, SKYBLUE);
    
    Rectangle matchup_rects[4];
    const char *matchup_names[] = {
        "FLINT vs THEA",
        "BOB vs FLINT", 
        "THEA vs BOB",
        "RANDOM MATCH"
    };
    const char *odds_text[] = {
        "THEA Win: 1.43x | FLINT Win: 3.33x",
        "BOB Win: 1.11x | FLINT Win: 10.00x",
        "BOB Win: 1.67x | THEA Win: 2.50x",
        "??? Win: 5.00x"
    };
    
    float start_y = 270.0f;
    for (int i = 0; i < 4; i++) {
        // FIX: Explicit float cast
        matchup_rects[i] = (Rectangle){CENTER_X - 350.0f, start_y + (float)i * 100.0f, 700.0f, 80.0f};
        bool selected = (g->betting.selected_matchup == i);
        
        DrawRectangleRec(matchup_rects[i], selected ? DARKGREEN : DARKGRAY);
        DrawRectangleLinesEx(matchup_rects[i], 3, selected ? LIME : GRAY);
        
        DrawText(matchup_names[i], 
                (int)(matchup_rects[i].x + 20.0f), 
                (int)(matchup_rects[i].y + 15.0f), 30, WHITE);
        DrawText(odds_text[i],
                (int)(matchup_rects[i].x + 20.0f),
                (int)(matchup_rects[i].y + 50.0f), 20, YELLOW);
    }
    
    // Bet input section
    DrawText("BET ON:", (int)(CENTER_X - 400.0f), 720, 30, ORANGE);
    
    Rectangle p1_bet_rect = {CENTER_X - 350.0f, 770.0f, 300.0f, 60.0f};
    Rectangle p2_bet_rect = {CENTER_X + 50.0f, 770.0f, 300.0f, 60.0f};
    
    bool p1_selected = (g->menu_selection == 4);
    bool p2_selected = (g->menu_selection == 5);
    
    DrawRectangleRec(p1_bet_rect, p1_selected ? DARKBLUE : DARKGRAY);
    DrawRectangleLinesEx(p1_bet_rect, 3, p1_selected ? SKYBLUE : GRAY);
    DrawText(TextFormat("P1: %.0f tokens", g->betting.p1_bet_amount),
            (int)(p1_bet_rect.x + 20.0f), (int)(p1_bet_rect.y + 18.0f), 24, WHITE);
    
    DrawRectangleRec(p2_bet_rect, p2_selected ? DARKBLUE : DARKGRAY);
    DrawRectangleLinesEx(p2_bet_rect, 3, p2_selected ? SKYBLUE : GRAY);
    DrawText(TextFormat("P2: %.0f tokens", g->betting.p2_bet_amount),
            (int)(p2_bet_rect.x + 20.0f), (int)(p2_bet_rect.y + 18.0f), 24, WHITE);
    
    // Start button
    Rectangle start_btn = {CENTER_X - 150.0f, 880.0f, 300.0f, 70.0f};
    bool can_bet = (g->betting.p1_bet_amount > 0 || g->betting.p2_bet_amount > 0);
    
    DrawRectangleRec(start_btn, can_bet ? GREEN : DARKGRAY);
    DrawRectangleLinesEx(start_btn, 3, can_bet ? LIME : GRAY);
    DrawText("PLACE BET & START", (int)(start_btn.x + 30.0f), (int)(start_btn.y + 20.0f), 28, BLACK);
    
    DrawText("Arrows/D-Pad: Navigate | +/- or A/B: Adjust Bet | Enter/Start: Confirm",
            (int)(CENTER_X - 420.0f), (int)(SCREEN_H - 60.0f), 20, GRAY);
}

void UpdateBettingSetup(LobbyState *g, Vector2 mouse)
{
    // FIX: Suppress unused parameter warning
    (void)mouse;
    
    int gamepad = GetActiveGamepad();
    
    // Navigation
    if (IsKeyPressed(KEY_UP) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 11))) {
        g->menu_selection = (g->menu_selection - 1 + 6) % 6;
    }
    if (IsKeyPressed(KEY_DOWN) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 12))) {
        g->menu_selection = (g->menu_selection + 1) % 6;
    }
    
    // Select matchup (0-3)
    if (g->menu_selection < 4) {
        g->betting.selected_matchup = g->menu_selection;
    }
    
    // Adjust bets
    if (g->menu_selection == 4) { // P1 bet
        if (IsKeyPressed(KEY_EQUAL) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 0))) {
            g->betting.p1_bet_amount += 10.0;
        }
        if (IsKeyPressed(KEY_MINUS) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 1))) {
            g->betting.p1_bet_amount = MAX(0.0, g->betting.p1_bet_amount - 10.0);
        }
    }
    
    if (g->menu_selection == 5) { // P2 bet
        if (IsKeyPressed(KEY_EQUAL) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 0))) {
            g->betting.p2_bet_amount += 10.0;
        }
        if (IsKeyPressed(KEY_MINUS) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 1))) {
            g->betting.p2_bet_amount = MAX(0.0, g->betting.p2_bet_amount - 10.0);
        }
    }
    
    // Place bet and start
    if (IsKeyPressed(KEY_ENTER) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 7))) {
        if (g->betting.p1_bet_amount > 0 || g->betting.p2_bet_amount > 0) {
            // Deduct tokens
            if (g->p1_account_index >= 0) {
                double total_bet = g->betting.p1_bet_amount + g->betting.p2_bet_amount;
                if (g->accounts[g->p1_account_index].tokens >= total_bet) {
                    g->accounts[g->p1_account_index].tokens -= total_bet;
                    g->betting.bet_placed = true;
                    
                    // FIX: Correct function call
                    SetupBettingMatch(g);
                    SwitchState(g, STATE_JOKERS_GAMBIT);
                } else {
                    ShowNotification(g, "INSUFFICIENT TOKENS", "Not enough tokens for this bet!");
                }
            }
        }
    }
    
    // Back
    if (IsKeyPressed(KEY_B) || IsKeyPressed(KEY_ESCAPE) ||
        (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 1))) {
        SwitchState(g, STATE_MODE_SELECTION);
    }
}

void SetupBettingMatch(LobbyState *g)
{
    int matchup = g->betting.selected_matchup;
    AIType p1_ai, p2_ai;
    
    switch (matchup) {
        case 0: // FLINT vs THEA
            p1_ai = AI_FLINT;
            p2_ai = AI_THEA;
            break;
        case 1: // BOB vs FLINT
            p1_ai = AI_BOB;
            p2_ai = AI_FLINT;
            break;
        case 2: // THEA vs BOB
            p1_ai = AI_THEA;
            p2_ai = AI_BOB;
            break;
        case 3: // RANDOM
        default:
            p1_ai = (AIType)(rand() % 3);
            p2_ai = (AIType)(rand() % 3);
            break;
    }
    
    // Login AI accounts
    for (int i = 0; i < g->account_count; i++) {
        if (g->accounts[i].is_ai && g->accounts[i].ai_type == p1_ai) {
            LoginAccount(g, i, 1);
            break;
        }
    }
    for (int i = 0; i < g->account_count; i++) {
        if (g->accounts[i].is_ai && g->accounts[i].ai_type == p2_ai) {
            LoginAccount(g, i, 2);
            break;
        }
    }
    
    g->game_state->mode = MODE_BETTING;
    InitGame(g);
}