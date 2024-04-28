#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));

    int ret = connect(client, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1) {
        perror("connect() failed");
        return 1;
    }

    struct SV {
        int MSSV;
        char HoTen[100];
        char NgaySinh[20];
        float CPA;
    } SinhVien;

    printf("Nhap Ma So Sinh Vien: \n");
    scanf("%d", &SinhVien.MSSV);

    printf("Nhap Ho va Ten: \n");
    getchar();
    fgets(SinhVien.HoTen, 101, stdin);
    SinhVien.HoTen[strcspn(SinhVien.HoTen, "\n")] = '\0';

    printf("Nhap Ngay Sinh: \n");
    scanf("%s", SinhVien.NgaySinh);
    
    printf("Nhap CPA: \n");
    scanf("%f", &SinhVien.CPA);

    send(client, &SinhVien, sizeof(SinhVien), 0);

    close(client);
    return 0;
}