#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>

#include "Platform_Zhb.h"
#include "FriendManager_Zhb.h"   // 好友管理(单例,含 QQ/微信 好友图)
#include "TcpServer_Zhb.h"
#include "TcpClient_Zhb.h"

using namespace std;

const int GO_BACK = -1;   // 输入 -1 表示取消本次操作并返回上一级

// ---------- 可开通的服务(用枚举) ----------
enum AccountService
{
    AS_QQ = 0,
    AS_WECHAT = 1,
    AS_WEIBO = 2
};

const char* serviceName(int s)
{
    switch (s)
    {
        case AS_QQ:     return "QQ";
        case AS_WECHAT: return "微信";
        case AS_WEIBO:  return "微博";
        default:        return "未知服务";
    }
}

// ---------- 健壮整数输入 ----------
int readInt()
{
    string line;
    while (getline(cin, line))
    {
        istringstream iss(line);
        int v;
        char rest;
        if (iss >> v && !(iss >> rest)) return v;
        cout << "输入无效,请输入一个数字(或 -1 返回): ";
    }
    return GO_BACK;
}

// y/n 选择:除 y/n 外一律视为“取消/返回”
bool askYesNo(const string& prompt)
{
    string line;
    while (true)
    {
        cout << prompt << " (y/n, 其它输入返回): ";
        getline(cin, line);
        if (line == "y" || line == "Y") return true;
        if (line == "n" || line == "N") return false;
        cout << "已返回。\n";
        return false;
    }
}

void hint(const string& what) { cout << "(输入 -1 返回上级)\n请输入" << what << ": "; }

// ---------- TCP 通信测试(后台起服务器,当前线程当客户端) ----------
void testTcp()
{
    cout << "\n----- 测试 TCP 通信 -----\n";

    TcpServer_Zhb* server = new TcpServer_Zhb();   // 堆上,线程结束后释放
    if (!server->start("127.0.0.1", 8888))
    {
        cout << "服务器启动失败。\n";
        delete server;
        return;
    }

    std::thread serverThread([server]() { server->serve(); });   // accept+回显 在后台

    TcpClient_Zhb client;
    if (client.connectTo("127.0.0.1", 8888))
    {
        cout << "请输入要发送的消息(输入空行结束): ";
        string msg;
        while (getline(cin, msg))          // 支持连发,空行退出
        {
            if (msg.empty()) break;
            client.sendMessage(msg);
            string reply;
            if (client.receiveMessage(reply))
                cout << "[客户端] 收到服务器回显: " << reply << "\n";
            else
            {
                cout << "[客户端] 未收到回显。\n";
                break;
            }
            cout << "再发一条(空行结束): ";
        }
        client.disconnect();
    }

    serverThread.join();       // 客户端断开后服务器线程结束
    server->stop();
    delete server;
    cout << "----- TCP 测试结束 -----\n";
}

// ==================== 注册新用户 ====================
int registerFlow(Platform_Zhb& p)
{
    cout << "\n----- 注册新用户 -----\n";
    int id;
    while (true)
    {
        hint("号码ID");
        id = readInt();
        if (id == GO_BACK) { cout << "已取消注册。\n"; return GO_BACK; }
        if (p.findUser(id) != nullptr) { cout << "该ID已被占用,请换一个。\n"; continue; }
        break;
    }
    string nick, birth, loc;
    cout << "(直接回车可取消)\n";
    cout << "昵称: ";          getline(cin, nick);  if (nick.empty()) { cout << "已取消注册。\n"; return GO_BACK; }
    cout << "出生时间(如20050101): "; getline(cin, birth); if (birth.empty()) { cout << "已取消注册。\n"; return GO_BACK; }
    cout << "所在地(不含空格): ";     getline(cin, loc);  if (loc.empty())  { cout << "已取消注册。\n"; return GO_BACK; }
    cout << "T龄(号码申请年限): ";  int tAge = readInt(); if (tAge == GO_BACK) { cout << "已取消注册。\n"; return GO_BACK; }

    User_Zhb u;
    u.setId(id); u.setNickname(nick); u.setBirth(birth);
    u.setLocation(loc); u.setTAge(tAge);
    if (p.registerUser(u)) { cout << "注册成功!ID: " << id << " 昵称: " << nick << "\n"; return id; }
    cout << "注册失败。\n";
    return GO_BACK;
}

// ==================== 开通服务流程 ====================
void openServiceFlow(Platform_Zhb& p, int uid)
{
    cout << "\n可选服务: 0=QQ  1=微信  2=微博  (-1 返回)\n请输入要开通的服务: ";
    int s = readInt();
    if (s == GO_BACK) return;
    if (s < 0 || s > 2) { cout << "无效服务编号(或输入 -1 返回)。\n"; return; }
    cout << (p.openService(uid, s) ? "开通成功! " : "你已开通过该服务。 ")
         << serviceName(s) << " 现为在线状态。\n";
}

// ==================== 查看服务状态 ====================
void showServices(Platform_Zhb& p, int uid)
{
    vector<int> opened = p.getOpenedServices(uid);
    if (opened.empty()) { cout << "你尚未开通任何服务。\n"; return; }
    cout << "已开通(登录后自动在线)的服务: ";
    for (size_t i = 0; i < opened.size(); ++i) cout << serviceName(opened[i]) << "(在线) ";
    cout << '\n';
}

// ==================== 好友管理子菜单 ====================
void friendMenu(Platform_Zhb& p, int uid, int svc)
{
    FriendManager_Zhb& mgr = FriendManager_Zhb::instance();
    mgr.setActiveService(svc);

    while (true)
    {
        cout << "\n----- 好友管理 (" << serviceName(svc) << ") -----\n"
             << "1. 添加好友   2. 删除好友   3. 查询好友\n"
             << "4. 共同好友   5. 推荐好友   6. 返回上级\n请选择: ";
        int c = readInt();
        if (c == 6) return;

        if (c == 1 || c == 2 || c == 4)
        {
            int other;
            while (true)
            {
                hint("对方用户ID");
                other = readInt();
                if (other == GO_BACK) break;
                if (p.findUser(other) == nullptr) { cout << "该用户不存在,请重输或 -1 返回。\n"; continue; }
                if (other == uid) { cout << "不能对自己操作,请重输或 -1 返回。\n"; continue; }
                break;
            }
            if (other == GO_BACK) continue;

            if (c == 1) { mgr.addFriend(uid, other); cout << "已添加好友 " << other << "\n"; }
            else if (c == 2)
                cout << (mgr.removeFriend(uid, other) ? "已删除好友 " + to_string(other) + "\n"
                                                      : "你们还不是好友。\n");
            else
            {
                vector<int> common = mgr.getCommonFriends(uid, other);
                cout << "共同好友: ";
                if (common.empty()) cout << "(无)";
                else for (size_t i = 0; i < common.size(); ++i) cout << common[i] << ' ';
                cout << '\n';
            }
        }
        else if (c == 3)
        {
            vector<int> fs = mgr.findFriends(uid);
            cout << "我的好友: ";
            if (fs.empty()) cout << "(无)";
            else for (size_t i = 0; i < fs.size(); ++i) cout << fs[i] << ' ';
            cout << '\n';
        }
        else if (c == 5)
        {
            int other = (svc == AS_QQ) ? AS_WECHAT : AS_QQ;
            vector<int> rec = mgr.recommendFriendsByOtherService(uid, other);
            cout << "推荐好友(" << serviceName(other) << "中还不是" << serviceName(svc)
                 << "好友的): ";
            if (rec.empty()) cout << "(无)";
            else for (size_t i = 0; i < rec.size(); ++i) cout << rec[i] << ' ';
            cout << '\n';
        }
        else cout << "无效选项,请重新输入(或选 6 返回)。\n";
    }
}

// ==================== 群管理子菜单 ====================
void groupMenu(Platform_Zhb& p, int uid)
{
    while (true)
    {
        cout << "\n----- 群管理 -----\n"
             << "1. 加入群   2. 退出群   3. 踢人\n"
             << "4. 查询成员   5. 切换群模式   6. 创建临时讨论组(QQ)\n"
             << "7. 测试TCP通信   8. 返回上级\n请选择: ";
        int c = readInt();
        if (c == 8) return;
        if (c == 7) { testTcp(); continue; }

        if (c == 1)
        {
            int gid;
            while (true)
            {
                hint("群号");
                gid = readInt();
                if (gid == GO_BACK) break;
                if (p.findGroup(gid) == nullptr) { cout << "群不存在,请重输或 -1 返回。\n"; continue; }
                break;
            }
            if (gid == GO_BACK) continue;
            bool rec = askYesNo("是否由群主/好友推荐加入?");
            cout << (p.joinGroup(gid, uid, rec) ? "加入成功。\n"
                                                : "加入失败(可能已在群内或未获推荐)。\n");
        }
        else if (c == 2)
        {
            int gid;
            while (true)
            {
                hint("要退出的群号");
                gid = readInt();
                if (gid == GO_BACK) break;
                if (!p.findGroup(gid)) { cout << "群不存在,请重输或 -1 返回。\n"; continue; }
                break;
            }
            if (gid == GO_BACK) continue;
            cout << (p.findGroup(gid)->removeMember(uid) ? "已退出该群。\n" : "你不是该群成员。\n");
        }
        else if (c == 3)
        {
            int gid, target;
            while (true)
            {
                hint("群号");
                gid = readInt();
                if (gid == GO_BACK) break;
                if (!p.findGroup(gid)) { cout << "群不存在,请重输或 -1 返回。\n"; continue; }
                break;
            }
            if (gid == GO_BACK) continue;
            while (true)
            {
                hint("被踢用户ID");
                target = readInt();
                if (target == GO_BACK) break;
                if (target == uid) { cout << "不能踢自己,请重输或 -1 返回。\n"; continue; }
                break;
            }
            if (target == GO_BACK) continue;
            cout << (p.kickMember(gid, uid, target) ? "已踢出。\n"
                      : "无权踢人或目标不在群(群主不可踢,微信群仅群主可踢)。\n");
        }
        else if (c == 4)
        {
            int gid;
            while (true)
            {
                hint("群号");
                gid = readInt();
                if (gid == GO_BACK) break;
                if (!p.findGroup(gid)) { cout << "群不存在,请重输或 -1 返回。\n"; continue; }
                break;
            }
            if (gid == GO_BACK) continue;
            Group_Zhb* g = p.findGroup(gid);
            cout << "群名: " << g->getGroupName()
                 << "  模式: " << (g->getMode() == QQ_MODE ? "QQ" : "微信")
                 << "  群主: " << g->getOwnerId() << "\n成员: ";
            const vector<int>& ms = g->getMemberIds();
            if (ms.empty()) cout << "(空)";
            else for (size_t i = 0; i < ms.size(); ++i) cout << ms[i] << ' ';
            cout << '\n';
        }
        else if (c == 5)
        {
            int gid;
            while (true)
            {
                hint("群号");
                gid = readInt();
                if (gid == GO_BACK) break;
                if (!p.findGroup(gid)) { cout << "群不存在,请重输或 -1 返回。\n"; continue; }
                break;
            }
            if (gid == GO_BACK) continue;
            cout << "切换为 0=QQ群 / 1=微信群 (-1 返回): ";
            int m = readInt();
            if (m == GO_BACK) continue;
            GroupMode nm = (m == 0) ? QQ_MODE : WECHAT_MODE;
            Group_Zhb* ng = p.switchMode(gid, nm);
            if (!ng) cout << "无需切换(已是该模式)。\n";
            else      cout << "已切换为 " << (ng->getMode() == QQ_MODE ? "QQ" : "微信")
                           << " 模式(成员数据保留)。\n";
        }
        else if (c == 6)
        {
            int gid;
            while (true)
            {
                hint("父群号");
                gid = readInt();
                if (gid == GO_BACK) break;
                if (!p.findGroup(gid)) { cout << "群不存在,请重输或 -1 返回。\n"; continue; }
                break;
            }
            if (gid == GO_BACK) continue;
            cout << "子群名称(直接回车返回上级): ";
            string sub; getline(cin, sub);
            if (sub.empty()) continue;
            Group_Zhb* subg = p.createSubGroup(gid, sub);
            if (!subg) cout << "创建失败:该群为微信群(仅QQ群可建临时讨论组)。\n";
            else       cout << "已创建临时讨论组,新群号: " << subg->getGroupId() << "\n";
        }
        else cout << "无效选项,请重新输入(或选 8 返回)。\n";
    }
}

// ==================== 用户子菜单 ====================
void userMenu(Platform_Zhb& p, int uid)
{
    const User_Zhb* me = p.findUser(uid);
    while (true)
    {
        cout << "\n======== 用户 [ " << (me ? me->getNickname() : "?") << " ] ========\n"
             << "1. 好友管理\n2. 群管理\n3. 查看已开通服务\n4. 注销(返回登录界面)\n"
             << "(在任意输入处输 -1 可返回上一级)\n请选择: ";
        int c = readInt();
        if (c == 4) break;

        if (c == 1)
        {
            vector<int> op = p.getOpenedServices(uid);
            int svc = -1;
            for (size_t i = 0; i < op.size(); ++i)
                if (op[i] == AS_QQ || op[i] == AS_WECHAT) { svc = op[i]; break; }
            if (svc == -1) cout << "请先开通 QQ 或 微信 后再进行好友管理。\n";
            else friendMenu(p, uid, svc);
        }
        else if (c == 2) groupMenu(p, uid);
        else if (c == 3) showServices(p, uid);
        else cout << "无效选项,请重新输入。\n";
    }
}

// ==================== 主菜单 + 注册 / 登录 / 开通 ====================
int main()
{
    Platform_Zhb platform;
    platform.loadData("data/users.txt", "data/groups.txt");

    int current = -1;

    while (true)
    {
        cout << "\n===== 欢迎来到立体社交平台 =====\n"
             << "1. 注册新账号\n"
             << "2. 登录\n"
             << "3. 开通新服务\n"
             << "4. 查看我的所有服务状态\n"
             << "0. 退出\n请选择: ";
        int c = readInt();
        if (c == 0) break;

        if (c == 1)
        {
            int id = registerFlow(platform);
            if (id == GO_BACK) continue;
            platform.login(id);
            current = id;
            userMenu(platform, current);
            platform.logout();
            current = -1;
        }
        else if (c == 2)
        {
            int id;
            while (true)
            {
                hint("用户ID");
                id = readInt();
                if (id == GO_BACK) break;
                if (platform.findUser(id) == nullptr) { cout << "该用户不存在,请先注册或 -1 返回。\n"; continue; }
                break;
            }
            if (id == GO_BACK) continue;

            platform.login(id);
            current = id;
            cout << "登录成功。";

            vector<int> opened = platform.getOpenedServices(id);
            if (opened.empty())
            {
                cout << "\n你尚未开通任何微X服务。";
                if (askYesNo("\n是否立即开通?")) openServiceFlow(platform, current);
                cout << "\n进入主功能…\n";
            }
            else
            {
                cout << "\n已自动登录以下服务(均在线): ";
                for (size_t i = 0; i < opened.size(); ++i) cout << serviceName(opened[i]) << ' ';
                cout << '\n';
            }

            userMenu(platform, current);
            platform.logout();
            current = -1;
            cout << "\n已注销,返回登录界面。\n";
        }
        else if (c == 3)
        {
            if (current == -1)
            {
                cout << "尚未登录,请先输入账号ID(-1 返回): ";
                int id = readInt();
                if (id == GO_BACK) continue;
                if (platform.findUser(id) == nullptr) { cout << "该用户不存在。\n"; continue; }
                platform.login(id);
                current = id;
            }
            openServiceFlow(platform, current);
        }
        else if (c == 4)
        {
            if (current == -1)
            {
                cout << "尚未登录,请先输入账号ID(-1 返回): ";
                int id = readInt();
                if (id == GO_BACK) continue;
                if (platform.findUser(id) == nullptr) { cout << "该用户不存在。\n"; continue; }
                current = id;
            }
            showServices(platform, current);
        }
        else cout << "无效选项,请重新输入。\n";
    }

    cout << "再见!\n";
    return 0;
}