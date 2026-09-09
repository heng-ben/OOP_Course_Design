#pragma once
#include <string>
#include <vector>

// 群管理模式
enum GroupMode
{
    QQ_MODE,       // QQ 群:可申请加入、可设管理员、可建子群
    WECHAT_MODE    // 微信群:仅群主特权、只能推荐加入、不可建子群
};

// 群多态基类:数据留在基类,行为规则交给派生类重写的虚函数
class Group_Zhb
{
public:
    Group_Zhb();
    virtual ~Group_Zhb();
    Group_Zhb(const Group_Zhb&) = default;              // 供 switchMode 跨类型拷贝
    Group_Zhb& operator=(const Group_Zhb&) = default;

    // ================= 多态行为规则(派生类重写) =================
    virtual GroupMode getMode() const = 0;              // 模式=对象的真实类型
    virtual bool canKick(int operatorId, int targetId) const = 0;   // 谁能踢谁
    virtual bool canJoinByRule(bool isRecommended) const = 0;       // 入群规则
    virtual bool canSetAdmin() const = 0;               // 能否设管理员
    virtual Group_Zhb* createSubGroup(const std::string& subName);  // 建子群,默认nullptr

    // 带 groupId 身份核对的便捷入口(匹配题目签名)
    Group_Zhb* createSubGroup(int groupId, const std::string& subName);

    // ================= 公共数据 get/set =================
    void setGroupId(int groupId);   int getGroupId() const;
    void setGroupName(const std::string& name);  const std::string& getGroupName() const;
    void setOwnerId(int ownerId);   int getOwnerId() const;
    void setParentId(int parentId); int getParentId() const;  // -1 表示非子群

    // ================= 成员 / 管理员公共操作 =================
    bool isMember(int userId) const;
    bool addMember(int userId);               // 加入(重复拒绝)
    bool removeMember(int userId);            // 退群/被踢底层(同步清除管理员身份)
    bool isAdmin(int userId) const;
    bool addAdmin(int userId);                // 设为管理员(须为群成员)
    bool removeAdmin(int userId);
    bool isOwner(int userId) const;
    const std::vector<int>& getMemberIds() const;
    const std::vector<int>& getAdminIds() const;

    // ================= 业务入口(内部按多态规则放行) =================
    bool joinGroup(int userId, int groupId, bool isRecommended);  // QQ任意入;微信须推荐
    bool setAdmin(int groupId, int adminId);                      // 仅QQ可设
    bool kickMember(int operatorId, int targetId);                // 依据 canKick

    // 返回“换成 newMode 后的新对象”,需 Platform 用它替换并 delete 旧指针;
    // 若已是该模式则返回 nullptr(继续用 this,勿删)。
    Group_Zhb* switchMode(GroupMode newMode);

protected:
    void clearAdmins();               // 派生类跨类型拷贝时清空管理员用(微信无管理员)

private:
    int            m_groupId;
    std::string    m_groupName;
    int            m_ownerId;
    std::vector<int> m_memberIds;
    std::vector<int> m_adminIds;
    int            m_parentId;   // -1 表示非子群
};