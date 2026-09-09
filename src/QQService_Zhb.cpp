#include "QQService_Zhb.h"
#include "FriendManager_Zhb.h"

QQService_Zhb::QQService_Zhb()
    : m_ownerId(0), m_friendMgr(&FriendManager_Zhb::instance())
{
}

QQService_Zhb::QQService_Zhb(int ownerId)
    : m_ownerId(ownerId), m_friendMgr(&FriendManager_Zhb::instance())
{
}

std::string QQService_Zhb::getType()
{
    return "QQ";
}

void QQService_Zhb::setOwner(int ownerId) { m_ownerId = ownerId; }
int  QQService_Zhb::getOwner() const       { return m_ownerId; }

void QQService_Zhb::setFriendMgr(FriendManager_Zhb* mgr)
{
    if (mgr) m_friendMgr = mgr;
}

// ---- 好友操作:先把活动服务切到 QQ,再调用管理器 ----
bool QQService_Zhb::addFriend(int otherId)
{
    m_friendMgr->setActiveService(SERVICE_QQ);
    return m_friendMgr->addFriend(m_ownerId, otherId);
}

bool QQService_Zhb::removeFriend(int otherId)
{
    m_friendMgr->setActiveService(SERVICE_QQ);
    return m_friendMgr->removeFriend(m_ownerId, otherId);
}

std::vector<int> QQService_Zhb::getFriendIds()
{
    m_friendMgr->setActiveService(SERVICE_QQ);
    return m_friendMgr->findFriends(m_ownerId);
}

std::vector<int> QQService_Zhb::recommendFromOtherService()
{
    m_friendMgr->setActiveService(SERVICE_QQ);               // 目标 = QQ
    return m_friendMgr->recommendFriendsByOtherService(m_ownerId, SERVICE_WECHAT); // 开采微信
}