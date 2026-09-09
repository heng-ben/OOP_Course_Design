#pragma once
#pragma comment(lib, "ws2_32.lib")

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <string>

// 简单 TCP 点对点服务器:bind -> listen -> accept -> 回显
class TcpServer_Zhb
{
public:
    TcpServer_Zhb();
    ~TcpServer_Zhb();

    // 初始化并 bind + listen(默认 127.0.0.1:8888),成功返回 true
    bool start(const std::string& ip = "127.0.0.1", unsigned short port = 8888);

    // 接受一个客户端并进入回显循环(会阻塞,需放到后台线程调用)
    void serve();

    // 关闭监听与连接
    void stop();

    SOCKET getListenSocket() const { return m_listenSock; }

private:
    void echoLoop(SOCKET client);

    SOCKET m_listenSock;    // 监听套接字
    bool   m_listening;     // 是否已处于监听状态
};