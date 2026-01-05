#include "reelsui.h"
#include "slotreels.h"
#include "jokersgambit.h"
float ui_x = 20.0f;
float ui_y = 20.0f;
float ui_w = 560.0f;
float ui_h = 300.0f;
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
Rectangle GetHoldButton(int reel_idx)
{
    float reel_x = REEL_START_X + (float)reel_idx * (REEL_WIDTH + REEL_GAP);
    float hold_y = REEL_START_Y + (VISIBLE_SYMBOLS * SYMBOL_SIZE) + 10.0f;
    return (Rectangle){reel_x, hold_y, REEL_WIDTH, HOLD_BTN_HEIGHT};
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
        .spin_y = REEL_START_Y + (VISIBLE_SYMBOLS * SYMBOL_SIZE) + 100.0f,
        .gamble_y = REEL_START_Y + (VISIBLE_SYMBOLS * SYMBOL_SIZE) + 200.0f};
}
static Rectangle GetSpinButton(void)
{
    ButtonConfig cfg = GetButtonConfig();
    float hold_end_x = REEL_START_X + (5 * (REEL_WIDTH + REEL_GAP)) + 20.0f;
    return (Rectangle){hold_end_x, cfg.spin_y, cfg.btn_width, cfg.btn_height};
}
static Rectangle GetGambleButton(void)
{
    ButtonConfig cfg = GetButtonConfig();
    float hold_end_x = REEL_START_X + (5 * (REEL_WIDTH + REEL_GAP)) + 20.0f;
    return (Rectangle){hold_end_x, cfg.gamble_y, cfg.btn_width, cfg.btn_height};
}
static Rectangle ApplyWaveEffect(Rectangle base, float time, float phase)
{
    float amplitude = 3.0f;
    float speed = 2.5f;
    float wave = sinf(time * speed + phase) * amplitude;
    Rectangle waved = base;
    waved.y += wave;
    return waved;
}
void DrawSlotReels(const LobbyState *core, const SlotReelsState *slot)
{
    const float TOTAL_REEL_WIDTH = (REELS_COUNT * REEL_WIDTH) + ((REELS_COUNT - 1) * REEL_GAP);
    const float START_X = CENTER_X - (TOTAL_REEL_WIDTH / 2.0f);
    const float CONTAINER_W = TOTAL_REEL_WIDTH + 40.0f;
    const float CONTAINER_H = (VISIBLE_SYMBOLS * SYMBOL_SIZE) + 40.0f;
    const float CONTAINER_X = CENTER_X - (CONTAINER_W / 2.0f);
    const float CONTAINER_Y = REEL_START_Y - 20.0f;
    const float CENTER_REEL_Y = REEL_START_Y + (VISIBLE_SYMBOLS * SYMBOL_SIZE) / 2.0f;
    DrawRectangle((int)CONTAINER_X, (int)CONTAINER_Y, (int)CONTAINER_W, (int)CONTAINER_H, BLACK);
    BeginScissorMode((int)CONTAINER_X, (int)REEL_START_Y, (int)CONTAINER_W, (int)(VISIBLE_SYMBOLS * SYMBOL_SIZE));
    for (int r = 0; r < REELS_COUNT; r++)
    {
        float reel_x = START_X + (float)r * (REEL_WIDTH + REEL_GAP);
        for (int s = -2; s <= VISIBLE_SYMBOLS + 1; s++)
        {
            float rawY = REEL_START_Y + ((float)s * SYMBOL_SIZE) + slot->offset_y[r];
            int idx = (s + 2) % (VISIBLE_SYMBOLS + 2);
            float distFromCenter = (rawY + (SYMBOL_SIZE / 2.0f) - CENTER_REEL_Y) / (SYMBOL_SIZE * 1.5f);
            float curveOffset = (distFromCenter * distFromCenter * distFromCenter) * 12.0f;
            float displayY = rawY + curveOffset;
            float squash = 1.0f - (fabsf(distFromCenter) * 0.10f);
            float displayH = SYMBOL_SIZE * squash;
            float finalY = displayY + (SYMBOL_SIZE - displayH) / 2.0f;
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
        int text_width = MeasureText(btn_text, 40);
        float center_offset = (float)text_width / 2.0f;
        float text_x_float = btn.x + btn.width / 2.0f - center_offset;
        float text_y_float = btn.y + 30.0f;
        DrawText(btn_text,
                 (int)text_x_float,
                 (int)text_y_float,
                 40,
                 text_color);
        const char *hint_text = "MOUSE CLICK or X BUTTON";
        int hint_width = MeasureText(hint_text, 24);
        float hint_x = CENTER_X - (float)hint_width / 2.0f;
        DrawText(hint_text, (int)hint_x, (int)(btn.y + 120.0f), 24, GRAY);
        const char *token_text = TextFormat("Tokens: %.0f", acc->tokens);
        int token_width = MeasureText(token_text, 40);
        DrawText(token_text, (int)(CENTER_X - (float)token_width / 2.0f), (int)(btn.y - 100.0f), 40, GOLD);
        if (!can_insert)
        {
            DrawText("Not enough tokens!", (int)(CENTER_X - 180.0f), (int)(btn.y - 160.0f), 36, RED);
            DrawText("Go to Shop or play Joker's Gambit",
                     (int)(CENTER_X - 280.0f), (int)(btn.y + 160.0f), 28, LIGHTGRAY);
        }
        const char *exit_text = "B BUTTON or BACKSPACE to exit";
        int exit_width = MeasureText(exit_text, 28);
        DrawText(exit_text, (int)(CENTER_X - (float)exit_width / 2.0f), (int)(SCREEN_H - 100.0f), 28, GRAY);
        return;
    }
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
    int gamepad = GetActiveGamepad();
    float wave_time = (float)GetTime();
    Rectangle spin_base = GetSpinButton();
    Rectangle gamble_base = GetGambleButton();
    Rectangle spin_btn = ApplyWaveEffect(spin_base, wave_time, 0.0f);
    Rectangle gamble_btn = ApplyWaveEffect(gamble_base, wave_time, 2.0f);
    bool gamble_selected = (XboxBtnPressed(gamepad, 3));
    Color gamble_bg = gamble_selected ? GRAY : BLACK;
    Color gamble_border = gamble_selected ? GOLD : LIME;
    DrawRectangleRec(gamble_btn, gamble_bg);
    DrawRectangleLinesEx(gamble_btn, gamble_selected ? 5.0f : 3.0f, gamble_border);
    const char *gamble_text = "GAMBLE";
    int gamble_w = MeasureText(gamble_text, 50);
    DrawText(gamble_text,
             (int)(gamble_btn.x + (gamble_btn.width - (float)gamble_w) / 2.0f),
             (int)(gamble_btn.y + (gamble_btn.height - 50) / 2.0f),
             50, YELLOW);
    bool spin_selected = (slot->selected_button == 7 || XboxBtnPressed(gamepad, BTN_X));
    Color spin_bg = spin_selected ? DARKBLUE : BLACK;
    Color spin_border = spin_selected ? GOLD : LIME;
    DrawRectangleRec(spin_btn, spin_bg);
    DrawRectangleLinesEx(spin_btn, spin_selected ? 5.0f : 3.0f, spin_border);
    const char *spin_text = "SPIN";
    int spin_w = MeasureText(spin_text, 50);
    DrawText(spin_text,
             (int)(spin_btn.x + (spin_btn.width - (float)spin_w) / 2.0f),
             (int)(spin_btn.y + (spin_btn.height - 50) / 2.0f),
             50, YELLOW);
    int payline_count = GetActivePaylineCount(slot->payline_mode);
    double total_bet = (double)slot->bet_amount * payline_count;
    DrawText(TextFormat("TOTAL BET: %.0f TOKENS", total_bet),
             (int)ui_x + 15, (int)ui_y + 184, 18, GOLD);
    DrawText("TAB/D-Pad: Navigate | A/Enter: Select | B: Back",
             (int)(CENTER_X - 300.0f), (int)(SCREEN_H - 30), 20, DARKGRAY);
    if (slot->state == SLOT_STATE_GAMBLE)
    {
        DrawRectangle(0, 0, (int)SCREEN_W, (int)SCREEN_H, Fade(BLACK, 0.85f));
        Rectangle fold_btn = {20, 20, 120, 50};
        DrawRectangleRec(fold_btn, DARKGREEN);
        DrawRectangleLinesEx(fold_btn, 2.0f, LIME);
        DrawText("FOLD", 50, 32, 24, WHITE);
        DrawText(TextFormat("%.0f", slot->gamble_current), 30, 55, 16, YELLOW);
        DrawText("DOUBLE OR NOTHING!", (int)(CENTER_X - 150.0f), 100, 40, GOLD);
        DrawText(TextFormat("WIN POT: %.0f TOKENS", slot->gamble_current),
                 (int)(CENTER_X - 140.0f), 150, 24, YELLOW);
        DrawText(TextFormat("STEP %d / %d", slot->gamble_steps + 1, MAX_GAMBLE_STEPS),
                 (int)(CENTER_X - 60.0f), 180, 20, DARKGRAY);
        float card_w = 140.0f;
        float card_h = 210.0f;
        float card_spacing = 20.0f;
        float total_w = (6 * card_w) + (5 * card_spacing);
        float start_x = CENTER_X - (total_w / 2.0f);
        float card_y = SCREEN_H / 2.0f - card_h / 2.0f;
        Rectangle cardSrc = GetAtlasSourceRect(slot->gamble_card.rank, slot->gamble_card.suit);
        Rectangle cardDest = {start_x, card_y, card_w, card_h};
        DrawTexturePro(g_atlas_texture, cardSrc, cardDest, (Vector2){0, 0}, 0.0f, WHITE);
        DrawRectangleLinesEx(cardDest, 3.0f, GOLD);
        DrawText("CURRENT", (int)(start_x + 30.0f), (int)(card_y - 30.0f), 16, WHITE);
        for (int i = 0; i < 5; i++)
        {
            float x = start_x + ((float)(i + 1) * (card_w + card_spacing));
            Rectangle dest = {x, card_y, card_w, card_h};
            if (i < slot->gamble_steps)
            {
                Symbol revealed = slot->gamble_deck[i];
                Rectangle src = GetAtlasSourceRect(revealed.rank, revealed.suit);
                DrawTexturePro(g_atlas_texture, src, dest, (Vector2){0, 0}, 0.0f, WHITE);
                DrawRectangleLinesEx(dest, 2.0f, LIME);
            }
            else if (slot->show_gamble_result && i == slot->gamble_steps)
            {
                Rectangle src = GetAtlasSourceRect(slot->gamble_next_card.rank, slot->gamble_next_card.suit);
                DrawTexturePro(g_atlas_texture, src, dest, (Vector2){0, 0}, 0.0f, WHITE);
                DrawRectangleLinesEx(dest, 3.0f, slot->gamble_current > 0 ? LIME : RED);
            }
            else
            {
                Rectangle back_src = GetAtlasBackCard();
                DrawTexturePro(g_atlas_texture, back_src, dest, (Vector2){0, 0}, 0.0f, WHITE);
                DrawRectangleLinesEx(dest, 2.0f, GRAY);
            }
        }
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