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
//��������
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

//��������
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
    cout << "�ͻ���" << endl;
    cout << endl;
    WSADATA wsaData = { 0 };//����׽�����Ϣ
    SOCKET ClientSocket = INVALID_SOCKET;//�ͻ����׽���
    SOCKADDR_IN ServerAddr = { 0 };//����˵�ַ
    USHORT uPort = 8200;//����˶˿�
    //��ʼ���׽���
    if (WSAStartup(MAKEWORD(2, 2), &wsaData))
    {
        cout << "�׽��ֳ�ʼ��ʧ��!  �������:" << WSAGetLastError() << endl;
        return -1;
    }
    //�ж��׽��ְ汾
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
    {
        cout << "�׽��ֲ��� 2.2 �汾!" << endl;
        return -1;
    }
    //�����׽���
    ClientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ClientSocket == INVALID_SOCKET)
    {
        cout << "�׽��ִ���ʧ��!  �������:" << WSAGetLastError() << endl;
        return -1;
    }
    //���������IP
    cout << "�����������IP��ַ:";
    char IP[32] = { 0 };
    gets_s(IP);
    cout << "������������˿ں�:";
    cin >> uPort;
    getchar();
    //���÷�������ַ
    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_port = htons(uPort);//�������˿� htons:�������ֽ����Ϊ�����ֽ�xv
    ServerAddr.sin_addr.S_un.S_addr = inet_addr(IP);//��������ַ  inet_addr:ת������������ַ�����ʮ���ƣ�Ϊ�����ֽ��������ֵ

    cout << "������..." << endl;
    //���ӷ�����
    if (SOCKET_ERROR == connect(ClientSocket, (SOCKADDR*)&ServerAddr, sizeof(ServerAddr)))
    {
        cout << "����ʧ��!  �������:" << WSAGetLastError() << endl;
        closesocket(ClientSocket);
        WSACleanup();
        return -1;
    }
    cout << "���ӳɹ�!" << endl;
    cout << "�������û���:";
    gets_s(userName);
    send(ClientSocket, userName, sizeof(userName), 0); //���͸��ͻ����Լ����ǳ�
    cout << endl;
    _beginthreadex(NULL, 0, ThreadRecv, &ClientSocket, 0, NULL); //������Ϣ�߳�
    _beginthreadex(NULL, 0, ThreadSend, &ClientSocket, 0, NULL); //������Ϣ�߳�
    for (int k = 0;k < 100;k++)
    {
        Sleep(10000000);
    }
    closesocket(ClientSocket);
    WSACleanup();
    system("pause");
    return 0;
}