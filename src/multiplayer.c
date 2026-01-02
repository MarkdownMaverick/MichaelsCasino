#include "multiplayer.h"
#include <stdio.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
typedef int SOCKET;
#endif

void InitNetworkSystem(void)
{
#ifdef _WIN32
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
}

void CloseNetworkSystem(void)
{
#ifdef _WIN32
    WSACleanup();
#endif
}

// Helper to set non-blocking
void SetNonBlocking(int sock)
{
#ifdef _WIN32
    u_long mode = 1;
    ioctlsocket(sock, FIONBIO, &mode);
#else
    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);
#endif
}

// NEW: Start hosting (non-blocking)
bool StartHosting(LobbyState *g, int port)
{
    SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock == INVALID_SOCKET)
        return false;

    // Make socket non-blocking immediately
    SetNonBlocking((int)listen_sock);

    // Allow port reuse
#ifdef _WIN32
    char optval = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
#else
    int optval = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
#endif

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons((uint16_t)port);

    if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        closesocket(listen_sock);
        return false;
    }

    if (listen(listen_sock, 1) == SOCKET_ERROR)
    {
        closesocket(listen_sock);
        return false;
    }

    g->net_listen_socket = (int)listen_sock;
    g->net_role = NET_HOST;
    
    printf("Hosting on port %d (non-blocking)...\n", port);
    return true;
}

// NEW: Check for incoming connection (call every frame)
bool CheckForClient(LobbyState *g)
{
    if (g->net_listen_socket < 0)
        return false;

    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    SOCKET client_sock = accept(g->net_listen_socket, 
                                (struct sockaddr *)&client_addr, 
                                &client_len);

    // Check if we got a connection
    if (client_sock == INVALID_SOCKET)
    {
#ifdef _WIN32
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK)
            return false; // No connection yet, keep waiting
#else
        if (errno == EWOULDBLOCK || errno == EAGAIN)
            return false; // No connection yet, keep waiting
#endif
        
        // Real error
        printf("Accept error\n");
        closesocket(g->net_listen_socket);
        g->net_listen_socket = -1;
        return false;
    }

    // Connection established!
    printf("Client connected!\n");
    
    // Close listen socket (we only accept one client)
    closesocket(g->net_listen_socket);
    g->net_listen_socket = -1;
    
    // Make client socket non-blocking
    SetNonBlocking((int)client_sock);
    
    g->net_socket = (int)client_sock;
    g->net_connected = true;
    g->rng_seed = (unsigned int)time(NULL);
    
    // Send seed to client
    SendPacket(g, PKT_SEED, (int)g->rng_seed);
    
    return true;
}



bool ConnectToGame(LobbyState *g, const char *ip, int port)
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
        return false;

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((uint16_t)port); // Fixed conversion warning
    inet_pton(AF_INET, ip, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        closesocket(sock);
        return false;
    }

    SetNonBlocking((int)sock);

    g->net_socket = (int)sock;
    g->net_connected = true;
    g->net_role = NET_CLIENT;

    return true;
}

void SendPacket(LobbyState *g, PacketType type, int data)
{
    if (!g->net_connected)
        return;
    NetPacket pkt = {type, data};
    send(g->net_socket, (const char *)&pkt, sizeof(NetPacket), 0);
}

bool ReceivePacket(LobbyState *g, NetPacket *out_packet)
{
    if (!g->net_connected)
        return false;
    // Fixed conversion warning: ssize_t to int
    int received = (int)recv(g->net_socket, (char *)out_packet, sizeof(NetPacket), 0);
    return (received == sizeof(NetPacket));
}

void CloseConnection(LobbyState *g)
{
    if (g->net_connected)
    {
        closesocket(g->net_socket);
        g->net_connected = false;
        g->net_role = NET_NONE;
    }
}