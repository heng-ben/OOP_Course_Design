#pragma once
#include <string>
#include <vector>

#include "User_Zhb.h"
#include "Group_Zhb.h"

// 平台总控类：管理全体用户、全体群及当前登录用户
class Platform_Zhb
{
public:
    Platform_Zhb();
    ~Platform_Zhb();   // 析构时调用 saveData() 写回原文件

    // ---- 数据持久化（I/O） ----
    void loadData(const std::string& userFile, const std::string& groupFile); // 启动时从文件读入
    void saveData();                                                          // 结束/析构时写回

    // ---- 用户管理 ----
    bool registerUser(const User_Zhb& user);      // 注册/开通账号
    bool removeUser(int userId);                  // 注销账号
    const User_Zhb* findUser(int userId) const;   // 按ID查用户（返回nullptr表示未找到）
    std::vector<User_Zhb>& getAllUsers();         // 用户列表（可修改）

    // ---- 登录管理 ----
    bool login(int userId);                       // 登录（同一平台其它服务视为自动登录）
    void logout();                                // 登出
    int  getCurrentUserId() const;                // 当前登录用户ID（-1 表示未登录）

    // ---- 群管理 ----
    bool addGroup(const Group_Zhb& group);        // 创建群
    bool removeGroup(int groupId);                // 解散群
    const Group_Zhb* findGroup(int groupId) const;
    std::vector<Group_Zhb>& getAllGroups();

private:
    std::vector<User_Zhb>  m_userList;     // 所有用户列表
    std::vector<Group_Zhb> m_groupList;    // 所有群列表
    int                    m_currentUserId; // 当前登录用户ID（-1 表示未登录）

    std::string m_userFile;   // users.txt 原路径（loadData 时记录，供 saveData 写回）
    std::string m_groupFile;  // groups.txt 原路径
};