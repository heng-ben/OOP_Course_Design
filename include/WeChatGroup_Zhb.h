#pragma once
#include "Group_Zhb.h"

// 微信群:只能推荐加入、仅群主特权、无管理员、不可建子群
class WeChatGroup_Zhb : public Group_Zhb
{
public:
    WeChatGroup_Zhb();
    WeChatGroup_Zhb(int groupId, const std::string& groupName, int ownerId);
    WeChatGroup_Zhb(const Group_Zhb& other);   // 从旧对象拷贝→转成微信群

    GroupMode getMode() const override;                 // WECHAT_MODE

    bool canKick(int operatorId, int targetId) const override;   // 仅群主能踢
    bool canJoinByRule(bool isRecommended) const override;       // 必须被推荐
    bool canSetAdmin() const override;                          // 无管理员
};