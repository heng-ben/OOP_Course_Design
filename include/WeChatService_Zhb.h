#pragma once
#include "Service_Zhb.h"
#include <string>
#include <vector>

class FriendManager_Zhb;

// 微信服务:持有一个 FriendManager 指针,实际操作全局唯一的管理器
class WeChatService_Zhb : public Service_Zhb
{
public:
    WeChatService_Zhb();
    explicit WeChatService_Zhb(int ownerId);

    std::string getType() override;         // 返回 "WeChat"

    void setOwner(int ownerId);
    int  getOwner() const;

    void setFriendMgr(FriendManager_Zhb* mgr);

    // ---- 好友操作(底层转给 FriendManager,落在微信好友图上) ----
    bool addFriend(int otherId);                   // 双向加微信好友
    bool removeFriend(int otherId);                // 双向删微信好友
    std::vector<int> getFriendIds();               // 当前微信好友(快照)
    std::vector<int> recommendFromOtherService();  // 用“QQ好友”补推荐到微信

private:
    int                 m_ownerId;
    FriendManager_Zhb*  m_friendMgr;
};