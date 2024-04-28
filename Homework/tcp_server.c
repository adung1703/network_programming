#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

int main (int argc, char *argv[]) { //tcp_server <cổng> <tệp tin chứa câu chào> <tệp tin lưu nội dung client gửi đến>

    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed\n");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed\n");
        return 1;
    }

    if (listen(listener, 5)) {
        perror("listen() failed");
        return 1;
    }

    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    int client = accept(listener, (struct sockaddr *)&client_addr, &client_addrlen);
    if (client == -1) {
        perror("accept() failed");
        return 1;
    }

    FILE *hello = fopen(argv[2], "r");
    FILE *content = fopen(argv[3], "a");

    char welcome[1024];
    size_t bytes_read;
    while ((bytes_read = fread(welcome, 1, sizeof(welcome), hello)) > 0) {
        send(client, welcome, bytes_read, 0);
        if (feof(hello)) break; 
    }
    fclose(hello);
    
    char buf[1024];
    int ret;
    while (1) {
        ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0) break;
        fwrite(buf, 1, ret, content);
    }
    
    fclose(content);
    close(client);
    close(listener);
    return 0;
}