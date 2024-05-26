#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

void *client_proc(void *);

int main() {
    // Tao socket cho ket noi
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

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
    if (listen(listener, 10)) {
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
    char buf[2048];

    // Nhan du lieu tu client
    int ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        pthread_exit(NULL);
    }

    buf[ret] = 0;
    printf("Received from %d: %s\n", client, buf);
    
    if (strncmp(buf, "GET /", 5) == 0 || strncmp(buf, "POST /", 6) == 0) {
        // GET /get?a=value1&b=value2&cmd=value3 HTTP/1.1\r\nHost: localhost:9000\r\n\r\n

        char *pos1 = strstr(buf, "a=") + 2; //Tro vao dau value1
        char *pos2 = strstr(pos1, "b=") + 2; //Tro vao dau value2
        char *pos3 = strstr(pos2, "cmd=") + 4; //Tro vao dau value3

        char a[16] = {0};
        memcpy(a, pos1, pos2-3 - pos1);
        double a_value = atof(a);
        
        char b[16] = {0};
        memcpy(b, pos2, (pos3-5) - pos2);
        double b_value = atof(b);

        double res;
        char operator;
        if (strncmp(pos3, "add", 3) == 0) {
            res = a_value + b_value;
            operator = '+';
        } else if (strncmp(pos3, "sub", 3) == 0) {
            res = a_value - b_value;
            operator = '-';
        } else if (strncmp(pos3, "mul", 3) == 0) {
            res = a_value * b_value;
            operator = '*';
        } else {
            if (b_value == 0) {
                char msg[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Khong the chia mot so cho 0</h1>";
                send(client, msg, strlen(msg), 0);
                close(client);
                pthread_exit(NULL);
            }
            res = a_value / b_value;
            operator = ':';
        }

        sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>%lf %c %lf = %lf</h1>", 
                a_value, 
                operator, 
                b_value, 
                res);
        send(client, buf, strlen(buf), 0);
    } 

    close(client);
    pthread_exit(NULL);
}