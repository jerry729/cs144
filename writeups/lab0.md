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


1. A participant sends an RST packet and abandons the connection
2. The other side receives an RST and then abandon the connection as well


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

## 双端队列
|名字|普通队列|双端队列|优先队列|
|---|-------|-------|-------|
|头文件|#include <queue>|#include<deque>|#include <queue>|
|声明(DateType 为任意数据类型)|queue<DateType> name|deque<DateType> name|priority_queue <DateType> name（默认从小到大排序)|
|取出队首的值*|name.front（）|name.front()|name.back()|name.top()|
|弹出元素（删掉，不取值）*|name.pop()|name.pop_back()->弹出队尾name.pop_front()->弹出队首|name.pop()|
|存入元素|name.push|name.push_back()->存入队尾name.push_front()->存入队首|name.push（)|
|求队列中元素个数|name.size()|name.size()|name.size()|


## gcc语法检查
Make sure the members appear in the initializer list in the same order as they appear in the class

```cpp
Class C {
   int a;
   int b;
   C():b(1),a(2){} //warning, should be C():a(2),b(1)
}
```
or you can turn -Wno-reorder

In some cases (not recommendable), b and a initialisation might depend on each other. A naive user might try to alter the initialisation order to get some effect and the Warning would make it clear that it doesn't work


## vscode + cmake 配置tasks.json and launch.json

在配置的时候会用到一些vscode的变量，用${}包裹起来的那些。
${workspaceFolder}是当前工作空间（或vscode所打开根文件夹）在操作系统中绝对路径
${workspaceFolderBasename}是当前工作空间（或vscode所打开根文件夹）的名称


tasks.json 这是VSCode任务的配置文件，通过配置它可以快速执行各种命令。这里我们利用它来配置编译构建流程。我们要执行的任务为建立build文件夹，在build文件夹中使用CMake生成并编译。通过这个任务配置，统一全平台下的程序编译命令。


### tasks.json
```json
{
    // See https://go.microsoft.com/fwlink/?LinkId=733558
    // for the documentation about the tasks.json format
    "version": "2.0.0",
    "tasks": [
        { // 在根文件夹中执行创建文件夹build的命令
            // 除windows系统外执行的命令为`mkdir -p build`
            // windows系统是在powershell中执行命令`mkdir -Force build`
            "label": "build_dir",
            "command": "mkdir",
            "type": "shell",
            "args": [
                "-p",
                "build"
            ],
            "options": {
                "cwd": "${workspaceFolder}"
            }
            "windows": {
                "options": {
                    "shell": {
                        "executable": "powershell.exe"
                    }
                },
                "args": [
                    "-Force",
                    "build"
                ],
            }
        },
        { // 在build文件夹中调用cmake进行项目配置
            // 除windows系统外执行的命令为`cmake -DCMAKE_BUILD_TYPE=<Debug|Release|RelWithDebInfo|MinSizeRel> ../`
            // windows系统是在visual stuido的环境中执行命令`cmake -DCMAKE_BUILD_TYPE=<Debug|Release|RelWithDebInfo|MinSizeRel>  ../ -G "CodeBlocks - NMake Makefiles"`
            "label": "cmake",
            "type": "shell",
            "command": "cmake",
            "args": [
                "-DCMAKE_BUILD_TYPE=${input:CMAKE_BUILD_TYPE}",
                "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON", // 生成compile_commands.json 供c/c++扩展提示使用
                "../"
            ],
            "options": {
                "cwd": "${workspaceFolder}/build",
            },
            "windows": {
                "args": [
                    "-DCMAKE_BUILD_TYPE=${input:CMAKE_BUILD_TYPE}",
                    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
                    "../",
                    "-G",
                    "\"CodeBlocks - NMake Makefiles\""
                ],
                "options": {
                    "shell": {
                        // "executable": "C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\vcvarsall.bat",
                        // 需要根据安装的vs版本调用vs工具命令提示符
                        "executable": "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Auxiliary\\Build\\vcvarsall.bat",
                        "args": [
                            "${input:PLATFORM}", //指定平台
                            "-vcvars_ver=${input:vcvars_ver}", //指定vc环境版本
                            "&&"
                        ]
                    }
                },
            },
            "dependsOn": [
                "build_dir" // 在task `build_dir` 后执行该task
            ]
        },
        { // 在build文件夹中调用cmake编译构建debug程序
            // 执行的命令为`cmake --build ./ --target all --`
            //  windows系统如上需要在visual stuido的环境中执行命令
            "label": "build",
            "group": "build",
            "type": "shell",
            "command": "cmake",
            "args": [
                "--build",
                "./",
                "--target",
                "all",
                "--"
            ],
            "options": {
                "cwd": "${workspaceFolder}/build",
            },
            "problemMatcher": "$gcc",
            "windows": {
                "options": {
                    "shell": {
                        // "executable": "C:\\Program Files (x86)\\Microsoft Visual Studio 12.0\\VC\\vcvarsall.bat",
                        "executable": "C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Auxiliary\\Build\\vcvarsall.bat",
                        "args": [
                            "${input:PLATFORM}",
                            "-vcvars_ver=${input:vcvars_ver}",
                            "&&"
                        ]
                    }
                },
                "problemMatcher": "$msCompile"
            },
            "dependsOn": [
                "cmake" // 在task `cmake` 后执行该task
            ]
        }
        {
            "label": "make",
            "type": "shell",
            "command": "make",
            "options": {
                "cwd": "${workspaceFolder}/build"
            },
            "dependsOn":[
                "build"
            ]
        }
    ],
    "inputs": [
        {
            "id": "CMAKE_BUILD_TYPE",
            "type": "pickString",
            "description": "What CMAKE_BUILD_TYPE do you want to create?",
            "options": [
                "Debug",
                "Release",
                "RelWithDebInfo",
                "MinSizeRel",
            ],
            "default": "Debug"
        },
        {
            "id": "PLATFORM",
            "type": "pickString",
            "description": "What PLATFORM do you want to create?",
            "options": [
                "x86",
                "amd64",
                "arm",
                "x86_arm",
                "x86_amd64",
                "amd64_x86",
                "amd64_arm",
            ],
            "default": "amd64"
        },
        {
            "id": "vcvars_ver",
            "type": "pickString",
            "description": "What vcvars_ver do you want to create?",
            "options": [
                "14.2", // 2019
                "14.1", // 2017
                "14.0", // 2015
            ],
            "default": "14.2"
        }
    ]
}
```


launch.json 这是VSCode运行调试的配置文件。全平台统一的调试体验就靠它了。依赖于VSCode的C/C++扩展。这里需要告诉VSCode你的C/C++程序在哪，以及运行参数，工作目录等，用哪个调试器调试。

### launch.json
Predefined variables
The following predefined variables are supported:

- ${workspaceFolder} - the path of the folder opened in VS Code
- ${workspaceFolderBasename} - the name of the folder opened in VS Code without any slashes (/)
- ${file} - the current opened file
- ${fileWorkspaceFolder} - the current opened file's workspace folder
- ${relativeFile} - the current opened file relative to workspaceFolder
- ${relativeFileDirname} - the current opened file's dirname relative to workspaceFolder
- ${fileBasename} - the current opened file's basename
- ${fileBasenameNoExtension} - the current opened file's basename with no file extension
- ${fileDirname} - the current opened file's dirname
- ${fileExtname} - the current opened file's extension
- ${cwd} - the task runner's current working directory on startup
- ${lineNumber} - the current selected line number in the active file
- ${selectedText} - the current selected text in the active file
- ${execPath} - the path to the running VS Code executable
- ${defaultBuildTask} - the name of the default build task
- ${pathSeparator} - the character used by the operating system to separate components in file paths
Note: The ${workspaceRoot} variable is deprecated in favor of the ${workspaceFolder} variable.


```json
{
    // Use IntelliSense to learn about possible attributes.
    // Hover to view descriptions of existing attributes.
    // For more information, visit: https://go.microsoft.com/fwlink/?linkid=830387
    "version": "0.2.0",
    "configurations": [
        {
            "name": "Launch Debug", //名称
            "type": "cppdbg", //调试类型，除使用msvc进行调试外，均为该类型
            "request": "launch",
            "program": "${workspaceFolder}/build/${relativeFileDirname}/${fileBasenameNoExtension}", //指定C/C++程序位置
            "args": [], //指定运行参数
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}", //指定工作目录
            "preLaunchTask": "make", //在调试前会先调用build_debug这个task编译构建程序
            "environment": [],
            "externalConsole": false,
            "osx": { //macOS的特定配置
                // "miDebuggerPath": "/Applications/Xcode.app/Contents/ Developer/usr/bin/lldb-mi", //修改使用的lldb-mi，一般不需要修改
                "MIMode": "lldb" //指定使用lldb进行调试
            },
            "linux": { //linux的特定配置
                "MIMode": "gdb", //指定使用gdb调试
                "setupCommands": [
                    {
                        "description": "Enable pretty-printing for gdb",
                        "text": "-enable-pretty-printing",
                        "ignoreFailures": true
                    }
                ]
            },
            "windows": { //windows的特定配置
                "type": "cppvsdbg", //指定使用msvc进行调试
                "program": "${workspaceFolder}/build/${workspaceFolderBasename}.exe", //指定C/C++程序位置
            }
        }
    ]
}
```