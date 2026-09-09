#include "WeChatService_Zhb.h"
#include "FriendManager_Zhb.h"

WeChatService_Zhb::WeChatService_Zhb()
    : m_ownerId(0), m_friendMgr(&FriendManager_Zhb::instance())
{
}

WeChatService_Zhb::WeChatService_Zhb(int ownerId)
    : m_ownerId(ownerId), m_friendMgr(&FriendManager_Zhb::instance())
{
}

std::string WeChatService_Zhb::getType()
{
    return "WeChat";
}

void WeChatService_Zhb::setOwner(int ownerId) { m_ownerId = ownerId; }
int  WeChatService_Zhb::getOwner() const       { return m_ownerId; }

void WeChatService_Zhb::setFriendMgr(FriendManager_Zhb* mgr)
{
    if (mgr) m_friendMgr = mgr;
}

// ---- 好友操作:先把活动服务切到微信,再调用管理器 ----
bool WeChatService_Zhb::addFriend(int otherId)
{
    m_friendMgr->setActiveService(SERVICE_WECHAT);
    return m_friendMgr->addFriend(m_ownerId, otherId);
}

bool WeChatService_Zhb::removeFriend(int otherId)
{
    m_friendMgr->setActiveService(SERVICE_WECHAT);
    return m_friendMgr->removeFriend(m_ownerId, otherId);
}

std::vector<int> WeChatService_Zhb::getFriendIds()
{
    m_friendMgr->setActiveService(SERVICE_WECHAT);
    return m_friendMgr->findFriends(m_ownerId);
}

std::vector<int> WeChatService_Zhb::recommendFromOtherService()
{
    m_friendMgr->setActiveService(SERVICE_WECHAT);            // 目标 = 微信
    return m_friendMgr->recommendFriendsByOtherService(m_ownerId, SERVICE_QQ); // 开采QQ
}