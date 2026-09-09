#include "WeChatGroup_Zhb.h"

WeChatGroup_Zhb::WeChatGroup_Zhb() {}

WeChatGroup_Zhb::WeChatGroup_Zhb(int groupId, const std::string& groupName, int ownerId)
{
    setGroupId(groupId);
    setGroupName(groupName);
    setOwnerId(ownerId);
}

WeChatGroup_Zhb::WeChatGroup_Zhb(const Group_Zhb& other) : Group_Zhb(other)
{
    clearAdmins();   // 转成微信群后不再有管理员(仅群主特权)
}

GroupMode WeChatGroup_Zhb::getMode() const { return WECHAT_MODE; }

// 仅群主本人可以踢人,且不能踢自己(群主)
bool WeChatGroup_Zhb::canKick(int operatorId, int targetId) const
{
    if (targetId == getOwnerId()) return false;
    if (!isMember(targetId))       return false;
    return operatorId == getOwnerId();   // 管理员不存在
}

// 微信群:只有被推荐(isRecommended==true)才能加入
bool WeChatGroup_Zhb::canJoinByRule(bool isRecommended) const
{
    return isRecommended;
}

// 微信群无管理员概念
bool WeChatGroup_Zhb::canSetAdmin() const { return false; }