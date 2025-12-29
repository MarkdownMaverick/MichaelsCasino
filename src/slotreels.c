#include "slotreels.h"
#include "main.h"           // ← This gives full LobbyState definition
#include "mainmenu.h"
#include "useraccount.h"
#include "jokersgambit.h"


// Classic Jacks or Better reel strip (rank-only, no suits)
static const Rank reel_strip[] = {
    RANK_10, RANK_JACK, RANK_QUEEN, RANK_KING, RANK_ACE,
    RANK_9,  RANK_10,  RANK_JACK, RANK_QUEEN, RANK_KING,
    RANK_8,  RANK_9,   RANK_10,  RANK_JACK, RANK_QUEEN,
    RANK_7,  RANK_8,   RANK_9,   RANK_10,  RANK_JACK,
    RANK_6,  RANK_7,   RANK_8,   RANK_9,   RANK_10,
    RANK_5,  RANK_6,   RANK_7,   RANK_8,   RANK_9,
    RANK_4,  RANK_5,   RANK_6,   RANK_7,   RANK_8,
    RANK_3,  RANK_4,   RANK_5,   RANK_6,   RANK_7,
    RANK_2,  RANK_3,   RANK_4,   RANK_5,   RANK_6,
    RANK_ACE, RANK_KING, RANK_QUEEN, RANK_JACK, RANK_10
};
static const int REEL_STRIP_LENGTH = sizeof(reel_strip) / sizeof(reel_strip[0]);

static Rank GetSymbolFromStrip(int position)
{
    return reel_strip[position % REEL_STRIP_LENGTH];
}

void InitSlotReels(SlotReelsState *slot)
{
    memset(slot, 0, sizeof(SlotReelsState));
    slot->bet_level = BET_1_TOKENS;
    slot->state = SLOT_STATE_IDLE;

    for (int r = 0; r < REELS_COUNT; r++) {
        int start = rand() % REEL_STRIP_LENGTH;
        for (int i = 0; i < VISIBLE_SYMBOLS + 2; i++) {
            int idx = (start + i) % REEL_STRIP_LENGTH;
            slot->symbols[r][i] = reel_strip[idx];
        }
        slot->offset_y[r] = 0.0f;
    }
}

static void GenerateResult(SlotReelsState *slot)
{
    for (int r = 0; r < REELS_COUNT; r++) {
        int stop_pos = rand() % REEL_STRIP_LENGTH;

        slot->final_grid[r][0] = GetSymbolFromStrip(stop_pos);           // Top
        slot->final_grid[r][1] = GetSymbolFromStrip(stop_pos + 1);       // Middle
        slot->final_grid[r][2] = GetSymbolFromStrip(stop_pos + 2);       // Bottom

        slot->target_offset[r] = -SYMBOL_SIZE;

        for (int i = 0; i < VISIBLE_SYMBOLS + 2; i++) {
            int idx = stop_pos + i - 1;
            if (idx < 0) idx += REEL_STRIP_LENGTH;
            slot->symbols[r][i] = GetSymbolFromStrip(idx);
        }
    }
}

static PokerHand EvaluateLine(const Rank grid[REELS_COUNT])
{
    Rank ranks[5];
    for (int i = 0; i < 5; i++) ranks[i] = grid[i];

    Rank sorted[5];
    memcpy(sorted, ranks, sizeof(ranks));
    for (int i = 0; i < 4; i++)
        for (int j = i + 1; j < 5; j++)
            if (sorted[i] > sorted[j]) {
                Rank t = sorted[i];
                sorted[i] = sorted[j];
                sorted[j] = t;
            }

    int count[13] = {0}; // RANK_2 to RANK_ACE
    for (int i = 0; i < 5; i++)
        if (ranks[i] >= RANK_2 && ranks[i] <= RANK_ACE)
            count[ranks[i]]++;

    int pairs = 0, threes = 0, fours = 0;
    for (int i = 0; i < 13; i++) {
        if (count[i] == 4) fours = 1;
        else if (count[i] == 3) threes = 1;
        else if (count[i] == 2) pairs++;
    }

    bool straight = (sorted[4] - sorted[0] == 4);
    bool royal = (sorted[0] == RANK_10 && sorted[4] == RANK_ACE);

    if (fours) return HAND_FOUR_OF_A_KIND;
    if (threes && pairs) return HAND_FULL_HOUSE;
    if (royal && straight) return HAND_ROYAL_FLUSH;
    if (straight) return HAND_STRAIGHT;
    if (threes) return HAND_THREE_OF_A_KIND;
    if (pairs == 2) return HAND_TWO_PAIR;
    if (pairs == 1) {
        for (int r = RANK_JACK; r <= RANK_ACE; r++)
            if (count[r] == 2) return HAND_JACKS_OR_BETTER;
    }
    return HAND_HIGH_CARD;
}

static int GetPayoutMultiplier(PokerHand hand)
{
    switch (hand) {
        case HAND_ROYAL_FLUSH:     return 800;
        case HAND_STRAIGHT_FLUSH:  return 50;
        case HAND_FOUR_OF_A_KIND:  return 25;
        case HAND_FULL_HOUSE:      return 9;
        case HAND_FLUSH:           return 6;
        case HAND_STRAIGHT:        return 4;
        case HAND_THREE_OF_A_KIND: return 3;
        case HAND_TWO_PAIR:        return 2;
        case HAND_JACKS_OR_BETTER: return 1;
        case HAND_HIGH_CARD:       return 0;
        default:                   return 0;
    }
}

static int GetActivePaylines(int bet_level)
{
    if (bet_level == BET_3_TOKENS) return 3;
    if (bet_level == BET_2_TOKENS) return 2;
    return 1;
}

void UpdateSlotReels(LobbyState *core, SlotReelsState *slot)
{
    Account *player = (core->p1_account_index >= 0) ? &core->accounts[core->p1_account_index] : NULL;
    int gamepad = GetActiveGamepad();

    if (slot->state == SLOT_STATE_IDLE || slot->state == SLOT_STATE_SHOW_WIN) {
        // Bet selection
        if (IsKeyPressed(KEY_ONE)) slot->bet_level = BET_1_TOKENS;
        if (IsKeyPressed(KEY_TWO)) slot->bet_level = BET_2_TOKENS;
        if (IsKeyPressed(KEY_THREE)) slot->bet_level = BET_3_TOKENS;

        // Start spin
        if ((IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) ||
             (gamepad >= 0 && IsGamepadButtonPressedSDL(gamepad, 0))) &&
            player && player->tokens >= slot->bet_level) {

            player->tokens -= slot->bet_level;
            player->stats.slots_spins++;

            slot->total_win_this_spin = 0;
            memset(slot->win_lines, 0, sizeof(slot->win_lines));
            slot->state = SLOT_STATE_SPINNING;
            slot->spin_timer = 0.0f;

            for (int r = 0; r < REELS_COUNT; r++) {
                slot->stopped[r] = false;
                slot->velocity[r] = 800.0f + (float)r * 100.0f;
                slot->offset_y[r] = 0.0f;
            }

            GenerateResult(slot);
            PlaySound(g_coin_sound);
        }

        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_B)) {
            SwitchState(core, STATE_LOBBY);
        }

        if (slot->state == SLOT_STATE_SHOW_WIN) {
            slot->win_timer -= GetFrameTime();
            if (slot->win_timer <= 0.0f) slot->state = SLOT_STATE_IDLE;
        }
    }

    if (slot->state == SLOT_STATE_SPINNING) {
        slot->spin_timer += GetFrameTime();
        bool all_stopped = true;

        for (int r = 0; r < REELS_COUNT; r++) {
            if (!slot->stopped[r]) {
                float stop_time = (float)r * STAGGER_DELAY + 0.8f;
                if (slot->spin_timer >= stop_time) {
                    slot->stopped[r] = true;
                } else {
                    slot->offset_y[r] -= slot->velocity[r] * GetFrameTime();
                    while (slot->offset_y[r] < -SYMBOL_SIZE * 4)
                        slot->offset_y[r] += SYMBOL_SIZE * 4;
                    all_stopped = false;
                }
            } else {
                slot->offset_y[r] += (slot->target_offset[r] - slot->offset_y[r]) * 0.2f;
                if (fabsf(slot->offset_y[r] - slot->target_offset[r]) < 1.0f)
                    slot->offset_y[r] = slot->target_offset[r];
            }
        }

        if (all_stopped) {
            double total_win = 0;
            int lines = GetActivePaylines(slot->bet_level);

            for (int line = 0; line < lines; line++) {
                PokerHand hand = EvaluateLine(slot->final_grid[line]);
                int mult = GetPayoutMultiplier(hand);
                if (mult > 0) {
                    slot->win_lines[line] = true;
                    total_win += (double)mult * slot->bet_level;
                }
            }

            slot->total_win_this_spin = total_win;
            if (player) {
                player->tokens += total_win;
                if (total_win > 0) {
                    player->stats.slots_wins++;
                    UpdateGameStats(core, core->p1_account_index, GAME_SLOT_REELS, total_win);
                    PlaySound(g_win_sound);
                    ShowNotification(core, "WIN!", TextFormat("%.0f TOKENS!", total_win));
                }
                SaveAllAccounts(core);
            }

            slot->state = SLOT_STATE_SHOW_WIN;
            slot->win_timer = 4.0f;
        }
    }
}

void DrawSlotReels(const LobbyState *core, const SlotReelsState *slot)
{
    const Account *player = (core->p1_account_index >= 0) ? &core->accounts[core->p1_account_index] : NULL;

    DrawRectangle(0, 0, (int)SCREEN_W, (int)SCREEN_H, Fade(BLACK, 0.75f));

    float mx = REEL_START_X - 60;
    float my = REEL_START_Y - 60;
    float mw = REELS_COUNT * REEL_WIDTH + (REELS_COUNT - 1) * REEL_GAP + 120;
    float mh = VISIBLE_SYMBOLS * SYMBOL_SIZE + 120;

    DrawRectangleRounded((Rectangle){mx, my, mw, mh}, 0.05f, 10, Fade(DARKGRAY, 0.9f));
    DrawRectangleLinesEx((Rectangle){mx, my, mw, mh}, 12, GOLD);

    for (int r = 0; r < REELS_COUNT; r++) {
        float reel_x = REEL_START_X + (float)r * (REEL_WIDTH + REEL_GAP);
        for (int s = -1; s <= VISIBLE_SYMBOLS; s++) {
            float y = REEL_START_Y + (float)s * SYMBOL_SIZE + slot->offset_y[r];
            int idx = s + 1;
            Rank rank = slot->symbols[r][idx];

            Rectangle src = GetAtlasSourceRect(rank, SUIT_HEARTS);
            Rectangle dest = { reel_x, y, REEL_WIDTH, SYMBOL_SIZE };
            DrawTexturePro(g_atlas_texture, src, dest, (Vector2){0, 0}, 0.0f, WHITE);
        }
    }

    if (slot->state == SLOT_STATE_SHOW_WIN) {
        for (int line = 0; line < 3; line++) {
            if (slot->win_lines[line]) {
                float ly = REEL_START_Y + (float)line * SYMBOL_SIZE + SYMBOL_SIZE / 2.0f;
                DrawRectangle(0, (int)(ly - 15), (int)SCREEN_W, 30, Fade(YELLOW, 0.4f));
                DrawText("WIN!", (int)CENTER_X - 100, (int)(ly - 30), 50, YELLOW);
            }
        }
    }

    DrawText("JACKS OR BETTER", (int)(CENTER_X - 280), 60, 60, GOLD);
    DrawText(TextFormat("TOKENS: %.0f", player ? player->tokens : 0.0), 50, 60, 40, WHITE);
    DrawText("BET: 1   2   3", (int)CENTER_X - 180, (int)SCREEN_H - 160, 40, WHITE);
    DrawText(TextFormat("CURRENT BET: %d", slot->bet_level), (int)CENTER_X - 160, (int)SCREEN_H - 110, 36, LIME);

    if (slot->total_win_this_spin > 0) {
        DrawText(TextFormat("YOU WON %.0f TOKENS!", slot->total_win_this_spin),
                 (int)CENTER_X - 320, (int)SCREEN_H / 2 - 50, 60, GOLD);
    }

    DrawText("SPACE / A = SPIN    ESC/B = BACK", 50, (int)SCREEN_H - 60, 30, LIGHTGRAY);
}