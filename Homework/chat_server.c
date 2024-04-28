#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <time.h>

char buf[256];
char accepted[FD_SETSIZE][24]; //Danh dau client da duoc chap nhan
char name[FD_SETSIZE][24]; //Luu ten username
char goodbye[] = "Da dong ket noi";

char* user_time(char name[]) {
    time_t rawtime;
    struct tm *timeinfo;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // Định dạng thời gian theo yêu cầu
    char *user = malloc(100); // Cấp phát bộ nhớ động
    if (user == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    strftime(user, 100, "%Y/%m/%d %I:%M:%S%p ", timeinfo);

    strcat(user, name);
    strcat(user, ": ");
    return user;
}

void Connect(int i, fd_set fdread) {
    char try_again[] = "Cu phap chua dung, hay nhap lai!\n";
    char cmd[10];
    do {
        int ret = recv(i, accepted[i], sizeof(accepted[i]), 0);
        if (ret <= 0) {
            send(i, goodbye, sizeof(goodbye), 0);
            accepted[i][0] = '\0';
            name[i][0] = '\0';
            close(i);
            FD_CLR(i, &fdread);
        } else {
        accepted[i][ret] = 0;
        }
        if (strncmp(accepted[i], "client_id:", 10)) {
            send(i, try_again, sizeof(try_again), 0);
        } else {
            break;
        }
    } while (1);
    // Chap nhan ket noi
    int n = sscanf(accepted[i], "%s%s", cmd, name[i]);
    char welcome[100] = "Da chap nhan ket noi client ";
    strcat(welcome, name[i]);
    strcat(welcome, "\n");
    send(i, welcome, sizeof(welcome), 0); 
}

void chat(int i, int listener, fd_set fdread) {
    int ret = recv(i, buf, sizeof(buf), 0);
    buf[ret] = 0;
    if (ret <= 0) {
        send(i, goodbye, sizeof(goodbye), 0);
        accepted[i][0] = '\0';
        name[i][0] = '\0';
        close(i);
        FD_CLR(i, &fdread);
    } else {
        for (int j = 0; j < FD_SETSIZE; j++) {
            if (FD_ISSET(j, &fdread) && 
                        j != listener && 
                        j != i &&
                        !strncmp(accepted[j], "client_id:", 10)) {
                char *user = user_time(name[i]);
                strcat(user, buf);
                int sent = send(j, user, strlen(user), 0);
                if (sent == -1) {
                    perror("send() failed");
                }
            }
        }
        printf("Received from %d: %s\n", i, buf);
    }
}

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
                        printf("New client connected: %d\n", client);
                    }
                } else {
                    // Co du lieu truyen den
                    int client = i;

                    // Neu chua ket noi thanh cong
                    if (strncmp(accepted[i], "client_id:", 10)) {
                        Connect(client, fdread);
                    } else {
                        chat(client, listener, fdread);
                    }
                }
            }
    }

    return 0;
}