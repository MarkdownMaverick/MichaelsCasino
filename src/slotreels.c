#include "slotreels.h"
#include "reelsui.h"  // Add this for DrawSlotReels and GetHoldButton
#include "main.h"
#include "mainmenu.h"
#include "useraccount.h"
#include "jokersgambit.h"
#include <math.h>

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
 PokerHand EvaluateLine(const Symbol hand[REELS_COUNT])
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
int GetActivePaylineCount(PaylineSelection mode)
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
