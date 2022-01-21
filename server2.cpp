#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include<WinSock2.h>
#include<process.h>
#include<stdio.h>
#include"stdlib.h"
#include<conio.h>
#include"iostream"
using namespace std;
#pragma comment(lib,"ws2_32.lib")

#define SEND_OVER 1             //�Ѿ�ת����Ϣ
#define SEND_YET 0             //��ûת����Ϣ

int peoplenumber = 0;//��ǰ����
int g_iStatus = SEND_YET;
SOCKET ServerSocket = INVALID_SOCKET;   //������׽���
SOCKADDR_IN g_ClientAddr = { 0 };      //�ͻ��˵�ַ
int g_iClientAddrLen = sizeof(g_ClientAddr);
bool g_bCheckConnect = false;        //����������
HANDLE g_hRecv[50] = { 0 };
//�ͻ�����Ϣ�ṹ��
typedef struct _Client
{
    SOCKET sClient;   //�ͻ����׽���
    char buf[128];    //���ݻ�����
    char userName[16];//�ͻ����û���
    char IP[50];     //�ͻ���IP
    UINT_PTR flag;   //��ǿͻ��ˣ��������ֲ�ͬ�Ŀͻ���
}Client;

Client g_Client[50] = { 0 };         //����һ���ͻ��˽ṹ��

//���������߳�
unsigned __stdcall ForwardMessage(void* param)
{
    int ret = 0;
    int flag = *(int*)param;
    SOCKET client = INVALID_SOCKET;//����һ����ʱ�׽��������Ҫת���Ŀͻ����׽���
    char temp[128] = { 0 };//����һ����ʱ�����ݻ�����
    memcpy(temp, g_Client[flag].buf, sizeof(temp));
    for (int i = 0;i < peoplenumber;i++)
    {
        if (i == flag)
            continue;
        sprintf_s(g_Client[i].buf, "%s:%s", g_Client[flag].userName, temp);//���һ���û���ͷ

        if (strlen(temp) != 0) //������ݲ�Ϊ���һ�ûת����ת��
        {
            ret = send(g_Client[i].sClient, g_Client[i].buf, sizeof(g_Client[i].buf), 0);
        }
        if (ret == SOCKET_ERROR)
            return 1;
        g_iStatus = SEND_OVER;  //ת���ɹ�������״̬Ϊ��ת��
    }
    return 0;
}

//���������߳�
unsigned __stdcall RecvMessage(void* param)
{
    SOCKET client = INVALID_SOCKET;
    int flag = 0;
    for (int i = 0;i < peoplenumber;i++)
    {
        if (*(int*)param == g_Client[i].flag) //�ж����ĸ��ͻ��˷�������Ϣ
        {
            client = g_Client[i].sClient;
            flag = i;
        }
    }
    char temp[128] = { 0 }; //��ʱ���ݻ�����
    while (1)
    {
        memset(temp, 0, sizeof(temp));
        int ret = recv(client, temp, sizeof(temp), 0); //��������
        if (ret == SOCKET_ERROR)
        {
            continue;
        }
        g_iStatus = SEND_YET;                //����ת��״̬Ϊδת��
        cout << g_Client[flag].userName << ":" << temp << endl;//�ڷ�������Ļ����ʾ��������Ϣ�Ŀͻ����û�������Ϣ
        memcpy(g_Client[flag].buf, temp, sizeof(g_Client[flag].buf));
        _beginthreadex(NULL, 0, ForwardMessage, &flag, 0, NULL); //����ת���߳�,flag���ת�����ĸ��ͻ���
    }
    return 0;
}

//��������
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
                    CloseHandle(g_hRecv[i]); //�����߳̾��
                    peoplenumber--;//���ߺ�������һ
                    cout << "IP��ַΪ" << g_Client[i].IP << "���û�" << g_Client[i].userName << "�Ͽ�����" 
                        << "    ��ǰ��������" << peoplenumber << endl;
                    closesocket(g_Client[i].sClient);  //��������Ϣʧ�ܣ���رո��׽���
                    g_Client[i].flag = 0;
                    g_Client[i].sClient = 0;
                }
            }
            Sleep(1000); //1s���һ��
        }
    }
    return 0;
}

//��������
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
            &g_iClientAddrLen)) == INVALID_SOCKET)//����пͻ����������Ӿͽ�������
        {
            cout << "����ʧ��!  �������:" << WSAGetLastError() << endl;
            closesocket(ServerSocket);  WSACleanup();   return -1;
        }
        recv(g_Client[i].sClient, g_Client[i].userName, sizeof(g_Client[i].userName), 0); //�����û���
        cout << "���ӳɹ�!  �ͻ���IP��ַ:" << inet_ntoa(g_ClientAddr.sin_addr) <<
            "    �˿ں�:" << htons(g_ClientAddr.sin_port) << "    �û���:" << g_Client[i].userName;
        memcpy(g_Client[i].IP, inet_ntoa(g_ClientAddr.sin_addr), sizeof(g_Client[i].IP)); //��¼�ͻ���IP
        g_Client[i].flag = g_Client[i].sClient; //��ͬ��socke�в�ͬUINT_PTR���͵���������ʶ
        peoplenumber++;
        cout << "    ��ǰ��������:" << peoplenumber << endl;
        cout << endl;
        if (g_Client[i].flag != temp[i])   //�رվ�����
        {
            if (g_hRecv[i])         //�ر��߳̾��
                CloseHandle(g_hRecv[i]);
            g_hRecv[i] = (HANDLE)_beginthreadex(NULL, 0, RecvMessage, &g_Client[i].flag, 0, NULL); //������Ϣ���߳�
        }  
        g_hRecv[i] = (HANDLE)_beginthreadex(NULL, 0, RecvMessage, &g_Client[i].flag, 0, NULL);
        temp[i] = g_Client[i].flag; //��ֹRecvMessage�̶߳�ο���
        Sleep(3000);i++;
    }
    return 0;
}

int main()
{
    cout << "������" << endl;
    cout << endl;
    //����׽�����Ϣ�Ľṹ
    WSADATA wsaData = { 0 };
    SOCKADDR_IN ServerAddr = { 0 };//��������ַ
    USHORT uPort = 8200;//�����������˿�

    //��ʼ���׽���
    if (WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        cout << "�׽��ֳ�ʼ��ʧ��!  �������:" << WSAGetLastError() << endl;
        return -1;
    }
    //�жϰ汾
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        cout << "�׽��ֲ��� 2.2 �汾!" << endl;
        return -1;
    }
    //�����׽���
    ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ServerSocket == INVALID_SOCKET)
    {
        cout << "�׽��ִ���ʧ��!  �������:" << WSAGetLastError() << endl;
        return -1;
    }

    //���÷�������ַ
    ServerAddr.sin_family = AF_INET;//���ӷ�ʽ
    ServerAddr.sin_port = htons(uPort);//�����������˿�
    ServerAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);//�κοͻ��˶��������Ӹ÷�����

    //�󶨷�����
    if (SOCKET_ERROR == bind(ServerSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr)))
    {
        cout << "��ʧ��!  �������:" << WSAGetLastError() << endl;
        closesocket(ServerSocket);
        return -1;
    }
    //���ü����ͻ���������
    if (SOCKET_ERROR == listen(ServerSocket, 50))
    {
        cout << "����ʧ��!  �������:" << WSAGetLastError() << endl;
        closesocket(ServerSocket);
        WSACleanup();
        return -1;
    }

    cout << "���������ڵȴ�����..." << endl;

    _beginthreadex(NULL, 0, AcceptMessage, NULL, 0, 0);
    for (int k = 0;k < 10000;k++) //�����߳������ҷ�ֹ�ر�TCP����
    {
        Sleep(10000000);
    }
    //�ر��׽���
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

