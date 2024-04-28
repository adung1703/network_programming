#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>

int main() {
    // Tao socket cho ket noi
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        return 1;
    }

    // Khai bao dia chi server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(9000);

    // Gan socket voi cau truc dia chi
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed");
        return 1;
    }

    // Chuyen socket sang trang thai cho ket noi
    if (listen(listener, 5)) {
        perror("listen() failed");
        return 1;
    }

    fd_set fdread, fdtest;
    FD_ZERO(&fdread);

    // Gan socket listener vao fdread
    FD_SET(listener, &fdread);

    char buf[256];
    int num_clients = 0;
    int client_sockets[FD_SETSIZE];


    while (1) {
        fdtest = fdread;
        int ret = select(FD_SETSIZE, &fdtest, NULL, NULL, NULL);
        if (ret == -1) {
            break;
        }

        for (int i = 0; i < FD_SETSIZE; i++)
            if (FD_ISSET(i, &fdtest)) {
                if (i == listener) {
                    // Co ket noi
                    int client = accept(listener, NULL, NULL);
                    if (client >= FD_SETSIZE) {
                        close(client);
                    } else {
                        FD_SET(client, &fdread);
                        num_clients++;
                        char msg[64];
                        sprintf(msg, "Xin chao. Hien co %d clients dang ket noi!\n", num_clients);
                        send(client, msg, strlen(msg), 0);
                        printf("New client connected: %d\n", client);
                    }
                } else {
                    // Co du lieu truyen den
                    int client = i;
                    ret = recv(client, buf, sizeof(buf), 0);
                    if (ret <= 0) {
                        num_clients--;
                        close(client);
                        FD_CLR(client, &fdread);
                    } else {
                        buf[ret] = 0;
                        if (strncmp(buf, "exit", 4) == 0) {
                            num_clients--;
                            close(client);
                            FD_CLR(client, &fdread);
                        }
                        for (int t = 0; t < strlen(buf); t++) {
                            if (buf[t] >= 'A' && buf[t] <= 'z') {
                                if (buf[t] == 'Z') buf[t] = 'A';
                                else if (buf[t] == 'z') buf[t] = 'a';
                                else buf[t] += 1;
                            } else if (buf[t] >= '0' && buf[t] <= '9') buf[t] = '9' - buf[t];
                        }
                        send(client, buf, strlen(buf), 0);

                        printf("Received from %d: %s\n", client, buf);
                    }
                }
            }
    }

    return 0;
}