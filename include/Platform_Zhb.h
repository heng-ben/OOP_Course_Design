#pragma once
#include <string>
#include <vector>
#include <map>

#include "User_Zhb.h"
#include "Group_Zhb.h"      // 提供 GroupMode + 多态基类

// 平台总控类:管理全体用户、全体群(多态指针)及当前登录用户
class Platform_Zhb
{
public:
    Platform_Zhb();
    ~Platform_Zhb();        // 释放 m_groupList 中的堆群对象
    Platform_Zhb(const Platform_Zhb&) = delete;             // 禁拷贝,避免重复 delete
    Platform_Zhb& operator=(const Platform_Zhb&) = delete;

    // ---- 数据持久化(I/O) ----
    void loadData(const std::string& userFile, const std::string& groupFile);
    void saveData();        // 析构/结束 时写回原文件

    // ---- 用户管理 ----
    bool registerUser(const User_Zhb& user);
    bool removeUser(int userId);
    User_Zhb* findUser(int userId);                 // 可修改版
    const User_Zhb* findUser(int userId) const;     // 只读版
    std::vector<User_Zhb>& getAllUsers();

    // ---- 登录管理 ----
    bool login(int userId);
    void logout();
    int  getCurrentUserId() const;

    // ---- 群管理(统一操作 Group_Zhb* 指针,体现多态) ----
    bool addGroup(Group_Zhb* group);        // 接管所有权(Platform 负责 delete)
    bool removeGroup(int groupId);          // 找到并释放
    Group_Zhb* findGroup(int groupId);              // 可修改版
    const Group_Zhb* findGroup(int groupId) const;  // 只读版
    std::vector<Group_Zhb*>& getAllGroups();

    // ---- 常用群操作(带 groupId,内部转调 Group_Zhb* 的多态方法) ----
    bool joinGroup(int groupId, int userId, bool isRecommended);  // QQ直接入;微信须推荐
    bool setAdmin(int groupId, int adminId);                      // 仅QQ群可设
    bool kickMember(int groupId, int operatorId, int targetId);   // 依 canKick

    // 建子群:QQ群→新建并登记、返回指针;微信群→返回 nullptr
    Group_Zhb* createSubGroup(int groupId, const std::string& subName);

    // 模式切换:返回“已登记替换后的新对象”;同模式/找不到→nullptr(内部已 delete 旧对象)
    Group_Zhb* switchMode(int groupId, GroupMode newMode);

private:
    int allocGroupId() const;               // 生成一个未占用的群号

    std::vector<User_Zhb>  m_userList;      // 用户按值存储
    std::vector<Group_Zhb*> m_groupList;    // 群为多态指针,Platform 拥有并负责 delete
    int                    m_currentUserId; // -1 表示未登录

    std::string m_userFile;   // users.txt 原路径
    std::string m_groupFile;  // groups.txt 原路径
};