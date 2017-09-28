#include <WinSock2.h>
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>

int main_recv(void);
int main_send(void);

#pragma comment(lib, "ws2_32.lib")

int main(int argc, char* argv[]) 
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);


    int cmd = 0;
    printf("1: recv\n2: send\n>>");
    scanf("%d", &cmd);


    if (1 == cmd) {
        main_recv();
    }
    else if (2 == cmd) {
        main_send();
    }

    system("pause");
    WSACleanup();
    return 0;
}