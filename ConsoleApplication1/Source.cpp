#include <stdio.h>
#include <stdlib.h>
#include <afxsock.h>
#include <signal.h>
#include <afx.h>
#include <windows.h>

unsigned int WINAPI GetClient(void *arg);
unsigned int WINAPI GetServer(void *arg);

int n = 0;
struct sockaddr_in pub_c_add[10] = { NULL, };
struct sockaddr_in pub_s_add = { NULL };
char s_add[20] = { NULL, };
int s_port = 0;
char c_add[10][20] = { {NULL,}, };
int c_port[10] = { 0, };
CRITICAL_SECTION cri;

int main(void) {
	HANDLE myHandle[2];
	UINT myThreadID, myThreadID2;
	InitializeCriticalSection(&cri);
	myHandle[0] = (HANDLE)_beginthreadex(0, 0, GetClient, 0, 0, &myThreadID);
	myHandle[1] = (HANDLE)_beginthreadex(0, 0, GetServer, 0, 0, &myThreadID2);
	//Sleep(100);
	//myHandle[1] = (HANDLE)_beginthreadex(0, 0, SendFrame, 0, 0, &myThreadID2);
	while (1) {
		DWORD retval = WaitForMultipleObjects(2, myHandle, false, INFINITE);
		
		switch (retval)
		{
		case WAIT_OBJECT_0: // hTread[0] 종료
			CloseHandle(myHandle[0]);
			myHandle[0] = (HANDLE)_beginthreadex(0, 0, GetClient, 0, 0, &myThreadID);
			break;
			/*case WAIT_OBJECT_0 + 1: // hTread[1] 종료
			Sleep(100);
			CloseHandle(myHandle[1]);
			myHandle[1] = (HANDLE)_beginthreadex(0, 0, dummypack, 0, 0, &myThreadID2);
			break;*/
		case WAIT_OBJECT_0 + 1: // hTread[1] 종료
			Sleep(100);
			CloseHandle(myHandle[1]);
			myHandle[1] = (HANDLE)_beginthreadex(0, 0, GetServer, 0, 0, &myThreadID2);
			break;
			/*	case WAIT_TIMEOUT:
			Sleep(100);
			CloseHandle(myHandle[0]);
			CloseHandle(myHandle[1]);
			CloseHandle(myHandle[2]);
			myHandle[0] = (HANDLE)_beginthreadex(0, 0, dummypack, 0, 0, &myThreadID);
			myHandle[1] = (HANDLE)_beginthreadex(0, 0, dummypack, 0, 0, &myThreadID2);
			Sleep(100);
			myHandle[2] = (HANDLE)_beginthreadex(0, 0, SendFrame, 0, 0, &myThreadID3);
			break;
		case WAIT_FAILED:    // 오류 발생
			break;
			*/
		}
	}
}

unsigned int WINAPI GetClient(void *arg) {
	SOCKET sock;
	WSADATA wsaData;

	int size = 0;
	int rev = 0;
	int send = 0;

	char space = ' ';

	char tmp[20] = { NULL, };
	char buff_recv[100] = { NULL, };
	char buf[4] = { '0' };
	char buff_send[100] = { NULL, };

	memset(&sock, 0, sizeof(sock));
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)// ws2_32.dll(socket library)을 초기화 
	{
		WSACleanup();
		return -1;
	}
	struct sockaddr_in server_addr, clnt_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(5000);
	server_addr.sin_addr.s_addr = inet_addr("192.168.0.4");

	memset(&clnt_addr, 0, sizeof(clnt_addr));
	size = sizeof(sockaddr);
	sock = socket(PF_INET, SOCK_DGRAM, 0);

	if (bind(sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) == SOCKET_ERROR)
		return 0;

	rev = recvfrom(sock, buff_recv, 100, 0, (struct sockaddr*)&clnt_addr, &size);
	if (rev == -1)
		return 0;
	while (TryEnterCriticalSection(&cri)) { Sleep(100); }
	for (int i = 0; i < rev; i++)
	{
		if (buff_recv[i] == ' ')
		{
			memcpy(tmp, &buff_recv[0], i);
			for (int i = 0; i < n; i++)
			{

				if (strcmp(inet_ntoa(clnt_addr.sin_addr), inet_ntoa(pub_c_add[i].sin_addr)) == 0 && strcmp(c_add[i], tmp) == 0)
					return 0;
			}
			memcpy(&pub_c_add[n], &clnt_addr,sizeof(sockaddr));
			memcpy(&c_add[n], &buff_recv[0], i);
			c_add[i] == 0;
			memcpy(&c_port[n], &buff_recv[i + 1], sizeof(int));
			n++;
			break;
		}
	}
	if (strlen(inet_ntoa(pub_s_add.sin_addr)) != 0)
	{
		memset(buff_send, 0, 100);
		memcpy(buff_send, &pub_s_add, sizeof(sockaddr));
		memcpy(buff_send+ sizeof(sockaddr), &space, 1);
		if (strlen(s_add) != 0) {
			memcpy(buff_send+ sizeof(sockaddr) +1, s_add, strlen(s_add));
			memcpy(buff_send + sizeof(sockaddr)+ 1 + strlen(s_add), &space, 1);
			memcpy(buff_send + sizeof(sockaddr) + 2 + strlen(s_add), &s_port, sizeof(int));
			send = sendto(sock, buff_send, strlen(buff_send) + 1, 0, (struct sockaddr*)&clnt_addr, sizeof(clnt_addr));
		}
		else {
			memset(buff_send, 0, 100);
			send = sendto(sock, buf, strlen(buf) + 1, 0, (struct sockaddr*)&clnt_addr, sizeof(clnt_addr));
		}
	}
	else {
		send = sendto(sock, buf, strlen(buf) + 1, 0, (struct sockaddr*)&clnt_addr, sizeof(clnt_addr));
	}
	LeaveCriticalSection(&cri);
	closesocket(sock);
	return 0;

}

unsigned int WINAPI GetServer(void *arg)
{
	SOCKET sock;
	WSADATA wsaData;

	int size = 0;
	int rev = 0;
	int send = 0;

	char space = ' ';

	char tmp[4] = { NULL, };
	char buff_recv[100] = { NULL, };
	char buf[4] = { '0' };
	char buff_send[100] = { NULL, };

	memset(&sock, 0, sizeof(sock));
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)// ws2_32.dll(socket library)을 초기화 
	{
		WSACleanup();
		return -1;
	}
	struct sockaddr_in server_addr, clnt_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = PF_INET;
	server_addr.sin_port = htons(5001);
	server_addr.sin_addr.s_addr = inet_addr("192.168.0.4");

	memset(&clnt_addr, 0, sizeof(clnt_addr));
	size = sizeof(sockaddr);
	sock = socket(PF_INET, SOCK_DGRAM, 0);

	if (bind(sock, (struct sockaddr*)&server_addr, sizeof(struct sockaddr)) == SOCKET_ERROR)
		return 0;
	rev = recvfrom(sock, buff_recv, 100, 0, (struct sockaddr*)&clnt_addr, &size);
	if (rev < 5)
	{
		send = sendto(sock, buf, strlen(buf) + 1, 0, (struct sockaddr*)&clnt_addr, sizeof(clnt_addr));
		return 0;
	}
	while (TryEnterCriticalSection(&cri)) { Sleep(100); }
	memcpy(&pub_s_add, &clnt_addr, sizeof(sockaddr));
	for (int i = 0; i < rev; i++)
	{
		if (buff_recv[i] == ' ')
		{
			memcpy(&s_add, &buff_recv[0], i);
			c_add[i] == 0;
			memcpy(&s_port, &buff_recv[i + 1], sizeof(int));
			break;
		}
	}

	memcpy(tmp, &n,sizeof(int));
	send = sendto(sock,tmp , sizeof(int) + 1, 0, (struct sockaddr*)&clnt_addr, sizeof(clnt_addr));
	if (send != 4)
	{
		return 0;
	}
	for (int i = 0; i < n; i++)
	{
		memset(buff_send, 0, 100);
		memcpy(buff_send, &pub_c_add[i], sizeof(sockaddr));
		memcpy(buff_send + sizeof(sockaddr), &space, 1);
		memcpy(buff_send + sizeof(sockaddr), s_add, strlen(s_add));
		memcpy(buff_send + sizeof(sockaddr) + strlen(s_add), &space, 1);
		memcpy(buff_send + sizeof(sockaddr) + strlen(s_add), &s_port, sizeof(int));
		send = sendto(sock, buff_send, strlen(buff_send) + 1, 0, (struct sockaddr*)&clnt_addr, sizeof(clnt_addr));
		if (send < strlen(buff_send))
			return 0;
	}
	LeaveCriticalSection(&cri);
	closesocket(sock);
	return 0;
}