#pragma once
#include <string>
#include <vector>

// 用户实体：平台上的一个账号
class User_Zhb
{
public:
    User_Zhb();
    ~User_Zhb();

    // ---- ID ----
    void setId(int id);
    int  getId() const;

    // ---- 昵称 ----
    void setNickname(const std::string& nickname);
    const std::string& getNickname() const;

    // ---- 出生时间 ----
    void setBirth(const std::string& birth);
    const std::string& getBirth() const;

    // ---- T龄（号码申请时长） ----
    void setTAge(int tAge);
    int  getTAge() const;

    // ---- 所在地 ----
    void setLocation(const std::string& location);
    const std::string& getLocation() const;

    // ---- 好友ID列表 ----
    void setFriendIds(const std::vector<int>& friendIds);
    const std::vector<int>& getFriendIds() const;
    bool addFriend(int friendId);                  // 好友管理辅助
    bool removeFriend(int friendId);

    // ---- 群号列表 ----
    void setGroupIds(const std::vector<int>& groupIds);
    const std::vector<int>& getGroupIds() const;
    bool addGroup(int groupId);                    // 加群
    bool removeGroup(int groupId);                 // 退群

private:
    int            m_id;          // 号码ID
    std::string    m_nickname;    // 昵称
    std::string    m_birth;       // 出生时间
    int            m_tAge;        // T龄
    std::string    m_location;    // 所在地
    std::vector<int> m_friendIds; // 好友ID列表
    std::vector<int> m_groupIds;  // 群号列表
};