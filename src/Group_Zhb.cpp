#include "Group_Zhb.h"
#include "QQGroup_Zhb.h"     // switchMode 要构造派生类,故包含其头文件
#include "WeChatGroup_Zhb.h"

#include <algorithm>
#include <iostream>

Group_Zhb::Group_Zhb()
    : m_groupId(0), m_groupName(""), m_ownerId(0), m_parentId(-1)
{
}

Group_Zhb::~Group_Zhb() {}

void Group_Zhb::clearAdmins() { m_adminIds.clear(); }

// ---------------- 数据 get/set ----------------
void Group_Zhb::setGroupId(int g)    { m_groupId = g; }
int  Group_Zhb::getGroupId() const   { return m_groupId; }
void Group_Zhb::setGroupName(const std::string& n) { m_groupName = n; }
const std::string& Group_Zhb::getGroupName() const { return m_groupName; }
void Group_Zhb::setOwnerId(int o) { m_ownerId = o; }
int  Group_Zhb::getOwnerId() const { return m_ownerId; }
void Group_Zhb::setParentId(int p) { m_parentId = p; }
int  Group_Zhb::getParentId() const { return m_parentId; }

// ---------------- 成员 ----------------
bool Group_Zhb::isMember(int userId) const
{
    return std::find(m_memberIds.begin(), m_memberIds.end(), userId) != m_memberIds.end();
}

bool Group_Zhb::addMember(int userId)
{
    if (isMember(userId)) return false;
    m_memberIds.push_back(userId);
    return true;
}

bool Group_Zhb::removeMember(int userId)
{
    auto it = std::find(m_memberIds.begin(), m_memberIds.end(), userId);
    if (it == m_memberIds.end()) return false;
    m_memberIds.erase(it);
    removeAdmin(userId);            // 退群/被踢后同步取消其管理员身份
    return true;
}

const std::vector<int>& Group_Zhb::getMemberIds() const { return m_memberIds; }

// ---------------- 管理员 ----------------
bool Group_Zhb::isAdmin(int userId) const
{
    return std::find(m_adminIds.begin(), m_adminIds.end(), userId) != m_adminIds.end();
}

bool Group_Zhb::addAdmin(int userId)
{
    if (!isMember(userId)) return false;   // 管理员必须是群成员
    if (isAdmin(userId))   return false;   // 已是管理员
    m_adminIds.push_back(userId);
    return true;
}

bool Group_Zhb::removeAdmin(int userId)
{
    auto it = std::find(m_adminIds.begin(), m_adminIds.end(), userId);
    if (it == m_adminIds.end()) return false;
    m_adminIds.erase(it);
    return true;
}

const std::vector<int>& Group_Zhb::getAdminIds() const { return m_adminIds; }

bool Group_Zhb::isOwner(int userId) const { return userId == m_ownerId; }

// ---------------- 建子群(带 groupId 身份核对) ----------------
Group_Zhb* Group_Zhb::createSubGroup(int groupId, const std::string& subName)
{
    if (groupId != m_groupId) return nullptr;   // 防串群
    return createSubGroup(subName);             // 分派给多态版本
}

// 基类默认:不支持建子群(微信群因此返回 nullptr)
Group_Zhb* Group_Zhb::createSubGroup(const std::string&)
{
    return nullptr;
}

// ---------------- 入群(规则走虚函数 canJoinByRule) ----------------
bool Group_Zhb::joinGroup(int userId, int groupId, bool isRecommended)
{
    if (groupId != m_groupId) return false;    // 防串群
    if (!canJoinByRule(isRecommended)) return false;  // QQ永远true;微信须推荐
    return addMember(userId);
}

// ---------------- 设管理员(规则走 canSetAdmin) ----------------
bool Group_Zhb::setAdmin(int groupId, int adminId)
{
    if (groupId != m_groupId) return false;
    if (!canSetAdmin())
    {
        std::cout << "该群为微信模式,无管理员权限(仅QQ群可设管理员)。\n";
        return false;
    }
    return addAdmin(adminId);
}

// ---------------- 踢人(规则走 canKick) ----------------
bool Group_Zhb::kickMember(int operatorId, int targetId)
{
    if (targetId == m_ownerId) return false;   // 群主不可被踢
    if (!canKick(operatorId, targetId)) return false;
    return removeMember(targetId);
}

// ---------------- 模式切换:数据保留,换一个“行为不同”的新对象 ----------------
Group_Zhb* Group_Zhb::switchMode(GroupMode newMode)
{
    if (newMode == getMode())
        return nullptr;                     // 已是指定模式,无需换新对象

    if (newMode == QQ_MODE)
        return new QQGroup_Zhb(*this);      // 拷贝本对象数据 → QQ群对象
    return new WeChatGroup_Zhb(*this);      // 拷贝本对象数据 → 微信群对象
}