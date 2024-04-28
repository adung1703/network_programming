#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

struct SinhVien {
    int MSSV;
    char HoTen[100];
    char NgaySinh[20];
    float CPA;
};


void write_log(FILE *log_file, struct sockaddr_in client_addr, struct SinhVien sv) {
    time_t current_time;
    char time_string[20];

    time(&current_time);
    strftime(time_string, sizeof(time_string), "%Y-%m-%d %H:%M:%S", localtime(&current_time));

    fprintf(log_file, "%s %s %d %s %s %.2f\n", inet_ntoa(client_addr.sin_addr), time_string, sv.MSSV, sv.HoTen, sv.NgaySinh, sv.CPA);
}

int main(int argc, char *argv[]) {
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
    addr.sin_port = htons(atoi(argv[1]));

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

    FILE *content = fopen(argv[2], "a");

    struct sockaddr_in client_addr;
    socklen_t client_addrlen = sizeof(client_addr);
    int client = accept(listener, (struct sockaddr *)&client_addr, &client_addrlen);

    struct SinhVien SV;
    
    int ret = recv(client, &SV, sizeof(SV), 0);

    write_log(content, addr, SV);

    fclose(content);

    close(client);
    close(listener);

    return 1;
}