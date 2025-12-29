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
// Account Management Logic
extern Music g_background_music;
void LoadSettings(LobbyState *g);
void SaveSettings(const LobbyState *g);
const char *GetPlayerNameByIndex(const LobbyState *g, int idx);
bool IsNameValid(const char *name);
void LogoutAccount(LobbyState *g, int player);
void LoginAccount(LobbyState *g, int index, int player);
const char *GetPlayerName(const LobbyState *g, int player);
void UpdateAccountCredits(LobbyState *g);
bool IsAlpha(int c);
const char *GetMemberStatusString(MEMBERSTATUS status);
void UpdateGameStats(LobbyState *g, int account_index, GameType game, double win_amount);

void InitAchievements(Account *acc);
void LoadAchievements(LobbyState *g);
void SaveAchievements(const LobbyState *g);
void CheckAchievements(Account *acc, LobbyState *g);
void InitGlobalAchievementDefs(void); 
void AutoLogoutP2(LobbyState *g);
#endif // USERACCOUNT_H