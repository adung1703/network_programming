#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>

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

    char buf[256];

    for (int i = 0; i < 8; i++) {
        if (fork() == 0) {
            while (1) {
                int client = accept(listener, NULL, NULL);
                printf("New client accepted: %d\n", client);
                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                    continue;
                
                buf[ret] = 0;
                printf("Received from %d: %s\n", client, buf);

                char cmd[16], format[16], msg[128]; 

                sscanf(buf, "%s %s", cmd, format);
                printf("cmd: %s\n", cmd);
                printf("format: %s\n", format);
                if (strcmp(cmd, "GET_TIME") != 0) strcpy(msg, "Cu phap chua dung!\n");
                else { 
                    time_t now;
                    struct tm *local;
                    now = time(NULL);
                    local = localtime(&now);    
                    
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
                }
                printf("%s", msg);
                send(client, msg, sizeof(msg), 0);

                close(client);
            }
            exit(0);
        }
    }

    getchar();
    killpg(0, SIGKILL);

    return 0;
}