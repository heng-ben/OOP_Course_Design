#pragma once
#include "Group_Zhb.h"

// QQ群:可申请加入、可设管理员、可建子群;群主或管理员能踢人
class QQGroup_Zhb : public Group_Zhb
{
public:
    QQGroup_Zhb();                                                // 默认建一个空QQ群
    QQGroup_Zhb(int groupId, const std::string& groupName, int ownerId);
    QQGroup_Zhb(const Group_Zhb& other);   // 从(可能QQ/微信)旧对象拷贝→转成QQ群

    GroupMode getMode() const override;                 // QQ_MODE

    bool canKick(int operatorId, int targetId) const override;    // 群主或管理员可踢
    bool canJoinByRule(bool isRecommended) const override;        // 任何人都能入群
    bool canSetAdmin() const override;                            // 支持设管理员
    Group_Zhb* createSubGroup(const std::string& subName) override; // 可建临时讨论组
};