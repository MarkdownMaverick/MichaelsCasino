#ifndef MAINMENU_H
#define MAINMENU_H
#include "main.h"
#include "jokersgambit.h"
// Main Menu
void DrawMainMenu(const LobbyState *g);
void UpdateMainMenu(LobbyState *g, Vector2 mouse);
// Lobby
void DrawLobby(const LobbyState *g);
void UpdateLobby(LobbyState *g, Vector2 mouse);
void SwitchState(LobbyState *g, UIState newState);
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
void ShowNotification(LobbyState *g, const char *title, const char *subtitle);
void DrawNotification(LobbyState *g);
void DrawModeSelection(const LobbyState *g);
void UpdateModeSelection(LobbyState *g, Vector2 mouse);
void DrawAISelection(const LobbyState *g);
void UpdateAISelection(LobbyState *g, Vector2 mouse);
void UpdatePVPSetupP1(LobbyState *g);
void DrawPVPSetupP1(const LobbyState *g);
void DrawPVPSetupP2(const LobbyState *g);

void DrawMultiplayerMode(const LobbyState *g);
void UpdateMultiplayer(LobbyState *g);
void DrawOnlineChoice(const LobbyState *g);
void UpdateOnlineChoice(LobbyState *g);
void UpdatePVPSetupP2(LobbyState *core);
void DrawHostingWaiting(const LobbyState *g);
void UpdateHostingWaiting(LobbyState *g);
void DrawBettingSetup(const LobbyState *g);
void UpdateBettingSetup(LobbyState *g, Vector2 mouse);
void SetupBettingMatch(LobbyState *g);
#endif // MAINMENU_H