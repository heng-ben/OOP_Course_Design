#include "User_Zhb.h"

#include <algorithm>   // std::find

// ---------------- 构造 / 析构 ----------------
User_Zhb::User_Zhb()
    : m_id(0), m_nickname(""), m_birth(""), m_tAge(0), m_location("")
{
    // vector 默认已为空
}

User_Zhb::~User_Zhb()
{
    // 无动态资源，空实现即可
}

// ---------------- ID ----------------
void User_Zhb::setId(int id)   { m_id = id; }
int  User_Zhb::getId() const   { return m_id; }

// ---------------- 昵称 ----------------
void User_Zhb::setNickname(const std::string& nickname) { m_nickname = nickname; }
const std::string& User_Zhb::getNickname() const        { return m_nickname; }

// ---------------- 出生时间 ----------------
void User_Zhb::setBirth(const std::string& birth) { m_birth = birth; }
const std::string& User_Zhb::getBirth() const     { return m_birth; }

// ---------------- T龄 ----------------
void User_Zhb::setTAge(int tAge) { m_tAge = tAge; }
int  User_Zhb::getTAge() const   { return m_tAge; }

// ---------------- 所在地 ----------------
void User_Zhb::setLocation(const std::string& location) { m_location = location; }
const std::string& User_Zhb::getLocation() const        { return m_location; }

// ---------------- 好友ID列表 ----------------
void User_Zhb::setFriendIds(const std::vector<int>& friendIds) { m_friendIds = friendIds; }
const std::vector<int>& User_Zhb::getFriendIds() const         { return m_friendIds; }

bool User_Zhb::addFriend(int friendId)
{
    // 已存在则不重复添加
    if (std::find(m_friendIds.begin(), m_friendIds.end(), friendId) != m_friendIds.end())
        return false;
    m_friendIds.push_back(friendId);
    return true;
}

bool User_Zhb::removeFriend(int friendId)
{
    auto it = std::find(m_friendIds.begin(), m_friendIds.end(), friendId);
    if (it == m_friendIds.end())
        return false;
    m_friendIds.erase(it);
    return true;
}

// ---------------- 群号列表 ----------------
void User_Zhb::setGroupIds(const std::vector<int>& groupIds) { m_groupIds = groupIds; }
const std::vector<int>& User_Zhb::getGroupIds() const        { return m_groupIds; }

bool User_Zhb::addGroup(int groupId)
{
    if (std::find(m_groupIds.begin(), m_groupIds.end(), groupId) != m_groupIds.end())
        return false;
    m_groupIds.push_back(groupId);
    return true;
}

bool User_Zhb::removeGroup(int groupId)
{
    auto it = std::find(m_groupIds.begin(), m_groupIds.end(), groupId);
    if (it == m_groupIds.end())
        return false;
    m_groupIds.erase(it);
    return true;
}