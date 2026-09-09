#pragma once
#include <map>
#include <vector>

// 服务类型标记(用于区分 QQ / 微信 的好友图)
enum ServiceType
{
    SERVICE_QQ = 0,
    SERVICE_WECHAT = 1
};

// 好友管理器(单例):统一维护“按服务区分”的双向好友关系
class FriendManager_Zhb
{
public:
    // 全局唯一实例
    static FriendManager_Zhb& instance();

    // ---- 当前活动服务(决定 add/remove/find/common 操作落在哪张图) ----
    void setActiveService(int serviceType);
    int  getActiveService() const;

    // ---- 双向好友(在 m_activeService 对应的服务图上) ----
    bool addFriend(int userIdA, int userIdB);      // A<->B 互相加为好友
    bool removeFriend(int userIdA, int userIdB);   // 互相删除

    // 返回某用户在“当前活动服务”上的全部好友ID
    std::vector<int> findFriends(int userId) const;

    // A、B 在当前活动服务上的共同好友(两者都认识的人)
    std::vector<int> getCommonFriends(int userIdA, int userIdB) const;

    // 跨服务推荐:取用户在 serviceType(来源服务)的好友,
    // 剔除其已在“当前活动服务”上的好友,返回可加的新朋友。
    std::vector<int> recommendFriendsByOtherService(int userId, int serviceType) const;

private:
    FriendManager_Zhb();               // 单例:私有构造
    ~FriendManager_Zhb();
    FriendManager_Zhb(const FriendManager_Zhb&) = delete;             // 禁拷贝
    FriendManager_Zhb& operator=(const FriendManager_Zhb&) = delete;

    // 返回 (user, service) 对应的好友列表引用(不存在则自动创建空列表)
    std::vector<int>& listRef(int userId, int serviceType);

    int m_activeService;                      // 当前活动服务
    std::map<int, std::vector<int>> m_qqList;      // QQ: userId -> 好友列表
    std::map<int, std::vector<int>> m_wechatList;  // 微信: userId -> 好友列表
};