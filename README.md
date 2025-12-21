<p align="center">Michael's Casino</p>


<p align="center">
<i>A multi-game casino platform featuring classic card games with persistent accounts and competitive leaderboards</i>
</p>

## About

**MichaelsCasino** is a fork of [Joker's Gambit](https://github.com/[original-repo]) expanded into a full casino experience. Play multiple card games, track your progress across accounts, and compete on global leaderboards.

### Currently Available Games
- **Joker's Gambit** - The original strategic card game (see [original repository](link) for full rules)

### Coming Soon
- **Blackjack** - Classic 21 with AI dealers of varying difficulty
- **Card Slots** - 5-reel poker-based slot machine with traditional hand payouts(JACKS OR BETTER)

---

## Features

✅ **Unified Account System** - One profile tracks credits, tokens, and stats across all games  
✅ **Cross-Game Leaderboard** - Top 100 rankings with game-specific filtering  
✅ **Persistent Economy** - Credits and tokens carry between sessions  
✅ **Multiple AI Opponents/Dealers** - BOB (Hard), THEA (Medium), FLINT (Easy)  
✅ **Shared Asset System** - Efficient texture atlas for all card games  

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

**Default Accounts:** "player one" and "player two" created on first launch  
**Quick Play:** Auto-assigns "player one" vs "BOB" if no accounts selected  
**Matchup Rules:** Any combination allowed (PvP, PvAI, AIvAI) except playing against yourself

---

## Credits
https://github.com/MarkdownMaverick/jokersgambit/blob/main/credits.md
*For detailed Joker's Gambit rules and mechanics,
**Joker's Gambit v4.55** - [(https://github.com/MarkdownMaverick/jokersgambit.git)]

---

