#include "TcpServer_Zhb.h"

#include <iostream>

TcpServer_Zhb::TcpServer_Zhb() : m_listenSock(INVALID_SOCKET), m_listening(false) {}

TcpServer_Zhb::~TcpServer_Zhb() { stop(); }

bool TcpServer_Zhb::start(const std::string& ip, unsigned short port)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        std::cout << "[服务器] WSAStartup 失败\n";
        return false;
    }

    m_listenSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_listenSock == INVALID_SOCKET)
    {
        std::cout << "[服务器] 创建套接字失败\n";
        WSACleanup();
        return false;
    }

    sockaddr_in addr = {};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // 允许地址复用,便于重复测试
    int yes = 1;
    setsockopt(m_listenSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

    if (bind(m_listenSock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        std::cout << "[服务器] bind 失败,错误码: " << WSAGetLastError() << "\n";
        closesocket(m_listenSock);
        WSACleanup();
        return false;
    }

    if (listen(m_listenSock, 1) == SOCKET_ERROR)
    {
        std::cout << "[服务器] listen 失败\n";
        closesocket(m_listenSock);
        WSACleanup();
        return false;
    }

    m_listening = true;
    std::cout << "[服务器] 已在 " << ip << ":" << port << " 监听\n";
    return true;
}

// accept 一次 + 回显(阻塞式,放后台线程)
void TcpServer_Zhb::serve()
{
    if (!m_listening) return;

    sockaddr_in clientAddr = {};
    int len = sizeof(clientAddr);
    SOCKET client = accept(m_listenSock, (sockaddr*)&clientAddr, &len);
    if (client == INVALID_SOCKET)
    {
        std::cout << "[服务器] accept 失败,错误码: " << WSAGetLastError() << "\n";
        return;
    }
    std::cout << "[服务器] 收到来自客户端的连接\n";

    echoLoop(client);   // 阻塞直到对端关闭
}

void TcpServer_Zhb::echoLoop(SOCKET client)
{
    char buf[1024];
    while (true)
    {
        int n = recv(client, buf, sizeof(buf) - 1, 0);
        if (n <= 0)
        {
            std::cout << "[服务器] 客户端已断开连接\n";
            break;
        }
        buf[n] = '\0';
        std::cout << "[服务器] 收到消息: " << buf << "\n";

        // 回显
        const char* reply = "Received";
        send(client, reply, (int)strlen(reply), 0);
    }
    closesocket(client);
}

void TcpServer_Zhb::stop()
{
    if (m_listenSock != INVALID_SOCKET)
    {
        closesocket(m_listenSock);
        m_listenSock = INVALID_SOCKET;
    }
    WSACleanup();
    m_listening = false;
}