#include "QQGroup_Zhb.h"

QQGroup_Zhb::QQGroup_Zhb() {}

QQGroup_Zhb::QQGroup_Zhb(int groupId, const std::string& groupName, int ownerId)
{
    setGroupId(groupId);
    setGroupName(groupName);
    setOwnerId(ownerId);
}

QQGroup_Zhb::QQGroup_Zhb(const Group_Zhb& other) : Group_Zhb(other)
{
    // 保留 other 的成员/管理员(跨类型切换时数据不丢)
}

GroupMode QQGroup_Zhb::getMode() const { return QQ_MODE; }

// 群主本人,或该群管理员,可以踢某群成员(但不能踢群主)
bool QQGroup_Zhb::canKick(int operatorId, int targetId) const
{
    if (targetId == getOwnerId()) return false;
    if (!isMember(targetId))       return false;
    return (operatorId == getOwnerId()) || isAdmin(operatorId);
}

// QQ群:任意用户都可直接加入,不要求推荐
bool QQGroup_Zhb::canJoinByRule(bool isRecommended) const
{
    (void)isRecommended;   // 忽略
    return true;
}

bool QQGroup_Zhb::canSetAdmin() const { return true; }

// QQ群可创建临时讨论组(子群),带 parentId
Group_Zhb* QQGroup_Zhb::createSubGroup(const std::string& subName)
{
    QQGroup_Zhb* sub = new QQGroup_Zhb();
    sub->setGroupName(subName);      // 子群名
    sub->setOwnerId(getOwnerId());   // 群主沿用
    sub->setParentId(getGroupId());  // 记录父群号
    sub->setGroupId(0);              // 群号待 Platform 分配唯一值后 setGroupId
    return sub;                      // 空成员的新子群对象
}