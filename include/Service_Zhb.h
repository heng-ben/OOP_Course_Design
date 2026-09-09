#pragma once
#include <string>

// 抽象基类：微X 服务的统一抽象
// 纯虚函数 getType() 用于多态识别具体服务类型
class Service_Zhb
{
public:
    virtual ~Service_Zhb() = default;              // 虚析构，保证派生类正确析构

    // 返回服务类型字符串，如 "QQ" / "WeChat"，由派生类重写
    virtual std::string getType() = 0;
};