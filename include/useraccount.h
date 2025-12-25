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
extern Music g_background_music;
void LoadSettings(LobbyState *g);
void SaveSettings(const LobbyState *g);
void LoadAchievements(LobbyState *g);
void SaveAchievements(const LobbyState *g);
const char *GetPlayerNameByIndex(const LobbyState *g, int idx);
bool IsNameValid(const char *name);
void LogoutAccount(LobbyState *g, int player);
void LoginAccount(LobbyState *g, int index, int player);
const char *GetPlayerName(const LobbyState *g, int player);
void UpdateAccountCredits(LobbyState *g);
bool IsAlpha(int c);
const char *GetMemberStatusString(MEMBERSTATUS status);
void InitAchievements(Account *acc);
void UpdateGameStats(Account *acc, GameType game, double win_amount);
void CheckAchievements(Account *acc, const LobbyState *g);
void InitGlobalAchievementDefs(void); 

#endif // USERACCOUNT_H