#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <dirent.h>
#include <netinet/in.h>
#include <sys/stat.h>

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

        DIR *dir;
        char msg[256];
        struct dirent *entry;
        if ((dir = opendir(".")) != NULL) {
            int file_count = 0;
            
            while ((entry = readdir(dir)) != NULL) {
                sprintf(msg, "%s%s\r\n", msg, entry->d_name);
                file_count++;
            }
            if (file_count > 0) {
                sprintf(msg, "OK %d\r\n%s\r\n\r\n", file_count, msg);
                send(client, msg, strlen(msg), 0);
            } else {
                send(client, "ERROR No files to download \r\n", strlen("ERROR No files to download \r\n"), 0);
                close(client);
            }

        }
                // Nhan ten file va xu li
        int ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            break;
        buf[ret] = 0;

        struct stat st;
        int file_size;
        char size[32];
        if (stat(buf, &st) == 0) {
            file_size = st.st_size;
            sprintf(size, "OK %d\r\n", file_size);
            send(client, size, strlen(size), 0);

            // Mở file và gửi nội dung
            FILE *file = fopen(buf, "rb");
            if (file) {
                while ((ret = fread(buf, 1, sizeof(buf), file)) > 0) {
                    send(client, buf, ret, 0);
                }
                fclose(file);
            }
        } else {
            send(client, "ERROR File not found \r\n", strlen("ERROR File not found \r\n"), 0);
        }        
    }

    close(client);
}
