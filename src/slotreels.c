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
        int j = rand() % (i + 1);
        Symbol temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
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
    slot->state = SLOT_STATE_IDLE;
    slot->selected_button = 7; // Start on spin button
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
            slot->symbols[r][i] = init_deck[deck_idx % deck_size];
            deck_idx++;
        }
        slot->offset_y[r] = 0.0f;
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
            slot->final_grid[0][r] = deck[deck_idx % deck_size]; deck_idx++; // Top
            slot->final_grid[1][r] = deck[deck_idx % deck_size]; deck_idx++; // Middle
            slot->final_grid[2][r] = deck[deck_idx % deck_size]; deck_idx++; // Bottom
            // FIXED: Match the drawing loop's indexing!
            // Drawing shows: symbols[2]=top, symbols[3]=middle, symbols[4]=bottom
            slot->symbols[r][2] = slot->final_grid[0][r]; // Top (s=0 → idx=2)
            slot->symbols[r][3] = slot->final_grid[1][r]; // Middle (s=1 → idx=3)
            slot->symbols[r][4] = slot->final_grid[2][r]; // Bottom (s=2 → idx=4)
            // Fill buffers above and below
            slot->symbols[r][0] = deck[deck_idx % deck_size]; deck_idx++;
            slot->symbols[r][1] = deck[deck_idx % deck_size]; deck_idx++;
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
static Rectangle GetSpinButton(void)
{
    return (Rectangle){SPIN_BTN_X, SPIN_BTN_Y, SPIN_BTN_WIDTH, SPIN_BTN_HEIGHT};
}
static Rectangle GetHoldButton(int reel_idx)
{
    float reel_x = REEL_START_X + (float)reel_idx * (REEL_WIDTH + REEL_GAP);
    float hold_y = REEL_START_Y + (VISIBLE_SYMBOLS * SYMBOL_SIZE) + 10.0f;
    return (Rectangle){reel_x, hold_y, REEL_WIDTH, HOLD_BTN_HEIGHT};
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
        slot->gamble_current = 0.0;
        slot->gamble_can_gamble = false;
        PlaySound(g_discard_sound);
    }
}
// ============================================================================
// UPDATE LOGIC
// ============================================================================
void UpdateSlotReels(LobbyState *core, SlotReelsState *slot)
{
    Account *player = (core->p1_account_index >= 0) ? &core->accounts[core->p1_account_index] : NULL;
    int gamepad = GetActiveGamepad();
    Vector2 mouse = GetMousePosition();
    //   bool hold_button_clicked = false;
    // Handle hold button clicks FIRST (when idle, before spins)
    if (slot->state == SLOT_STATE_IDLE)
    {
        for (int r = 0; r < REELS_COUNT; r++)
        {
            Rectangle hold_rect = GetHoldButton(r);
            if (CheckCollisionPointRec(mouse, hold_rect) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                // Can only toggle if this reel wasn't held in the previous spin
                if (!slot->hold_locked[r])
                {
                    slot->hold_reel[r] = !slot->hold_reel[r];
                    PlaySound(g_place_sound);
                    //  hold_button_clicked = true;
                }
                else
                {
                    ShowNotification(core, "REEL LOCKED", "Cannot hold same reel twice in a row");
                }
                break; // Only process one hold button per frame
            }
        }
    }
    if (slot->state == SLOT_STATE_IDLE || slot->state == SLOT_STATE_SHOW_WIN)
    {
        // Navigation for Bet Amount (Left Side) - LT & RT
        if (XboxBtnPressed(gamepad, 10))
        { // LT
            slot->bet_amount = (slot->bet_amount == BET_AMOUNT_1) ? BET_AMOUNT_10 : (slot->bet_amount == BET_AMOUNT_5) ? BET_AMOUNT_1
                                                                                                                       : BET_AMOUNT_5;
        }
        if (XboxBtnPressed(gamepad, 4))
        { // RT
            slot->bet_amount = (slot->bet_amount == BET_AMOUNT_1) ? BET_AMOUNT_5 : (slot->bet_amount == BET_AMOUNT_5) ? BET_AMOUNT_10
                                                                                                                      : BET_AMOUNT_1;
        }
        // Bet Multiplier (D-Pad Up/Down)
        if (XboxBtnPressed(gamepad, 11))
        { // D-Pad Up
            slot->bet_level++;
        }
        if (XboxBtnPressed(gamepad, 12) && slot->bet_level > 1)
        { // D-Pad Down
            slot->bet_level--;
        }
        // Payline Navigation (Right Side) - LB & RB
        if (XboxBtnPressed(gamepad, 9))
        { // LB
            slot->payline_mode = (PaylineSelection)((slot->payline_mode + 3) % 4);
        }
        if (XboxBtnPressed(gamepad, 5))
        { // RB
            slot->payline_mode = (PaylineSelection)((slot->payline_mode + 1) % 4);
        }
        // D-pad Left/Right: Navigate hold buttons
        static int selected_hold = -1;
        if (XboxBtnPressed(gamepad, 13)) // D-pad left
        {
            if (selected_hold > 0)
                selected_hold--;
            else
                selected_hold = REELS_COUNT - 1;
        }
        if (XboxBtnPressed(gamepad, 14)) // D-pad right
        {
            if (selected_hold < REELS_COUNT - 1)
                selected_hold++;
            else
                selected_hold = 0;
        }
        // A button: Hold selected reel
        if (XboxBtnPressed(gamepad, 0)) // A button
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
        if (XboxBtnPressed(gamepad, 2)) // X button
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
                // Update hold locks
                for (int r = 0; r < REELS_COUNT; r++)
                {
                    slot->hold_locked[r] = slot->hold_reel[r];
                    slot->hold_reel[r] = false;
                }
                PlaySound(g_coin_sound);
            }
            else if (player)
            {
                ShowNotification(core, "INSUFFICIENT TOKENS",
                                 TextFormat("Need %.0f tokens to spin", total_bet));
            }
        }
        // Y button: Gamble (only during win state)
        if (slot->state == SLOT_STATE_SHOW_WIN && XboxBtnPressed(gamepad, 3)) // Y button
        {
            if (slot->total_win_this_spin > 0)
            {
                InitGamble(slot);
                slot->state = SLOT_STATE_GAMBLE;
                PlaySound(g_coin_sound);
            }
        }
        // Keep keyboard controls
        if (IsKeyPressed(KEY_G) && slot->state == SLOT_STATE_SHOW_WIN && slot->total_win_this_spin > 0)
        {
            InitGamble(slot);
            slot->state = SLOT_STATE_GAMBLE;
            PlaySound(g_coin_sound);
        }
        if (IsKeyPressed(KEY_C) || XboxBtnPressed(gamepad, 1)) // C key or B button
        {
            if (slot->state == SLOT_STATE_SHOW_WIN)
            {
                memset(slot->hold_locked, 0, sizeof(slot->hold_locked));
                slot->state = SLOT_STATE_IDLE;
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
            // Evaluate ONLY the selected payline(s)
            // Grid storage: [0]=TOP, [1]=MIDDLE, [2]=BOTTOM
            switch (slot->payline_mode)
            {
            case PAYLINE_TOP: // Evaluates grid[0]
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
            case PAYLINE_MIDDLE: // Evaluates grid[1]
            {
                // DEBUG: Print the actual cards being evaluated
                printf("=== EVALUATING MIDDLE LINE ===\n");
                for (int r = 0; r < REELS_COUNT; r++)
                {
                    printf("Reel %d: Rank=%d Suit=%d\n", r,
                           slot->final_grid[1][r].rank,
                           slot->final_grid[1][r].suit);
                }
                PokerHand hand = EvaluateLine(slot->final_grid[1]);
                int mult = GetPayoutMultiplier(hand);
                printf("Hand result: %d, Multiplier: %d\n", hand, mult);
                printf("==============================\n");
                if (mult > 0)
                {
                    slot->win_lines[1] = true;
                    total_win += (double)mult * slot->bet_amount;
                    if (player)
                        TrackHandAchievements(player, hand);
                }
                break;
            }
            case PAYLINE_BOTTOM: // Evaluates grid[2]
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
            case PAYLINE_ALL: // Evaluates all three
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
            slot->win_timer = 4.0f;
        }
    }
    // Gamble state
    if (slot->state == SLOT_STATE_GAMBLE)
    {
        // Fold button
        if (IsKeyPressed(KEY_F) || XboxBtnPressed(gamepad, 1))
        {
            if (player)
            {
                player->tokens += slot->gamble_current;
                SaveAllAccounts(core);
            }
            memset(slot->hold_locked, 0, sizeof(slot->hold_locked));
            slot->state = SLOT_STATE_IDLE;
            return;
        }
        if (slot->show_gamble_result)
        {
            // Show result briefly, then allow next action
            if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER) ||
                XboxBtnPressed(gamepad, 0) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
            {
                if (slot->gamble_current > 0 && slot->gamble_can_gamble)
                {
                    slot->show_gamble_result = false;
                }
                else
                {
                    // Lost or maxed out
                    if (player)
                    {
                        player->tokens += slot->gamble_current;
                        SaveAllAccounts(core);
                    }
                    memset(slot->hold_locked, 0, sizeof(slot->hold_locked));
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
    // Highlight cards being evaluated on the active payline(s)
    if (slot->state == SLOT_STATE_SHOW_WIN || slot->state == SLOT_STATE_IDLE)
    {
        for (int line = 0; line < 3; line++)
        {
            bool should_highlight = false;
            if (slot->payline_mode == PAYLINE_ALL)
                should_highlight = true;
            else if (slot->payline_mode == PAYLINE_TOP && line == 0)
                should_highlight = true;
            else if (slot->payline_mode == PAYLINE_MIDDLE && line == 1)
                should_highlight = true;
            else if (slot->payline_mode == PAYLINE_BOTTOM && line == 2)
                should_highlight = true;
            if (should_highlight)
            {
                for (int r = 0; r < REELS_COUNT; r++)
                {
                    float reel_x = START_X + (float)r * (REEL_WIDTH + REEL_GAP);
                    float card_y = REEL_START_Y + ((float)line * SYMBOL_SIZE);
                    Rectangle card_rect = {reel_x, card_y, REEL_WIDTH, SYMBOL_SIZE};
                    // Draw subtle overlay on evaluated cards
                    Color overlay = (slot->state == SLOT_STATE_SHOW_WIN && slot->win_lines[line])
                                        ? Fade(GOLD, 0.3f)
                                        : Fade(YELLOW, 0.15f);
                    DrawRectangleRec(card_rect, overlay);
                }
            }
        }
    }
    EndScissorMode();
    // Draw paylines
    for (int i = 0; i < 3; i++)
    {
        float lineY = REEL_START_Y + ((float)i * SYMBOL_SIZE) + (SYMBOL_SIZE / 2.0f);
        Color lineColor = Fade(WHITE, 0.1f);
        float thick = 1.0f;
        // FIXED: Highlight selected lines (now i directly matches the enum)
        if (slot->payline_mode == PAYLINE_ALL ||
            (slot->payline_mode == PAYLINE_TOP && i == 0) ||
            (slot->payline_mode == PAYLINE_MIDDLE && i == 1) ||
            (slot->payline_mode == PAYLINE_BOTTOM && i == 2))
        {
            lineColor = Fade(YELLOW, 0.4f);
            thick = 2.0f;
        }
        // Highlight winning lines
        if (slot->state == SLOT_STATE_SHOW_WIN && slot->win_lines[i])
        {
            lineColor = GOLD;
            thick = 4.0f;
        }
        DrawLineEx((Vector2){CONTAINER_X, lineY},
                   (Vector2){CONTAINER_X + CONTAINER_W, lineY},
                   thick, lineColor);
    }
    DrawRectangleLinesEx((Rectangle){CONTAINER_X, CONTAINER_Y, CONTAINER_W, CONTAINER_H}, 2.0f, DARKGRAY);
    // Draw hold buttons
    for (int r = 0; r < REELS_COUNT; r++)
    {
        Rectangle hold_rect = GetHoldButton(r);
        Color bg = slot->hold_reel[r] ? DARKGREEN : (slot->hold_locked[r] ? MAROON : DARKGRAY);
        Color border = slot->hold_reel[r] ? LIME : (slot->hold_locked[r] ? RED : GRAY);
        DrawRectangleRec(hold_rect, bg);
        DrawRectangleLinesEx(hold_rect, 2.0f, border);
        const char *text = slot->hold_reel[r] ? "HELD" : (slot->hold_locked[r] ? "LOCKED" : "HOLD");
        int text_w = MeasureText(text, 14);
        DrawText(text,
                 (int)(hold_rect.x + (hold_rect.width - (float)text_w) / 2.0f),
                 (int)(hold_rect.y + (hold_rect.height - 14) / 2.0f),
                 14, WHITE);
    }
    // Player UI panel
    if (core->p1_account_index >= 0)
    {
        const Account *acc = &core->accounts[core->p1_account_index];
        const char *pName = GetPlayerNameByIndex(core, core->p1_account_index);
        const char *pStatus = GetMemberStatusString(acc->member_status);
        float ui_x = 20.0f;
        float ui_y = 20.0f;
        float ui_w = 560.0f;
        float ui_h = 300.0f;
        DrawRectangle((int)ui_x, (int)ui_y, (int)ui_w, (int)ui_h, Fade(BLACK, 0.9f));
        DrawRectangleLinesEx((Rectangle){ui_x, ui_y, ui_w, ui_h}, 2.0f, GOLD);
        DrawText(TextFormat("NAME: %s", pName), (int)ui_x + 15, (int)ui_y + 15, 20, WHITE);
        int status_w = MeasureText(pStatus, 18);
        DrawText(pStatus, (int)(ui_x + ui_w - (float)status_w - 15.0f), (int)(ui_y + 15.0f), 18, GOLD);
        DrawText(TextFormat("CREDITS: $%.2f", acc->credits), (int)ui_x + 15, (int)ui_y + 45, 20, LIME);
        DrawText(TextFormat("TOKENS: %.0f", acc->tokens), (int)ui_x + 15, (int)ui_y + 70, 20, YELLOW);
        // Show winning hand info
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
                    const char *line_name = (line == 0) ? "TOP" : (line == 1) ? "MIDDLE"
                                                                              : "BOTTOM";
                    DrawText(TextFormat("%s LINE: %s", line_name, hand_names[h]),
                             (int)ui_x + 15, (int)ui_y + 130 + (line * 20), 18, LIME);
                }
            }
            DrawText(TextFormat("TOTAL WIN: %.0f TOKENS", slot->total_win_this_spin),
                     (int)ui_x + 15, (int)ui_y + 200, 20, GOLD);
            // Show gamble option
            DrawText("PRESS G TO GAMBLE or C TO COLLECT", (int)ui_x + 15, (int)ui_y + 230, 16, YELLOW);
        }
    }
    // Draw bet buttons
    const char *bet_labels[] = {"1", "5", "10"};
    BetAmount bet_values[] = {BET_AMOUNT_1, BET_AMOUNT_5, BET_AMOUNT_10};
    DrawText("BET AMOUNT", (int)BET_BTN_START_X, (int)BET_BTN_START_Y - 40, 20, YELLOW);
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
    }
    // Draw spin button
    Rectangle spin_btn = GetSpinButton();
    bool spin_selected = (slot->selected_button == 7);
    Color spin_bg = spin_selected ? MAROON : DARKGREEN;
    Color spin_border = spin_selected ? GOLD : LIME;
    DrawRectangleRec(spin_btn, spin_bg);
    DrawRectangleLinesEx(spin_btn, spin_selected ? 5.0f : 3.0f, spin_border);
    const char *spin_text = "SPIN";
    int spin_w = MeasureText(spin_text, 50);
    DrawText(spin_text,
             (int)(spin_btn.x + (spin_btn.width - (float)spin_w) / 2.0f),
             (int)(spin_btn.y + (spin_btn.height - 50) / 2.0f),
             50, WHITE);
    // Draw total bet info
    int payline_count = GetActivePaylineCount(slot->payline_mode);
    double total_bet = (double)slot->bet_amount * payline_count;
    DrawText(TextFormat("TOTAL BET: %.0f TOKENS", total_bet),
             (int)(CENTER_X - 120.0f), (int)(SPIN_BTN_Y - 40), 25, ORANGE);
    // Footer controls
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
                    DrawText("PRESS ANY KEY TO CONTINUE", (int)(CENTER_X - 150.0f), (int)(instr_y + 40.0f), 20, WHITE);
                }
                else
                {
                    DrawText("MAX WINS REACHED! PRESS ANY KEY", (int)(CENTER_X - 180.0f), (int)(instr_y + 40.0f), 20, GOLD);
                }
            }
            else
            {
                DrawText("YOU LOST!", (int)(CENTER_X - 80.0f), (int)instr_y, 28, RED);
                DrawText("PRESS ANY KEY TO CONTINUE", (int)(CENTER_X - 150.0f), (int)(instr_y + 40.0f), 20, WHITE);
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