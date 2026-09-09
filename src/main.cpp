#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "Platform_Zhb.h"
#include "FriendManager_Zhb.h"   // 好友管理(单例,含 QQ/微信 好友图)

using namespace std;

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

// ---------- 健壮输入 ----------
int readInt()
{
    string line;
    while (getline(cin, line))
    {
        istringstream iss(line);
        int v;
        char rest;
        if (iss >> v && !(iss >> rest)) return v;
        cout << "输入无效,请输入一个数字: ";
    }
    return -1;   // EOF
}

bool askYesNo(const string& prompt)
{
    string line;
    while (true)
    {
        cout << prompt << " (y/n): ";
        getline(cin, line);
        if (line == "y" || line == "Y") return true;
        if (line == "n" || line == "N") return false;
        cout << "请回答 y 或 n。\n";
    }
}

// ==================== 注册新用户 ====================
int registerFlow(Platform_Zhb& p)
{
    cout << "\n----- 注册新用户 -----\n";
    int id;
    while (true)
    {
        cout << "请输入号码ID: ";
        id = readInt();
        if (p.findUser(id) != nullptr) { cout << "该ID已被占用,请换一个。\n"; continue; }
        break;
    }
    string nick, birth, loc;
    int tAge;
    cout << "昵称: ";            getline(cin, nick);
    cout << "出生时间(如 20050101): "; getline(cin, birth);
    cout << "所在地(不含空格): ";     getline(cin, loc);
    cout << "T龄(号码申请年限,整数): "; tAge = readInt();

    User_Zhb u;
    u.setId(id);
    u.setNickname(nick);
    u.setBirth(birth);
    u.setLocation(loc);
    u.setTAge(tAge);

    if (p.registerUser(u))
    {
        cout << "注册成功!ID: " << id << " 昵称: " << nick << "\n";
        return id;
    }
    cout << "注册失败。\n";
    return -1;
}

// ==================== 开通服务流程 ====================
void openServiceFlow(Platform_Zhb& p, int uid)
{
    cout << "\n可选服务: 0=QQ  1=微信  2=微博\n请输入要开通的服务: ";
    int s = readInt();
    if (s < 0 || s > 2) { cout << "无效的服务编号。\n"; return; }
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
void friendMenu(Platform_Zhb& p, int uid, int svc)   // svc: QQ 或 微信
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
            cout << "请输入对方用户ID: ";
            int other = readInt();
            if (p.findUser(other) == nullptr) { cout << "该用户不存在。\n"; continue; }
            if (other == uid) { cout << "不能对自己操作。\n"; continue; }

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
        else cout << "无效选项,请重新输入。\n";
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
             << "7. 返回上级\n请选择: ";
        int c = readInt();
        if (c == 7) return;

        if (c == 1)
        {
            cout << "请输入群号: ";  int gid = readInt();
            if (p.findGroup(gid) == nullptr) { cout << "群不存在。\n"; continue; }
            bool rec = askYesNo("是否由群主/好友推荐加入?");
            cout << (p.joinGroup(gid, uid, rec) ? "加入成功。\n"
                                                : "加入失败(可能已在群内或未获推荐)。\n");
        }
        else if (c == 2)
        {
            cout << "请输入要退出的群号: "; int gid = readInt();
            Group_Zhb* g = p.findGroup(gid);
            cout << (g && g->removeMember(uid) ? "已退出该群。\n" : "你不是该群成员或群不存在。\n");
        }
        else if (c == 3)
        {
            cout << "群号: ";  int gid = readInt();
            cout << "被踢用户ID: "; int target = readInt();
            cout << (p.kickMember(gid, uid, target) ? "已踢出。\n"
                      : "无权踢人或目标不在群(群主不可踢,微信群仅群主可踢)。\n");
        }
        else if (c == 4)
        {
            cout << "群号: "; int gid = readInt();
            Group_Zhb* g = p.findGroup(gid);
            if (!g) { cout << "群不存在。\n"; continue; }
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
            cout << "群号: "; int gid = readInt();
            if (!p.findGroup(gid)) { cout << "群不存在。\n"; continue; }
            cout << "切换为 0=QQ群 / 1=微信群: "; int m = readInt();
            GroupMode nm = (m == 0) ? QQ_MODE : WECHAT_MODE;
            Group_Zhb* ng = p.switchMode(gid, nm);
            if (!ng) cout << "无需切换(已是该模式)或群不存在。\n";
            else      cout << "已切换为 " << (ng->getMode() == QQ_MODE ? "QQ" : "微信")
                           << " 模式(成员数据保留)。\n";
        }
        else if (c == 6)
        {
            cout << "父群号: "; int gid = readInt();
            cout << "子群名称: "; string sub; getline(cin, sub);
            Group_Zhb* subg = p.createSubGroup(gid, sub);
            if (!subg) cout << "创建失败:该群不存在或为微信群(仅QQ群可建临时讨论组)。\n";
            else       cout << "已创建临时讨论组,新群号: " << subg->getGroupId() << "\n";
        }
        else cout << "无效选项,请重新输入。\n";
    }
}

// ==================== 用户子菜单 ====================
void userMenu(Platform_Zhb& p, int uid)
{
    const User_Zhb* me = p.findUser(uid);
    while (true)
    {
        cout << "\n======== 用户 [ " << (me ? me->getNickname() : "?") << " ] ========\n"
             << "1. 好友管理\n2. 群管理\n3. 查看已开通服务\n4. 注销(返回登录界面)\n请选择: ";
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

    int current = -1;   // 当前操作账号, -1 = 未登录

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

        if (c == 1)   // ---------- 注册 ----------
        {
            int id = registerFlow(platform);
            if (id != -1) { platform.login(id); current = id; }   // 注册后直接进入
            else continue;
            userMenu(platform, current);   // 进入子菜单
            platform.logout();
            current = -1;
        }
        else if (c == 2)   // ---------- 登录 ----------
        {
            cout << "请输入用户ID: ";
            int id = readInt();
            if (platform.findUser(id) == nullptr) { cout << "该用户不存在,请先注册。\n"; continue; }

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
        else if (c == 3)   // ---------- 开通新服务 ----------
        {
            if (current == -1)
            {
                cout << "请先登录(输入账号ID): ";
                int id = readInt();
                if (platform.findUser(id) == nullptr) { cout << "该用户不存在,请先注册。\n"; continue; }
                platform.login(id);
                current = id;
            }
            openServiceFlow(platform, current);
        }
        else if (c == 4)   // ---------- 查看状态 ----------
        {
            if (current == -1)
            {
                cout << "请先登录(输入账号ID): ";
                int id = readInt();
                if (platform.findUser(id) == nullptr) { cout << "该用户不存在,请先注册。\n"; continue; }
                current = id;
            }
            showServices(platform, current);
        }
        else cout << "无效选项,请重新输入。\n";
    }

    cout << "再见!\n";   // Platform 析构自动 saveData
    return 0;
}