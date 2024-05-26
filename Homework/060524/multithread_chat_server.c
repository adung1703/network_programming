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
int paired[64] = {0}; // Bieu dien ghep cap theo danh sach ke
int queue[64];
int front = 0, rear = -1;

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
        
        queue[++rear] = client;
        if ((rear - front + 1) % 2 == 0) {
            paired[queue[front]] = queue[front + 1];
            paired[queue[front + 1]] = queue[front];
            
            char msg1[64];
            sprintf(msg1, "Ban la client %d, duoc ket noi voi client: %d\n", queue[front + 1], queue[front]);
            send(queue[front + 1], msg1, strlen(msg1), 0);

            char msg2[64];
            sprintf(msg2, "Ban la client %d, duoc ket noi voi client: %d\n", queue[front], queue[front + 1]);
            send(queue[front], msg2, strlen(msg2), 0);

            front += 2;
        }
        
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
        
        char msg[256];
        sprintf(msg, "Client %d: %s", client, buf);
        
        if (paired[client] != 0) {
            if (send(paired[client], msg, strlen(msg), 0) <= 0) 
                break;
        }
    }
    close(paired[client]);
    close(client);
}
