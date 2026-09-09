#pragma once
#pragma comment(lib, "ws2_32.lib")

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <string>

// 简单 TCP 客户端:连接服务器并收发消息
class TcpClient_Zhb
{
public:
    TcpClient_Zhb();
    ~TcpClient_Zhb();

    // 连接到 127.0.0.1:8888,成功返回 true
    bool connectTo(const std::string& ip = "127.0.0.1", unsigned short port = 8888);

    // 发送一行文本;服务器会回显 "Received"
    bool sendMessage(const std::string& msg);

    // 接收服务器回显(单次)
    bool receiveMessage(std::string& out);

    bool isConnected() const { return m_sock != INVALID_SOCKET; }
    void disconnect();

private:
    SOCKET m_sock;
};