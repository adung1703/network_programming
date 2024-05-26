#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

void *client_proc(void *);

int clients[64];

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
    addr.sin_port = htons(8000);

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

    while (1) {
        printf("Waiting for new client\n");
        int client = accept(listener, NULL, NULL);
        printf("New client accepted, client = %d\n", client);
        
        pthread_t tid;
        pthread_create(&tid, NULL, client_proc, &client);
        pthread_detach(tid);
    }

    return 0;
}

void *client_proc(void *arg) {
    int client = *(int *)arg;
    char buf[256];

    // Nhan du lieu tu client
    while (1) {
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;

        buf[ret] = 0;
        printf("Received from %d: %s\n", client, buf);

        char cmd[32], format[32], tmp[32];
        int n = sscanf(buf, "%s%s%s", cmd, format, tmp);
        if (n!=2) {
            char *msg = "Sai cu phap, hay nhap lai! \n";
            send(client, msg, strlen(msg), 0);
        } else {
            if (strcmp(cmd, "GET_TIME") != 0) {
                char *msg = "Sai cu phap, hay nhap lai! \n";
                send(client, msg, strlen(msg), 0);
            } else {
                time_t now;
                struct tm *local;
                now = time(NULL);
                local = localtime(&now);    
                char msg[32];
                if(strcmp(format, "dd/mm/yyyy") == 0) {
                    strftime(msg, 11, "%d/%m/%Y\n", local);
                }
                else if (strcmp(format, "dd/mm/yy") == 0) {
                    strftime(msg, 9, "%d/%m/%y\n", local);
                }
                else if (strcmp(format, "mm/dd/yyyy") == 0) {
                    strftime(msg, 11, "%m/%d/%Y\n", local);
                }
                else if (strcmp(format, "mm/dd/yy") == 0) {
                    strftime(msg, 9, "%m/%d/%y\n", local);
                }
                else strcpy(msg, "Dinh dang khong ho tro!\n");
                send(client, msg, strlen(msg), 0);
            }           

        }
    }

    close(client);
}