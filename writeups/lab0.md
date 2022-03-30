Lab 0 Writeup
=============

My name: [your name here]

My SUNet ID: [your sunetid here]

I collaborated with: [list sunetids here]

This lab took me about [n] hours to do. I [did/did not] attend the lab session.

My secret code from section 2.1 was: [code here]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]

## 左值右值 移动语义
lvalue:具名且不可被移动
xvaue:具名且可被移动
prvalue:不具名且可被移动
glvalue:具名，lvalue和xvalue都属于glvalue
rvalue:可被移动的表达式，prvalue和xvalue都属于rvalue
![](https://pica.zhimg.com/80/v2-60953ef4f577788e2f96dbcbb960d3a4_1440w.jpg?source=1940ef5c)
![](https://pic1.zhimg.com/80/v2-b188e928a74fa3d3b08b5cb75c8d5c58_1440w.jpg?source=1940ef5c)

## 重载 类型转换
operator sockaddr *();
operator const sockaddr *() const;
类型转换重载 对象分别转换成sockaddr指针类型 和sockaddr常量指针
const 在 * 前   常量指针 指向内容不可变
const 在 * 后   指针常量 指针不可变
方法后的const是说这个方法不会改变对象的成员

## address
sockaddr_storage 包装 sockaddr
Address 包装 sockaddr_storage
