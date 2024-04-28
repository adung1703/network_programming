#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int arg, char *argv[]) { //tcp_client <địa chỉ IP> <cổng>

    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1) {
        perror("Ket noi den server that bai!");
        return 1;
    }

    char mess[1024];
    while (1) {
        printf("Hay nhap thong diep: \n");
        fgets(mess, 1024, stdin);
        
        if (strncmp(mess, "exit", 4) == 0) break;

        send(client, mess, strlen(mess), 0);
    }

    close(client);

    return 0;
}