#pragma once
#include "Service_Zhb.h"
#include <string>
#include <vector>

class FriendManager_Zhb;   // 前向声明,避免与 FriendManager_Zhb.h 成环

// QQ 服务:持有一个 FriendManager 指针,实际操作全局唯一的管理器
class QQService_Zhb : public Service_Zhb
{
public:
    QQService_Zhb();                        // 默认 ownerId=0,绑定单例管理器
    explicit QQService_Zhb(int ownerId);    // 指明是哪个用户开通的QQ

    std::string getType() override;         // 返回 "QQ"

    void setOwner(int ownerId);
    int  getOwner() const;

    void setFriendMgr(FriendManager_Zhb* mgr);   // 绑定管理器(默认已指向单例)

    // ---- 好友操作(底层都转给 FriendManager,落在 QQ 好友图上) ----
    bool addFriend(int otherId);                   // 双向加QQ好友
    bool removeFriend(int otherId);                // 双向删QQ好友
    std::vector<int> getFriendIds();               // 当前QQ好友(快照)
    std::vector<int> recommendFromOtherService();  // 用“微信好友”补推荐到QQ

private:
    int                 m_ownerId;    // 该服务属于哪个用户
    FriendManager_Zhb*  m_friendMgr;  // 指向全局唯一的好友管理器
};