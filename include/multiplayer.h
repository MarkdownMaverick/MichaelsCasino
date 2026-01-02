#ifndef MULTIPLAYER_H
#define MULTIPLAYER_H

#include "main.h"

// Platform independent socket init
void InitNetworkSystem(void);
void CloseNetworkSystem(void);

// Host/Client logic
bool HostGame(LobbyState *g, int port);
bool ConnectToGame(LobbyState *g, const char *ip, int port);
void NetworkUpdate(LobbyState *g); // Call this every frame in game
void CloseConnection(LobbyState *g);

// Data transmission
void SendPacket(LobbyState *g, PacketType type, int data);
bool ReceivePacket(LobbyState *g, NetPacket *out_packet);

// Helper prototype
void SetNonBlocking(int sock);

#endif