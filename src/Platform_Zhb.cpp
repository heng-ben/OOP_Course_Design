#include "Platform_Zhb.h"
#include "QQGroup_Zhb.h"      // loadData 里按模式 new 出具体子类
#include "WeChatGroup_Zhb.h"

#include <fstream>
#include <sstream>
#include <iostream>

namespace
{
    std::string trim(const std::string& s)
    {
        std::string t = s;
        while (!t.empty() && (t.front() == ' ' || t.front() == '\t')) t.erase(t.begin());
        while (!t.empty() && (t.back()  == ' ' || t.back()  == '\t')) t.pop_back();
        return t;
    }

    std::string joinInts(const std::vector<int>& v)
    {
        std::string out;
        for (std::size_t i = 0; i < v.size(); ++i)
        {
            if (i) out += ",";
            out += std::to_string(v[i]);
        }
        return out;
    }
}

// ---------------- 构造 / 析构 ----------------
Platform_Zhb::Platform_Zhb() : m_currentUserId(-1) {}

Platform_Zhb::~Platform_Zhb()
{
    for (std::size_t i = 0; i < m_groupList.size(); ++i)
        delete m_groupList[i];   // 释放群对象
    saveData();                  // 析构时写回,实现断电保存
}

// ---------------- 数据加载 ----------------
void Platform_Zhb::loadData(const std::string& userFile, const std::string& groupFile)
{
    m_userFile  = userFile;
    m_groupFile = groupFile;

    // 清空旧数据(含释放已存在的群对象)
    for (std::size_t i = 0; i < m_groupList.size(); ++i) delete m_groupList[i];
    m_groupList.clear();
    m_userList.clear();

    // ---- 读 users.txt:ID 昵称 出生时间 T龄 所在地 ----
    std::ifstream uin(userFile.c_str());
    std::string line;
    while (std::getline(uin, line))
    {
        if (trim(line).empty()) continue;
        std::istringstream iss(line);
        int id = 0, tAge = 0;
        std::string nick, birth;
        iss >> id >> nick >> birth >> tAge;
        std::string rest; std::getline(iss, rest);

        User_Zhb u;
        u.setId(id); u.setNickname(nick); u.setBirth(birth);
        u.setTAge(tAge); u.setLocation(trim(rest));
        m_userList.push_back(u);
    }
    uin.close();

    // ---- 读 groups.txt:群号 群名 群主ID 模式(0=QQ/1=微信) 成员ID1,... ----
    std::ifstream gin(groupFile.c_str());
    while (std::getline(gin, line))
    {
        if (trim(line).empty()) continue;
        std::istringstream iss(line);
        int gid = 0, owner = 0, modeInt = 0;
        std::string gname;
        iss >> gid >> gname >> owner >> modeInt;

        // 按模式 new 出具体子类(多态)
        Group_Zhb* g = nullptr;
        if (modeInt == 0) g = new QQGroup_Zhb(gid, gname, owner);
        else              g = new WeChatGroup_Zhb(gid, gname, owner);

        std::string csv;
        if (iss >> csv)
        {
            std::stringstream ss(csv);
            std::string token;
            while (std::getline(ss, token, ','))
                if (!token.empty()) g->addMember(std::stoi(token));
        }
        m_groupList.push_back(g);
    }
    gin.close();
}

// ---------------- 数据保存(覆盖写回原文件) ----------------
void Platform_Zhb::saveData()
{
    if (m_userFile.empty() || m_groupFile.empty()) return;

    std::ofstream uout(m_userFile.c_str());
    for (std::size_t i = 0; i < m_userList.size(); ++i)
    {
        const User_Zhb& u = m_userList[i];
        uout << u.getId() << ' ' << u.getNickname() << ' '
             << u.getBirth() << ' ' << u.getTAge() << ' '
             << u.getLocation() << '\n';
    }
    uout.close();

    std::ofstream gout(m_groupFile.c_str());
    for (std::size_t i = 0; i < m_groupList.size(); ++i)
    {
        Group_Zhb* g = m_groupList[i];
        gout << g->getGroupId() << ' ' << g->getGroupName() << ' '
             << g->getOwnerId() << ' '
             << (g->getMode() == QQ_MODE ? 0 : 1) << ' '   // 虚函数取模式
             << joinInts(g->getMemberIds()) << '\n';
    }
    gout.close();
}

// ---------------- 用户管理 ----------------
bool Platform_Zhb::registerUser(const User_Zhb& user)
{
    if (findUser(user.getId()) != nullptr) return false;
    m_userList.push_back(user);
    return true;
}

bool Platform_Zhb::removeUser(int userId)
{
    for (auto it = m_userList.begin(); it != m_userList.end(); ++it)
    {
        if (it->getId() == userId)
        {
            m_userList.erase(it);
            if (m_currentUserId == userId) m_currentUserId = -1;
            return true;
        }
    }
    return false;
}

User_Zhb* Platform_Zhb::findUser(int userId)
{
    for (std::size_t i = 0; i < m_userList.size(); ++i)
        if (m_userList[i].getId() == userId) return &m_userList[i];
    return nullptr;
}

const User_Zhb* Platform_Zhb::findUser(int userId) const
{
    for (std::size_t i = 0; i < m_userList.size(); ++i)
        if (m_userList[i].getId() == userId) return &m_userList[i];
    return nullptr;
}

std::vector<User_Zhb>& Platform_Zhb::getAllUsers() { return m_userList; }
// ================= 服务开通管理 =================
bool Platform_Zhb::openService(int userId, int serviceId)
{
    // 已开通则不再重复添加
    std::vector<int>& opened = m_userOpened[userId];
    for (std::size_t i = 0; i < opened.size(); ++i)
        if (opened[i] == serviceId) return false;
    opened.push_back(serviceId);
    return true;
}

bool Platform_Zhb::isServiceOpened(int userId, int serviceId) const
{
    auto it = m_userOpened.find(userId);
    if (it == m_userOpened.end()) return false;
    for (std::size_t i = 0; i < it->second.size(); ++i)
        if (it->second[i] == serviceId) return true;
    return false;
}

std::vector<int> Platform_Zhb::getOpenedServices(int userId) const
{
    auto it = m_userOpened.find(userId);
    if (it == m_userOpened.end()) return std::vector<int>();
    return it->second;
}

// ---------------- 登录管理 ----------------
bool Platform_Zhb::login(int userId)
{
    if (findUser(userId) == nullptr) return false;
    m_currentUserId = userId;
    return true;
}

void Platform_Zhb::logout() { m_currentUserId = -1; }
int  Platform_Zhb::getCurrentUserId() const { return m_currentUserId; }

// ---------------- 群管理 ----------------
bool Platform_Zhb::addGroup(Group_Zhb* group)
{
    if (!group) return false;
    if (findGroup(group->getGroupId()) != nullptr) { delete group; return false; } // 群号重复
    m_groupList.push_back(group);
    return true;
}

bool Platform_Zhb::removeGroup(int groupId)
{
    for (auto it = m_groupList.begin(); it != m_groupList.end(); ++it)
    {
        if ((*it)->getGroupId() == groupId)
        {
            delete *it;
            m_groupList.erase(it);
            return true;
        }
    }
    return false;
}

Group_Zhb* Platform_Zhb::findGroup(int groupId)
{
    for (std::size_t i = 0; i < m_groupList.size(); ++i)
        if (m_groupList[i]->getGroupId() == groupId) return m_groupList[i];
    return nullptr;
}

const Group_Zhb* Platform_Zhb::findGroup(int groupId) const
{
    for (std::size_t i = 0; i < m_groupList.size(); ++i)
        if (m_groupList[i]->getGroupId() == groupId) return m_groupList[i];
    return nullptr;
}

std::vector<Group_Zhb*>& Platform_Zhb::getAllGroups() { return m_groupList; }

// ---------------- 常用群操作(内部转调多态方法) ----------------
bool Platform_Zhb::joinGroup(int groupId, int userId, bool isRecommended)
{
    Group_Zhb* g = findGroup(groupId);
    return g ? g->joinGroup(userId, groupId, isRecommended) : false;
}

bool Platform_Zhb::setAdmin(int groupId, int adminId)
{
    Group_Zhb* g = findGroup(groupId);
    return g ? g->setAdmin(groupId, adminId) : false;
}

bool Platform_Zhb::kickMember(int groupId, int operatorId, int targetId)
{
    Group_Zhb* g = findGroup(groupId);
    return g ? g->kickMember(operatorId, targetId) : false;
}

// ---------------- 建子群 ----------------
Group_Zhb* Platform_Zhb::createSubGroup(int groupId, const std::string& subName)
{
    Group_Zhb* g = findGroup(groupId);
    if (!g) return nullptr;
    Group_Zhb* sub = g->createSubGroup(groupId, subName);  // 微信返回 nullptr
    if (!sub) return nullptr;
    sub->setGroupId(allocGroupId());          // 分配唯一群号
    m_groupList.push_back(sub);               // 登记进列表,由 Platform 拥有
    return sub;
}

// ---------------- 模式切换 ----------------
Group_Zhb* Platform_Zhb::switchMode(int groupId, GroupMode newMode)
{
    for (std::size_t i = 0; i < m_groupList.size(); ++i)
    {
        Group_Zhb* g = m_groupList[i];
        if (g->getGroupId() == groupId)
        {
            Group_Zhb* ng = g->switchMode(newMode);   // 同模式→nullptr
            if (!ng) return nullptr;
            delete g;                 // 释放旧对象
            m_groupList[i] = ng;      // 列表指向新对象(成员数据已保留)
            return ng;
        }
    }
    return nullptr;
}

// ---------------- 私有:生成未占用的群号 ----------------
int Platform_Zhb::allocGroupId() const
{
    int candidate = 1;
    bool taken = true;
    while (taken)
    {
        taken = false;
        for (std::size_t i = 0; i < m_groupList.size(); ++i)
            if (m_groupList[i]->getGroupId() == candidate) { taken = true; ++candidate; break; }
    }
    return candidate;
}