#include "FriendManager_Zhb.h"

#include <algorithm>   // std::find, std::remove

FriendManager_Zhb& FriendManager_Zhb::instance()
{
    static FriendManager_Zhb s_instance;   // 唯一实例
    return s_instance;
}

FriendManager_Zhb::FriendManager_Zhb()
    : m_activeService(SERVICE_QQ)
{
}

FriendManager_Zhb::~FriendManager_Zhb()
{
}

// 按 (user, service) 取列表引用
std::vector<int>& FriendManager_Zhb::listRef(int userId, int serviceType)
{
    if (serviceType == SERVICE_WECHAT)
        return m_wechatList[userId];   // operator[] 不存在时自动插入空列表
    return m_qqList[userId];
}

// ---- 活动服务 ----
void FriendManager_Zhb::setActiveService(int serviceType)
{
    m_activeService = serviceType;
}

int FriendManager_Zhb::getActiveService() const
{
    return m_activeService;
}

// ---- 双向添加(落在活动服务图上) ----
bool FriendManager_Zhb::addFriend(int userIdA, int userIdB)
{
    if (userIdA == userIdB)
        return false;

    std::vector<int>& la = listRef(userIdA, m_activeService);
    std::vector<int>& lb = listRef(userIdB, m_activeService);

    if (std::find(la.begin(), la.end(), userIdB) == la.end())
        la.push_back(userIdB);              // A 加 B
    if (std::find(lb.begin(), lb.end(), userIdA) == lb.end())
        lb.push_back(userIdA);              // B 加 A(双向)
    return true;
}

// ---- 双向删除 ----
bool FriendManager_Zhb::removeFriend(int userIdA, int userIdB)
{
    bool removed = false;
    std::vector<int>& la = listRef(userIdA, m_activeService);
    auto itA = std::find(la.begin(), la.end(), userIdB);
    if (itA != la.end()) { la.erase(itA); removed = true; }

    std::vector<int>& lb = listRef(userIdB, m_activeService);
    auto itB = std::find(lb.begin(), lb.end(), userIdA);
    if (itB != lb.end()) { lb.erase(itB); removed = true; }
    return removed;
}

// ---- 活动服务上某用户的全部好友 ----
std::vector<int> FriendManager_Zhb::findFriends(int userId) const
{
    // 用 const 路径读,不新增空条目:能查 map 就返回,否则空
    if (m_activeService == SERVICE_WECHAT)
    {
        auto it = m_wechatList.find(userId);
        if (it != m_wechatList.end()) return it->second;
    }
    else
    {
        auto it = m_qqList.find(userId);
        if (it != m_qqList.end()) return it->second;
    }
    return std::vector<int>();
}

// ---- 活动服务上 A、B 的共同好友 ----
std::vector<int> FriendManager_Zhb::getCommonFriends(int userIdA, int userIdB) const
{
    std::vector<int> a = findFriends(userIdA);
    std::vector<int> b = findFriends(userIdB);
    std::vector<int> common;

    // 遍历较短的列表,判断是否也在另一人的好友里
    if (a.size() > b.size()) a.swap(b);
    for (std::size_t i = 0; i < a.size(); ++i)
    {
        if (std::find(b.begin(), b.end(), a[i]) != b.end())
            common.push_back(a[i]);
    }
    return common;
}

// ---- 跨服务推荐 ----
// 说明:目标 = 当前活动服务(想在这上面加好友),来源 = serviceType 指定服务。
// 结果 = 用户在“来源服务”的好友 中,还不是“目标服务”好友的人。
std::vector<int> FriendManager_Zhb::recommendFriendsByOtherService(int userId, int serviceType) const
{
    int target = m_activeService;                 // 活动态即目标服务
    if (target == serviceType)                    // 来源不能等于目标,否则无意义
        return std::vector<int>();

    std::vector<int> pool;        // 来源服务的好友
    if (serviceType == SERVICE_WECHAT)
    {
        auto it = m_wechatList.find(userId);
        if (it != m_wechatList.end()) pool = it->second;
    }
    else
    {
        auto it = m_qqList.find(userId);
        if (it != m_qqList.end()) pool = it->second;
    }

    std::vector<int> existing;    // 目标服务上已有好友
    if (target == SERVICE_WECHAT)
    {
        auto it = m_wechatList.find(userId);
        if (it != m_wechatList.end()) existing = it->second;
    }
    else
    {
        auto it = m_qqList.find(userId);
        if (it != m_qqList.end()) existing = it->second;
    }

    // pool - existing
    std::vector<int> result;
    for (std::size_t i = 0; i < pool.size(); ++i)
        if (std::find(existing.begin(), existing.end(), pool[i]) == existing.end())
            result.push_back(pool[i]);
    return result;
}