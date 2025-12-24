#ifndef MAINMENU_H
#define MAINMENU_H
#include "main.h"
// Main Menu
void DrawMainMenu(const LobbyState *g);
void UpdateMainMenu(LobbyState *g, Vector2 mouse);
// Lobby
void DrawLobby(const LobbyState *g);
void UpdateLobby(LobbyState *g, Vector2 mouse);
// Settings
void DrawSettings(const LobbyState *g);
void UpdateSettings(LobbyState *g, Vector2 mouse);
// Accounts Manager
void DrawAccountsManager(const LobbyState *g);
void UpdateAccountsManager(LobbyState *g, Vector2 mouse);
void ShowAccountStatus(LobbyState *g, const char *msg);
// Shop
void DrawShop(const LobbyState *g);
void UpdateShop(LobbyState *g, Vector2 mouse);
// Leaderboard
void DrawLeaderboard(const LobbyState *g);
void UpdateLeaderboard(LobbyState *g, Vector2 mouse);
void SortLeaderboard(LobbyState *g);
void DrawAchievements(const LobbyState *g);
void UpdateAchievements(LobbyState *g, Vector2 mouse);
extern Sound g_menu_navigate_sound;
extern Sound g_menu_confirm_sound;
extern Sound g_coin_sound;
extern Sound g_place_sound;
#endif // MAINMENU_H