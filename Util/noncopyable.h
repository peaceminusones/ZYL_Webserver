#pragma once

//#ifndef WEBSERVER_NONCOPYABLE_H
//#define WEBSERVER_NONCOPYABLE_H

// class noncopyable
// {
// public:
//     noncopyable(const noncopyable&) = delete;
//     noncopyable& operator=(const noncopyable&) = delete;

// protected:
//     noncopyable() = default;
//     ~noncopyable() = default;

// };

// 把构造函数和析构函数设置成protected权限
// 子类可以调用，外面的类不能调用
// 那么当子类需要定义构造函数时不至于通不过编译
// 然后把拷贝构造函数和拷贝赋值函数变成私有
// 意味着除非子类自己定义自己的拷贝构造函数
// 否则外面的调用者是不能通过赋值和拷贝构造等手段来产生一个新的子类对象
class noncopyable
{
protected:
    noncopyable() {}
    ~noncopyable() {}

private: // emphasize the following members are private
    noncopyable(const noncopyable &);
    const noncopyable &operator=(const noncopyable &);
};

//#endif //WEBSERVER_NONCOPYABLE_H
