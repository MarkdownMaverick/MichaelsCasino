#include "aibots.h"
#include "main.h"
static void AI_Bob_SelectDiscard(Card *hand, int hand_size, int *best_idx)
{
    int joker_idx = -1;
    int low_card_idx = -1;
    for (int i = 0; i < hand_size; i++)
    {
        if (hand[i].rank == RANK_JOKER)
        {
            joker_idx = i;
            break;
        }
        if (hand[i].rank >= RANK_2 && hand[i].rank <= RANK_9)
        {
            if (low_card_idx == -1)
                low_card_idx = i;
        }
    }
    if (joker_idx != -1)
        *best_idx = joker_idx;
    else if (low_card_idx != -1)
        *best_idx = low_card_idx;
    else
        *best_idx = 0;
}
static void AI_Thea_SelectDiscard(Card *hand, int hand_size, int *best_idx)
{
    (void)hand;
    *best_idx = rand() % hand_size;
}
static void AI_Flint_SelectDiscard(Card *hand, int hand_size, int *best_idx)
{
    int low_card_idx = -1;
    int non_joker_idx = -1;
    for (int i = 0; i < hand_size; i++)
    {
        if (hand[i].rank != RANK_JOKER)
        {
            if (non_joker_idx == -1)
                non_joker_idx = i;
            if (hand[i].rank >= RANK_2 && hand[i].rank <= RANK_9)
            {
                if (low_card_idx == -1)
                    low_card_idx = i;
            }
        }
    }
    if (low_card_idx != -1)
        *best_idx = low_card_idx;
    else if (non_joker_idx != -1)
        *best_idx = non_joker_idx;
    else
        *best_idx = 0;
}
void AI_SelectDiscard(GameState *g, int player)
{
    Card *hand = (player == 1) ? g->player1_hand : g->player2_hand;
    int hand_size = (player == 1) ? g->p1_hand_size : g->p2_hand_size;
    int best_idx = 0;
    int account_idx = (player == 1) ? g->p1_account_index : g->p2_account_index;
    AIType ai_type = g->selected_opponent_ai;
    if (account_idx >= 0 && account_idx < g->account_count)
    {
        if (g->accounts[account_idx].is_ai)
        {
            ai_type = g->accounts[account_idx].ai_type;
        }
    }
    switch (ai_type)
    {
    case AI_BOB:
        AI_Bob_SelectDiscard(hand, hand_size, &best_idx);
        break;
    case AI_THEA:
        AI_Thea_SelectDiscard(hand, hand_size, &best_idx);
        break;
    case AI_FLINT:
        AI_Flint_SelectDiscard(hand, hand_size, &best_idx);
        break;
    default:
        AI_Thea_SelectDiscard(hand, hand_size, &best_idx);
        break;
    }
    if (player == 1)
    {
        g->p1_discard_idx = best_idx;
        g->p1_selected = true;
    }
    else
    {
        g->p2_discard_idx = best_idx;
        g->p2_selected = true;
    }
}
void AI_UpdatePlacementPhase(GameState *g, int player)
{
    Card *hand = (player == 1) ? g->player1_hand : g->player2_hand;
    int *hand_size = (player == 1) ? &g->p1_hand_size : &g->p2_hand_size;
    Card(*slots)[3] = (player == 1) ? g->p1_slots : g->p2_slots;
    float *credits = (player == 1) ? &g->p1_credits : &g->p2_credits;
    float *winnings = (player == 1) ? &g->p1_game_winnings : &g->p2_game_winnings;
    int *ranks_complete = (player == 1) ? &g->p1_completed_ranks : &g->p2_completed_ranks;
    bool *done_placing = (player == 1) ? &g->p1_done_placing : &g->p2_done_placing;
    if (*done_placing)
        return;
    float *timer = (player == 1) ? &g->ai_timer : &g->Reshuffle_cover_timer;
    *timer += GetFrameTime();
    if (*timer > g->ai_move_delay)
    {
        *timer = 0;
        float mult = GetRewardMultiplier(*ranks_complete);
        float placement_reward = REWARD_PLACEMENT * mult;
        for (int i = 0; i < *hand_size; i++)
        {
            Card c = hand[i];
            if (c.suit != SUIT_HEARTS && c.rank != RANK_JOKER)
            {
                for (int k = 0; k < KEYCARDS; k++)
                {
                    if (c.rank == g->keycards[k].rank)
                    {
                        int cards_before = 0;
                        for (int s_check = 0; s_check < 3; s_check++)
                            if (slots[k][s_check].is_valid)
                                cards_before++;
                        if (cards_before == 3)
                            continue;
                        for (int s = 0; s < 3; s++)
                        {
                            if (!slots[k][s].is_valid)
                            {
                                slots[k][s] = c;
                                *credits += placement_reward;
                                *winnings += placement_reward;
                                PlaySound(g_place_sound);
                                CheckRankCompletionBonus(g, player, k, cards_before);
                                for (int j = i; j < *hand_size - 1; j++)
                                    hand[j] = hand[j + 1];
                                (*hand_size)--;
                                hand[*hand_size] = DrawFromDeck(g);
                                (*hand_size)++;
                                return;
                            }
                        }
                    }
                }
            }
        }
        *done_placing = true;
        *credits -= COST_PER_ROUND;
        *winnings -= COST_PER_ROUND;
        PlaySound(g_coin_sound);
        if (player == 1)
            g->p1_ai_done_placing_rounds = true;
    }
}