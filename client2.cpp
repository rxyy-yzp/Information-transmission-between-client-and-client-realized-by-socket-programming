#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include"iostream"
using namespace std;
#pragma comment(lib,"ws2_32.lib")

#define RECV_OVER 1
#define RECV_YET 0
char userName[16] = { 0 };
int iStatus = RECV_YET;
//接受数据
unsigned __stdcall ThreadRecv(void* param)
{
    char buf[128] = { 0 };
    while (1)
    {
        int ret = recv(*(SOCKET*)param, buf, sizeof(buf), 0);
        if (ret == SOCKET_ERROR)
        {
            Sleep(500);
            continue;
        }
        if (strlen(buf) != 0)
        {
            printf("%s\n", buf);
            iStatus = RECV_OVER;
        }
        else
            Sleep(100);
    }
    return 0;
}

//发送数据
unsigned __stdcall ThreadSend(void* param)
{
    char buf[128] = { 0 };
    int ret = 0;
    while (1)
    {
        int c = _getch();
        cout << userName << ":";
        gets_s(buf);
        ret = send(*(SOCKET*)param, buf, sizeof(buf), 0);
        if (ret == SOCKET_ERROR)
            return 1;
    }
    return 0;
}
int main()
{
    cout << "客户端" << endl;
    cout << endl;
    WSADATA wsaData = { 0 };//存放套接字信息
    SOCKET ClientSocket = INVALID_SOCKET;//客户端套接字
    SOCKADDR_IN ServerAddr = { 0 };//服务端地址
    USHORT uPort = 8200;//服务端端口
    //初始化套接字
    if (WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        cout << "套接字初始化失败!  错误代码:" << WSAGetLastError() << endl;
        return -1;
    }
    //判断套接字版本
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        cout << "套接字不是 2.2 版本!" << endl;
        return -1;
    }
    //创建套接字
    ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ClientSocket == INVALID_SOCKET)
    {
        cout << "套接字创建失败!  错误代码:" << WSAGetLastError() << endl;
        return -1;
    }
    //输入服务器IP
    cout << "请输入服务器IP地址:";
    char IP[32] = { 0 };
    gets_s(IP);
    cout << "请输入服务器端口号:";
    cin >> uPort;
    getchar();
    //设置服务器地址
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_port = htons(uPort);//服务器端口 htons:将主机字节序变为网络字节xv
    ServerAddr.sin_addr.S_un.S_addr = inet_addr(IP);//服务器地址  inet_addr:转换网络主机地址（点分十进制）为网络字节序二进制值

    cout << "连接中..." << endl;
    //连接服务器
    if (SOCKET_ERROR == connect(ClientSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr)))
    {
        cout << "连接失败!  错误代码:" << WSAGetLastError() << endl;
        closesocket(ClientSocket);
        WSACleanup();
        return -1;
    }
    cout << "连接成功!" << endl;
    cout << "请输入用户名:";
    gets_s(userName);
    send(ClientSocket, userName, sizeof(userName), 0); //发送给客户端自己的昵称
    cout << endl;
    _beginthreadex(NULL, 0, ThreadRecv, &ClientSocket, 0, NULL); //接收消息线程
    _beginthreadex(NULL, 0, ThreadSend, &ClientSocket, 0, NULL); //发送消息线程
    for (int k = 0;k < 100;k++)
    {
        Sleep(10000000);
    }
    closesocket(ClientSocket);
    WSACleanup();
    system("pause");
    return 0;
}