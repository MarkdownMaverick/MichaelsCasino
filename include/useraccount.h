#ifndef USERACCOUNT_H
#define USERACCOUNT_H

#include "main.h"

// Account initialization
void InitAiAccounts(LobbyState *g);
void InitPlayerAccounts(LobbyState *g);

// I/O Functions
void LoadAllAccounts(LobbyState *g);
void SaveAllAccounts(const LobbyState *g);
void LoadLeaderboard(LobbyState *g);
void SaveLeaderboard(const LobbyState *g);
void CreateDefaultLeaderboard(LobbyState *g);

// Account Management Logic
bool IsNameValid(const char *name);
void LogoutAccount(LobbyState *g, int player);
void LoginAccount(LobbyState *g, int index, int player);
const char* GetPlayerName(const LobbyState *g, int player);
void UpdateAccountCredits(LobbyState *g);
bool IsAlpha(int c);

#endif // USERACCOUNT_H