Lab 3 Writeup
=============

My name: [your name here]

My SUNet ID: [your sunetid here]

I collaborated with: [list sunetids here]

I would like to thank/reward these classmates for their help: [list sunetids here]

This lab took me about [n] hours to do. I [did/did not] attend the lab session.

Program Structure and Design of the TCPSender:
[]

Implementation Challenges:
[]

Remaining Bugs:
[]

- Optional: I had unexpected difficulty with: [describe]

- Optional: I think you could make this lab better by: [describe]

- Optional: I was surprised by: [describe]

- Optional: I'm not sure about: [describe]


## 成员变量初始化
c++成员变量初始化方法：声明时初始化、初始化列表、构造函数初始化

有人说在声明的时候初始化相当于在构造函数中初始化，其实不是的，成员变量初始化的顺序为：先进行声明时初始化，然后进行初始化列表初始化，最后进行构造函数初始化，如下代码：（另外初始化列表中初始化的顺序是和变量声明的顺序一样，而与列表中的顺序无关）
结果如图，可看出，初始化列表初始化的变量值会覆盖掉声明时初始化的值，而构造函数中初始化的值又会覆盖掉初始化列表的，足以看出初始化的顺序

## inline成员函数
为什么要使用内联函数？

当程序执行函数call指令时，CPU存储函数调用之后的指令的内存地址，复制栈中的函数的参数并且最后将控制权转移到指定的函数。然后CPU执行函数的代码，将函数的返回值存储在预定义的内存位置/寄存器中，并将控制返回给调用的函数。如果从调用函数到被调用函数(被调用者)的切换时间大于函数的执行时间，则这可能成为开销。

对于大型和/或执行复杂任务的函数，与函数运行所花费的时间相比，函数调用的开销通常是微不足道的。但是，对于常用的小函数，进行函数调用所需的时间通常比实际执行函数代码所需的时间多得多。小功能的开销是因为小功能的执行时间小于切换时间。

内联函数的效果,每当调用内联函数,在该函数位置替换为函数体内的语句。 这可以避免函数调用伴随一些(堆栈处理和参数传递带来的)开销,从而达到加速程序的执行。 内联函数是一个在调用时按行扩展的函数。 当调用内联函数时，内联函数的整个代码在内联函数调用点插入或替换。 此替换由C ++编译器在编译时执行。 内联函数如果很小则可以提高效率。

注意，使用内联函数可能会导致程序中这些函数的代码多次出现：内联函数的每次调用都需要一个副本。如果该函数很小，并且需要快速执行,即是可以接受的。如果函数内的代码块的消耗是O(n)，那不是很理想。编译器也知道这一点，并且将内联函数用作请求而不是命令。如果编译器认为该函数过长,通常是O(n)级以上的消耗，编译器会忽略该请求。

**C++要求对一般的内置函数要用关键字inline声明，但对类内定义的成员函数，可以省略inline，因为这些成员函数已经被隐含地指定为内置函数了。应该注意的是：如果成员函数不在类体内定义，而在类体外定义，系统并不是把它默认为内置函数，调用这些成员函数的过程和调用一般函数的过程是相同的。如果想将这些成员函数指定为内置函数，则应该加inline关键字**