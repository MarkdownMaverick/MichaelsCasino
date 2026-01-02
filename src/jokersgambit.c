
#include "jokersgambit.h"
#include "aibots.h"
#include "mainmenu.h"
#include "gamepad_sdl.h"
#include "useraccount.h"
#include "main.h"
#include <time.h>
#include "multiplayer.h"

#define UI_FRAME_W 480
#define UI_FRAME_H 150
#define P1_UI_X 10
#define P2_UI_X (SCREEN_W - UI_FRAME_W - 10)
#define UI_TEXT_OFFSET_X 45
#define UI_LINE_SPACING 20
#define P1_HAND_Y (SCREEN_H - CARD_H_SCALED - 130)
#define P2_HAND_Y (SCREEN_H - CARD_H_SCALED - 130)
#define HAND_CARD_GAP 5
#define P1_HAND_START_X 10
#define P2_HAND_START_X (SCREEN_W - 10 - (HAND_SIZE * CARD_W_SCALED + (HAND_SIZE - 1) * HAND_CARD_GAP))
#define DISCARD_PILE_Y (GRID_START_Y + KEYCARDS * ROW_SPACING + 0)
#define P1_DISCARD_X (KEYCARD_COL_X - CARD_W_SCALED - 100)
#define P2_DISCARD_X (KEYCARD_COL_X + CARD_W_SCALED + 100)
#define ATLAS_WIDTH 2600
#define ATLAS_HEIGHT 1500
#define ATLAS_COLS 13
#define ATLAS_ROWS 5
#define CARD_SOURCE_W (ATLAS_WIDTH / ATLAS_COLS)
#define CARD_SOURCE_H (ATLAS_HEIGHT / ATLAS_ROWS)
Texture2D g_atlas_texture = {0};
Texture2D g_background_texture = {0};
Texture2D g_ui_frame_texture = {0};
Texture2D g_button_texture = {0};
Sound g_discard_sound = {0};
Sound g_place_sound = {0};
Sound g_filled_rank_sound = {0};
Sound g_reveal_sound = {0};
Sound g_win_sound = {0};
Sound g_joker_sound = {0};
Sound g_matching_jokers_sound = {0};
Sound g_matching_cards_sound = {0};
Sound g_continue_sound = {0};
Sound g_coin_sound = {0};
Sound g_shuffle_sound = {0};
GameState g_initial_state = {0};

void LoadCardAtlas(void)
{
    g_atlas_texture = LoadTexture("assets/DECK0.png");
    if (g_atlas_texture.id == 0 || g_atlas_texture.width == 0 || g_atlas_texture.height == 0)
    {
        printf("ERROR: Failed to load card atlas DECK0.png (id=%u, size=%dx%d)\n",
               g_atlas_texture.id, g_atlas_texture.width, g_atlas_texture.height);
    }
    else
    {
        printf("Card atlas loaded successfully: %dx%d\n",
               g_atlas_texture.width, g_atlas_texture.height);
    }
}
void UnloadCardAtlas(void)
{
    UnloadTexture(g_atlas_texture);
}
Rectangle AtlasRect(int row, int col)
{
    return (Rectangle){
        (float)(col * ATLAS_CARD_WIDTH),
        (float)(row * ATLAS_CARD_HEIGHT),
        (float)ATLAS_CARD_WIDTH,
        (float)ATLAS_CARD_HEIGHT};
}
Rectangle GetAtlasSourceRect(Rank rank, Suit suit)
{
    int row = -1;
    int col = -1;
    if (rank == RANK_JOKER)
        return AtlasRect(4, 0);
    if (rank == RANK_BACK || rank == RANK_NONE)
        return AtlasRect(4, 2);
    switch (suit)
    {
    case SUIT_HEARTS:
        row = 0;
        break;
    case SUIT_CLUBS:
        row = 1;
        break;
    case SUIT_DIAMONDS:
        row = 2;
        break;
    case SUIT_SPADES:
        row = 3;
        break;
    case SUIT_NONE:
        row = 4;
        break;
    default:
        row = 4;
        break;
    }
    switch (rank)
    {
    case RANK_2:
        col = 0;
        break;
    case RANK_3:
        col = 1;
        break;
    case RANK_4:
        col = 2;
        break;
    case RANK_5:
        col = 3;
        break;
    case RANK_6:
        col = 4;
        break;
    case RANK_7:
        col = 5;
        break;
    case RANK_8:
        col = 6;
        break;
    case RANK_9:
        col = 7;
        break;
    case RANK_10:
        col = 8;
        break;
    case RANK_JACK:
        col = 9;
        break;
    case RANK_QUEEN:
        col = 10;
        break;
    case RANK_KING:
        col = 11;
        break;
    case RANK_ACE:
        col = 12;
        break;
    case RANK_JOKER:
        col = 0;
        break;
    case RANK_BACK:
        col = 2;
        break;
    case RANK_NONE:
        col = 2;
        break;
    default:
        col = 2;
        break;
    }
    return AtlasRect(row, col);
}
Rectangle GetAtlasBackCard(void)
{
    return AtlasRect(4, 2);
}
Rectangle GetAtlasTempCover(void)
{
    return AtlasRect(4, 3);
}
Rectangle GetAtlasJoker(int joker_index)
{
    if (joker_index == 0)
        return AtlasRect(4, 0);
    else
        return AtlasRect(4, 1);
}
Rectangle GetDealingAnimationRect(void)
{
    int frame_count = 11;
    double time = GetTime();
    int frame = ((int)(time * 12.0)) % frame_count;
    return AtlasRect(4, 2 + frame);
}
Rectangle GetCardSourceRect(Card c)
{
    int col;
    int row;
    if (c.rank == RANK_NONE)
    {
        row = 4;
        col = 2;
        return (Rectangle){(float)(col * CARD_SOURCE_W), (float)(row * CARD_SOURCE_H), (float)CARD_SOURCE_W, (float)CARD_SOURCE_H};
    }
    if (c.rank == RANK_JOKER)
    {
        row = 4;
        if (c.filename[1] == 'B')
            col = 1;
        else
            col = 0;
        return (Rectangle){(float)(col * CARD_SOURCE_W), (float)(row * CARD_SOURCE_H), (float)CARD_SOURCE_W, (float)CARD_SOURCE_H};
    }
    switch (c.suit)
    {
    case SUIT_HEARTS:
        row = 0;
        break;
    case SUIT_CLUBS:
        row = 1;
        break;
    case SUIT_DIAMONDS:
        row = 2;
        break;
    case SUIT_SPADES:
        row = 3;
        break;
    case SUIT_NONE:
        row = 4;
        break;
    default:
        row = 0;
        break;
    }
    col = (int)c.rank;
    return (Rectangle){(float)(col * CARD_SOURCE_W), (float)(row * CARD_SOURCE_H), (float)CARD_SOURCE_W, (float)CARD_SOURCE_H};
}
float GetRankY(int key_idx)
{
    return GRID_START_Y + (float)key_idx * ROW_SPACING;
}
Rectangle KeyCardRect(int key_idx)
{
    float y = GetRankY(key_idx);
    return (Rectangle){KEYCARD_COL_X, y, CARD_W_SCALED, CARD_H_SCALED};
}
Rectangle SlotRect(int player, int key_idx, int slot_idx)
{
    float y = GetRankY(key_idx);
    float base_x = KEYCARD_COL_X + (player == 1 ? -UI_PADDING_SMALL : CARD_W_SCALED + UI_PADDING_SMALL);
    float x;
    if (player == 1)
        x = base_x - (float)(slot_idx + 1) * CARD_W_SCALED - (float)slot_idx * UI_PADDING_SMALL;
    else
        x = base_x + (float)slot_idx * (CARD_W_SCALED + UI_PADDING_SMALL);
    return (Rectangle){x, y, CARD_W_SCALED, CARD_H_SCALED};
}
Rectangle HandRect(int player, int idx)
{
    float y = (player == 1) ? P1_HAND_Y : P2_HAND_Y;
    float start_x = (player == 1) ? P1_HAND_START_X : P2_HAND_START_X;
    float x = start_x + (float)idx * (CARD_W_SCALED + HAND_CARD_GAP);
    return (Rectangle){x, y, CARD_W_SCALED, CARD_H_SCALED};
}
Rectangle ButtonRect(int player, int idx)
{
    Rectangle card_rect = HandRect(player, idx);
    float x = card_rect.x + (CARD_W_SCALED / 2.0f) - (BUTTON_W / 2.0f);
    float y = card_rect.y + CARD_H_SCALED + UI_PADDING_LARGE;
    return (Rectangle){x, y, BUTTON_W, BUTTON_H};
}
Rectangle DiscardPileRect(int player)
{
    float x = (player == 1) ? P1_DISCARD_X : P2_DISCARD_X;
    return (Rectangle){x, DISCARD_PILE_Y, CARD_W_SCALED, CARD_H_SCALED};
}
void DrawCard(Card c, Rectangle rect, Color tint)
{
    if (!c.is_valid)
        return;
    Rectangle source = GetCardSourceRect(c);
    if (g_atlas_texture.id != 0)
        DrawTexturePro(g_atlas_texture, source, rect, (Vector2){0, 0}, 0.0f, tint);
}
void DrawButton(Rectangle rect, bool is_hovered, bool is_enabled, const char *text)
{
    Color base_color = is_enabled ? RAYWHITE : DARKGRAY;
    Color tint = is_hovered ? Fade(base_color, 0.7f) : base_color;
    if (g_button_texture.id != 0)
        DrawTexturePro(g_button_texture, (Rectangle){0, 0, (float)g_button_texture.width, (float)g_button_texture.height}, rect, (Vector2){0, 0}, 0.0f, tint);
    int text_len = MeasureText(text, 14);
    int text_x = (int)(rect.x + rect.width / 2.0f - (float)text_len / 2.0f);
    int text_y = (int)(rect.y + rect.height / 2.0f - 7.0f);
    DrawText(text, text_x, text_y, 14, BLACK);
}
Rectangle ContinueButtonRect(int player)
{
    Rectangle ref_rect;
    float x;
    if (player == 1)
    {
        ref_rect = ButtonRect(1, 4);
        x = ref_rect.x + ref_rect.width + 10.0f;
    }
    else
    {
        ref_rect = ButtonRect(2, 0);
        x = ref_rect.x - 80.0f - 10.0f;
    }
    return (Rectangle){x, ref_rect.y, 80.0f, ref_rect.height};
}
void DrawContinueButtons(const GameState *g, Vector2 mouse)
{
    if (g->state != STATE_WAIT_FOR_TURN)
        return;
    for (int p = 1; p <= 2; p++)
    {
        bool is_human_active = (p == 1) ? (!g->p1_done_placing && !IsPlayerAI(g, 1))
                                        : (!g->p2_done_placing && !IsPlayerAI(g, 2));
        if (is_human_active)
        {
            Rectangle btn = ContinueButtonRect(p);
            bool hover = CheckCollisionPointRec(mouse, btn);
            Color tint = hover ? Fade(RAYWHITE, 0.7f) : RAYWHITE;
            if (g_button_texture.id != 0)
                DrawTexturePro(g_button_texture, (Rectangle){0, 0, (float)g_button_texture.width, (float)g_button_texture.height}, btn, (Vector2){0, 0}, 0.0f, tint);
            else
                DrawRectangleRec(btn, hover ? GOLD : ORANGE);
            int text_w = MeasureText("PASS", 20);
            int text_x = (int)(btn.x + (btn.width - (float)text_w) / 2.0f);
            int text_y = (int)(btn.y + (btn.height / 2.0f - 10.0f));
            DrawText("PASS", text_x, text_y, 20, BLACK);
        }
    }
}
void DrawPlayerUI(LobbyState *core, const GameState *g, int player)
{
    float adjusted_y = UI_Y - 20.0f;
    Rectangle frame_rect = (player == 1) ? (Rectangle){P1_UI_X, adjusted_y, UI_FRAME_W, UI_FRAME_H}
                                         : (Rectangle){P2_UI_X, adjusted_y, UI_FRAME_W, UI_FRAME_H};
    float credits = (player == 1) ? g->p1_credits : g->p2_credits;
    float temp_credits = (player == 1) ? g->p1_temp_credits : g->p2_temp_credits;
    int ranks = (player == 1) ? g->p1_completed_ranks : g->p2_completed_ranks;
    Color neon_color = (player == 1) ? SKYBLUE : (Color){255, 100, 100, 255};
    const char *player_name = GetPlayerName(core, player); // FIXED: removed + 1
    float margin_x = 25.0f;
    float margin_y = 25.0f;
    Rectangle screen_rect = {
        frame_rect.x + margin_x,
        frame_rect.y + margin_y,
        frame_rect.width - (margin_x * 2.0f),
        frame_rect.height - (margin_y * 1.5f)};
    DrawRectangleRec(screen_rect, Fade(BLACK, 0.85f));
    DrawRectangleLinesEx(screen_rect, 2.0f, Fade(neon_color, 0.5f));
    int text_x = (int)(screen_rect.x + 15.0f);
    int text_y = (int)(screen_rect.y + 15.0f);
    int line_h = 24;
    DrawText(TextFormat("USER: %s", player_name), text_x, text_y, 20, neon_color);
    text_y += line_h;
    DrawText(TextFormat("CREDITS: $%.2f", credits), text_x, text_y, 20, neon_color);
    text_y += line_h;
    // Temp Credits Display
    DrawText(TextFormat("GAME BALANCE: $%.2f", temp_credits), text_x, text_y, 20, YELLOW);
    text_y += line_h;
    DrawText(TextFormat("COMPLETED: %d/%d ROUND #%d...", ranks, KEYCARDS, g->total_rounds), text_x, text_y, 20, neon_color);
}
void DrawGameLayout(LobbyState *core, const GameState *g)
{
    Vector2 mouse = GetMousePosition();
    int gamepad = GetActiveGamepad();

    DrawPlayerUI(core, g, 1);
    DrawPlayerUI(core, g, 2);

    // Draw keycards and slots
    for (int k = 0; k < KEYCARDS; k++)
    {
        DrawCard(g->keycards[k], KeyCardRect(k), RAYWHITE);

        for (int s = 0; s < 3; s++)
        {
            Rectangle slot_rect = SlotRect(1, k, s);
            if (g->p1_slots[k][s].is_valid)
                DrawCard(g->p1_slots[k][s], slot_rect, RAYWHITE);
            else
                DrawTexturePro(g_atlas_texture, GetAtlasBackCard(), slot_rect, (Vector2){0, 0}, 0.0f, Fade(WHITE, 0.25f));
        }

        for (int s = 0; s < 3; s++)
        {
            Rectangle slot_rect = SlotRect(2, k, s);
            if (g->p2_slots[k][s].is_valid)
                DrawCard(g->p2_slots[k][s], slot_rect, RAYWHITE);
            else
                DrawTexturePro(g_atlas_texture, GetAtlasBackCard(), slot_rect, (Vector2){0, 0}, 0.0f, Fade(WHITE, 0.25f));
        }
    }

    // Draw player hands
    for (int i = 0; i < HAND_SIZE; i++)
    {
        Rectangle p1_rect = HandRect(1, i);
        if (i < g->p1_hand_size)
        {
            if (g->cover_p1_cards)
            {
                DrawTexturePro(g_atlas_texture, GetAtlasTempCover(), p1_rect, (Vector2){0, 0}, 0.0f, WHITE);
            }
            else
            {
                Color tint = (g->p1_discard_ready && i == g->p1_discard_idx) ? YELLOW : RAYWHITE;
                DrawCard(g->player1_hand[i], p1_rect, tint);
            }
        }

        bool p1_btn_enabled = false;
        const char *p1_btn_text = "---";
        if (g->state == STATE_P1_SELECT_DISCARD && i < g->p1_hand_size)
        {
            if (!IsPlayerAI(g, 1))
            {
                p1_btn_enabled = !g->p1_discard_ready;
                p1_btn_text = g->p1_discard_ready ? "READY" : "DISCARD";
            }
            else
                p1_btn_text = "AI";
        }
        else if (g->state == STATE_WAIT_FOR_TURN)
            p1_btn_text = "PLACE";

        DrawButton(ButtonRect(1, i), CheckCollisionPointRec(mouse, ButtonRect(1, i)), p1_btn_enabled, p1_btn_text);

        Rectangle p2_rect = HandRect(2, i);
        if (i < g->p2_hand_size)
        {

            bool should_cover = g->cover_p2_cards &&
                                ((!IsPlayerAI(g, 1) && g->state == STATE_P1_SELECT_DISCARD) ||
                                 g->mode == MODE_PVP);

            if (should_cover)
            {
                DrawTexturePro(g_atlas_texture, GetAtlasTempCover(), p2_rect, (Vector2){0, 0}, 0.0f, WHITE);
            }
            else
            {
                Color tint = (g->p2_discard_ready && i == g->p2_discard_idx) ? PURPLE : RAYWHITE;
                DrawCard(g->player2_hand[i], p2_rect, tint);
            }
        }

        bool p2_btn_enabled = false;
        const char *p2_btn_text = "---";
        if (g->state == STATE_P1_SELECT_DISCARD && i < g->p2_hand_size)
        {
            if (!IsPlayerAI(g, 2))
            {
                p2_btn_enabled = !g->p2_discard_ready;
                p2_btn_text = g->p2_discard_ready ? "READY" : "DISCARD";
            }
            else
                p2_btn_text = "AI";
        }
        else if (g->state == STATE_WAIT_FOR_TURN)
            p2_btn_text = "PLACE";

        DrawButton(ButtonRect(2, i), CheckCollisionPointRec(mouse, ButtonRect(2, i)), p2_btn_enabled, p2_btn_text);
    }

    // Draw discard piles
    Rectangle d1_rect = DiscardPileRect(1);
    if (g->revealed_p1.is_valid)
        DrawCard(g->revealed_p1, d1_rect, RAYWHITE);
    else
        DrawTexturePro(g_atlas_texture, GetAtlasBackCard(), d1_rect, (Vector2){0, 0}, 0.0f, WHITE);

    Rectangle d2_rect = DiscardPileRect(2);
    if (g->revealed_p2.is_valid)
        DrawCard(g->revealed_p2, d2_rect, RAYWHITE);
    else
        DrawTexturePro(g_atlas_texture, GetAtlasBackCard(), d2_rect, (Vector2){0, 0}, 0.0f, WHITE);

    DrawContinueButtons(g, mouse);

    // Keyboard shortcuts
    if (IsKeyPressed(KEY_COMMA) || (XboxBtnPressed(gamepad, 6)))
    {
        AutoLogout(core);
        SwitchState(core, STATE_MAIN_MENU);
    }
    
}
void AutoLogout(LobbyState *g)
{
    if (g->p2_account_index >= 0)
    {
        printf("[AUTO-LOGOUT] Logging out P2: %s\n",
               GetPlayerName(g, 2));
        LogoutAccount(g, 2);
    }
    if (g->game_state->mode == MODE_AIVAI)
    {
        printf("[AUTO-LOGOUT] Logging out Ai: %s\n",
               GetPlayerName(g, 1));
    }
    if (g->game_state->mode == MODE_BETTING)
    {
        printf("%s is logged in!\n",
               GetPlayerName(g, 1));
    }
}
void DrawGameOver(LobbyState *core, GameState *g)
{
    if (g->mode == MODE_BETTING)
    {
        Vector2 mouse = GetMousePosition();
        Rectangle menu_btn = {CENTER_X - 140, SCREEN_H - 150, 280, 80};
        DrawRectangleRec(menu_btn, CheckCollisionPointRec(mouse, menu_btn) ? SKYBLUE : BLUE);
        DrawText("MAIN MENU", (int)(menu_btn.x + 40), (int)(menu_btn.y + 25), 30, WHITE);
    }
    if (g->mode == MODE_PVP)
    {
    }
    if (g->mode == MODE_PVAI)
    {
        DrawRectangle(0, 0, (int)SCREEN_W, (int)SCREEN_H, Fade(BLACK, 0.8f));
        const char *winner_name = (g->winner == 1) ? GetPlayerName(core, 1) : GetPlayerName(core, 2);
        char win_text[128];
        snprintf(win_text, sizeof(win_text), "The Winner: %s!", winner_name);
        DrawText(win_text, (int)(CENTER_X - (float)MeasureText(win_text, 90) / 2.0f), (int)(SCREEN_H / 2 - 250), 90, GOLD);
        DrawText("CONGRATULATIONS!", (int)(CENTER_X - (float)MeasureText("CONGRATULATIONS!", 60) / 2.0f), (int)(SCREEN_H / 2 - 150), 60, YELLOW);
        // Final Score Display
        float winner_score = (g->winner == 1) ? g->final_score_p1 : g->final_score_p2;
        // Note: This score is calculated in State Machine, just display it
        DrawText(TextFormat("Final Result: $%.2f", winner_score), (int)(CENTER_X - (float)MeasureText(TextFormat("Final Result: $%.2f", winner_score), 40) / 2.0f), (int)(SCREEN_H / 2 - 50), 40, LIME);
        Vector2 mouse = GetMousePosition();
        Rectangle menu_btn = {CENTER_X - 140, SCREEN_H - 150, 280, 80};
        DrawRectangleRec(menu_btn, CheckCollisionPointRec(mouse, menu_btn) ? SKYBLUE : BLUE);
        DrawText("MAIN MENU", (int)(menu_btn.x + 40), (int)(menu_btn.y + 25), 30, WHITE);
    }
}
// Jokers gambit
float GetRewardMultiplier(int completed_ranks)
{
    if (completed_ranks >= 2)
        return 4.0f;
    if (completed_ranks == 1)
        return 2.0f;
    return 1.0f;
}
Card BlankCard(void)
{
    Card c = {0};
    c.is_valid = false;
    c.texture = (Texture2D){0};
    return c;
}
static bool IsRankCompleted(const Card slots[3])
{
    int count = 0;
    for (int i = 0; i < 3; i++)
        if (slots[i].is_valid)
            count++;
    return count == 3;
}
void CheckRankCompletionBonus(GameState *g, int player, int key_idx, int cards_before)
{
    Card(*slots)[3] = (player == 1) ? g->p1_slots : g->p2_slots;
    int cards_after = 0;
    for (int s = 0; s < 3; s++)
    {
        if (slots[key_idx][s].is_valid)
            cards_after++;
    }
    if (cards_after == 3 && cards_before < 3)
    {
        float *temp_credits = (player == 1) ? &g->p1_temp_credits : &g->p2_temp_credits;
        int *completed = (player == 1) ? &g->p1_completed_ranks : &g->p2_completed_ranks;
        float mult = GetRewardMultiplier(*completed);
        float reward = REWARD_COMPLETION * mult;
        *temp_credits += reward;
        (*completed)++;
        PlaySound(g_filled_rank_sound);
    }
}
Card DrawFromDeck(GameState *g)
{
    if (g->top_card_index < 0)
        return BlankCard();
    Card c = g->deck[g->top_card_index--];
    g->current_deck_size--;
    return c;
}
void ReturnToDeck(GameState *g, Card c)
{
    if (!c.is_valid || g->current_deck_size >= TOTAL_DECK_CARDS)
        return;
    g->top_card_index++;
    g->deck[g->top_card_index] = c;
    g->current_deck_size++;
    if (g->top_card_index > 0)
    {
        int r = rand() % (g->top_card_index + 1);
        Card t = g->deck[g->top_card_index];
        g->deck[g->top_card_index] = g->deck[r];
        g->deck[r] = t;
    }
}
static void TriggerSweep(GameState *g, int player)
{
    for (int k = 0; k < KEYCARDS; k++)
    {
        if (player == 0 || player == 1)
        {
            if (!IsRankCompleted(g->p1_slots[k]))
            {
                for (int s = 0; s < 3; s++)
                    if (g->p1_slots[k][s].is_valid)
                    {
                        ReturnToDeck(g, g->p1_slots[k][s]);
                        g->p1_slots[k][s] = BlankCard();
                    }
            }
        }
        if (player == 0 || player == 2)
        {
            if (!IsRankCompleted(g->p2_slots[k]))
            {
                for (int s = 0; s < 3; s++)
                    if (g->p2_slots[k][s].is_valid)
                    {
                        ReturnToDeck(g, g->p2_slots[k][s]);
                        g->p2_slots[k][s] = BlankCard();
                    }
            }
        }
    }
}
static void JokersGambit(GameState *g)
{
    TriggerSweep(g, 0);
    for (int i = 0; i < g->p1_hand_size; i++)
        ReturnToDeck(g, g->player1_hand[i]);
    for (int i = 0; i < g->p2_hand_size; i++)
        ReturnToDeck(g, g->player2_hand[i]);
    for (int i = 0; i < HAND_SIZE; i++)
    {
        g->player1_hand[i] = DrawFromDeck(g);
        g->player2_hand[i] = DrawFromDeck(g);
    }
    g->p1_hand_size = g->p2_hand_size = HAND_SIZE;
}
void ResolveDiscards(GameState *g)
{
    ProcessPendingDiscards(g);
    Card d1 = g->revealed_p1;
    Card d2 = g->revealed_p2;
    bool j1 = (d1.rank == RANK_JOKER);
    bool j2 = (d2.rank == RANK_JOKER);
    // Apply costs to TEMP credits
    g->p1_temp_credits -= COST_DISCARD;
    g->p2_temp_credits -= COST_DISCARD;
    float mult1 = GetRewardMultiplier(g->p1_completed_ranks);
    float mult2 = GetRewardMultiplier(g->p2_completed_ranks);
    if (j1 && j2)
    {
        JokersGambit(g);
        g->p1_temp_credits -= JOKER_DISCARD;
        g->p2_temp_credits -= JOKER_DISCARD;
        float reward1 = REWARD_DOUBLE_JOKER * mult1;
        float reward2 = REWARD_DOUBLE_JOKER * mult2;
        g->p1_temp_credits += reward1;
        g->p2_temp_credits += reward2;
        g->delay_discard_p1 = d1;
        g->delay_discard_p2 = d2;
        g->discards_pending = true;
        PlaySound(g_matching_jokers_sound);
    }
    else if (j1 || j2)
    {
        int opp = j1 ? 2 : 1;
        TriggerSweep(g, opp);
        if (j1)
            g->p1_temp_credits -= JOKER_DISCARD;
        if (j2)
            g->p2_temp_credits -= JOKER_DISCARD;
        g->player1_hand[g->p1_hand_size++] = DrawFromDeck(g);
        g->player2_hand[g->p2_hand_size++] = DrawFromDeck(g);
        g->delay_discard_p1 = d1;
        g->delay_discard_p2 = d2;
        g->discards_pending = true;
        PlaySound(g_joker_sound);
    }
    else if (d1.rank == d2.rank)
    {
        TriggerSweep(g, 0);
        float reward1 = REWARD_MATCH * mult1;
        float reward2 = REWARD_MATCH * mult2;
        g->p1_temp_credits += reward1;
        g->p2_temp_credits += reward2;
        g->player1_hand[g->p1_hand_size++] = DrawFromDeck(g);
        g->player2_hand[g->p2_hand_size++] = DrawFromDeck(g);
        g->delay_discard_p1 = d1;
        g->delay_discard_p2 = d2;
        g->discards_pending = true;
        PlaySound(g_matching_cards_sound);
    }
    else
    {
        g->player1_hand[g->p1_hand_size++] = DrawFromDeck(g);
        g->player2_hand[g->p2_hand_size++] = DrawFromDeck(g);
        g->delay_discard_p1 = d1;
        g->delay_discard_p2 = d2;
        g->discards_pending = true;
    }
    g->p1_done_placing = false;
    g->p2_done_placing = false;
    g->total_rounds++;
}
void RefreshHands(GameState *g)
{
    for (int i = 0; i < g->p1_hand_size; i++)
        ReturnToDeck(g, g->player1_hand[i]);
    for (int i = 0; i < g->p2_hand_size; i++)
        ReturnToDeck(g, g->player2_hand[i]);
    for (int i = 0; i < HAND_SIZE; i++)
    {
        g->player1_hand[i] = DrawFromDeck(g);
        g->player2_hand[i] = DrawFromDeck(g);
    }
    g->p1_hand_size = g->p2_hand_size = HAND_SIZE;
}
void UpdateWinStats(GameState *g)
{
    g->p1_completed_ranks = 0;
    g->p2_completed_ranks = 0;
    for (int k = 0; k < KEYCARDS; k++)
    {
        if (IsRankCompleted(g->p1_slots[k]))
            g->p1_completed_ranks++;
        if (IsRankCompleted(g->p2_slots[k]))
            g->p2_completed_ranks++;
    }
}
void ProcessPendingDiscards(GameState *g)
{
    if (g->discards_pending)
    {
        if (rand() % 2 == 0)
        {
            if (g->delay_discard_p1.is_valid)
            {
                ReturnToDeck(g, g->delay_discard_p1);
                g->delay_discard_p1 = BlankCard();
            }
            if (g->delay_discard_p2.is_valid)
            {
                ReturnToDeck(g, g->delay_discard_p2);
                g->delay_discard_p2 = BlankCard();
            }
        }
        else
        {
            if (g->delay_discard_p2.is_valid)
            {
                ReturnToDeck(g, g->delay_discard_p2);
                g->delay_discard_p2 = BlankCard();
            }
            if (g->delay_discard_p1.is_valid)
            {
                ReturnToDeck(g, g->delay_discard_p1);
                g->delay_discard_p1 = BlankCard();
            }
        }
        g->discards_pending = false;
    }
}
void InitGame(LobbyState *core)
{
    GameState *g = core->game_state; // Use pointer
    // Save references that we need to preserve
    GameMode saved_mode = g->mode;
    AIType saved_opponent_ai = g->selected_opponent_ai;
    // Zero out entire game state
    memset(g, 0, sizeof(GameState));
    // Restore preserved data
    g->mode = saved_mode;
    g->selected_opponent_ai = saved_opponent_ai;
    // Copy account data FROM LobbyState TO GameState
    memcpy(g->accounts, core->accounts, sizeof(core->accounts));
    g->account_count = core->account_count;
    g->p1_account_index = core->p1_account_index;
    g->p2_account_index = core->p2_account_index;
    // Copy settings from LobbyState
    g->cover_p2_cards = core->cover_p2_cards;
    // Set AI delay based on mode
    switch (core->ai_delay_mode)
    {
    case AI_DELAY_FAST:
        g->ai_move_delay = 0.5f;
        break;
    case AI_DELAY_NORMAL:
        g->ai_move_delay = 1.0f;
        break;
    case AI_DELAY_SLOW:
        g->ai_move_delay = 2.0f;
        break;
    default:
        g->ai_move_delay = 1.0f;
        break;
    }
    // Initialize TEMP CREDITS
    g->p1_temp_credits = P1_TEMP_CREDITS_START;
    g->p2_temp_credits = P2_TEMP_CREDITS_START;
    // Initialize keycards
    Rank keys[5] = {RANK_ACE, RANK_KING, RANK_QUEEN, RANK_JACK, RANK_10};
    int idx = 0;
    // Build deck (all cards except keycards)
    for (int s = 0; s < 4; s++)
    {
        for (int r = 0; r < 13; r++)
        {
            bool is_key = (s == SUIT_HEARTS &&
                           (r == RANK_10 || r == RANK_JACK || r == RANK_QUEEN ||
                            r == RANK_KING || r == RANK_ACE));
            if (!is_key)
            {
                g->deck[idx].rank = (Rank)r;
                g->deck[idx].suit = (Suit)s;
                g->deck[idx].is_valid = true;
                g->deck[idx].texture = (Texture2D){0};
                idx++;
            }
        }
    }
    // Add jokers
    for (int i = 0; i < 2; i++)
    {
        g->deck[idx].rank = RANK_JOKER;
        g->deck[idx].suit = SUIT_NONE;
        g->deck[idx].is_valid = true;
        g->deck[idx].texture = (Texture2D){0};
        snprintf(g->deck[idx].filename, 16, i == 0 ? "JA.png" : "JB.png");
        idx++;
    }
    g->current_deck_size = idx;
    g->top_card_index = idx - 1;
    // Initialize keycards
    for (int i = 0; i < KEYCARDS; i++)
    {
        g->keycards[i].rank = keys[i];
        g->keycards[i].suit = SUIT_HEARTS;
        g->keycards[i].is_valid = true;
        g->keycards[i].texture = (Texture2D){0};
    }
    // Shuffle deck
    for (int i = g->top_card_index; i > 0; i--)
    {
        int j = rand() % (i + 1);
        Card t = g->deck[i];
        g->deck[i] = g->deck[j];
        g->deck[j] = t;
    }
    // Deal initial hands
    for (int i = 0; i < HAND_SIZE; i++)
    {
        g->player1_hand[i] = DrawFromDeck(g);
        g->player2_hand[i] = DrawFromDeck(g);
    }
    g->p1_hand_size = g->p2_hand_size = HAND_SIZE;
    // Set Initial Account Credits for Display
    if (g->p1_account_index != -1)
    {
        g->p1_credits = (float)g->accounts[g->p1_account_index].credits;
    }
    else
    {
        g->p1_credits = DEFAULT_CREDITS;
    }
    if (g->p2_account_index != -1)
    {
        g->p2_credits = (float)g->accounts[g->p2_account_index].credits;
    }
    else
    {
        g->p2_credits = DEFAULT_CREDITS;
    }
    g->delay_discard_p1 = BlankCard();
    g->delay_discard_p2 = BlankCard();
    g->discards_pending = false;
    g->state = STATE_P1_SELECT_DISCARD;
    g->p1_completed_ranks = g->p2_completed_ranks = 0;
    g->total_rounds = 0;
    g->placement_phases_count = 0;
    g->p1_done_placing = g->p2_done_placing = false;
    g->game_over = false;
    g->p1_selected = g->p2_selected = false;
    g->p1_discard_ready = false;
    g->p2_discard_ready = false;
}
void RestartGameKeepingAccounts(LobbyState *core)
{
    // Sync updated account data back to LobbyState before restarting
    memcpy(core->accounts, core->game_state->accounts, sizeof(core->accounts));
    core->account_count = core->game_state->account_count;
    InitGame(core);
}
// Add leaderboard entry for completed game
void AddLeaderboardEntry(LobbyState *core, int winner)
{
    GameState *g = core->game_state;
    // PvP matches are friendly - NO Leaderboard entry
    if (g->mode == MODE_PVP)
        return;
    if (core->leaderboard_count >= MAX_LEADERBOARD_ENTRIES)
    {
        int worst_idx = 0;
        float worst_score = core->leaderboard[0].total_winnings;
        for (int i = 1; i < core->leaderboard_count; i++)
        {
            if (core->leaderboard[i].total_winnings < worst_score)
            {
                worst_score = core->leaderboard[i].total_winnings;
                worst_idx = i;
            }
        }
        for (int i = worst_idx; i < MAX_LEADERBOARD_ENTRIES - 1; i++)
            core->leaderboard[i] = core->leaderboard[i + 1];
        core->leaderboard_count = MAX_LEADERBOARD_ENTRIES - 1;
    }
    LeaderboardEntry *e = &core->leaderboard[core->leaderboard_count];
    const char *p1_name_str = GetPlayerName(core, 1);
    const char *p2_name_str = GetPlayerName(core, 2);
    snprintf(e->entry_name, 64, "%s_vs_%s", p1_name_str, p2_name_str);
    snprintf(e->winner_name, 32, "%s", (winner == 1) ? p1_name_str : p2_name_str);
    float winner_temp = (winner == 1) ? g->p1_temp_credits : g->p2_temp_credits; // For Leaderboard, we track the TEMP credits earned (plus bonus)
    float winner_bonus = REWARD_MATCH * (float)g->total_rounds;
    e->total_winnings = winner_temp + winner_bonus;
    e->final_credits = winner_temp;
    e->bonus = winner_bonus;
    e->total_rounds = g->total_rounds;
    e->game_played = GAME_JOKERS_GAMBIT;
    time_t timer;
    time(&timer);
    struct tm *tm_info = localtime(&timer);
    strftime(e->timestamp, 32, "%m/%d/%Y %H:%M", tm_info); // Added %H:%M for time
    core->leaderboard_count++;
    // Sort by winnings (descending)
    for (int i = 0; i < core->leaderboard_count - 1; i++)
    {
        for (int j = i + 1; j < core->leaderboard_count; j++)
        {
            if (core->leaderboard[j].total_winnings > core->leaderboard[i].total_winnings)
            {
                LeaderboardEntry temp = core->leaderboard[i];
                core->leaderboard[i] = core->leaderboard[j];
                core->leaderboard[j] = temp;
            }
        }
    }
    SaveLeaderboard(core);
}
void UpdateJokersGambit(LobbyState *core, Vector2 mouse)
{
    GameState *g = core->game_state;

    // --- NETWORK PACKET HANDLING ---
    if (core->net_connected)
    {
        NetPacket pkt;
        while (ReceivePacket(core, &pkt))
        {
            if (pkt.type == PKT_SEED)
            {
                srand((unsigned int)pkt.data); // Fixed sign conversion
                InitGame(core);
            }
            else if (pkt.type == PKT_DISCARD)
            {
                if (core->net_role == NET_HOST)
                {
                    g->p2_discard_idx = pkt.data;
                    g->p2_discard_ready = true;
                }
                else
                {
                    g->p1_discard_idx = pkt.data;
                    g->p1_discard_ready = true;
                }
                PlaySound(g_discard_sound);
            }
            else if (pkt.type == PKT_PASS)
            {
                if (core->net_role == NET_HOST)
                    g->p2_done_placing = true;
                else
                    g->p1_done_placing = true;
            }
        }
    }
    // Handle game over state
    if (g->game_over)
    {
        Rectangle menu_btn = {CENTER_X - 140, SCREEN_H - 150, 280, 80};

        // Mouse input
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
             if (CheckCollisionPointRec(mouse, menu_btn))
            {
                AutoLogout(core);
                SwitchState(core, STATE_MAIN_MENU);
            }
        }

        return;
    }

    switch (g->state)
    {
    case STATE_ROUND_START:
    {
        if (g->mode == MODE_PVAI)
        {
            core->accounts[core->p1_account_index].credits -= P1_TEMP_CREDITS_START;
        }
        g->state = STATE_P1_SELECT_DISCARD;
        break;
    }

    case STATE_P1_SELECT_DISCARD:
    {
        // AI P1 logic
        if (IsPlayerAI(g, 1) && !g->p1_discard_ready)
        {
            AI_SelectDiscard(g, 1);
            g->p1_discard_ready = true;
        }
        // Human P1 logic
        else if (!IsPlayerAI(g, 1) && !g->p1_discard_ready)
        {
            for (int i = 0; i < g->p1_hand_size; i++)
            {
                Rectangle btn_rect = ButtonRect(1, i);
                if (CheckCollisionPointRec(mouse, btn_rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    g->p1_discard_idx = i;
                    g->p1_discard_ready = true;
                    PlaySound(g_discard_sound);
                    break;
                }
            }
        }

        // AI P2 logic
        if (IsPlayerAI(g, 2) && !g->p2_discard_ready)
        {
            AI_SelectDiscard(g, 2);
            g->p2_discard_ready = true;
        }
        // Human P2 logic
        else if (!IsPlayerAI(g, 2) && !g->p2_discard_ready)
        {
            for (int i = 0; i < g->p2_hand_size; i++)
            {
                Rectangle btn_rect = ButtonRect(2, i);
                if (CheckCollisionPointRec(mouse, btn_rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    g->p2_discard_idx = i;
                    g->p2_discard_ready = true;
                    PlaySound(g_discard_sound);
                    break;
                }
            }
        }

        // Check if both players are ready
        if (g->p1_discard_ready && g->p2_discard_ready)
        {
            g->state = STATE_REVEAL_AND_RESOLVE;
        }
        break;
    }

    case STATE_REVEAL_AND_RESOLVE:
    {
        g->revealed_p1 = g->player1_hand[g->p1_discard_idx];
        g->revealed_p2 = g->player2_hand[g->p2_discard_idx];
        g->p1_discard_ready = false;
        g->p2_discard_ready = false;

        // Remove discarded cards from hands
        for (int i = g->p1_discard_idx; i < g->p1_hand_size - 1; i++)
            g->player1_hand[i] = g->player1_hand[i + 1];
        g->p1_hand_size--;

        for (int i = g->p2_discard_idx; i < g->p2_hand_size - 1; i++)
            g->player2_hand[i] = g->player2_hand[i + 1];
        g->p2_hand_size--;

        ResolveDiscards(g);
        g->state = STATE_WAIT_FOR_TURN;
        g->ai_timer = 0;
        g->Reshuffle_cover_timer = 0;
        break;
    }

    case STATE_WAIT_FOR_TURN:
    {
        // AI placement updates
        if (IsPlayerAI(g, 1) && !g->p1_done_placing)
            AI_UpdatePlacementPhase(g, 1);
        if (IsPlayerAI(g, 2) && !g->p2_done_placing)
            AI_UpdatePlacementPhase(g, 2);

        // Check if both done
        if (g->p1_done_placing && g->p2_done_placing)
        {
            g->state = STATE_HAND_RESHUFFLE;
            g->p1_ai_done_placing_rounds = false;
            break;
        }

        float mult1 = GetRewardMultiplier(g->p1_completed_ranks);
        float mult2 = GetRewardMultiplier(g->p2_completed_ranks);
        float reward1 = REWARD_PLACEMENT * mult1;
        float reward2 = REWARD_PLACEMENT * mult2;

        // P1 Human Placement
        if (!IsPlayerAI(g, 1) && !g->p1_done_placing)
        {
            for (int i = 0; i < g->p1_hand_size; i++)
            {
                Rectangle card_rect = HandRect(1, i);
                if (CheckCollisionPointRec(mouse, card_rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    Card c = g->player1_hand[i];
                    if (c.suit != SUIT_HEARTS && c.rank != RANK_JOKER)
                    {
                        for (int k = 0; k < KEYCARDS; k++)
                        {
                            if (c.rank == g->keycards[k].rank)
                            {
                                int cards_before = 0;
                                for (int s_check = 0; s_check < 3; s_check++)
                                    if (g->p1_slots[k][s_check].is_valid)
                                        cards_before++;

                                if (cards_before < 3)
                                {
                                    for (int s = 0; s < 3; s++)
                                    {
                                        if (!g->p1_slots[k][s].is_valid)
                                        {
                                            g->p1_slots[k][s] = c;
                                            g->p1_temp_credits += reward1;
                                            CheckRankCompletionBonus(g, 1, k, cards_before);

                                            // Remove from hand
                                            for (int j = i; j < g->p1_hand_size - 1; j++)
                                                g->player1_hand[j] = g->player1_hand[j + 1];
                                            g->p1_hand_size--;

                                            // Draw new card
                                            g->player1_hand[g->p1_hand_size++] = DrawFromDeck(g);
                                            PlaySound(g_place_sound);
                                            break;
                                        }
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }

        // P2 Human Placement
        if (!IsPlayerAI(g, 2) && !g->p2_done_placing)
        {
            for (int i = 0; i < g->p2_hand_size; i++)
            {
                Rectangle card_rect = HandRect(2, i);
                if (CheckCollisionPointRec(mouse, card_rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
                {
                    Card c = g->player2_hand[i];
                    if (c.suit != SUIT_HEARTS && c.rank != RANK_JOKER)
                    {
                        for (int k = 0; k < KEYCARDS; k++)
                        {
                            if (c.rank == g->keycards[k].rank)
                            {
                                int cards_before = 0;
                                for (int s_check = 0; s_check < 3; s_check++)
                                    if (g->p2_slots[k][s_check].is_valid)
                                        cards_before++;

                                if (cards_before < 3)
                                {
                                    for (int s = 0; s < 3; s++)
                                    {
                                        if (!g->p2_slots[k][s].is_valid)
                                        {
                                            g->p2_slots[k][s] = c;
                                            g->p2_temp_credits += reward2;
                                            CheckRankCompletionBonus(g, 2, k, cards_before);

                                            // Remove from hand
                                            for (int j = i; j < g->p2_hand_size - 1; j++)
                                                g->player2_hand[j] = g->player2_hand[j + 1];
                                            g->p2_hand_size--;

                                            // Draw new card
                                            g->player2_hand[g->p2_hand_size++] = DrawFromDeck(g);
                                            PlaySound(g_place_sound);
                                            break;
                                        }
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }

        // PASS BUTTON LOGIC
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        {
            Rectangle p1_pass = ContinueButtonRect(1);
            Rectangle p2_pass = ContinueButtonRect(2);

            if (!IsPlayerAI(g, 1) && !g->p1_done_placing && CheckCollisionPointRec(mouse, p1_pass))
            {
                g->p1_done_placing = true;
                g->p1_temp_credits -= COST_PER_ROUND;
                PlaySound(g_coin_sound);
            }
            if (!IsPlayerAI(g, 2) && !g->p2_done_placing && CheckCollisionPointRec(mouse, p2_pass))
            {
                g->p2_done_placing = true;
                g->p2_temp_credits -= COST_PER_ROUND;
                PlaySound(g_coin_sound);
            }
        }

        int gamepad = GetActiveGamepad();

        // Keyboard Shortcuts
        bool p1_continue = IsKeyPressed(KEY_ONE) || (XboxBtnPressed(gamepad, 1));
        bool p2_continue = IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_KP_2);

        if (p1_continue && !g->p1_done_placing && !IsPlayerAI(g, 1))
        {
            g->p1_done_placing = true;
            g->p1_temp_credits -= COST_PER_ROUND;
            PlaySound(g_coin_sound);
        }
        if (p2_continue && !g->p2_done_placing && !IsPlayerAI(g, 2))
        {
            g->p2_done_placing = true;
            g->p2_temp_credits -= COST_PER_ROUND;
            PlaySound(g_coin_sound);
        }
        break;
    }

    case STATE_HAND_RESHUFFLE:
    {
        g->placement_phases_count++;
        if (g->placement_phases_count % 5 == 0)
        {
            g->state = STATE_COVER_ANIMATION;
        }
        else
        {
            g->revealed_p1 = BlankCard();
            g->revealed_p2 = BlankCard();
            g->state = STATE_CHECK_WIN;
        }
        break;
    }

    case STATE_COVER_ANIMATION:
    {
        g->Reshuffle_cover_timer += GetFrameTime();

        if (g->Reshuffle_cover_timer >= 2.0f)
        {
            RefreshHands(g);
            g->revealed_p1 = BlankCard();
            g->revealed_p2 = BlankCard();
            g->Reshuffle_cover_timer = 0.0f;
            g->state = STATE_CHECK_WIN;
            PlaySound(g_shuffle_sound);
        }
        break;
    }

    case STATE_CHECK_WIN:
    {
        UpdateWinStats(g);

        // Check win condition (3 or more completed ranks)
        if (g->p1_completed_ranks >= 3 || g->p2_completed_ranks >= 3)
        {
            g->winner = (g->p1_completed_ranks >= 3) ? 1 : 2;
            g->game_over = true;
            g->win_timer_start = GetTime();

            // Calculate final scores
            float winner_temp = (g->winner == 1) ? g->p1_temp_credits : g->p2_temp_credits;
            float loser_temp = (g->winner == 1) ? g->p2_temp_credits : g->p1_temp_credits;
            float winner_bonus = REWARD_MATCH * (float)g->total_rounds;

            g->final_score_p1 = (g->winner == 1) ? (winner_temp + winner_bonus) : (loser_temp - LOSER_PENALTY);
            g->final_score_p2 = (g->winner == 2) ? (winner_temp + winner_bonus) : (loser_temp - LOSER_PENALTY);

            // Update account credits based on mode
            if (g->mode == MODE_PVP)
            {
                // Friendly match, no credit updates, no stats
            }
            else if (g->mode == MODE_AIVAI && !g->accounts->active_bet)
            {
                // Friendly AI match, no updates
            }
            else if (g->mode == MODE_PVAI)
            {
                // Update P1 account (human player)
                if (core->p1_account_index >= 0)
                {
                    double net_change;
                    if (g->winner == 1)
                    {
                        net_change = (double)(winner_temp + winner_bonus - P1_TEMP_CREDITS_START);
                        core->accounts[core->p1_account_index].credits += (double)(winner_temp + winner_bonus);
                        core->accounts[core->p1_account_index].wins++;
                    }
                    else
                    {
                        net_change = (double)(loser_temp - LOSER_PENALTY - P1_TEMP_CREDITS_START);
                        core->accounts[core->p1_account_index].credits += (double)(loser_temp - LOSER_PENALTY);
                        core->accounts[core->p1_account_index].losses++;
                    }
                    core->accounts[core->p1_account_index].tokens += 1.0;

                    if (!core->accounts[core->p1_account_index].is_ai)
                    {
                        UpdateGameStats(core, core->p1_account_index, GAME_JOKERS_GAMBIT, net_change);
                    }
                }

                // Update AI opponent account (simplified - no stats tracking)
                if (core->p2_account_index >= 0)
                {
                    if (g->winner == 2)
                    {
                        core->accounts[core->p2_account_index].credits += (double)(winner_temp + winner_bonus);
                        core->accounts[core->p2_account_index].wins++;
                    }
                    else
                    {
                        core->accounts[core->p2_account_index].credits += (double)(loser_temp - LOSER_PENALTY);
                        core->accounts[core->p2_account_index].losses++;
                    }
                    core->accounts[core->p2_account_index].tokens += 1.0;
                    // No stats tracking for AI opponents
                }
            }

            else if (g->mode == MODE_BETTING)
            {
                // Calculate betting payout (ONLY HERE, not in STATE_GAME_OVER)
                double payout_odds[][2] = {
                    {1.43, 3.33},  // FLINT vs THEA (THEA win, FLINT win)
                    {1.11, 10.00}, // BOB vs FLINT
                    {1.67, 2.50},  // THEA vs BOB
                    {5.00, 5.00}   // RANDOM
                };

                int matchup = core->betting.selected_matchup;
                double multiplier = (g->winner == 1) ? payout_odds[matchup][1] : payout_odds[matchup][0];

                bool player_won = false;
                double winnings = 0;

                if (g->winner == core->betting.bet_on_player)
                {
                    double bet_amount = (core->betting.bet_on_player == 1) ? core->betting.p1_bet_amount : core->betting.p2_bet_amount;
                    winnings = bet_amount * multiplier;
                    player_won = true;
                }

                if (player_won && core->betting.original_player >= 0)
                {
                    core->accounts[core->betting.original_player].tokens += winnings;
                    core->betting.net_profit = winnings - (core->betting.p1_bet_amount + core->betting.p2_bet_amount);
                    core->betting.payout_multiplier = multiplier;
                    core->betting.player_won_bet = true;
                    ShowNotification(core, "BET WON!", TextFormat("You won %.0f tokens!", winnings));
                }
                else
                {
                    core->betting.player_won_bet = false;
                    core->betting.net_profit = -(core->betting.p1_bet_amount + core->betting.p2_bet_amount);
                    ShowNotification(core, "BET LOST", "Better luck next time!");
                }

                // Clear betting flag
                if (core->betting.original_player >= 0)
                {
                    core->accounts[core->betting.original_player].active_bet = false;
                }
            }

            SaveAllAccounts(core);
            SaveAchievements(core);
            UpdateAccountCredits(core);
            AddLeaderboardEntry(core, g->winner);
            PlaySound(g_win_sound);

            g->state = STATE_GAME_OVER;
        }
        else
        {
            // Continue game
            g->revealed_p1 = BlankCard();
            g->revealed_p2 = BlankCard();
            g->state = STATE_P1_SELECT_DISCARD;
        }
        break;
    }

    case STATE_GAME_OVER:
        // No update logic needed here - handled at top of function
        break;

    case STATE_P2_SELECT_DISCARD:
    case STATE_ROUNDS_COMPLETED:
    default:
        break;
    }
}

 
void StartPVPGame(LobbyState *g)
{
    // Set mode
    g->game_state->mode = MODE_PVP;

    // Initialize game
    InitGame(g);

    // Show notification if split-screen selected
    if (g->pvp_multiplayer)
    {
        ShowNotification(g, "SPLIT-SCREEN MODE", "Each player sees their own view!");
    }

    // Start game
    SwitchState(g, STATE_JOKERS_GAMBIT);
}
void DrawJokersGambit(const LobbyState *core)
{
    const GameState *g = core->game_state;

    if (g->game_over)
    {
        // Game over screen (full screen)
        DrawGameOver((LobbyState *)core, (GameState *)g);
        return;
    }

    // Check if we're in split-screen mode
    if (g->mode == MODE_PVP && core->pvp_multiplayer)
    {
        // === SPLIT-SCREEN MODE ===
        float half_width = SCREEN_W / 2.0f;

        // Clear entire screen first
        ClearBackground(BLACK);

        // Draw P1 view on LEFT half
        BeginScissorMode(0, 0, (int)half_width, (int)SCREEN_H);
         EndScissorMode();

        // Draw P2 view on RIGHT half
        BeginScissorMode((int)half_width, 0, (int)half_width, (int)SCREEN_H);
         EndScissorMode();

        // Draw divider line
        DrawRectangle((int)half_width - 2, 0, 4, (int)SCREEN_H, GOLD);

        // Draw player labels
        DrawText("PLAYER 1", 40, 30, 40, SKYBLUE);
        DrawText("PLAYER 2", (int)half_width + 40, 30, 40, ORANGE);
    }
    else
    {
        // === NORMAL SINGLE-VIEW MODE ===
        if (g_background_texture.id != 0)
        {
            DrawTexturePro(g_background_texture,
                           (Rectangle){0, 0, (float)g_background_texture.width, (float)g_background_texture.height},
                           (Rectangle){0, 0, SCREEN_W, SCREEN_H},
                           (Vector2){0, 0}, 0.0f, WHITE);
        }
        else
        {
            ClearBackground((Color){20, 30, 40, 255});
        }

        DrawGameLayout((LobbyState *)core, g);
    }
}

void UpdateDiscardSelection(GameState *g, Vector2 mouse) // UNDER CONSTRUCTION XXX
{
    if (g->mode == MODE_PVP)
    {
        if (!IsPlayerAI(g, 1) && !g->p1_discard_ready)
        {
            for (int i = 0; i < g->p1_hand_size; i++)
            {
                Rectangle btn = ButtonRect(1, i);
                // Mouse input
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, btn))
                {
                    g->p1_discard_idx = i;
                    g->p1_discard_ready = true;
                    g->p1_selected = true;
                    PlaySound(g_discard_sound);
                }
                // Keyboard input (1-5 keys)
                if (IsKeyPressed(KEY_ONE + i))
                {
                    g->p1_discard_idx = i;
                    g->p1_discard_ready = true;
                    g->p1_selected = true;
                    PlaySound(g_discard_sound);
                }
            }
        }
        // Handle human P2 input PVP
        if (!IsPlayerAI(g, 2) && !g->p2_discard_ready)
        {
            for (int i = 0; i < g->p2_hand_size; i++)
            {
                Rectangle btn = ButtonRect(2, i);
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, btn))
                {
                    g->p2_discard_idx = i;
                    g->p2_discard_ready = true;
                    g->p2_selected = true;
                    PlaySound(g_discard_sound);
                }
            }
        }
    }
    if (g->mode == MODE_PVAI)
    {
        if (!IsPlayerAI(g, 1) && !g->p1_discard_ready)
        {
            for (int i = 0; i < g->p1_hand_size; i++)
            {
                Rectangle btn = ButtonRect(1, i);
                // Mouse input
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, btn))
                {
                    g->p1_discard_idx = i;
                    g->p1_discard_ready = true;
                    g->p1_selected = true;
                    PlaySound(g_discard_sound);
                }
                // Keyboard input (1-5 keys)
                if (IsKeyPressed(KEY_ONE + i))
                {
                    g->p1_discard_idx = i;
                    g->p1_discard_ready = true;
                    g->p1_selected = true;
                    PlaySound(g_discard_sound);
                }
            }
        }
        if (!g->p2_selected && IsPlayerAI(g, 2))
        {
            AI_SelectDiscard(g, 2);
        }
        if (g->mode == MODE_AIVAI || g->mode == MODE_BETTING)
            if (!g->p1_selected && IsPlayerAI(g, 1))
            {
                AI_SelectDiscard(g, 1);
            }
        if (!g->p2_selected && IsPlayerAI(g, 2))
        {
            AI_SelectDiscard(g, 2);
        }
        if (g->p1_selected && g->p2_selected)
        {
            // Reveal cards
            g->revealed_p1 = g->player1_hand[g->p1_discard_idx];
            g->revealed_p2 = g->player2_hand[g->p2_discard_idx];
            // Remove from hands
            for (int j = g->p1_discard_idx; j < g->p1_hand_size - 1; j++)
                g->player1_hand[j] = g->player1_hand[j + 1];
            g->p1_hand_size--;
            for (int j = g->p2_discard_idx; j < g->p2_hand_size - 1; j++)
                g->player2_hand[j] = g->player2_hand[j + 1];
            g->p2_hand_size--;
            PlaySound(g_reveal_sound);
            // Process discards and move to placement phase
            ResolveDiscards(g);
            g->state = STATE_WAIT_FOR_TURN;
            g->p1_selected = false;
            g->p2_selected = false;
            g->p1_discard_ready = false;
            g->p2_discard_ready = false;
        }
    }
}
void UpdatePlacementPhase(GameState *g, Vector2 mouse)
{
    // Update AI players
    if (!g->p1_done_placing && IsPlayerAI(g, 1))
    {
        AI_UpdatePlacementPhase(g, 1);
    }
    if (!g->p2_done_placing && IsPlayerAI(g, 2))
    {
        AI_UpdatePlacementPhase(g, 2);
    }
    // Handle human P1 placement
    if (!g->p1_done_placing && !IsPlayerAI(g, 1))
    {
        // Check for PASS button click
        Rectangle continue_btn = ContinueButtonRect(1);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, continue_btn))
        {
            g->p1_done_placing = true;
            g->p1_temp_credits -= COST_PER_ROUND;
            PlaySound(g_continue_sound);
        }
    }
    // Handle human P2 placement (if not AI)
    if (!g->p2_done_placing && !IsPlayerAI(g, 2))
    {
        Rectangle continue_btn = ContinueButtonRect(2);
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(mouse, continue_btn))
        {
            g->p2_done_placing = true;
            g->p2_temp_credits -= COST_PER_ROUND;
            PlaySound(g_continue_sound);
        }
    }
    // Check if both players are done placing
    if (g->p1_done_placing && g->p2_done_placing)
    {
        g->placement_phases_count++;
        g->state = STATE_CHECK_WIN;
    }
}
void CheckWinCondition(GameState *g, LobbyState *core)
{
    UpdateWinStats(g);
    // Check for winner
    if (g->p1_completed_ranks >= KEYCARDS || g->p2_completed_ranks >= KEYCARDS)
    {
        g->winner = (g->p1_completed_ranks >= KEYCARDS) ? 1 : 2;
        // Calculate final scores using TEMP credits
        g->final_score_p1 = g->p1_temp_credits;
        g->final_score_p2 = g->p2_temp_credits;
        // Only update account credits/stats in PvAI mode
        if (g->mode == MODE_PVAI)
        {
            // Sync game state accounts back to lobby state
            memcpy(core->accounts, g->accounts, sizeof(core->accounts));
            core->account_count = g->account_count;
            // Update P1 account
            if (core->p1_account_index >= 0)
            {
                Account *p1_acc = &core->accounts[core->p1_account_index];
                float net_change = g->p1_temp_credits - P1_TEMP_CREDITS_START;
                p1_acc->credits += (double)net_change;
                // Update stats
                UpdateGameStats(core, core->p1_account_index, GAME_JOKERS_GAMBIT, (double)net_change);
                // Check for achievements
                CheckAchievements(p1_acc, core);
            }
            // Update AI opponent account
            if (core->p2_account_index >= 0 && core->accounts[core->p2_account_index].is_ai)
            {
                float net_change = g->p2_temp_credits - P2_TEMP_CREDITS_START;
                core->accounts[core->p2_account_index].credits += (double)net_change;
            }
            SaveAllAccounts(core);
            SaveAchievements(core);
            UpdateAccountCredits(core);
        }
        else if (g->mode == MODE_BETTING)
        {
            int original_idx = core->betting.original_player;
            if (original_idx >= 0)
            {
                bool won = (core->betting.bet_on_player == g->winner);
                double bet_amount = (core->betting.bet_on_player == 1 ? core->betting.p1_bet_amount : core->betting.p2_bet_amount);
                if (won)
                {
                    // Assuming 2x payout for simplicity; adjust as needed
                    double multiplier = 2.0;
                    double payout = bet_amount * multiplier;
                    core->accounts[original_idx].tokens += payout;
                    ShowNotification(core, "YOU WON THE BET!", TextFormat("+%.0f TOKENS", payout));
                    core->betting.player_won_bet = true;
                    core->betting.net_profit = payout - bet_amount;
                    core->betting.payout_multiplier = multiplier;
                }
                else
                {
                    ShowNotification(core, "BET LOST", "Better luck next time!");
                    core->betting.player_won_bet = false;
                    core->betting.net_profit = -bet_amount;
                }
                // Update stats if desired
                SaveAllAccounts(core);
            }
            // Logout AIs and restore original player
            LogoutAccount(core, 1);
            LogoutAccount(core, 2);
            LoginAccount(core, original_idx, 1);
            core->accounts[original_idx].active_bet = false; // Clear flag
        }
        // Auto-logout P2 after game ends (PVP or AI modes)
        if (g->mode == MODE_PVP || g->mode == MODE_AIVAI)
        {
            AutoLogout(core);
        }
        g->game_over = true;
        PlaySound(g_win_sound);
    }
    else
    {
        // Continue to next round
        g->state = STATE_P1_SELECT_DISCARD;
        g->p1_done_placing = false;
        g->p2_done_placing = false;
    }
}
bool IsPlayerAI(const GameState *g, int player)
{
    int account_idx = (player == 1) ? g->p1_account_index : g->p2_account_index;
    if (account_idx < 0 || account_idx >= g->account_count)
        return false;
    return g->accounts[account_idx].is_ai;
}