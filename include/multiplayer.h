#ifndef MULTIPLAYER_H
#define MULTIPLAYER_H
#include "main.h"
#ifndef _WIN32
    #include <unistd.h>
    #define closesocket close
#endif
void InitNetworkSystem(void);
void CloseNetworkSystem(void);
bool HostGame(LobbyState *g, int port);
bool ConnectToGame(LobbyState *g, const char *ip, int port);
void NetworkUpdate(LobbyState *g); 
void CloseConnection(LobbyState *g);
void SendPacket(LobbyState *g, PacketType type, int data);
bool ReceivePacket(LobbyState *g, NetPacket *out_packet);
void SetNonBlocking(int sock);
bool StartHosting(LobbyState *g, int port);
bool CheckForClient(LobbyState *g);
#endif