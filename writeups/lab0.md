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


## diffrence between Shutdown and Close
A standard TCP connection gets terminated by 4-way finalization:

1. Once a participant has no more data to send, it sends a FIN packet to the other
2. The other party returns an ACK for the FIN.
3. When the other party also finished data transfer, it sends another FIN packet
4. The initial participant returns an ACK and finalizes transfer.


However, there is another "emergent" way to close a TCP connection:


A participant sends an RST packet and abandons the connection
The other side receives an RST and then abandon the connection as well
In my test with Wireshark, with default socket options, shutdown sends a FIN packet to the other end but it is all it does. Until the other party send you the FIN packet you are still able to receive data. Once this happened, your Receive will get an 0 size result. So if you are the first one to shut down "send", you should close the socket once you finished receiving data.


On the other hand, if you call close whilst the connection is still active (the other side is still active and you may have unsent data in the system buffer as well), an RST packet will be sent to the other side. This is good for errors. For example, if you think the other party provided wrong data or it refused to provide data (DOS attack?), you can close the socket straight away.


My opinion of rules would be:


Consider shutdown before close when possible
If you finished receiving (0 size data received) before you decided to shutdown, close the connection after the last send (if any) finished.
If you want to close the connection normally, shutdown the connection (with SHUT_WR, and if you don't care about receiving data after this point, with SHUT_RD as well), and wait until you receive a 0 size data, and then close the socket.
In any case, if any other error occurred (timeout for example), simply close the socket.
Ideal implementations for SHUT_RD and SHUT_WR


The following haven't been tested, trust at your own risk. However, I believe this is a reasonable and practical way of doing things.


If the TCP stack receives a shutdown with SHUT_RD only, it shall mark this connection as no more data expected. Any pending and subsequent read requests (regardless whichever thread they are in) will then returned with zero sized result. However, the connection is still active and usable -- you can still receive OOB data, for example. Also, the OS will drop any data it receives for this connection. But that is all, no packages will be sent to the other side.


If the TCP stack receives a shutdown with SHUT_WR only, it shall mark this connection as no more data can be sent. All pending write requests will be finished, but subsequent write requests will fail. Furthermore, a FIN packet will be sent to another side to inform them we don't have more data to send.