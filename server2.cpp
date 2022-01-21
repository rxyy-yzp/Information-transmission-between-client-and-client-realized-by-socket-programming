#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<WinSock2.h>
#include<process.h>
#include<stdio.h>
#include"stdlib.h"
#include<conio.h>
#include"iostream"
using namespace std;
#pragma comment(lib,"ws2_32.lib")

#define SEND_OVER 1             //已经转发消息
#define SEND_YET 0             //还没转发消息

int peoplenumber = 0;//当前人数
int g_iStatus = SEND_YET;
SOCKET ServerSocket = INVALID_SOCKET;   //服务端套接字
SOCKADDR_IN g_ClientAddr = { 0 };      //客户端地址
int g_iClientAddrLen = sizeof(g_ClientAddr);
bool g_bCheckConnect = false;        //检查连接情况
HANDLE g_hRecv[50] = { 0 };
//客户端信息结构体
typedef struct _Client
{
    SOCKET sClient;   //客户端套接字
    char buf[128];    //数据缓冲区
    char userName[16];//客户端用户名
    char IP[50];     //客户端IP
    UINT_PTR flag;   //标记客户端，用来区分不同的客户端
}Client;

Client g_Client[50] = { 0 };         //创建一个客户端结构体

//发送数据线程
unsigned __stdcall ForwardMessage(void* param)
{
    int ret = 0;
    int flag = *(int*)param;
    SOCKET client = INVALID_SOCKET;//创建一个临时套接字来存放要转发的客户端套接字
    char temp[128] = { 0 };//创建一个临时的数据缓冲区
    memcpy(temp, g_Client[flag].buf, sizeof(temp));
    for (int i = 0;i < peoplenumber;i++)
    {
        if (i == flag)
            continue;
        sprintf_s(g_Client[i].buf, "%s:%s", g_Client[flag].userName, temp);//添加一个用户名头

        if (strlen(temp) != 0) //如果数据不为空且还没转发则转发
        {
            ret = send(g_Client[i].sClient, g_Client[i].buf, sizeof(g_Client[i].buf), 0);
        }
        if (ret == SOCKET_ERROR)
            return 1;
        g_iStatus = SEND_OVER;  //转发成功后设置状态为已转发
    }
    return 0;
}

//接受数据线程
unsigned __stdcall RecvMessage(void* param)
{
    SOCKET client = INVALID_SOCKET;
    int flag = 0;
    for (int i = 0;i < peoplenumber;i++)
    {
        if (*(int*)param == g_Client[i].flag) //判断是哪个客户端发来的消息
        {
            client = g_Client[i].sClient;
            flag = i;
        }
    }
    char temp[128] = { 0 }; //临时数据缓冲区
    while (1)
    {
        memset(temp, 0, sizeof(temp));
        int ret = recv(client, temp, sizeof(temp), 0); //接收数据
        if (ret == SOCKET_ERROR)
        {
            continue;
        }
        g_iStatus = SEND_YET;                //设置转发状态为未转发
        cout << g_Client[flag].userName << ":" << temp << endl;//在服务器屏幕上显示出发送信息的客户端用户名和信息
        memcpy(g_Client[flag].buf, temp, sizeof(g_Client[flag].buf));
        _beginthreadex(NULL, 0, ForwardMessage, &flag, 0, NULL); //开启转发线程,flag标记转发给哪个客户端
    }
    return 0;
}

//管理连接
unsigned __stdcall ManagerClient(void* param)
{
    while (1)
    {
        for (int i = 0;i < peoplenumber;i++)
        {
            if (send(g_Client[i].sClient, "", sizeof(""), 0) == SOCKET_ERROR)
            {
                if (g_Client[i].sClient != 0)
                {
                    CloseHandle(g_hRecv[i]); //这里线程句柄
                    peoplenumber--;//下线后人数减一
                    cout << "IP地址为" << g_Client[i].IP << "的用户" << g_Client[i].userName << "断开连接" 
                        << "    当前在线人数" << peoplenumber << endl;
                    closesocket(g_Client[i].sClient);  //若发送消息失败，则关闭该套接字
                    g_Client[i].flag = 0;
                    g_Client[i].sClient = 0;
                }
            }
            Sleep(1000); //1s检查一次
        }
    }
    return 0;
}

//接受请求
unsigned __stdcall AcceptMessage(void* param)
{
    int i = 0;
    int temp[50] = { 0 };
    _beginthreadex(NULL, 0, ManagerClient, NULL, 0, NULL);
    while (1)
    {
        if (g_Client[i].flag != 0)
        {
            ++i;    continue;
        }
        if ((g_Client[i].sClient = accept(ServerSocket, (SOCKADDR*)&g_ClientAddr, 
            &g_iClientAddrLen)) == INVALID_SOCKET)//如果有客户端申请连接就接受连接
        {
            cout << "连接失败!  错误代码:" << WSAGetLastError() << endl;
            closesocket(ServerSocket);  WSACleanup();   return -1;
        }
        recv(g_Client[i].sClient, g_Client[i].userName, sizeof(g_Client[i].userName), 0); //接收用户名
        cout << "连接成功!  客户端IP地址:" << inet_ntoa(g_ClientAddr.sin_addr) <<
            "    端口号:" << htons(g_ClientAddr.sin_port) << "    用户名:" << g_Client[i].userName;
        memcpy(g_Client[i].IP, inet_ntoa(g_ClientAddr.sin_addr), sizeof(g_Client[i].IP)); //记录客户端IP
        g_Client[i].flag = g_Client[i].sClient; //不同的socke有不同UINT_PTR类型的数字来标识
        peoplenumber++;
        cout << "    当前在线人数:" << peoplenumber << endl;
        cout << endl;
        if (g_Client[i].flag != temp[i])   //关闭旧连接
        {
            if (g_hRecv[i])         //关闭线程句柄
                CloseHandle(g_hRecv[i]);
            g_hRecv[i] = (HANDLE)_beginthreadex(NULL, 0, RecvMessage, &g_Client[i].flag, 0, NULL); //接收消息的线程
        }  
        g_hRecv[i] = (HANDLE)_beginthreadex(NULL, 0, RecvMessage, &g_Client[i].flag, 0, NULL);
        temp[i] = g_Client[i].flag; //防止RecvMessage线程多次开启
        Sleep(3000);i++;
    }
    return 0;
}

int main()
{
    cout << "服务器" << endl;
    cout << endl;
    //存放套接字信息的结构
    WSADATA wsaData = { 0 };
    SOCKADDR_IN ServerAddr = { 0 };//服务器地址
    USHORT uPort = 8200;//服务器监听端口

    //初始化套接字
    if (WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        cout << "套接字初始化失败!  错误代码:" << WSAGetLastError() << endl;
        return -1;
    }
    //判断版本
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        cout << "套接字不是 2.2 版本!" << endl;
        return -1;
    }
    //创建套接字
    ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ServerSocket == INVALID_SOCKET)
    {
        cout << "套接字创建失败!  错误代码:" << WSAGetLastError() << endl;
        return -1;
    }

    //设置服务器地址
    ServerAddr.sin_family = AF_INET;//连接方式
    ServerAddr.sin_port = htons(uPort);//服务器监听端口
    ServerAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//任何客户端都可以连接该服务器

    //绑定服务器
    if (SOCKET_ERROR == bind(ServerSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr)))
    {
        cout << "绑定失败!  错误代码:" << WSAGetLastError() << endl;
        closesocket(ServerSocket);
        return -1;
    }
    //设置监听客户端连接数
    if (SOCKET_ERROR == listen(ServerSocket, 50))
    {
        cout << "监听失败!  错误代码:" << WSAGetLastError() << endl;
        closesocket(ServerSocket);
        WSACleanup();
        return -1;
    }

    cout << "服务器正在等待连接..." << endl;

    _beginthreadex(NULL, 0, AcceptMessage, NULL, 0, 0);
    for (int k = 0;k < 10000;k++) //让主线程休眠且防止关闭TCP连接
    {
        Sleep(10000000);
    }
    //关闭套接字
    for (int j = 0;j < peoplenumber;j++)
    {
        if (g_Client[j].sClient != INVALID_SOCKET)
            closesocket(g_Client[j].sClient);
    }
    closesocket(ServerSocket);
    WSACleanup();
    system("pause");
    return 0;
}

