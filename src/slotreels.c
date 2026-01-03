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

// Button positions
#define BET_BTN_START_X (SCREEN_W * 0.05f)
#define BET_BTN_START_Y (SCREEN_H * 0.5f)
#define LINE_BTN_START_X (SCREEN_W - BET_BTN_START_X - BTN_WIDTH)
#define LINE_BTN_START_Y BET_BTN_START_Y
#define SPIN_BTN_X (CENTER_X - SPIN_BTN_WIDTH / 2.0f)
#define SPIN_BTN_Y (SCREEN_H - SPIN_BTN_HEIGHT - 50.0f)

#define DECK_SIZE 54 // 52 cards + 2 jokers

// ============================================================================
// DECK MANAGEMENT
// ============================================================================

// Build a fresh 54-card deck
static void BuildDeck(Symbol deck[DECK_SIZE])
{
    int idx = 0;

    // Add 52 standard cards: 4 suits (Rows 1-4) x 13 ranks (Cols 1-13)
    for (int suit = 0; suit < 4; suit++)
    {
        for (int rank = 0; rank < 13; rank++)
        {
            deck[idx].rank = (Rank)rank;
            deck[idx].suit = (Suit)suit;
            idx++;
        }
    }

    // Add exactly 2 jokers (Row 5, Cols 1 and 2)
    // Assuming RANK_JOKER is 0 and SUIT_NONE/SPECIAL is 4
    for (int i = 0; i < 2; i++)
    {
        deck[idx].rank = (Rank)i;
        deck[idx].suit = (Suit)4; // Row 5
        idx++;
    }
}
// Fisher-Yates shuffle
static void ShuffleDeck(Symbol deck[DECK_SIZE])
{
    for (int i = DECK_SIZE - 1; i > 0; i--)
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

    // Initialize with random cards from a deck (never use RANK_BACK in gameplay)
    Symbol init_deck[DECK_SIZE];
    BuildDeck(init_deck);
    ShuffleDeck(init_deck);

    int deck_idx = 0;
    for (int r = 0; r < REELS_COUNT; r++)
    {
        for (int i = 0; i < VISIBLE_SYMBOLS + 2; i++)
        {
            slot->symbols[r][i] = init_deck[deck_idx % DECK_SIZE];
            deck_idx++;
        }
        slot->offset_y[r] = 0.0f;
    }
}

// ============================================================================
// RESULT GENERATION - Now uses shared deck
// ============================================================================

static void GenerateResult(SlotReelsState *slot)
{
    // Build and shuffle a fresh 54-card deck
    Symbol deck[DECK_SIZE];
    BuildDeck(deck);
    ShuffleDeck(deck);

    int deck_idx = 0;

    // Deal 5 cards for the center row - this is the actual hand being evaluated
    for (int r = 0; r < REELS_COUNT; r++)
    {
        slot->final_grid[0][r] = deck[deck_idx++]; // Middle row = the 5-card hand
    }

    // Deal 5 cards for top row (visual only, not scored)
    for (int r = 0; r < REELS_COUNT; r++)
    {
        slot->final_grid[1][r] = deck[deck_idx++];
    }

    // Deal 5 cards for bottom row (visual only, not scored)
    for (int r = 0; r < REELS_COUNT; r++)
    {
        slot->final_grid[2][r] = deck[deck_idx++];
    }

    // Set up symbol arrays for animation (the visible cards during spin)
    for (int r = 0; r < REELS_COUNT; r++)
    {
        slot->symbols[r][0] = slot->final_grid[1][r]; // Top
        slot->symbols[r][1] = slot->final_grid[0][r]; // Middle (the hand)
        slot->symbols[r][2] = slot->final_grid[2][r]; // Bottom

        // Add extra symbols for smooth scrolling effect (2 more per reel)
        slot->symbols[r][3] = deck[deck_idx++];
        slot->symbols[r][4] = deck[deck_idx++];

        slot->target_offset[r] = -SYMBOL_SIZE;
    }

    // Total cards used: 5 (middle) + 5 (top) + 5 (bottom) + 10 (extras) = 25 cards
    // Well within our 54-card deck, so no risk of running out
}

// ============================================================================
// POKER HAND EVALUATION - Now properly handles jokers as wilds
// ============================================================================

// Check if we have a flush (all same suit, ignoring jokers)
static bool CheckFlush(const Symbol hand[REELS_COUNT], int joker_count)
{
    if (joker_count >= 4)
        return true; // MAXIMUM OF 2 JOKERS IN THE GAME

    int suit_counts[4] = {0};
    for (int i = 0; i < REELS_COUNT; i++)
    {
        if (hand[i].rank != RANK_JOKER && hand[i].suit != SUIT_NONE)
        {
            suit_counts[hand[i].suit]++;
        }
    }

    // Check if any suit + jokers = 5 ;MAXIMUM OF 2 JOKERS IN THE GAME
    for (int s = 0; s < 4; s++)
    {
        if (suit_counts[s] + joker_count >= 5)
        {
            return true;
        }
    }
    return false;
}

// Check for straight (handle jokers as wildcards) ;MAXIMUM OF 2 JOKERS IN THE GAME
static bool CheckStraight(const Symbol hand[REELS_COUNT], int joker_count)
{
    if (joker_count >= 4)
        return true; // 4+ jokers = auto straight;MAXIMUM OF 2 JOKERS IN THE GAME

    bool has_rank[13] = {false};

    // Mark which ranks we have (excluding jokers);MAXIMUM OF 2 JOKERS IN THE GAME
    for (int i = 0; i < REELS_COUNT; i++)
    {
        if (hand[i].rank != RANK_JOKER && hand[i].rank >= RANK_2 && hand[i].rank <= RANK_ACE)
        {
            has_rank[hand[i].rank - RANK_2] = true;
        }
    }

    // Try all possible 5-card sequences
    for (int start = 0; start <= 8; start++)
    { // 0-8 covers 2-A
        int gaps = 0;
        for (int offset = 0; offset < 5; offset++)
        {
            if (!has_rank[start + offset])
            {
                gaps++;
            }
        }
        if (gaps <= joker_count)
        {
            return true;
        }
    }

    // Check A-2-3-4-5 wheel (ace low)
    int wheel_gaps = 0;
    if (!has_rank[12])
        wheel_gaps++; // Ace
    if (!has_rank[0])
        wheel_gaps++; // 2
    if (!has_rank[1])
        wheel_gaps++; // 3
    if (!has_rank[2])
        wheel_gaps++; // 4
    if (!has_rank[3])
        wheel_gaps++; // 5

    return wheel_gaps <= joker_count;
}

static PokerHand EvaluateLine(const Symbol hand[REELS_COUNT])
{
    // 1. Identify Jokers based on your Atlas (Row 5, first two columns) MAXIMUM OF 2 JOKERS IN THE GAME
    int joker_count = 0;
    for (int i = 0; i < REELS_COUNT; i++)
    {
        if (hand[i].suit == 4)
            joker_count++;
    }

    if (joker_count >= 5)
        return HAND_ROYAL_FLUSH;

    // 2. Count standard rank frequencies (0-12)
    int rank_freq[13] = {0};
    int max_rank_count = 0;
    for (int i = 0; i < REELS_COUNT; i++)
    {
        if (hand[i].suit < 4)
        { // Only count if not a Joker/Special
            int r = (int)hand[i].rank;
            if (r >= 0 && r < 13)
            {
                rank_freq[r]++;
                if (rank_freq[r] > max_rank_count)
                    max_rank_count = rank_freq[r];
            }
        }
    }

    // 3. Pattern Detection
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

    int best_set = max_rank_count + joker_count;
    bool is_flush = CheckFlush(hand, joker_count);
    bool is_straight = CheckStraight(hand, joker_count);

    // 4. Return Payouts (Descending Order)
    if (is_straight && is_flush)
        return HAND_STRAIGHT_FLUSH; // Simplified Royal check
    if (best_set >= 4)
        return HAND_FOUR_OF_A_KIND;
    if ((threes && pairs) || (joker_count >= 1 && pairs >= 2))
        return HAND_FULL_HOUSE;
    if (is_flush)
        return HAND_FLUSH;
    if (is_straight)
        return HAND_STRAIGHT;
    if (best_set >= 3)
        return HAND_THREE_OF_A_KIND;
    if (pairs >= 2)
        return HAND_TWO_PAIR;

    // 5. Jacks or Better (Safe indexing)
    // Standard Ranks: 9=J, 10=Q, 11=K, 12=A
    for (int i = 9; i <= 12; i++)
    {
        if (rank_freq[i] >= 2 || (rank_freq[i] == 1 && joker_count >= 1))
        {
            return HAND_JACKS_OR_BETTER;
        }
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
    case PAYLINE_MIDDLE:
    case PAYLINE_TOP:
    case PAYLINE_BOTTOM:
        return 1;
    case PAYLINE_ALL:
        return 3;
    default:
        return 1;
    }
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

// ============================================================================
// UPDATE LOGIC
// ============================================================================

void UpdateSlotReels(LobbyState *core, SlotReelsState *slot)
{
    Account *player = (core->p1_account_index >= 0) ? &core->accounts[core->p1_account_index] : NULL;
    int gamepad = GetActiveGamepad();
    Vector2 mouse = GetMousePosition();

    if (slot->state == SLOT_STATE_IDLE || slot->state == SLOT_STATE_SHOW_WIN)
    {
        // Controller navigation
        if (IsKeyPressed(KEY_LEFT) || XboxBtnPressed(gamepad, 13))
        {
            if (slot->selected_button >= 0 && slot->selected_button <= 2)
                slot->selected_button = (slot->selected_button - 1 + 3) % 3;
            else if (slot->selected_button >= 3 && slot->selected_button <= 6)
                slot->selected_button = ((slot->selected_button - 3 - 1 + 4) % 4) + 3;
        }

        if (IsKeyPressed(KEY_RIGHT) || XboxBtnPressed(gamepad, 14))
        {
            if (slot->selected_button >= 0 && slot->selected_button <= 2)
                slot->selected_button = (slot->selected_button + 1) % 3;
            else if (slot->selected_button >= 3 && slot->selected_button <= 6)
                slot->selected_button = ((slot->selected_button - 3 + 1) % 4) + 3;
        }

        if (IsKeyPressed(KEY_UP) || XboxBtnPressed(gamepad, 11))
        {
            if (slot->selected_button == 7)
                slot->selected_button = 0;
            else if (slot->selected_button >= 0 && slot->selected_button <= 2)
                slot->selected_button = (slot->selected_button - 1 + 3) % 3;
            else if (slot->selected_button >= 3 && slot->selected_button <= 6)
                slot->selected_button = ((slot->selected_button - 3 - 1 + 4) % 4) + 3;
        }

        if (IsKeyPressed(KEY_DOWN) || XboxBtnPressed(gamepad, 12))
        {
            if (slot->selected_button >= 0 && slot->selected_button <= 6)
                slot->selected_button = 7;
            else if (slot->selected_button == 7)
                slot->selected_button = 3;
        }

        if (IsKeyPressed(KEY_TAB))
        {
            slot->selected_button = (slot->selected_button + 1) % 8;
        }

        // Mouse hover detection
        for (int i = 0; i < 3; i++)
        {
            if (CheckCollisionPointRec(mouse, GetBetButton(i)))
                slot->selected_button = i;
        }
        for (int i = 0; i < 4; i++)
        {
            if (CheckCollisionPointRec(mouse, GetPaylineButton(i)))
                slot->selected_button = 3 + i;
        }
        if (CheckCollisionPointRec(mouse, GetSpinButton()))
            slot->selected_button = 7;

        // Button activation
        bool activate = (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
                         XboxBtnPressed(gamepad, 0) ||
                         IsMouseButtonPressed(MOUSE_LEFT_BUTTON));

        if (activate)
        {
            // Update selected button based on mouse position
            for (int i = 0; i < 3; i++)
            {
                if (CheckCollisionPointRec(mouse, GetBetButton(i)))
                {
                    slot->selected_button = i;
                    break;
                }
            }
            for (int i = 0; i < 4; i++)
            {
                if (CheckCollisionPointRec(mouse, GetPaylineButton(i)))
                {
                    slot->selected_button = 3 + i;
                    break;
                }
            }
            if (CheckCollisionPointRec(mouse, GetSpinButton()))
            {
                slot->selected_button = 7;
            }

            // Execute button action
            switch (slot->selected_button)
            {
            case 0:
                slot->bet_amount = BET_AMOUNT_1;
                break;
            case 1:
                slot->bet_amount = BET_AMOUNT_5;
                break;
            case 2:
                slot->bet_amount = BET_AMOUNT_10;
                break;
            case 3:
                slot->payline_mode = PAYLINE_MIDDLE;
                break;
            case 4:
                slot->payline_mode = PAYLINE_TOP;
                break;
            case 5:
                slot->payline_mode = PAYLINE_BOTTOM;
                break;
            case 6:
                slot->payline_mode = PAYLINE_ALL;
                break;
            case 7: // SPIN
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
                    PlaySound(g_coin_sound);
                }
                else if (player)
                {
                    ShowNotification(core, "INSUFFICIENT TOKENS",
                                     TextFormat("Need %.0f tokens to spin", total_bet));
                }
                break;
            }
            default:
                break;
            }
        }

        // Back button
        if (IsKeyPressed(KEY_B) || XboxBtnPressed(gamepad, 1))
        {
            SwitchState(core, STATE_LOBBY);
        }

        // Win timer
        if (slot->state == SLOT_STATE_SHOW_WIN)
        {
            slot->win_timer -= GetFrameTime();
            if (slot->win_timer <= 0.0f)
                slot->state = SLOT_STATE_IDLE;
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

            // Evaluate based on payline mode - NOW USES THE 5-CARD HAND
            switch (slot->payline_mode)
            {
            case PAYLINE_MIDDLE:
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
            case PAYLINE_TOP:
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
    // 1. Calculate Layout Dimensions (Same as before)
    const float TOTAL_REEL_WIDTH = (REELS_COUNT * REEL_WIDTH) + ((REELS_COUNT - 1) * REEL_GAP);
    const float START_X = CENTER_X - (TOTAL_REEL_WIDTH / 2.0f);
    const float CONTAINER_W = TOTAL_REEL_WIDTH + 40.0f;
    const float CONTAINER_H = (VISIBLE_SYMBOLS * SYMBOL_SIZE) + 40.0f;
    const float CONTAINER_X = CENTER_X - (CONTAINER_W / 2.0f);
    const float CONTAINER_Y = REEL_START_Y - 20.0f;
    const float VIEW_CENTER_Y = REEL_START_Y + (VISIBLE_SYMBOLS * SYMBOL_SIZE) / 2.0f;

    // 2. Draw Background
    DrawRectangle((int)CONTAINER_X, (int)CONTAINER_Y, (int)CONTAINER_W, (int)CONTAINER_H, BLACK);

    // 3. Draw Reels (Scissor Mode)
    BeginScissorMode((int)CONTAINER_X, (int)REEL_START_Y, (int)CONTAINER_W, (int)(VISIBLE_SYMBOLS * SYMBOL_SIZE));

    for (int r = 0; r < REELS_COUNT; r++)
    {
        float reel_x = START_X + (float)r * (REEL_WIDTH + REEL_GAP);

        for (int s = -2; s <= VISIBLE_SYMBOLS + 1; s++)
        {
            float rawY = REEL_START_Y + ((float)s * SYMBOL_SIZE) + slot->offset_y[r];
            int idx = (s + 2) % (VISIBLE_SYMBOLS + 2);

            // Curve math for visual pop
            float distFromCenter = (rawY + (SYMBOL_SIZE / 2.0f) - VIEW_CENTER_Y) / (SYMBOL_SIZE * 1.5f);
            float curveOffset = (distFromCenter * distFromCenter * distFromCenter) * 12.0f;
            float displayY = rawY + curveOffset;
            float squash = 1.0f - (fabsf(distFromCenter) * 0.10f);
            float displayH = SYMBOL_SIZE * squash;
            float finalY = displayY + (SYMBOL_SIZE - displayH) / 2.0f;

            // Blur effect calculation
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
            Color tint = {(unsigned char)(255 * tintVal), (unsigned char)(255 * tintVal), (unsigned char)(255 * tintVal), (unsigned char)(255 * alphaFade)};

            DrawTexturePro(g_atlas_texture, src, dest, (Vector2){0, 0}, 0.0f, tint);
        }
    }
    EndScissorMode();

    // 4. Draw Paylines
    for (int i = 0; i < 3; i++)
    {
        float lineY = REEL_START_Y + ((float)i * SYMBOL_SIZE) + (SYMBOL_SIZE / 2.0f);
        Color lineColor = Fade(WHITE, 0.1f);
        float thick = 1.0f;

        // Highlight selected lines
        if (slot->payline_mode == PAYLINE_ALL ||
            (slot->payline_mode == PAYLINE_MIDDLE && i == 0) ||
            (slot->payline_mode == PAYLINE_TOP && i == 1) ||
            (slot->payline_mode == PAYLINE_BOTTOM && i == 2))
        {
            lineColor = Fade(YELLOW, 0.4f);
            thick = 2.0f;
        }

        // Highlight WINNING lines
        if (slot->state == SLOT_STATE_SHOW_WIN && slot->win_lines[i])
        {
            lineColor = GOLD;
            thick = 4.0f;
        }

        DrawLineEx((Vector2){CONTAINER_X, lineY}, (Vector2){CONTAINER_X + CONTAINER_W, lineY}, thick, lineColor);
    }

    // Border
    DrawRectangleLinesEx((Rectangle){CONTAINER_X, CONTAINER_Y, CONTAINER_W, CONTAINER_H}, 2.0f, DARKGRAY);

    // ====================================================================
    // NEW PLAYER UI PANEL (Top Left)
    // ====================================================================
    if (core->p1_account_index >= 0)
    {
        const Account *acc = &core->accounts[core->p1_account_index];
        const char *pName = GetPlayerNameByIndex(core, core->p1_account_index);
        const char *pStatus = GetMemberStatusString(acc->member_status);

        // UI Dimensions
        float ui_x = 20.0f;
        float ui_y = 20.0f;
        float ui_w = 380.0f;
        float ui_h = 130.0f;

        // Draw Panel Background
        DrawRectangle((int)ui_x, (int)ui_y, (int)ui_w, (int)ui_h, Fade(BLACK, 0.9f));
        DrawRectangleLinesEx((Rectangle){ui_x, ui_y, ui_w, ui_h}, 2.0f, GOLD);

        // -- Row 1: Name & Status --
        DrawText(TextFormat("NAME: %s", pName), (int)ui_x + 15, (int)ui_y + 15, 20, WHITE);

        // Align status to the right
        int status_w = MeasureText(pStatus, 18);
        DrawText(pStatus, (int)(ui_x + ui_w - (float)status_w - 15.0f), (int)(ui_y + 15.0f), 18, GOLD);
        // -- Row 2: Credits & Tokens --
        DrawText(TextFormat("CREDITS: $%.2f", acc->credits), (int)ui_x + 15, (int)ui_y + 45, 20, LIME);
        DrawText(TextFormat("TOKENS: %.0f", acc->tokens), (int)ui_x + 15, (int)ui_y + 70, 20, YELLOW);

        // -- Row 3: Winning Hand / Payout --
        if (slot->state == SLOT_STATE_SHOW_WIN && slot->total_win_this_spin > 0)
        {
            PokerHand best_hand = HAND_HIGH_CARD;
            const char *hand_names[] = {
                "HIGH CARD", "JACKS OR BETTER", "TWO PAIR", "THREE OF A KIND",
                "STRAIGHT", "FLUSH", "FULL HOUSE", "FOUR OF A KIND",
                "STRAIGHT FLUSH", "ROYAL FLUSH"};

            if (slot->win_lines[0])
            {
                PokerHand h = EvaluateLine(slot->final_grid[0]);
                if (h > best_hand)
                    best_hand = h;
                      // Draw Payout Info
            DrawText(TextFormat("PAYOUT BOTTOM LINE: %s $%.0f", hand_names[best_hand], slot->total_win_this_spin),
                     (int)ui_x + 15, (int)ui_y + 100, 22, ORANGE);
            }
            else if (slot->win_lines[1])
            {
                PokerHand h = EvaluateLine(slot->final_grid[1]);
                if (h > best_hand)
                    best_hand = h;
                      // Draw Payout Info
            DrawText(TextFormat("PAYOUT MIDDLE LINE: %s $%.0f", hand_names[best_hand], slot->total_win_this_spin),
                     (int)ui_x + 15, (int)ui_y + 120, 22, ORANGE);
            }
            else if (slot->win_lines[2])
            {
                PokerHand h = EvaluateLine(slot->final_grid[2]);
                if (h > best_hand)
                    best_hand = h;
                      // Draw Payout Info
            DrawText(TextFormat("PAYOUT TOP LINE: %s $%.0f", hand_names[best_hand], slot->total_win_this_spin),
                     (int)ui_x + 15, (int)ui_y + 130, 22, ORANGE);
            }

          
        }
        else
        {
            // Idle text
            DrawText("SOMETHING GOT F'D UP", (int)ui_x + 15, (int)ui_y + 100, 22, DARKGRAY);
        }
    }

    // ====================================================================
    // BUTTONS
    // ====================================================================

    // Draw bet amount buttons
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
    const char *line_labels[] = {"MIDDLE", "TOP", "BOTTOM", "ALL"};
    PaylineSelection line_values[] = {PAYLINE_MIDDLE, PAYLINE_TOP, PAYLINE_BOTTOM, PAYLINE_ALL};

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

        int text_w = MeasureText(line_labels[i], 20);
        DrawText(line_labels[i],
                 (int)(btn.x + (btn.width - (float)text_w) / 2.0f),
                 (int)(btn.y + (btn.height - 20) / 2.0f),
                 20, WHITE);
    }

    // Draw Spin Button
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

    // Draw Total Bet Info
    int payline_count = GetActivePaylineCount(slot->payline_mode);
    double total_bet = (double)slot->bet_amount * payline_count;
    DrawText(TextFormat("TOTAL BET: %.0f TOKENS", total_bet),
             (int)(CENTER_X - 120), (int)(SPIN_BTN_Y - 40), 25, ORANGE);

    // Footer Controls
    DrawText("TAB/D-Pad: Navigate | A/Enter: Select | B: Back",
             (int)(CENTER_X - 300), (int)(SCREEN_H - 30), 20, DARKGRAY);
}