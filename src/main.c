#include "main.h"
#include "mainmenu.h"
#include "useraccount.h"
#include "gamepad_sdl.h"
#include "jokersgambit.h"
#include "slotreels.h"
Music g_background_music = {0};
Texture2D g_achievements_atlas = {0};
SlotReelsState g_slot_state = {0};
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
void DrawModeSelection(const LobbyState *g)
{
    DrawText("SELECT GAME MODE", (int)(CENTER_X - 300.0f), 100, 70, GOLD);
    Rectangle rects[4];
    rects[0] = (Rectangle){CENTER_X - 450, 300, 300, 120};
    rects[1] = (Rectangle){CENTER_X - 150, 300, 300, 120};
    rects[2] = (Rectangle){CENTER_X + 150, 300, 300, 120};
    rects[3] = (Rectangle){CENTER_X + 450, 300, 300, 120};
    const char *titles[] = {"PLAYER vs PLAYER", "PLAYER vs AI", "AI vs AI", "BETTING"};
    const char *subtitles[] = {"Fun Mode", "Ranked", "Fun Mode", "Gambit betting"};
    for (int i = 0; i < 4; i++)
    {
        bool selected = (g->menu_selection == i);
        DrawRectangleRec(rects[i], selected ? LIME : DARKGREEN);
        DrawRectangleLinesEx(rects[i], 4, selected ? WHITE : GOLD);
        DrawText(titles[i], (int)(rects[i].x + 20), (int)(rects[i].y + 30), 24, WHITE);
        DrawText(subtitles[i], (int)(rects[i].x + 80), (int)(rects[i].y + 70), 20, YELLOW);
    }
    DrawText("Only PvAI affects your stats and credits!", (int)(CENTER_X - 280), 500, 28, ORANGE);
    DrawText("PVP auto-logs in P2 (logs out after game)", (int)(CENTER_X - 260), 540, 24, LIGHTGRAY);
    DrawText("BACK (B)", (int)(CENTER_X - 70), (int)SCREEN_H - 150, 35, WHITE);
}
void UpdateModeSelection(LobbyState *g, Vector2 mouse)
{
    int gamepad = GetActiveGamepad();
    // === INPUT ===
    if (IsKeyPressed(KEY_LEFT) || XboxBtnPressed(gamepad, 13))
    {
        g->menu_selection--;
        if (g->menu_selection < 0)
            g->menu_selection = 3;
    }
    if (IsKeyPressed(KEY_RIGHT) || XboxBtnPressed(gamepad, 14))
    {
        g->menu_selection++;
        if (g->menu_selection > 3)
            g->menu_selection = 0;
    }
    if (IsKeyPressed(KEY_B) || XboxBtnPressed(gamepad, 1))
    {
        SwitchState(g, STATE_LOBBY);
        return;
    }
    // === SELECTION ===
    bool trigger = IsKeyPressed(KEY_ENTER) || (gamepad >= 0 && XboxBtnPressed(gamepad, 0));
    // === MOUSE INPUT ===
    Rectangle mode_rects[4];
    mode_rects[0] = (Rectangle){CENTER_X - 450, 300, 300, 120}; // PVP
    mode_rects[1] = (Rectangle){CENTER_X - 150, 300, 300, 120}; // PvAI
    mode_rects[2] = (Rectangle){CENTER_X + 150, 300, 300, 120}; // AIvAI
    mode_rects[3] = (Rectangle){CENTER_X + 450, 300, 300, 120}; // BETTING
    for (int i = 0; i < 4; i++)
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, mode_rects[i]))
        {
            g->menu_selection = i;
            trigger = true;
        }
    }
    if (trigger)
    {
        switch (g->menu_selection)
        {
        case 0:                          // PLAYER vs PLAYER
            SwitchState(g, MULTIPLAYER); // Go to choice screen first
            break;
        case 1: // Player vs AI
            g->game_state->mode = MODE_PVAI;
            InitGame(g);
            SwitchState(g, STATE_AI_SELECTION);
            break;
        case 2: // AI vs AI (Fun Mode)
            g->game_state->mode = MODE_AIVAI;
            InitGame(g);
            SwitchState(g, STATE_AI_SELECTION);
            break;
        case 3: // BETTING MODE
            g->game_state->mode = MODE_BETTING;
            SwitchState(g, STATE_BETTING_SETUP);
            break;
        default:
            break;
        }
    }
}
void DrawAISelection(const LobbyState *g)
{
    DrawText("SELECT AI OPPONENT", (int)(CENTER_X - 300.0f), 100, 70, GOLD);
    Rectangle rects[3];
    rects[0] = (Rectangle){CENTER_X - 450, 300, 300, 120};
    rects[1] = (Rectangle){CENTER_X - 150, 300, 300, 120};
    rects[2] = (Rectangle){CENTER_X + 150, 300, 300, 120};
    const char *titles[] = {"BOB", "THEA", "FLINT"};
    const char *difficulties[] = {"Hard", "Medium", "Easy"};
    for (int i = 0; i < 3; i++)
    {
        bool selected = (g->menu_selection == i);
        DrawRectangleRec(rects[i], selected ? LIME : DARKBLUE);
        DrawRectangleLinesEx(rects[i], 4, selected ? WHITE : GOLD);
        DrawText(titles[i], (int)(rects[i].x + 100), (int)(rects[i].y + 30), 35, WHITE);
        DrawText(difficulties[i], (int)(rects[i].x + 80), (int)(rects[i].y + 75), 22, YELLOW);
    }
    DrawText("BACK (B)", (int)(CENTER_X - 70), (int)SCREEN_H - 150, 35, WHITE);
}
void DrawAIP2Selection(const LobbyState *g)
{
    DrawText("SELECT AI FOR P2", (int)(CENTER_X - 300.0f), 100, 70, GOLD);
    // Show P1 selection
    const char *p1_ai_name = (g->selected_p1_ai == AI_BOB) ? "BOB" : (g->selected_p1_ai == AI_THEA) ? "THEA"
                                                                                                    : "FLINT";
    DrawText(TextFormat("P1: %s", p1_ai_name), (int)(CENTER_X - 150), 180, 40, LIME);
    Rectangle rects[3];
    rects[0] = (Rectangle){CENTER_X - 450, 300, 300, 120};
    rects[1] = (Rectangle){CENTER_X - 150, 300, 300, 120};
    rects[2] = (Rectangle){CENTER_X + 150, 300, 300, 120};
    const char *titles[] = {"BOB", "THEA", "FLINT"};
    const char *difficulties[] = {"HARD", "MEDIUM", "EASY"};
    for (int i = 0; i < 3; i++)
    {
        bool selected = (g->menu_selection == i);
        // Disable if same as P1
        bool is_p1 = ((i == 0 && g->selected_p1_ai == AI_BOB) ||
                      (i == 1 && g->selected_p1_ai == AI_THEA) ||
                      (i == 2 && g->selected_p1_ai == AI_FLINT));
        Color bg_color = is_p1 ? DARKGRAY : (selected ? LIME : DARKBLUE);
        DrawRectangleRec(rects[i], bg_color);
        DrawRectangleLinesEx(rects[i], 4, selected ? WHITE : GOLD);
        DrawText(titles[i], (int)(rects[i].x + 100), (int)(rects[i].y + 30), 35, WHITE);
        DrawText(difficulties[i], (int)(rects[i].x + 80), (int)(rects[i].y + 75), 22, YELLOW);
        if (is_p1)
        {
            DrawText("(P1)", (int)(rects[i].x + 110), (int)(rects[i].y + 100), 18, RED);
        }
    }
    DrawText("BACK (B)", (int)(CENTER_X - 70), (int)SCREEN_H - 150, 35, WHITE);
}
void UpdateAISelection(LobbyState *g, Vector2 mouse)
{
    int gamepad = GetActiveGamepad();
    // Navigation...
    if (IsKeyPressed(KEY_LEFT) || XboxBtnPressed(gamepad, 13))
    {
        g->menu_selection--;
        if (g->menu_selection < 0)
            g->menu_selection = 2;
    }
    if (IsKeyPressed(KEY_RIGHT) || XboxBtnPressed(gamepad, 14))
    {
        g->menu_selection++;
        if (g->menu_selection > 2)
            g->menu_selection = 0;
    }
    if (IsKeyPressed(KEY_B) || XboxBtnPressed(gamepad, 1))
    {
        SwitchState(g, STATE_MODE_SELECTION);
        return;
    }
    bool trigger = IsKeyPressed(KEY_ENTER) || (gamepad >= 0 && XboxBtnPressed(gamepad, 0));
    Rectangle ai_rects[3];
    ai_rects[0] = (Rectangle){CENTER_X - 450, 300, 300, 120};
    ai_rects[1] = (Rectangle){CENTER_X - 150, 300, 300, 120};
    ai_rects[2] = (Rectangle){CENTER_X + 150, 300, 300, 120};
    for (int i = 0; i < 3; i++)
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, ai_rects[i]))
        {
            g->menu_selection = i;
            trigger = true;
        }
    }
    if (trigger)
    {
        AIType selected_ai = (g->menu_selection == 0) ? AI_BOB : (g->menu_selection == 1) ? AI_THEA
                                                                                          : AI_FLINT;
        g->game_state->selected_opponent_ai = selected_ai;
        if (g->game_state->mode == MODE_PVAI)
        {
            // Player vs AI: P2 becomes the selected AI
            for (int i = 0; i < g->account_count; i++)
            {
                if (g->accounts[i].is_ai && g->accounts[i].ai_type == selected_ai)
                {
                    LoginAccount(g, i, 2);
                    break;
                }
            }
            InitGame(g);
            SwitchState(g, STATE_JOKERS_GAMBIT);
        }
        else if (g->game_state->mode == MODE_AIVAI)
        {
            // AI vs AI: Save P1 selection and move to P2 selection
            g->selected_p1_ai = selected_ai;
            g->menu_selection = 0; // Reset for P2 selection
            SwitchState(g, STATE_AI_P2_SELECTION);
        }
    }
}
void UpdateAIP2Selection(LobbyState *g, Vector2 mouse)
{
    int gamepad = GetActiveGamepad();
    // === KEYBOARD INPUT ===
    if (IsKeyPressed(KEY_LEFT) || XboxBtnPressed(gamepad, 13))
    {
        g->menu_selection--;
        if (g->menu_selection < 0)
            g->menu_selection = 2;
    }
    if (IsKeyPressed(KEY_RIGHT) || XboxBtnPressed(gamepad, 14))
    {
        g->menu_selection++;
        if (g->menu_selection > 2)
            g->menu_selection = 0;
    }
    if (IsKeyPressed(KEY_B) || XboxBtnPressed(gamepad, 1))
    {
        SwitchState(g, STATE_AI_SELECTION);
        return;
    }
    // === SELECTION ===
    bool trigger = IsKeyPressed(KEY_ENTER) || (gamepad >= 0 && XboxBtnPressed(gamepad, 0));
    // === MOUSE INPUT ===
    Rectangle ai_rects[3];
    ai_rects[0] = (Rectangle){CENTER_X - 450, 300, 300, 120};
    ai_rects[1] = (Rectangle){CENTER_X - 150, 300, 300, 120};
    ai_rects[2] = (Rectangle){CENTER_X + 150, 300, 300, 120};
    for (int i = 0; i < 3; i++)
    {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, ai_rects[i]))
        {
            g->menu_selection = i;
            trigger = true;
        }
    }
    if (trigger)
    {
        AIType selected_ai = (g->menu_selection == 0) ? AI_BOB : (g->menu_selection == 1) ? AI_THEA
                                                                                          : AI_FLINT;
        // Can't select same AI as P1
        if (selected_ai == g->selected_p1_ai)
        {
            return; // Do nothing
        }
        g->selected_p2_ai = selected_ai;
        // Login both AIs
        for (int i = 0; i < g->account_count; i++)
        {
            if (g->accounts[i].is_ai && g->accounts[i].ai_type == g->selected_p1_ai)
            {
                LoginAccount(g, i, 1);
                break;
            }
        }
        for (int i = 0; i < g->account_count; i++)
        {
            if (g->accounts[i].is_ai && g->accounts[i].ai_type == g->selected_p2_ai)
            {
                LoginAccount(g, i, 2);
                break;
            }
        }
        InitGame(g);
        SwitchState(g, STATE_JOKERS_GAMBIT);
    }
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
    g_coin_sound = LoadSound("sfx/coin.wav");
    g_discard_sound = LoadSound("sfx/discard.wav");
    g_place_sound = LoadSound("sfx/place.wav");
    g_filled_rank_sound = LoadSound("sfx/filledrank.wav");
    g_win_sound = LoadSound("sfx/win.wav");
    g_joker_sound = LoadSound("sfx/onejoker.wav");
    g_matching_jokers_sound = LoadSound("sfx/twojokers.wav");
    g_matching_cards_sound = LoadSound("sfx/matchingcards.wav");
    g_continue_sound = LoadSound("sfx/continue.wav");
    g_shuffle_sound = LoadSound("sfx/shuffle.wav");
    g_achievements_atlas = LoadTexture("atlas/achievements.png");
    // Initialize game state
    LobbyState game_state = {0};
    game_state.game_state = (GameState *)malloc(sizeof(GameState));
    if (!game_state.game_state)
    {
        printf("ERROR: Failed to allocate GameState!\n");
        CloseWindow();
        return 1;
    }
    memset(game_state.game_state, 0, sizeof(GameState));
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
    LoadCardAtlas();
    // Main game loop
    while (!WindowShouldClose())
    {
        UpdateGamepadSDL();
        Vector2 mouse = GetMousePosition();
        if (game_state.music_enabled) // Update music
        {
            UpdateMusicStream(g_background_music);
        }
        int gamepad = GetActiveGamepad(); // Global fullscreen toggle
        if (IsKeyPressed(KEY_F11) || (gamepad >= 0 && XboxBtnPressed(gamepad, 8)))
        {
            ToggleAppFullscreen(&game_state);
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
        case STATE_MODE_SELECTION:
            UpdateModeSelection(&game_state, mouse);
            break;
        case STATE_AI_SELECTION:
            UpdateAISelection(&game_state, mouse);
            break;
        case STATE_AI_P2_SELECTION:
            UpdateAIP2Selection(&game_state, mouse);
            break;
        case STATE_JOKERS_GAMBIT:
            UpdateJokersGambit(&game_state, mouse);
            break;
        case STATE_BETTING_SETUP:
            UpdateBettingSetup(&game_state, mouse);
            break;
        case STATE_SLOT_REELS:
            UpdateSlotReels(&game_state, &g_slot_state);
            break;
        case STATE_PVP_SETUP_P1:
            UpdatePVPSetupP1(&game_state);
            break;
        case STATE_PVP_SETUP_P2:
            UpdatePVPSetupP2(&game_state);
            break;
        case MULTIPLAYER:
            UpdateMultiplayer(&game_state);
            break;
        case STATE_ONLINE_CHOICE:
            UpdateOnlineChoice(&game_state);
            break;
        default:
            break;
        }
        BeginDrawing(); //   DRAW STATES
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
        case STATE_MODE_SELECTION:
            DrawModeSelection(&game_state);
            break;
        case STATE_AI_SELECTION:
            DrawAISelection(&game_state);
            break;
        case STATE_AI_P2_SELECTION:
            DrawAIP2Selection(&game_state);
            break;
        case STATE_JOKERS_GAMBIT:
            DrawJokersGambit(&game_state);
            break;
        case STATE_BETTING_SETUP:
            DrawBettingSetup(&game_state);
            break;
        case STATE_SLOT_REELS:
            DrawSlotReels(&game_state, &g_slot_state);
            break;
        case STATE_PVP_SETUP_P1:
            DrawPVPSetupP1(&game_state);
            break;
        case STATE_PVP_SETUP_P2:
            DrawPVPSetupP2(&game_state);
            break;
        case MULTIPLAYER:
            DrawMultiplayerMode(&game_state);
            break;
        case STATE_ONLINE_CHOICE:
            DrawOnlineChoice(&game_state);
            break;
        default:
            break;
        }
        DrawNotification(&game_state);
        EndDrawing();
    }
    UnloadTexture(g_atlas_texture);
    UnloadTexture(g_background_texture);
    UnloadTexture(g_ui_frame_texture);
    UnloadTexture(g_button_texture);
    UnloadSound(g_discard_sound);
    UnloadSound(g_filled_rank_sound);
    UnloadSound(g_win_sound);
    UnloadSound(g_place_sound);
    UnloadSound(g_reveal_sound);
    UnloadSound(g_joker_sound);
    UnloadSound(g_matching_jokers_sound);
    UnloadSound(g_matching_cards_sound);
    UnloadSound(g_continue_sound);
    UnloadSound(g_coin_sound);
    UnloadSound(g_shuffle_sound);
    UnloadTexture(g_achievements_atlas);
    UnloadMusicStream(g_background_music);
    free(game_state.game_state);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}