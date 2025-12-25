#include "main.h"
#include "mainmenu.h"
#include "useraccount.h"
#include "gamepad_sdl.h"
Music g_background_music = {0};
Sound g_menu_navigate_sound = {0};
Sound g_menu_confirm_sound = {0};
Sound g_coin_sound = {0};
Sound g_place_sound = {0};
Texture2D g_achievements_atlas = {0};
// Dynamic screen dimensions
float SCREEN_W = DEFAULT_SCREEN_W;
float SCREEN_H = DEFAULT_SCREEN_H;
float CENTER_X = DEFAULT_SCREEN_W / 2.0f;
void UpdateScreenDimensions(void)
{
    CENTER_X = SCREEN_W / 2.0f;
}
void ApplyWindowScale(LobbyState *g)
{
    float scale = 1.0f;
    switch (g->window_scale)
    {
    case SCALE_75:
        scale = 0.75f;
        break;
    case SCALE_100:
        scale = 1.0f;
        break;
    case SCALE_125:
        scale = 1.25f;
        break;
    case SCALE_150:
        scale = 1.5f;
        break;
    default:
        break;
    }
    SCREEN_W = DEFAULT_SCREEN_W * scale;
    SCREEN_H = DEFAULT_SCREEN_H * scale;
    UpdateScreenDimensions();
    if (!g->is_fullscreen)
    {
        SetWindowSize((int)SCREEN_W, (int)SCREEN_H);
    }
}
void ToggleAppFullscreen(LobbyState *g)
{
    if (g->is_fullscreen)
    {
        // Exit fullscreen
        ToggleBorderlessWindowed();
        g->is_fullscreen = false;
        ApplyWindowScale(g);
    }
    else
    {
        // Enter fullscreen
        int monitor = GetCurrentMonitor();
        SCREEN_W = (float)GetMonitorWidth(monitor);
        SCREEN_H = (float)GetMonitorHeight(monitor);
        UpdateScreenDimensions();
        ToggleBorderlessWindowed();
        g->is_fullscreen = true;
    }
}
int GetActiveGamepad(void)
{
    for (int i = 0; i < 4; i++)
    {
        if (IsGamepadAvailableSDL(i))
        {
            return i;
        }
    }
    return -1;
}
int main(void)
{
    // Initialize window
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow((int)SCREEN_W, (int)SCREEN_H, "Michael's Casino");
    SetTargetFPS(60);
    InitGamepadSDL();
    // Initialize audio
    InitAudioDevice();
    g_background_music = LoadMusicStream("sfx/track.mp3");
    g_menu_navigate_sound = LoadSound("sfx/place.wav");
    g_menu_confirm_sound = LoadSound("sfx/continue.wav");
    g_coin_sound = LoadSound("sfx/coin.wav");
    g_achievements_atlas = LoadTexture("atlas/achievements.png");
    // Initialize game state
    LobbyState game_state = {0};
    game_state.debug_force_check_achievements = false;
    game_state.state = STATE_MAIN_MENU;
    game_state.menu_selection = 0;
    game_state.p1_account_index = -1;
    game_state.p2_account_index = -1;
    game_state.cover_p2_cards = true;
    game_state.ai_delay_mode = AI_DELAY_NORMAL; // Default 1.0s
    game_state.music_enabled = true;
    game_state.window_scale = SCALE_100;
    game_state.is_fullscreen = false;
    game_state.leaderboard_game_filter = GAME_JOKERS_GAMBIT;
    game_state.leaderboard_sort_mode = 0;
    game_state.leaderboard_loaded = false;
    game_state.account_status_timer = 0.0;
    game_state.achievement_cursor_col = 0;
    game_state.achievement_cursor_row = 0;
    game_state.achievement_scroll_row = 0;
    game_state.editing_name = false;
    memset(game_state.edit_first_name, 0, sizeof(game_state.edit_first_name));
    memset(game_state.edit_last_name, 0, sizeof(game_state.edit_last_name));
    game_state.edit_cursor_pos = 0;
    game_state.edit_timer = 0.0;
    LoadAllAccounts(&game_state);
    LoadLeaderboard(&game_state);
    SortLeaderboard(&game_state);
    InitGlobalAchievementDefs();
    LoadAchievements(&game_state);
    LoadSettings(&game_state);
    printf("Michael's Casino Initialized\n");
    printf("Loaded %d accounts\n", game_state.account_count);
    printf("Loaded %d leaderboard entries\n", game_state.leaderboard_count);
    // Start background music
    if (game_state.music_enabled)
    {
        SetMusicVolume(g_background_music, 0.6f);
        PlayMusicStream(g_background_music);
    }
    // Main game loop
    while (!WindowShouldClose())
    {
        UpdateGamepadSDL();
        Vector2 mouse = GetMousePosition();
        // Update music
        if (game_state.music_enabled)
        {
            UpdateMusicStream(g_background_music);
        }
        // Global fullscreen toggle
        int gamepad = GetActiveGamepad();
        if (IsKeyPressed(KEY_F11) || (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 8)))
        {
            ToggleAppFullscreen(&game_state);
        }
        if (IsGamepadButtonPressedSDL(gamepad, 6))
        {
            TakeScreenshot("screenshot.png");
            DrawText("Screenshot Taken!", (int)(CENTER_X - 180), 400, 30, GREEN);
        }
        switch (game_state.state)
        {
        case STATE_MAIN_MENU:
            UpdateMainMenu(&game_state, mouse);
            break;
        case STATE_LOBBY:
            UpdateLobby(&game_state, mouse);
            break;
        case STATE_ACCOUNTS_MANAGER:
            UpdateAccountsManager(&game_state, mouse);
            break;
        case STATE_SHOP:
            UpdateShop(&game_state, mouse);
            break;
        case STATE_SETTINGS:
            UpdateSettings(&game_state, mouse);
            break;
        case STATE_LEADERBOARD:
            UpdateLeaderboard(&game_state, mouse);
            break;
        case STATE_ACHIEVEMENTS:
            UpdateAchievements(&game_state, mouse);
            break;
        default:
            break;
        }
        // Draw
        BeginDrawing();
        ClearBackground(DARKGRAY);
        switch (game_state.state)
        {
        case STATE_MAIN_MENU:
            DrawMainMenu(&game_state);
            break;
        case STATE_LOBBY:
            DrawLobby(&game_state);
            break;
        case STATE_ACCOUNTS_MANAGER:
            DrawAccountsManager(&game_state);
            break;
        case STATE_SHOP:
            DrawShop(&game_state);
            break;
        case STATE_SETTINGS:
            DrawSettings(&game_state);
            break;
        case STATE_LEADERBOARD:
            DrawLeaderboard(&game_state);
            break;
        case STATE_ACHIEVEMENTS:
            DrawAchievements(&game_state);
            break;
        default:
            break;
        }
        EndDrawing();
    }
    // Cleanup
    UnloadTexture(g_achievements_atlas);
    UnloadSound(g_coin_sound);
    UnloadSound(g_place_sound);
    UnloadSound(g_menu_navigate_sound);
    UnloadSound(g_menu_confirm_sound);
    UnloadMusicStream(g_background_music);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}