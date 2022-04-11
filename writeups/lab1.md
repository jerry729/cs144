Lab 1 Writeup
=============

My name: [your name here]

My SUNet ID: [your sunetid here]

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: [list sunetids here]

This lab took me about [n] hours to do. I [did/did not] attend the lab session.

Program Structure and Design of the StreamReassembler:
[]

Implementation Challenges:
[]

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]

## set自定义对象 重载小于号
需要重载小于号

重载要点：

1. 两个const

2. 两个不同的类必须能比较出大小来 

为什么不用重载等号set也能去重？

因为由 `A < B false, B < A false`  可推出 `A == B`

## 构造函数后加冒号是初始化表达式：
有四种情况下应该使用初始化表达式来初始化成员：
1. 初始化const成员
2. 初始化引用成员
3. 当调用基类的构造函数，而它拥有一组参数时

4. 当调用成员类的构造函数，而它拥有一组参数时。

在程序中定义变量并初始化的机制中,有两种形式,一个是我们传统的初始化的形式,即赋值运算符赋值,还有一种是括号赋值,如: 
```cpp
int a=10; 
char b='r';//赋值运算符赋值 

int a(10);
char b('r');//括号赋值 
```
以上定义并初始化的形式是正确的,可以通过编译,但括号赋值只能在变量定义并初始化中,不能用在变量定义后再赋值

冒号初始化是给数据成员分配内存空间时就进行初始化,就是说分配一个数据成员只要冒号后有此数据成员的赋值表达式(此表达式必须是括号赋值表达式),那么分配了内存空间后在进入函数体之前给数据成员赋值，就是说初始化这个数据成员此时函数体还未执行。 对于在函数中初始化,是在所有的数据成员被分配内存空间后才进行的。这样是有好处的,有的数据成员需要在构造函数调入之后函数体执行之前就进行初始化如引用数据成员,常量数据成员和对象数据成员。

## 再探左值右值 左值引用右值引用
值类别：左值，右值
类型：左值引用，右值引用

一个右值引用类型绑定到右值但其本身值类别是左值

**const左值引用可以绑定到右值**

```cpp
void func0(T& t)
{
    T local = t
}
//只可传左值 调用copy构造

void func1(const T& t)
{
    T local = t;
    // use local
}
//若传左值 调用copy构造
//若传右值 调用copy构造

void func2(T t)
{
    //use t
}
//若传左值 调用copy构造
//若传右值 调用move构造

void func2(T&& t)
{
    //use t
}

```

引用折叠

只有纯右值`&& && = &&`，沾上一个左值引用就变左值引用
```
A& & 变成 A&

A& && 变成 A&

A&& & 变成 A&

A&& && 变成 A&&
```

### 函数形参什么时候用左值引用什么时候用右值引用

当你需要在函数内copy参数 并且 要将copy的结果保存在非该函数的栈内 时。这两个条件必须同时都满足。最典型的例子，就是STL的那些支持add操作的容器：因为在你给一个容器add元素时，你第一copy了外面传递进来的元素第二将这个元素存储在了容器里。比如vector的push_back就有两个版本：一个push_back的参数类型是const &，另一个是&&。这么做的好处就是如果传递进来的是一个右值，那么此时在push_back里就只需要move而不需要copy。比如，vec.push_back(MyClass());  // 此时参数为右值，调用第二个版本的push_back
