#include "main.h"
#include "mainmenu.h"
#include "useraccount.h"
#include "gamepad_sdl.h"
Music g_background_music = {0};
// DEBUG FUNCTION
// Updated debug function//
//void DebugControllerInput(void)
//{
//    static int frame_count = 0;
//    frame_count++;
//    int gamepad = GetActiveGamepad();
//    if (gamepad >= 0)
//    {
//        // Print controller name once per second
//        if (frame_count % 60 == 0)
//        {
//            printf("\n=== Gamepad Debug (SDL2) ===\n");
//            printf("Gamepad %d: %s\n", gamepad, GetGamepadNameSDL(gamepad));
//        }
//        // Print button presses immediately
//        if (IsGamepadButtonPressedSDL(gamepad, 0))
//            printf("A pressed!\n");
//        if (IsGamepadButtonPressedSDL(gamepad, 1))
//            printf("B pressed!\n");
//        if (IsGamepadButtonPressedSDL(gamepad, 2))
//            printf("X pressed!\n");
//        if (IsGamepadButtonPressedSDL(gamepad, 3))
//            printf("Y pressed!\n");
//        if (IsGamepadButtonPressedSDL(gamepad, 11))
//            printf("D-Pad UP pressed!\n");
//        if (IsGamepadButtonPressedSDL(gamepad, 12))
//            printf("D-Pad DOWN pressed!\n");
//        if (IsGamepadButtonPressedSDL(gamepad, 13))
//            printf("D-Pad LEFT pressed!\n");
//        if (IsGamepadButtonPressedSDL(gamepad, 14))
//            printf("D-Pad RIGHT pressed!\n");
//        // Show stick movement
//        float leftX = GetGamepadAxisSDL(gamepad, 0);
//        float leftY = GetGamepadAxisSDL(gamepad, 1);
//        if (fabs(leftX) > 0.3f || fabs(leftY) > 0.3f)
//        {
//            printf("Left Stick: X=%.2f Y=%.2f\n", leftX, leftY);
//        }
//    }
//}
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
    InitWindow((int)SCREEN_W, (int)SCREEN_H, "Michael's Casino");
    SetTargetFPS(60);
    InitGamepadSDL();
    // Initialize audio
    InitAudioDevice();
    g_background_music = LoadMusicStream("sfx/track.mp3");
    SetMusicVolume(g_background_music, 0.6f);
    PlayMusicStream(g_background_music);
    // Initialize game state
    LobbyState game_state = {0};
    game_state.state = STATE_MAIN_MENU;
    game_state.menu_selection = 0;
    game_state.p1_account_index = -1;
    game_state.p2_account_index = -1;
    game_state.cover_p2_cards = true;
    game_state.ai_move_delay = 2.0f;
    game_state.leaderboard_game_filter = GAME_JOKERS_GAMBIT;
    game_state.leaderboard_sort_mode = 0;
    game_state.leaderboard_loaded = false;
    game_state.account_status_timer = 0.0;
    LoadAllAccounts(&game_state);
    LoadLeaderboard(&game_state);
    SortLeaderboard(&game_state);
    printf("Michael's Casino Initialized\n");
    printf("Loaded %d accounts\n", game_state.account_count);
    printf("Loaded %d leaderboard entries\n", game_state.leaderboard_count);
    // Main game loop
    while (!WindowShouldClose())
    {
        UpdateGamepadSDL();
        Vector2 mouse = GetMousePosition();
        UpdateMusicStream(g_background_music);
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
        default:
            break;
        }
        // Debug overlay
       // DebugControllerInput();
        EndDrawing();
    }
    // Cleanup
    UnloadMusicStream(g_background_music);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}