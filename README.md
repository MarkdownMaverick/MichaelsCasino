<p align="center">Michael's Casino</p>


<p align="center">
<i>A multi-game casino platform featuring classic card games with persistent accounts and competitive leaderboards</i>
</p>

## About

**MichaelsCasino** is a fork of [Joker's Gambit](https://github.com/[original-repo]) expanded into a full casino experience. Play multiple card games, track your progress across accounts, and compete on global leaderboards.

### No currently Available Games they will be added when the core is complete

- **Joker's Gambit** - The original strategic card game (see [original repository](link) for full rules)
- **Blackjack** - Classic 21 with AI dealers of varying difficulty
- **Card Slots** - 5-reel poker-based slot machine with traditional hand payouts(JACKS OR BETTER)

---

## Features

✅ **Xbox Controler support** - Testing on xbox series x controller
✅ **Unified Account System** - One profile tracks credits, tokens, and stats across all games  
✅ **Cross-Game Leaderboard** - Top 100 rankings with game-specific filtering  
✅ **Persistent Economy** - Credits and tokens carry between sessions  
✅ **Multiple AI Opponents/Dealers** - BOB (Hard), THEA (Medium), FLINT (Easy)  
✅ **Shared Asset System** - Efficient texture atlas for all card games  
DECK0.png (2600×1500px) is a perfect size for a high-res experience.

Joker's Gambit uses the cards for its placement grid.

Blackjack will use the exact same cards but likely drawn at a larger scale for the "Table" feel.

Card Slots will treat the cards as "Symbols" on the reels.

By keeping one atlas_manager, I ensure that the GPU only ever has to hold that one image in memory, no matter which game the user is playing.
### Leaderboard Sorting
**Filter by game**
-      By credits (highest/lowest)
-      By moves made (fewest/most)
-      By date (newest/oldest)


---

## Technical Details

**Display:** 1900×1080 max resolution with scaling support  
**Assets:** Unified texture atlas (DECK0.png - 2600×1500px, 13×5 card layout)  
**Audio:** Sound effects and background music  
**Architecture:** State machine managing game transitions and shared resources

### Current Development Status
🔨 **In Progress:** Restructuring from single-game to multi-game casino platform  
- Core lobby system  
- Game state management  
- Cross-game account integration

---

## Account System

**Default Accounts:** "player one" and "player two" and ai are created on first launch  
**Matchup Rules:** (PvP, PvAI, AIvAI) no playing against yourself
Ai vs Ai will be its own section of the game later for betting (the ai would face themselves in a game of jokers gambit 
player can bet on P1 || P2 to win, watch the game,if win get bet they will get a variable reward based on the bet,,.Money taken first for bets to stop player quitting before losing)

---

## Credits
https://github.com/MarkdownMaverick/jokersgambit/blob/main/credits.md
*For detailed Joker's Gambit rules and mechanics,
**Joker's Gambit v4.55** - [(https://github.com/MarkdownMaverick/jokersgambit.git)]

---

**Michael's Casino – Core Project Description**

Michael's Casino is a stylish, retro-inspired 2D casino game suite built with Raylib (C99), featuring multiple classic and original gambling games in a unified lobby environment.

**Current Status & Focus**  
The project is currently in the **core framework phase** – all foundational systems are being built and polished before integrating the full game modes. The goal is a rock-solid, extensible base that feels premium from the moment the player launches the game.

**In Progress Core Features**
- **Main Menu & Navigation**: Clean main menu with access to Lobby, Accounts, Shop, Settings, Leaderboard, and Achievements. Full keyboard, mouse, and gamepad support (SDL2).
- **Accounts System**: Up to 5 accounts (3 fixed AI opponents + 2 human players). Persistent JSON storage with credits, tokens, stats, and member status tiers (Member → Presidential).
- **Settings**: Music toggle, AI delay, card cover, window scale, fullscreen – all persisted via JSON.
- **Shop**: Token purchase packs with tiered discounts and sell-back option. Modern card-style UI.
- **Achievements**: 50 achievements displayed in a 5×10 grid using a texture atlas. Full persistence via `achievements.json`, with detailed info panel, hover/selection, and debug force-check button.
- **Leaderboard**: Persistent high-score tracking across games.
- **Data Persistence**: Separate JSON files in a `save/` folder for accounts, leaderboard, achievements, and settings – easy to edit manually for testing/cheating.
- **UI Polish**: Consistent colors, hover/selection feedback, navigation sounds, centered layout with dynamic scaling.

**Games in Progress**
- **Joker's Gambit**: Fully built and ready for lobby integration (original card-ranking game with jokers).
- Blackjack and Slot Reels planned next.

**Technical Highlights**
- Raylib for rendering, audio, input
- SDL2 for robust gamepad support
- cJSON for clean, human-readable save files
- Modular structure (main, mainmenu, useraccount, gamepad_sdl)
- Strict warning-as-error compilation for code quality

**Vision**  
A polished, addictive casino experience with deep progression (achievements, member ranks, token economy), local multiplayer feel (vs AI personalities: Bob, Thea, Flint), and a satisfying loop of earning credits/tokens across multiple games. The core is designed to be easily extensible for future additions like Poker, Roulette, etc.
