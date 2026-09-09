#include "TcpClient_Zhb.h"

#include <cstring>
#include <iostream>

TcpClient_Zhb::TcpClient_Zhb() : m_sock(INVALID_SOCKET) {}

TcpClient_Zhb::~TcpClient_Zhb() { disconnect(); }

bool TcpClient_Zhb::connectTo(const std::string& ip, unsigned short port)
{
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        std::cout << "[客户端] WSAStartup 失败\n";
        return false;
    }

    m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (m_sock == INVALID_SOCKET)
    {
        std::cout << "[客户端] 创建套接字失败\n";
        WSACleanup();
        return false;
    }

    sockaddr_in addr = {};
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (::connect(m_sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        std::cout << "[客户端] 连接失败,错误码: " << WSAGetLastError()
                  << " (服务器是否已启动?)\n";
        closesocket(m_sock);
        m_sock = INVALID_SOCKET;
        WSACleanup();
        return false;
    }
    std::cout << "[客户端] 已连接 " << ip << ":" << port << "\n";
    return true;
}

bool TcpClient_Zhb::sendMessage(const std::string& msg)
{
    int sent = send(m_sock, msg.c_str(), (int)msg.size(), 0);
    return sent == (int)msg.size();
}

bool TcpClient_Zhb::receiveMessage(std::string& out)
{
    char buf[1024];
    int n = recv(m_sock, buf, sizeof(buf) - 1, 0);
    if (n <= 0) return false;
    buf[n] = '\0';
    out = buf;
    return true;
}

void TcpClient_Zhb::disconnect()
{
    if (m_sock != INVALID_SOCKET)
    {
        closesocket(m_sock);
        m_sock = INVALID_SOCKET;
        WSACleanup();
    }
}