#include "Platform_Zhb.h"

#include <algorithm>   // std::find
#include <fstream>
#include <sstream>

// ================= 文件内部辅助函数（匿名命名空间） =================
namespace
{
    // 去掉字符串首尾空白（用于「所在地」这类行尾字段）
    std::string trim(const std::string& s)
    {
        std::string t = s;
        while (!t.empty() && (t.front() == ' ' || t.front() == '\t')) t.erase(t.begin());
        while (!t.empty() && (t.back()  == ' ' || t.back()  == '\t')) t.pop_back();
        return t;
    }

    // 把 int 列表拼成 "a,b,c"，空列表返回空串
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
Platform_Zhb::Platform_Zhb()
    : m_currentUserId(-1)   // -1 表示未登录
{
}

Platform_Zhb::~Platform_Zhb()
{
    saveData();   // 析构时自动写回，实现断电保存
}

// ---------------- 数据加载 ----------------
void Platform_Zhb::loadData(const std::string& userFile, const std::string& groupFile)
{
    m_userFile  = userFile;    // 记住路径，供 saveData 写回
    m_groupFile = groupFile;

    // 清空旧数据
    m_userList.clear();
    m_groupList.clear();

    // ---- 读 users.txt：ID 昵称 出生时间 T龄 所在地 ----
    std::ifstream uin(userFile.c_str());
    std::string line;
    while (std::getline(uin, line))
    {
        if (trim(line).empty()) continue;              // 跳过空行
        std::istringstream iss(line);
        int  id = 0, tAge = 0;
        std::string nick, birth, location;
        iss >> id >> nick >> birth >> tAge;            // 取前 4 个字段
        std::string rest;
        std::getline(iss, rest);                       // 剩余作为所在地（可含空格）
        location = trim(rest);

        User_Zhb u;
        u.setId(id);
        u.setNickname(nick);
        u.setBirth(birth);
        u.setTAge(tAge);
        u.setLocation(location);
        m_userList.push_back(u);
    }
    uin.close();

    // ---- 读 groups.txt：群号 群名 群主ID 模式(0/1) 成员ID1,ID2,... ----
    std::ifstream gin(groupFile.c_str());
    while (std::getline(gin, line))
    {
        if (trim(line).empty()) continue;
        std::istringstream iss(line);
        int gid = 0, owner = 0, modeInt = 0;
        std::string gname;
        iss >> gid >> gname >> owner >> modeInt;

        Group_Zhb g;
        g.setGroupId(gid);
        g.setGroupName(gname);
        g.setOwnerId(owner);
        g.setMode(modeInt == 0 ? QQ_MODE : WECHAT_MODE);

        // 读取成员ID串（逗号分隔，可能为空）
        std::string csv;
        if (iss >> csv)
        {
            std::stringstream ss(csv);
            std::string token;
            while (std::getline(ss, token, ','))
            {
                if (!token.empty())
                    g.addMember(std::stoi(token));
            }
        }
        m_groupList.push_back(g);
    }
    gin.close();
}

// ---------------- 数据保存（覆盖写回原文件） ----------------
void Platform_Zhb::saveData()
{
    if (m_userFile.empty() || m_groupFile.empty())
        return;   // 从未 loadData 过，无目标文件可写

    // 写回 users.txt
    std::ofstream uout(m_userFile.c_str());
    for (std::size_t i = 0; i < m_userList.size(); ++i)
    {
        const User_Zhb& u = m_userList[i];
        uout << u.getId() << ' '
             << u.getNickname() << ' '
             << u.getBirth() << ' '
             << u.getTAge() << ' '
             << u.getLocation() << '\n';
    }
    uout.close();

    // 写回 groups.txt
    std::ofstream gout(m_groupFile.c_str());
    for (std::size_t i = 0; i < m_groupList.size(); ++i)
    {
        const Group_Zhb& g = m_groupList[i];
        gout << g.getGroupId() << ' '
             << g.getGroupName() << ' '
             << g.getOwnerId() << ' '
             << (g.getMode() == QQ_MODE ? 0 : 1) << ' '
             << joinInts(g.getMemberIds()) << '\n';
    }
    gout.close();
}

// ---------------- 用户管理 ----------------
bool Platform_Zhb::registerUser(const User_Zhb& user)
{
    if (findUser(user.getId()) != nullptr)   // ID 不能重复
        return false;
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

const User_Zhb* Platform_Zhb::findUser(int userId) const
{
    for (std::size_t i = 0; i < m_userList.size(); ++i)
        if (m_userList[i].getId() == userId)
            return &m_userList[i];
    return nullptr;
}

std::vector<User_Zhb>& Platform_Zhb::getAllUsers() { return m_userList; }

// ---------------- 登录管理 ----------------
bool Platform_Zhb::login(int userId)
{
    if (findUser(userId) == nullptr)
        return false;                 // 用户不存在
    m_currentUserId = userId;         // 同一平台其它服务视为自动登录
    return true;
}

void Platform_Zhb::logout()
{
    m_currentUserId = -1;
}

int Platform_Zhb::getCurrentUserId() const { return m_currentUserId; }

// ---------------- 群管理 ----------------
bool Platform_Zhb::addGroup(const Group_Zhb& group)
{
    if (findGroup(group.getGroupId()) != nullptr)   // 群号不能重复
        return false;
    m_groupList.push_back(group);
    return true;
}

bool Platform_Zhb::removeGroup(int groupId)
{
    for (auto it = m_groupList.begin(); it != m_groupList.end(); ++it)
    {
        if (it->getGroupId() == groupId)
        {
            m_groupList.erase(it);
            return true;
        }
    }
    return false;
}

const Group_Zhb* Platform_Zhb::findGroup(int groupId) const
{
    for (std::size_t i = 0; i < m_groupList.size(); ++i)
        if (m_groupList[i].getGroupId() == groupId)
            return &m_groupList[i];
    return nullptr;
}

std::vector<Group_Zhb>& Platform_Zhb::getAllGroups() { return m_groupList; }