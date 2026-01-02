```markdown
<p align="center">Michael's Casino</p>
<p align="center">
<i>A multi-game casino platform featuring classic card games with persistent accounts and competitive leaderboards</i>
</p>

## About

**Michael's Casino** is an expanding casino suite built in C with Raylib, forked and heavily extended from the original [Joker's Gambit](https://github.com/MarkdownMaverick/jokersgambit).  
It now features a unified lobby, persistent player progression, and multiple playable games sharing the same high-quality card atlas.

### Available Games (Now Compiling & Playable)

- **Joker's Gambit** – The original strategic 5-rank card placement game with jokers (fully playable)
- **Jacks or Better Slots** – Classic 5-reel, 3-row video slot machine using poker hand payouts (Jacks or Better rules) with 1–3 token bet levels and 1–3 active paylines

**Blackjack** and additional games are planned next.

---

## Features

✅ **Full Xbox Controller Support** – Tested on Xbox Series X/S controller (SDL2 integration)  
✅ **Unified Account System** – 5 accounts (3 AI personalities + 2 human slots). Tracks credits, tokens, stats, win streaks, and member status across all games  
✅ **Cross-Game Leaderboard** – Top 100 entries with game-specific filtering and multiple sort modes  
✅ **Persistent Economy** – Credits and tokens saved between sessions; used for betting and shop purchases  
✅ **Multiple AI Opponents** – BOB (aggressive/hard), THEA (balanced/medium), FLINT (conservative/easy)  
✅ **Shared Asset System** – Single high-resolution card atlas (`DECK0.png`, 2600×1500, 13×5 layout) used efficiently by all games (Joker's Gambit grid, future Blackjack table, and Slot Reels symbols)  
✅ **Token-Based Betting in Slots** – 1/2/3 token bets activate 1/2/3 paylines with classic Jacks or Better payout table (Royal Flush 800:1)  
✅ **Full Persistence** – Separate JSON files (`save/` folder) for accounts, leaderboard, achievements, and settings – easy to inspect or modify  

✅### Leaderboard Sorting
- Filter by specific game
- Sort by final credits, moves made, or date/time (ascending/descending)

---

## Technical Details

- **Engine**: Raylib (C99) for rendering, audio, and input
- **Controller Support**: SDL2 for robust gamepad handling
- **Data**: cJSON for clean, human-readable save files
- **Resolution**: 1920×1080 base with scaling options (75%, 100%, 125%, 150%) and borderless fullscreen
- **Audio**: Background music + extensive sound effects (discards, placements, wins, coins, etc.)
- **Architecture**: Clean state machine with modular files (`main`, `mainmenu`, `useraccount`, `jokersgambit`, `slotreels`, `gamepad_sdl`, etc.)
- **Build**: Strict `-Werror` compilation for code quality

### Current Development Status

**Core framework complete and compiling cleanly**  
All foundational systems are now stable and integrated:

- Main menu → Lobby → Mode selection flow
- Account management (create, login/logout, name editing)
- Settings (music, AI delay, card cover, scaling, fullscreen)
- Shop (token purchases with tiered pricing)
- Achievements system (50 slots, grid view, persistence)
- Leaderboard (load/save, sorting, filtering)
- Full game transitions to both Joker's Gambit and Jacks or Better Slots

---

**Michael's Casino – Vision**

A premium-feeling 2D casino suite with deep progression (achievements, member ranks, token economy), satisfying audio/visual feedback, and local "multiplayer" personality through distinct AI opponents. Designed to be easily extensible for future games while maintaining a cohesive, polished experience.

## Credits

Original Joker's Gambit concept and core mechanics:  
https://github.com/MarkdownMaverick/jokersgambit  
(Full credits: [credits.md](credits.md))

Raylib by raysan5 – https://www.raylib.com  
cJSON by Dave Gamble – https://github.com/DaveGamble/cJSON  
SDL2 for gamepad support

---
🎰♣️♦️♥️♠️
```