#include "slotreels.h"
#include "main.h"
#include "mainmenu.h"
#include "useraccount.h"
#include "jokersgambit.h"
#include <math.h>
// Button layout constants
#define BTN_WIDTH 120.0f
#define BTN_HEIGHT 60.0f
#define BTN_SPACING 15.0f
#define SPIN_BTN_WIDTH 200.0f
#define SPIN_BTN_HEIGHT 100.0f
#define P1_UI_X 10
#define HOLD_BTN_HEIGHT 40.0f
// Button positions
#define BET_BTN_START_X (SCREEN_W * 0.05f)
#define BET_BTN_START_Y (SCREEN_H * 0.5f)
#define LINE_BTN_START_X (SCREEN_W - BET_BTN_START_X - BTN_WIDTH)
#define LINE_BTN_START_Y BET_BTN_START_Y
#define SPIN_BTN_X (CENTER_X - SPIN_BTN_WIDTH / 2.0f)
#define SPIN_BTN_Y (SCREEN_H - SPIN_BTN_HEIGHT - 50.0f)
#define MAX_DECK_SIZE 52
#define MAX_GAMBLE_STEPS 5
float ui_x = 20.0f;
float ui_y = 20.0f;
float ui_w = 560.0f;
float ui_h = 300.0f;
// ============================================================================
// DECK MANAGEMENT
// ============================================================================
static int BuildDeck(Symbol deck[MAX_DECK_SIZE])
{
    int idx = 0;
    // Add 52 standard cards: 4 suits x 13 ranks
    for (int suit = 0; suit < 4; suit++)
    {
        for (int rank = 0; rank < 13; rank++)
        {
            deck[idx].rank = (Rank)rank;
            deck[idx].suit = (Suit)suit;
            idx++;
        }
    }
    return idx; // FIXED: Return 52, not 0!
}
static void ShuffleDeck(Symbol deck[MAX_DECK_SIZE], int deck_size)
{
    for (int i = deck_size - 1; i > 0; i--)
    {
        size_t j = ((size_t)rand() % ((size_t)i + 1U));
        Symbol temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
}
// ============================================================================
// RESULT GENERATION
// ============================================================================
static void GenerateResult(SlotReelsState *slot)
{
    Symbol deck[MAX_DECK_SIZE];
    int deck_size = BuildDeck(deck);
    ShuffleDeck(deck, deck_size);
    int deck_idx = 0;
    for (int r = 0; r < REELS_COUNT; r++)
    {
        if (!slot->hold_reel[r])
        {
            // Deal cards
            slot->final_grid[0][r] = deck[deck_idx % deck_size];
            deck_idx++; // Top
            slot->final_grid[1][r] = deck[deck_idx % deck_size];
            deck_idx++; // Middle
            slot->final_grid[2][r] = deck[deck_idx % deck_size];
            deck_idx++; // Bottom
            // FIXED: Match the drawing loop's indexing!
            // Drawing shows: symbols[2]=top, symbols[3]=middle, symbols[4]=bottom
            slot->symbols[r][2] = slot->final_grid[0][r]; // Top (s=0 → idx=2)
            slot->symbols[r][3] = slot->final_grid[1][r]; // Middle (s=1 → idx=3)
            slot->symbols[r][4] = slot->final_grid[2][r]; // Bottom (s=2 → idx=4)
            // Fill buffers above and below
            slot->symbols[r][0] = deck[deck_idx % deck_size];
            deck_idx++;
            slot->symbols[r][1] = deck[deck_idx % deck_size];
            deck_idx++;
        }
        slot->target_offset[r] = -SYMBOL_SIZE;
    }
}
// ============================================================================
// POKER HAND EVALUATION
// ============================================================================
static bool CheckFlush(const Symbol hand[REELS_COUNT])
{
    int suit_counts[4] = {0};
    for (int i = 0; i < REELS_COUNT; i++)
    {
        if (hand[i].suit < 4)
        {
            suit_counts[hand[i].suit]++;
        }
    }
    // Check if any suit has all 5 cards
    for (int s = 0; s < 4; s++)
    {
        if (suit_counts[s] >= 5)
            return true;
    }
    return false;
}
static bool CheckStraight(const Symbol hand[REELS_COUNT])
{
    bool has_rank[13] = {false};
    for (int i = 0; i < REELS_COUNT; i++)
    {
        if (hand[i].suit < 4 && hand[i].rank >= RANK_2 && hand[i].rank <= RANK_ACE)
        {
            has_rank[hand[i].rank - RANK_2] = true;
        }
    }
    // Check for 5 consecutive ranks - FIXED: Should be <= 8, not <= 13-5
    for (int i = 0; i <= 8; i++) // 0-8 allows checking sequences starting from 2 through 10
    {
        if (has_rank[i] && has_rank[i + 1] && has_rank[i + 2] && has_rank[i + 3] && has_rank[i + 4])
            return true;
    }
    // Special Case: Ace-Low Straight (A, 2, 3, 4, 5)
    if (has_rank[12] && has_rank[0] && has_rank[1] && has_rank[2] && has_rank[3])
        return true;
    return false;
}
static PokerHand EvaluateLine(const Symbol hand[REELS_COUNT])
{
    int rank_freq[13] = {0};
    int max_rank_count = 0;
    for (int i = 0; i < REELS_COUNT; i++)
    {
        if (hand[i].suit < 4)
        {
            int r = (int)hand[i].rank;
            if (r >= 0 && r < 13)
            {
                rank_freq[r]++;
                if (rank_freq[r] > max_rank_count)
                    max_rank_count = rank_freq[r];
            }
        }
    }
    // Pattern detection
    int pairs = 0, threes = 0, fours = 0;
    for (int i = 0; i < 13; i++)
    {
        if (rank_freq[i] == 4)
            fours++;
        else if (rank_freq[i] == 3)
            threes++;
        else if (rank_freq[i] == 2)
            pairs++;
    }
    int best_set = max_rank_count;
    bool is_flush = CheckFlush(hand);
    bool is_straight = CheckStraight(hand);
    // Return hands in descending order
    if (is_straight && is_flush)
        return HAND_STRAIGHT_FLUSH;
    if (best_set >= 4)
        return HAND_FOUR_OF_A_KIND;
    if (threes && pairs)
        return HAND_FULL_HOUSE;
    if (is_flush)
        return HAND_FLUSH;
    if (is_straight)
        return HAND_STRAIGHT;
    if (best_set >= 3)
        return HAND_THREE_OF_A_KIND;
    if (pairs >= 2)
        return HAND_TWO_PAIR;
    // NEW: Check for Jacks or Better (pair of J, Q, K, or A)
    // Ranks: 9=J, 10=Q, 11=K, 12=A
    for (int i = 9; i <= 12; i++)
    {
        if (rank_freq[i] >= 2)
            return HAND_JACKS_OR_BETTER;
    }
    return HAND_HIGH_CARD;
}
// ============================================================================
// PAYOUT TABLE
// ============================================================================
static int GetPayoutMultiplier(PokerHand hand)
{
    switch (hand)
    {
    case HAND_ROYAL_FLUSH:
        return 800;
    case HAND_STRAIGHT_FLUSH:
        return 50;
    case HAND_FOUR_OF_A_KIND:
        return 25;
    case HAND_FULL_HOUSE:
        return 9;
    case HAND_FLUSH:
        return 6;
    case HAND_STRAIGHT:
        return 4;
    case HAND_THREE_OF_A_KIND:
        return 3;
    case HAND_TWO_PAIR:
        return 2;
    case HAND_JACKS_OR_BETTER:
        return 1;
    case HAND_HIGH_CARD:
    default:
        return 0;
    }
}
// ============================================================================
// ACHIEVEMENT TRACKING
// ============================================================================
static void TrackHandAchievements(Account *acc, PokerHand hand)
{
    if (!acc || acc->is_ai)
        return;
    switch (hand)
    {
    case HAND_FLUSH:
        acc->stats.cs_had_flush = true;
        break;
    case HAND_FOUR_OF_A_KIND:
        acc->stats.cs_had_four_kind = true;
        break;
    case HAND_FULL_HOUSE:
        acc->stats.cs_had_full_house = true;
        break;
    case HAND_STRAIGHT_FLUSH:
        acc->stats.cs_had_straight_flush = true;
        break;
    case HAND_ROYAL_FLUSH:
        acc->stats.cs_had_royal_flush = true;
        break;
    case HAND_HIGH_CARD:
    case HAND_JACKS_OR_BETTER:
    case HAND_TWO_PAIR:
    case HAND_THREE_OF_A_KIND:
    case HAND_STRAIGHT:
        // No achievement tracking for these hands
        break;
    default:
        break;
    }
}
// ============================================================================
// UI HELPERS
// ============================================================================
static int GetActivePaylineCount(PaylineSelection mode)
{
    switch (mode)
    {
    case PAYLINE_TOP:
    case PAYLINE_MIDDLE:
    case PAYLINE_BOTTOM:
        return 1;
    case PAYLINE_ALL:
        return 3;
    default:
        break;
    }
    return 1;
}
static Rectangle GetBetButton(int idx)
{
    return (Rectangle){
        BET_BTN_START_X,
        BET_BTN_START_Y + (float)idx * (BTN_HEIGHT + BTN_SPACING),
        BTN_WIDTH,
        BTN_HEIGHT};
}
static Rectangle GetPaylineButton(int idx)
{
    return (Rectangle){
        LINE_BTN_START_X,
        LINE_BTN_START_Y + (float)idx * (BTN_HEIGHT + BTN_SPACING),
        BTN_WIDTH,
        BTN_HEIGHT};
}
typedef struct
{
    float btn_width;
    float btn_height;
    float gap;
    float center_x;
    float spin_y;
    float gamble_y;
} ButtonConfig;

static ButtonConfig GetButtonConfig(void)
{
    return (ButtonConfig){
        .btn_width = 250.0f,
        .btn_height = 80.0f,
        .gap = 30.0f,
        .center_x = (float)GetScreenWidth() / 2.0f,
        .spin_y = REEL_START_Y + (VISIBLE_SYMBOLS * SYMBOL_SIZE) + 100.0f,  // below reels
        .gamble_y = REEL_START_Y + (VISIBLE_SYMBOLS * SYMBOL_SIZE) + 200.0f // below spin
    };
}


static Rectangle GetSpinButton(void)
{
    ButtonConfig cfg = GetButtonConfig();
    // Right of rightmost hold button (REELS_COUNT=5, REEL_GAP=10ish)
    float hold_end_x = REEL_START_X + (5 * (REEL_WIDTH + REEL_GAP)) + 20.0f;
    return (Rectangle){hold_end_x, cfg.spin_y, cfg.btn_width, cfg.btn_height};
}

static Rectangle GetGambleButton(void)
{
    ButtonConfig cfg = GetButtonConfig();
    float hold_end_x = REEL_START_X + (5 * (REEL_WIDTH + REEL_GAP)) + 20.0f;
    return (Rectangle){hold_end_x, cfg.gamble_y, cfg.btn_width, cfg.btn_height};
}

static Rectangle GetHoldButton(int reel_idx)
{
    float reel_x = REEL_START_X + (float)reel_idx * (REEL_WIDTH + REEL_GAP);
    float hold_y = REEL_START_Y + (VISIBLE_SYMBOLS * SYMBOL_SIZE) + 10.0f;
    return (Rectangle){reel_x, hold_y, REEL_WIDTH, HOLD_BTN_HEIGHT};
}
// Add after GetGambleButton()
static Rectangle ApplyWaveEffect(Rectangle base, float time, float phase)
{
    float amplitude = 3.0f; // pixels up/down
    float speed = 2.5f;
    float wave = sinf(time * speed + phase) * amplitude;

    Rectangle waved = base;
    waved.y += wave;
    return waved;
}

// ============================================================================
// GAMBLE FEATURE
// ============================================================================
static Symbol GetRandomGambleCard(void)
{
    Symbol s;
    s.rank = (Rank)(rand() % 13); // 2 through Ace
    s.suit = (Suit)(rand() % 4);
    return s;
}
static void InitGamble(SlotReelsState *slot)
{
    slot->gamble_current = slot->total_win_this_spin;
    slot->gamble_can_gamble = true;
    slot->show_gamble_result = false;
    slot->gamble_steps = 0;
    // Generate 6 cards for the gamble sequence
    slot->gamble_card = GetRandomGambleCard(); // First card (face up)
    for (int i = 0; i < 5; i++)
    {
        slot->gamble_deck[i] = GetRandomGambleCard();
    }
}
static void ResolveGamble(SlotReelsState *slot, bool guess_high)
{
    if (slot->gamble_steps >= 5)
    {
        slot->gamble_can_gamble = false;
        return;
    }
    Symbol next = slot->gamble_deck[slot->gamble_steps];
    bool next_is_higher = (next.rank > slot->gamble_card.rank);
    bool win = (guess_high == next_is_higher);
    slot->gamble_next_card = next;
    slot->show_gamble_result = true;
    if (win)
    {
        slot->gamble_current *= 2.0;
        slot->gamble_card = next;
        slot->gamble_steps++;
        PlaySound(g_filled_rank_sound);
        if (slot->gamble_steps >= MAX_GAMBLE_STEPS)
        {
            slot->gamble_can_gamble = false;
        }
    }
    else
    {
        // FIXED: Player loses everything on wrong guess
        slot->gamble_current = 0.0;
        slot->gamble_can_gamble = false;
        PlaySound(g_discard_sound);
    }
}
// ============================================================================
// UPDATE LOGIC
// ============================================================================
static void CleanupGambleState(SlotReelsState *slot)
{
    memset(slot->hold_reel, 0, sizeof(slot->hold_reel));
    // Clear win display
    memset(slot->win_lines, 0, sizeof(slot->win_lines));
    slot->total_win_this_spin = 0;
    slot->win_timer = 0.0f;
    // Clear gamble state completely
    slot->gamble_current = 0;
    slot->gamble_can_gamble = false;
    slot->show_gamble_result = false;
    slot->gamble_steps = 0;
}
// ============================================================================
// GAME INITIALIZATION
// ============================================================================
void InitSlotReels(SlotReelsState *slot)
{
    memset(slot, 0, sizeof(SlotReelsState));
    slot->bet_amount = BET_AMOUNT_1;
    slot->payline_mode = PAYLINE_MIDDLE;
    slot->bet_level = 1;
    slot->state = SLOT_STATE_INSERT_TOKENS; // FORCE new state
    slot->has_inserted_tokens = false;      // Explicit flag
    slot->selected_button = -1;             // Disable selection
    Symbol init_deck[MAX_DECK_SIZE];
    int deck_size = BuildDeck(init_deck); // Now returns 52
    ShuffleDeck(init_deck, deck_size);
    int deck_idx = 0;
    for (int r = 0; r < REELS_COUNT; r++)
    {
        slot->hold_reel[r] = false;
        slot->hold_locked[r] = false; // NEW: Track if reel was held last spin
        for (int i = 0; i < VISIBLE_SYMBOLS + 2; i++)
        {
            slot->symbols[r][2] = init_deck[deck_idx % deck_size];
            deck_idx++;
        }
        slot->offset_y[r] = 0.0f;
    }
}
void UpdateSlotReels(LobbyState *core, SlotReelsState *slot)
{
    Account *acc = &core->accounts[core->p1_account_index];
    Account *player = (core->p1_account_index >= 0) ? &core->accounts[core->p1_account_index] : NULL;
    int gamepad = GetActiveGamepad();
    Vector2 mouse = GetMousePosition();
    if (slot->state == SLOT_STATE_INSERT_TOKENS)
    {
        Rectangle insert_btn = {CENTER_X - 200.0f, SCREEN_H * 0.6f, 400.0f, 100.0f};
        bool mouse_over = CheckCollisionPointRec(GetMousePosition(), insert_btn);
        bool mouse_press = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
        if ((acc->tokens >= 1.0 && mouse_over && mouse_press) || (acc->tokens >= 1.0 && XboxBtnPressed(gamepad, 0)))
        {
            acc->tokens -= 1.0;
            slot->has_inserted_tokens = true;
            slot->bet_amount = BET_AMOUNT_1;
            slot->payline_mode = PAYLINE_MIDDLE;
            slot->bet_level = 1;
            slot->selected_button = 7;
            PlaySound(g_spin_sound);
            slot->state = SLOT_STATE_SPINNING;
            slot->spin_timer = 0.0f;
            for (int r = 0; r < REELS_COUNT; r++)
            {
                slot->stopped[r] = false;
                slot->velocity[r] = 800.0f + (float)r * 100.0f;
                slot->offset_y[r] = 0.0f;
            }
            GenerateResult(slot);
            for (int r = 0; r < REELS_COUNT; r++)
            {
                slot->hold_locked[r] = slot->hold_reel[r];
                slot->hold_reel[r] = false;
            }
        }
        // Exit: B button OR Backspace
        bool back_pressed = IsKeyPressed(KEY_BACKSPACE) || XboxBtnPressed(gamepad, 1);
        if (back_pressed)
        {
            slot->state = SLOT_STATE_INSERT_TOKENS;
            SwitchState(core, STATE_LOBBY);
            slot->has_inserted_tokens = false; // Explicit flag
            return;
        }
        DrawSlotReels(core, slot);
        return;
    }
    // Handle hold button clicks when idle
    if (slot->state == SLOT_STATE_IDLE)
    {
        for (int r = 0; r < REELS_COUNT; r++)
        {
            Rectangle hold_rect = GetHoldButton(r);
            if (CheckCollisionPointRec(mouse, hold_rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                if (!slot->hold_locked[r])
                {
                    slot->hold_reel[r] = !slot->hold_reel[r];
                    PlaySound(g_place_sound);
                }
                else
                {
                    ShowNotification(core, "REEL LOCKED", "Cannot hold same reel twice in a row");
                }
                break;
            }
        }
    }
    // FIXED: Decrement win timer to hide green lines after 5 seconds
    if (slot->state == SLOT_STATE_SHOW_WIN)
    {
        if (slot->win_timer > 0.0f)
        {
            slot->win_timer -= GetFrameTime();
            if (slot->win_timer <= 0.0f)
            {
                memset(slot->win_lines, 0, sizeof(slot->win_lines));
            }
        }
    }
    if (slot->state == SLOT_STATE_IDLE || slot->state == SLOT_STATE_SHOW_WIN)
    {
        // Shoulder buttons with line lock
        if ((XboxBtnPressed(gamepad, 4)) || (XboxBtnPressed(gamepad, 5)))
        {
            for (int r = 0; r < REELS_COUNT; r++)
            {
                slot->hold_reel[r] = false;
                slot->hold_locked[r] = true; // NEW: Track if reel was held last spin
                slot->offset_y[r] = 0.0f;
            }
        }
        if (XboxBtnPressed(gamepad, 4))
        {
            slot->bet_amount = (slot->bet_amount == BET_AMOUNT_1) ? BET_AMOUNT_5 : (slot->bet_amount == BET_AMOUNT_5) ? BET_AMOUNT_10
                                                                                                                      : BET_AMOUNT_1;
            PlaySound(g_coin_sound);
        }
        if (XboxBtnPressed(gamepad, 5))
        {
            slot->payline_mode = (PaylineSelection)((slot->payline_mode + 1) % 4);
            PlaySound(g_place_sound);
        }
        // D-pad hold controls
        static int selected_hold = -1;
        if (XboxBtnPressed(gamepad, 13))
        {
            if (selected_hold > 0)
                selected_hold--;
            else
                selected_hold = REELS_COUNT - 1;
        }
        if (XboxBtnPressed(gamepad, 14))
        {
            if (selected_hold < REELS_COUNT - 1)
                selected_hold++;
            else
                selected_hold = 0;
        }
        if (XboxBtnPressed(gamepad, 0))
        {
            if (selected_hold >= 0 && selected_hold < REELS_COUNT)
            {
                if (!slot->hold_locked[selected_hold])
                {
                    slot->hold_reel[selected_hold] = !slot->hold_reel[selected_hold];
                    PlaySound(g_place_sound);
                }
                else
                {
                    ShowNotification(core, "REEL LOCKED", "Cannot hold same reel twice in a row");
                }
            }
        }
        // X button: Spin
        if (XboxBtnPressed(gamepad, 2))
        {
            int payline_count = GetActivePaylineCount(slot->payline_mode);
            double total_bet = (double)slot->bet_amount * payline_count;
            if (player && player->tokens >= total_bet)
            {
                player->tokens -= total_bet;
                player->stats.slots_spins++;
                slot->total_win_this_spin = 0;
                memset(slot->win_lines, 0, sizeof(slot->win_lines));
                slot->state = SLOT_STATE_SPINNING;
                slot->spin_timer = 0.0f;
                for (int r = 0; r < REELS_COUNT; r++)
                {
                    slot->stopped[r] = false;
                    slot->velocity[r] = 800.0f + (float)r * 100.0f;
                    slot->offset_y[r] = 0.0f;
                }
                GenerateResult(slot);
                for (int r = 0; r < REELS_COUNT; r++)
                {
                    slot->hold_locked[r] = slot->hold_reel[r];
                    slot->hold_reel[r] = false;
                }
                PlaySound(g_place_sound);
                PlaySound(g_spin_sound);
            }
            else if (player)
            {
                ShowNotification(core, "INSUFFICIENT TOKENS",
                                 TextFormat("Need %.0f tokens to spin", total_bet));
            }
        }
        // Y button: Gamble
        if (slot->state == SLOT_STATE_SHOW_WIN && XboxBtnPressed(gamepad, 3))
        {
            if (slot->total_win_this_spin > 0)
            {
                InitGamble(slot);
                slot->state = SLOT_STATE_GAMBLE;
                PlaySound(g_coin_sound);
            }
        }
        if (IsKeyPressed(KEY_G) && slot->state == SLOT_STATE_SHOW_WIN && slot->total_win_this_spin > 0)
        {
            InitGamble(slot);
            slot->state = SLOT_STATE_GAMBLE;
            PlaySound(g_coin_sound);
        }
        // FIXED: Exit with proper cleanup
        if (IsKeyPressed(KEY_B) || XboxBtnPressed(gamepad, 1))
        {
            if (slot->state == SLOT_STATE_SHOW_WIN || slot->state == SLOT_STATE_IDLE)
            {
                if (player)
                {
                    CheckAchievements(player, core);
                    SaveAllAccounts(core);
                }
                slot->state = SLOT_STATE_INSERT_TOKENS;
                SwitchState(core, STATE_LOBBY);
                slot->has_inserted_tokens = false; // Explicit flag
                CleanupGambleState(slot);
            }
        }
    }
    if (slot->state == SLOT_STATE_SPINNING)
    {
        slot->spin_timer += GetFrameTime();
        bool all_stopped = true;
        for (int r = 0; r < REELS_COUNT; r++)
        {
            float stop_time = (float)r * STAGGER_DELAY + 0.5f;
            if (!slot->stopped[r])
            {
                slot->offset_y[r] += slot->velocity[r] * GetFrameTime();
                if (slot->offset_y[r] >= SYMBOL_SIZE)
                    slot->offset_y[r] -= SYMBOL_SIZE;
                if (slot->spin_timer >= stop_time)
                    slot->stopped[r] = true;
                all_stopped = false;
            }
            else
            {
                float target = 0.0f;
                float diff = target - slot->offset_y[r];
                slot->offset_y[r] += diff * 0.15f;
                if (fabsf(diff) > 0.1f)
                {
                    all_stopped = false;
                }
                else
                {
                    slot->offset_y[r] = target;
                }
            }
        }
        if (all_stopped)
        {
            double total_win = 0;
            switch (slot->payline_mode)
            {
            case PAYLINE_TOP:
            {
                PokerHand hand = EvaluateLine(slot->final_grid[0]);
                int mult = GetPayoutMultiplier(hand);
                if (mult > 0)
                {
                    slot->win_lines[0] = true;
                    total_win += (double)mult * slot->bet_amount;
                    if (player)
                        TrackHandAchievements(player, hand);
                }
                break;
            }
            case PAYLINE_MIDDLE:
            {
                PokerHand hand = EvaluateLine(slot->final_grid[1]);
                int mult = GetPayoutMultiplier(hand);
                if (mult > 0)
                {
                    slot->win_lines[1] = true;
                    total_win += (double)mult * slot->bet_amount;
                    if (player)
                        TrackHandAchievements(player, hand);
                }
                break;
            }
            case PAYLINE_BOTTOM:
            {
                PokerHand hand = EvaluateLine(slot->final_grid[2]);
                int mult = GetPayoutMultiplier(hand);
                if (mult > 0)
                {
                    slot->win_lines[2] = true;
                    total_win += (double)mult * slot->bet_amount;
                    if (player)
                        TrackHandAchievements(player, hand);
                }
                break;
            }
            case PAYLINE_ALL:
            {
                for (int line = 0; line < 3; line++)
                {
                    PokerHand hand = EvaluateLine(slot->final_grid[line]);
                    int mult = GetPayoutMultiplier(hand);
                    if (mult > 0)
                    {
                        slot->win_lines[line] = true;
                        total_win += (double)mult * slot->bet_amount;
                        if (player)
                            TrackHandAchievements(player, hand);
                    }
                }
                break;
            }
            default:
                break;
            }
            slot->total_win_this_spin = total_win;
            if (player)
            {
                player->tokens += total_win;
                if (total_win > 0)
                {
                    player->stats.slots_wins++;
                    UpdateGameStats(core, core->p1_account_index, GAME_SLOT_REELS, total_win);
                    CheckAchievements(player, core);
                    PlaySound(g_filled_rank_sound);
                    ShowNotification(core, "WIN!", TextFormat("%.0f TOKENS!", total_win));
                }
                SaveAllAccounts(core);
            }
            slot->state = SLOT_STATE_SHOW_WIN;
            slot->win_timer = 5.0f; // FIXED: Changed from 4.0f to 5.0f
        }
    }
    // FIXED: Gamble state with exploit prevention
    if (slot->state == SLOT_STATE_GAMBLE)
    {
        // Fold button - FIXED to prevent exploit
        if (IsKeyPressed(KEY_F) || XboxBtnPressed(gamepad, 1))
        {
            if (player && slot->gamble_current > 0)
            {
                player->tokens += slot->gamble_current;
                UpdateGameStats(core, core->p1_account_index, GAME_SLOT_REELS, slot->gamble_current);
                CheckAchievements(player, core);
                SaveAllAccounts(core);
                ShowNotification(core, "COLLECTED", TextFormat("%.0f TOKENS!", slot->gamble_current));
            }
            else if (player && slot->gamble_current <= 0)
            {
                ShowNotification(core, "FOLD", "Nothing to collect");
            }
            // CRITICAL: Clean up state before returning to idle
            CleanupGambleState(slot);
            slot->state = SLOT_STATE_IDLE;
            return;
        }
        if (slot->show_gamble_result)
        {
            if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) ||
                XboxBtnPressed(gamepad, 0) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                if (slot->gamble_current > 0 && slot->gamble_can_gamble)
                {
                    // Won and can continue gambling
                    slot->show_gamble_result = false;
                }
                else
                {
                    // Lost or maxed out
                    if (player && slot->gamble_current > 0)
                    {
                        // Won max or chose to stop
                        player->tokens += slot->gamble_current;
                        UpdateGameStats(core, core->p1_account_index, GAME_SLOT_REELS, slot->gamble_current);
                        CheckAchievements(player, core);
                        SaveAllAccounts(core);
                        ShowNotification(core, "MAX WIN!", TextFormat("%.0f TOKENS!", slot->gamble_current));
                    }
                    else if (player)
                    {
                        // Lost the gamble
                        ShowNotification(core, "GAMBLE LOST", "All winnings forfeited!");
                    }
                    // CRITICAL: Clean up before returning
                    CleanupGambleState(slot);
                    slot->state = SLOT_STATE_IDLE;
                }
            }
        }
        else
        {
            // High/Low choice
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_H) || XboxBtnPressed(gamepad, 11))
            {
                ResolveGamble(slot, true);
            }
            else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_L) || XboxBtnPressed(gamepad, 12))
            {
                ResolveGamble(slot, false);
            }
        }
    }
}
// ============================================================================
// DRAW LOGIC
// ============================================================================
void DrawSlotReels(const LobbyState *core, const SlotReelsState *slot)
{
    // Calculate layout dimensions
    const float TOTAL_REEL_WIDTH = (REELS_COUNT * REEL_WIDTH) + ((REELS_COUNT - 1) * REEL_GAP);
    const float START_X = CENTER_X - (TOTAL_REEL_WIDTH / 2.0f);
    const float CONTAINER_W = TOTAL_REEL_WIDTH + 40.0f;
    const float CONTAINER_H = (VISIBLE_SYMBOLS * SYMBOL_SIZE) + 40.0f;
    const float CONTAINER_X = CENTER_X - (CONTAINER_W / 2.0f);
    const float CONTAINER_Y = REEL_START_Y - 20.0f;
    const float CENTER_REEL_Y = REEL_START_Y + (VISIBLE_SYMBOLS * SYMBOL_SIZE) / 2.0f;
    // Draw background
    DrawRectangle((int)CONTAINER_X, (int)CONTAINER_Y, (int)CONTAINER_W, (int)CONTAINER_H, BLACK);
    // Draw reels (with scissor mode for clipping)
    BeginScissorMode((int)CONTAINER_X, (int)REEL_START_Y, (int)CONTAINER_W, (int)(VISIBLE_SYMBOLS * SYMBOL_SIZE));
    for (int r = 0; r < REELS_COUNT; r++)
    {
        float reel_x = START_X + (float)r * (REEL_WIDTH + REEL_GAP);
        for (int s = -2; s <= VISIBLE_SYMBOLS + 1; s++)
        {
            float rawY = REEL_START_Y + ((float)s * SYMBOL_SIZE) + slot->offset_y[r];
            int idx = (s + 2) % (VISIBLE_SYMBOLS + 2);
            // Curve effect
            float distFromCenter = (rawY + (SYMBOL_SIZE / 2.0f) - CENTER_REEL_Y) / (SYMBOL_SIZE * 1.5f);
            float curveOffset = (distFromCenter * distFromCenter * distFromCenter) * 12.0f;
            float displayY = rawY + curveOffset;
            float squash = 1.0f - (fabsf(distFromCenter) * 0.10f);
            float displayH = SYMBOL_SIZE * squash;
            float finalY = displayY + (SYMBOL_SIZE - displayH) / 2.0f;
            // Blur effect
            float blur = 0.0f;
            if (slot->state == SLOT_STATE_SPINNING && !slot->stopped[r])
            {
                blur = slot->velocity[r] * 0.02f;
            }
            Symbol sym = slot->symbols[r][idx];
            Rectangle src = GetAtlasSourceRect(sym.rank, sym.suit);
            Rectangle dest = {
                reel_x,
                finalY - (blur / 2.0f),
                REEL_WIDTH,
                displayH + blur};
            float tintVal = 1.0f - (distFromCenter * distFromCenter * 0.5f);
            float alphaFade = (blur > 0) ? 0.8f : 1.0f;
            Color tint = {
                (unsigned char)(255 * tintVal),
                (unsigned char)(255 * tintVal),
                (unsigned char)(255 * tintVal),
                (unsigned char)(255 * alphaFade)};
            DrawTexturePro(g_atlas_texture, src, dest, (Vector2){0, 0}, 0.0f, tint);
        }
    }
    EndScissorMode();
    // Draw paylines
    for (int i = 0; i < 3; i++)
    {
        float lineY = REEL_START_Y + ((float)i * SYMBOL_SIZE) + (SYMBOL_SIZE / 2.0f);
        Color lineColor = WHITE;
        float thick = 2.0f;
        if (slot->payline_mode == PAYLINE_ALL ||
            (slot->payline_mode == PAYLINE_TOP && i == 0) ||
            (slot->payline_mode == PAYLINE_MIDDLE && i == 1) ||
            (slot->payline_mode == PAYLINE_BOTTOM && i == 2))
        {
            lineColor = GOLD;
            thick = 2.2f;
        }
        // Only show green line if timer is still active
        if (slot->state == SLOT_STATE_SHOW_WIN && slot->win_lines[i] && slot->win_timer > 0.0f)
        {
            lineColor = GREEN;
            thick = 4.0f;
        }
        DrawLineEx((Vector2){CONTAINER_X, lineY},
                   (Vector2){CONTAINER_X + CONTAINER_W, lineY},
                   thick, lineColor);
    }
    DrawRectangleLinesEx((Rectangle){CONTAINER_X, CONTAINER_Y, CONTAINER_W, CONTAINER_H}, 4.0f, GOLD);
    // Draw hold buttons
    for (int r = 0; r < REELS_COUNT; r++)
    {
        Rectangle hold_rect = GetHoldButton(r);
        Color bg = slot->hold_reel[r] ? DARKGREEN : (slot->hold_locked[r] ? MAROON : DARKGRAY);
        Color border = slot->hold_reel[r] ? LIME : (slot->hold_locked[r] ? RED : LIME);
        DrawRectangleRec(hold_rect, bg);
        DrawRectangleLinesEx(hold_rect, 2.0f, border);
        const char *text = slot->hold_reel[r] ? "HELD" : (slot->hold_locked[r] ? "LOCKED" : "HOLD");
        int text_w = MeasureText(text, 14);
        DrawText(text,
                 (int)(hold_rect.x + (hold_rect.width - (float)text_w) / 2.0f),
                 (int)(hold_rect.y + (hold_rect.height - 14) / 2.0f),
                 14, WHITE);
    }
    if (slot->state == SLOT_STATE_INSERT_TOKENS)
    {
        const Account *acc = &core->accounts[core->p1_account_index];
        DrawRectangle(0, 0, (int)SCREEN_W, (int)SCREEN_H, Fade(BLACK, 0.7f));
        Rectangle btn = {CENTER_X - 200.0f, SCREEN_H * 0.6f, 400.0f, 100.0f};
        bool can_insert = (acc->tokens >= 1.0);
        Color btn_color = can_insert ? DARKGREEN : DARKGRAY;
        Color text_color = can_insert ? WHITE : GRAY;
        DrawRectangleRec(btn, btn_color);
        DrawRectangleLinesEx(btn, 6.0f, can_insert ? LIME : GRAY);
        const char *btn_text = "INSERT TOKENS (1)";
        // === FIXED: Safe centering with float math BEFORE cast ===
        int text_width = MeasureText(btn_text, 40);
        float center_offset = (float)text_width / 2.0f;
        float text_x_float = btn.x + btn.width / 2.0f - center_offset;
        float text_y_float = btn.y + 30.0f;
        DrawText(btn_text,
                 (int)text_x_float, // Safe: float → int after all math done
                 (int)text_y_float,
                 40,
                 text_color);
        // Controls hint — centered safely
        const char *hint_text = "MOUSE CLICK or X BUTTON";
        int hint_width = MeasureText(hint_text, 24);
        float hint_x = CENTER_X - (float)hint_width / 2.0f;
        DrawText(hint_text, (int)hint_x, (int)(btn.y + 120.0f), 24, GRAY);
        // Token display
        const char *token_text = TextFormat("Tokens: %.0f", acc->tokens);
        int token_width = MeasureText(token_text, 40);
        DrawText(token_text, (int)(CENTER_X - (float)token_width / 2.0f), (int)(btn.y - 100.0f), 40, GOLD);
        if (!can_insert)
        {
            DrawText("Not enough tokens!", (int)(CENTER_X - 180.0f), (int)(btn.y - 160.0f), 36, RED);
            DrawText("Go to Shop or play Joker's Gambit",
                     (int)(CENTER_X - 280.0f), (int)(btn.y + 160.0f), 28, LIGHTGRAY);
        }
        // Exit hint — compliant keybind (no ESCAPE)
        const char *exit_text = "B BUTTON or BACKSPACE to exit";
        int exit_width = MeasureText(exit_text, 28);
        DrawText(exit_text, (int)(CENTER_X - (float)exit_width / 2.0f), (int)(SCREEN_H - 100.0f), 28, GRAY);
        return;
    }
    // Player UI panel
    if (core->p1_account_index >= 0)
    {
        const Account *acc = &core->accounts[core->p1_account_index];
        const char *pName = GetPlayerNameByIndex(core, core->p1_account_index);
        const char *pStatus = GetMemberStatusString(acc->member_status);
        DrawRectangle((int)ui_x, (int)ui_y, (int)ui_w, (int)ui_h, Fade(BLACK, 0.9f));
        DrawRectangleLinesEx((Rectangle){ui_x, ui_y, ui_w, ui_h}, 2.0f, GOLD);
        DrawText(TextFormat("NAME: %s", pName), (int)ui_x + 15, (int)ui_y + 15, 25, WHITE);
        int status_w = MeasureText(pStatus, 18);
        DrawText(pStatus, (int)(ui_x + ui_w - (float)status_w - 15.0f), (int)(ui_y + 15.0f), 25, GOLD);
        DrawText(TextFormat("CREDITS: $%.2f", acc->credits), (int)ui_x + 15, (int)ui_y + 45, 24, LIME);
        DrawText(TextFormat("TOKENS: %.0f", acc->tokens), (int)ui_x + 15, (int)ui_y + 70, 24, YELLOW);
        DrawText(TextFormat("TOP LINE: \nMIDDLE LINE: \nBOTTOM LINE:"),
                 (int)ui_x + 15, (int)ui_y + 100, 18, RED);
        DrawText(TextFormat("TOTAL BET: "),
                 (int)ui_x + 15, (int)ui_y + 184, 18, GOLD);
        DrawText(TextFormat("TOTAL WIN: %.0f TOKENS", slot->total_win_this_spin),
                 (int)ui_x + 15, (int)ui_y + 202, 18, GOLD);
        DrawText("PRESS `Y` TO GAMBLE", (int)ui_x + 15, (int)ui_y + 220, 16, YELLOW);
        // xxx
        //  Show winning hand info
        if (slot->state == SLOT_STATE_SHOW_WIN && slot->total_win_this_spin > 0)
        {
            const char *hand_names[] = {
                "HIGH CARD", "JACKS OR BETTER", "TWO PAIR", "THREE OF A KIND",
                "STRAIGHT", "FLUSH", "FULL HOUSE", "FOUR OF A KIND",
                "STRAIGHT FLUSH", "ROYAL FLUSH"};
            for (int line = 0; line < 3; line++)
            {
                if (slot->win_lines[line])
                {
                    PokerHand h = EvaluateLine(slot->final_grid[line]);
                    DrawText(TextFormat(" %s", hand_names[h]),
                             (int)ui_x + 150, (int)ui_y + 100 + (line * 20), 18, LIME);
                }
            }
        }
    }
    // Draw bet buttons
    const char *bet_labels[] = {"1 TOKEN", "5 TOKENS", "10 TOKENS"};
    BetAmount bet_values[] = {BET_AMOUNT_1, BET_AMOUNT_5, BET_AMOUNT_10};
    DrawText("BET AMOUNT", (int)BET_BTN_START_X, (int)BET_BTN_START_Y - 40, 20, GOLD);
    for (int i = 0; i < 3; i++)
    {
        Rectangle btn = GetBetButton(i);
        bool selected = (slot->selected_button == i);
        bool active = (slot->bet_amount == bet_values[i]);
        Color bg = active ? DARKGREEN : (selected ? DARKBLUE : DARKGRAY);
        Color border = selected ? GOLD : (active ? LIME : GRAY);
        DrawRectangleRec(btn, bg);
        DrawRectangleLinesEx(btn, selected ? 4.0f : 2.0f, border);
        int text_w = MeasureText(bet_labels[i], 30);
        DrawText(bet_labels[i],
                 (int)(btn.x + (btn.width - (float)text_w) / 2.0f),
                 (int)(btn.y + (btn.height - 30) / 2.0f),
                 30, WHITE);
    }
    // Draw payline buttons
    const char *line_labels[] = {"TOP", "MIDDLE", "BOTTOM", "ALL"};
    PaylineSelection line_values[] = {PAYLINE_TOP, PAYLINE_MIDDLE, PAYLINE_BOTTOM, PAYLINE_ALL};
    DrawText("PAYLINES", (int)LINE_BTN_START_X, (int)LINE_BTN_START_Y - 40, 20, YELLOW);
    for (int i = 0; i < 4; i++)
    {
        Rectangle btn = GetPaylineButton(i);
        bool selected = (slot->selected_button == 3 + i);
        bool active = (slot->payline_mode == line_values[i]);
        Color bg = active ? DARKGREEN : (selected ? DARKBLUE : DARKGRAY);
        Color border = selected ? GOLD : (active ? LIME : GRAY);
        DrawRectangleRec(btn, bg);
        DrawRectangleLinesEx(btn, selected ? 4.0f : 2.0f, border);
        int text_w = MeasureText(line_labels[i], 18);
        DrawText(line_labels[i],
                 (int)(btn.x + (btn.width - (float)text_w) / 2.0f),
                 (int)(btn.y + (btn.height - 18) / 2.0f),
                 18, WHITE);
    }int gamepad = GetActiveGamepad();
float wave_time = (float)GetTime();

Rectangle spin_base = GetSpinButton();
Rectangle gamble_base = GetGambleButton();
Rectangle spin_btn = ApplyWaveEffect(spin_base, wave_time, 0.0f);     // phase 0
Rectangle gamble_btn = ApplyWaveEffect(gamble_base, wave_time, 2.0f); // phase

// Draw spin button & Gamble button
bool gamble_selected = (XboxBtnPressed(gamepad, 3));
Color gamble_bg = gamble_selected ? GRAY : BLACK;
Color gamble_border = gamble_selected ? GOLD : LIME;
DrawRectangleRec(gamble_btn, gamble_bg);  // ← USE gamble_btn (waved)
DrawRectangleLinesEx(gamble_btn, gamble_selected ? 5.0f : 3.0f, gamble_border);
const char *gamble_text = "GAMBLE";
int gamble_w = MeasureText(gamble_text, 50);
DrawText(gamble_text,
         (int)(gamble_btn.x + (gamble_btn.width - (float)gamble_w) / 2.0f),
         (int)(gamble_btn.y + (gamble_btn.height - 50) / 2.0f),
         50, YELLOW);

// SPIN BUTTON
bool spin_selected = (slot->selected_button == 7 || XboxBtnPressed(gamepad, 2));
Color spin_bg = spin_selected ? DARKBLUE : BLACK;
Color spin_border = spin_selected ? GOLD : LIME;
DrawRectangleRec(spin_btn, spin_bg);  // ← USE spin_btn (waved)
DrawRectangleLinesEx(spin_btn, spin_selected ? 5.0f : 3.0f, spin_border);
const char *spin_text = "SPIN";
int spin_w = MeasureText(spin_text, 50);
DrawText(spin_text,
         (int)(spin_btn.x + (spin_btn.width - (float)spin_w) / 2.0f),
         (int)(spin_btn.y + (spin_btn.height - 50) / 2.0f),
         50, YELLOW);

// Draw total bet info
int payline_count = GetActivePaylineCount(slot->payline_mode);
double total_bet = (double)slot->bet_amount * payline_count;
DrawText(TextFormat("TOTAL BET: %.0f TOKENS", total_bet),
         (int)ui_x + 15, (int)ui_y + 184, 18, GOLD);

    DrawText("TAB/D-Pad: Navigate | A/Enter: Select | B: Back",
             (int)(CENTER_X - 300.0f), (int)(SCREEN_H - 30), 20, DARKGRAY);
    // Draw gamble screen
    if (slot->state == SLOT_STATE_GAMBLE)
    {
        DrawRectangle(0, 0, (int)SCREEN_W, (int)SCREEN_H, Fade(BLACK, 0.85f));
        // Fold button (top left)
        Rectangle fold_btn = {20, 20, 120, 50};
        DrawRectangleRec(fold_btn, DARKGREEN);
        DrawRectangleLinesEx(fold_btn, 2.0f, LIME);
        DrawText("FOLD", 50, 32, 24, WHITE);
        DrawText(TextFormat("%.0f", slot->gamble_current), 30, 55, 16, YELLOW);
        // Title
        DrawText("DOUBLE OR NOTHING!", (int)(CENTER_X - 150.0f), 100, 40, GOLD);
        DrawText(TextFormat("WIN POT: %.0f TOKENS", slot->gamble_current),
                 (int)(CENTER_X - 140.0f), 150, 24, YELLOW);
        DrawText(TextFormat("STEP %d / %d", slot->gamble_steps + 1, MAX_GAMBLE_STEPS),
                 (int)(CENTER_X - 60.0f), 180, 20, DARKGRAY);
        // Card layout: 6 cards in a row
        float card_w = 140.0f;
        float card_h = 210.0f;
        float card_spacing = 20.0f;
        float total_w = (6 * card_w) + (5 * card_spacing);
        float start_x = CENTER_X - (total_w / 2.0f);
        float card_y = SCREEN_H / 2.0f - card_h / 2.0f;
        // Draw first card (face up)
        Rectangle cardSrc = GetAtlasSourceRect(slot->gamble_card.rank, slot->gamble_card.suit);
        Rectangle cardDest = {start_x, card_y, card_w, card_h};
        DrawTexturePro(g_atlas_texture, cardSrc, cardDest, (Vector2){0, 0}, 0.0f, WHITE);
        DrawRectangleLinesEx(cardDest, 3.0f, GOLD);
        DrawText("CURRENT", (int)(start_x + 30.0f), (int)(card_y - 30.0f), 16, WHITE);
        // Draw remaining 5 cards
        for (int i = 0; i < 5; i++)
        {
            float x = start_x + ((float)(i + 1) * (card_w + card_spacing));
            Rectangle dest = {x, card_y, card_w, card_h};
            if (i < slot->gamble_steps)
            {
                // Already revealed
                Symbol revealed = slot->gamble_deck[i];
                Rectangle src = GetAtlasSourceRect(revealed.rank, revealed.suit);
                DrawTexturePro(g_atlas_texture, src, dest, (Vector2){0, 0}, 0.0f, WHITE);
                DrawRectangleLinesEx(dest, 2.0f, LIME);
            }
            else if (slot->show_gamble_result && i == slot->gamble_steps)
            {
                // Currently revealing
                Rectangle src = GetAtlasSourceRect(slot->gamble_next_card.rank, slot->gamble_next_card.suit);
                DrawTexturePro(g_atlas_texture, src, dest, (Vector2){0, 0}, 0.0f, WHITE);
                DrawRectangleLinesEx(dest, 3.0f, slot->gamble_current > 0 ? LIME : RED);
            }
            else
            {
                // Face down
                Rectangle back_src = GetAtlasBackCard();
                DrawTexturePro(g_atlas_texture, back_src, dest, (Vector2){0, 0}, 0.0f, WHITE);
                DrawRectangleLinesEx(dest, 2.0f, GRAY);
            }
        }
        // Instructions
        float instr_y = card_y + card_h + 40.0f;
        if (slot->show_gamble_result)
        {
            if (slot->gamble_current > 0)
            {
                DrawText("YOU WIN! DOUBLED!", (int)(CENTER_X - 120.0f), (int)instr_y, 28, LIME);
                if (slot->gamble_can_gamble)
                {
                    DrawText("PRESS A BUTTON TO CONTINUE", (int)(CENTER_X - 150.0f), (int)(instr_y + 40.0f), 20, WHITE);
                }
                else
                {
                    DrawText("MAX WINS REACHED! PRESS A", (int)(CENTER_X - 180.0f), (int)(instr_y + 40.0f), 20, GOLD);
                }
            }
            else
            {
                DrawText("YOU LOST!", (int)(CENTER_X - 80.0f), (int)instr_y, 28, RED);
                DrawText("PRESS A BUTTON TO CONTINUE", (int)(CENTER_X - 150.0f), (int)(instr_y + 40.0f), 20, WHITE);
            }
        }
        else
        {
            DrawText("Will the next card be HIGHER or LOWER?",
                     (int)(CENTER_X - 220.0f), (int)instr_y, 20, WHITE);
            // Draw buttons
            float btn_y = instr_y + 40.0f;
            float btn_w = 150.0f;
            float btn_h = 60.0f;
            float btn_gap = 40.0f;
            Rectangle high_btn = {CENTER_X - btn_w - btn_gap / 2.0f, btn_y, btn_w, btn_h};
            Rectangle low_btn = {CENTER_X + btn_gap / 2.0f, btn_y, btn_w, btn_h};
            DrawRectangleRec(high_btn, DARKGREEN);
            DrawRectangleLinesEx(high_btn, 3.0f, LIME);
            DrawText("HIGHER", (int)(high_btn.x + 30.0f), (int)(high_btn.y + 18.0f), 24, WHITE);
            DrawText("UP / H", (int)(high_btn.x + 35.0f), (int)(high_btn.y + 45.0f), 14, DARKGRAY);
            DrawRectangleRec(low_btn, DARKGREEN);
            DrawRectangleLinesEx(low_btn, 3.0f, LIME);
            DrawText("LOWER", (int)(low_btn.x + 35.0f), (int)(low_btn.y + 18.0f), 24, WHITE);
            DrawText("DOWN / L", (int)(low_btn.x + 30.0f), (int)(low_btn.y + 45.0f), 14, DARKGRAY);
        }
        DrawText("Press F to FOLD and collect winnings", (int)(CENTER_X - 200.0f), (int)(SCREEN_H - 50.0f), 18, ORANGE);
    }
}