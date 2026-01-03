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
    srand(g->rng_seed); // Host seeds immediately
    // Send seed to client
    SendPacket(g, PKT_SEED, (int)g->rng_seed);
    SendPacket(g, PKT_HANDSHAKE, 1); // Version 1
    SendPacket(g, PKT_PLAYER_NAME, g->p1_account_index);

    return true;
}
bool ConnectToGame(LobbyState *g, const char *ip, int port)
{
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
        return false;

    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((uint16_t)port);

    // Validate IP string
    int ip_ok = inet_pton(AF_INET, ip, &server_addr.sin_addr);
    if (ip_ok <= 0)
    {
        // ip_ok == 0 -> invalid string; ip_ok < 0 -> system error
        printf("ConnectToGame: invalid IP address '%s'\n", ip);
        closesocket(sock);
        return false;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        printf("ConnectToGame: connect() failed\n");
        closesocket(sock);
        return false;
    }

    SetNonBlocking((int)sock);

    SendPacket(g, PKT_HANDSHAKE, 1); // Version 1
    SendPacket(g, PKT_PLAYER_NAME, g->p2_account_index);

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
    size_t to_send = sizeof(NetPacket);

    ssize_t sent = send(g->net_socket,
                        (const char *)&pkt,
                        to_send,
                        0);

    if (sent != (ssize_t)to_send)
    {
        if (sent < 0)
        {
#ifdef _WIN32
            int err = WSAGetLastError();
            printf("SendPacket: send() failed, err=%d, type=%d, data=%d\n",
                   err, (int)type, data);
#else
            printf("SendPacket: send() failed, errno=%d (%s), type=%d, data=%d\n",
                   errno, strerror(errno), (int)type, data);
#endif
        }
        else
        {
            printf("SendPacket: partial send (%zd/%zu bytes), type=%d, data=%d\n",
                   sent, to_send, (int)type, data);
        }
    }
}

bool ReceivePacket(LobbyState *g, NetPacket *out_packet)
{
    if (!g->net_connected)
        return false;

    int received = (int)recv(g->net_socket,
                             (char *)out_packet,
                             sizeof(NetPacket),
                             0);

    if (received == (int)sizeof(NetPacket))
        return true;

    // No data yet on non‑blocking socket: not an error
#ifdef _WIN32
    if (received < 0)
    {
        int err = WSAGetLastError();
        if (err == WSAEWOULDBLOCK)
            return false;
    }
#else
    if (received < 0 && (errno == EWOULDBLOCK || errno == EAGAIN))
        return false;
#endif

    // Anything else: disconnect or fatal error
    CloseConnection(g); // will clear net_connected/net_role
    g->net_socket = -1; // make it obviously invalid
    return false;
}
void CloseConnection(LobbyState *g)
{
    if (g->net_connected)
    {
        closesocket(g->net_socket);
        g->net_connected = false;
        g->net_role = NET_NONE;
        g->net_socket = -1;
    }
}
