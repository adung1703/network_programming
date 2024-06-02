#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <errno.h>

int main() 
{
    int client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("10.13.32.211");
    addr.sin_port = htons(21);

    int ret = connect(client, (struct sockaddr *) &addr, sizeof(addr));
    if (ret == -1) {
        perror("connect() failed");
        return 1;
    }

    char buf[2048];

    // Nhan xau chao
    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    // Dang nhap
    char user[32], pass[32];
    printf("Nhap username: ");
    fgets(user, sizeof(user), stdin);
    printf("Nhap password: ");
    fgets(pass, sizeof(pass), stdin);

    // Xoa ky tu xuong trong
    user[strlen(user) - 1] = 0;
    pass[strlen(pass) - 1] = 0;

    // Gui lenh USER
    sprintf(buf, "USER %s\r\n", user);
    send(client, buf, strlen(buf), 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    // Gui lenh PASS
    sprintf(buf, "PASS %s\r\n", pass);
    send(client, buf, strlen(buf), 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    // Kiem tra dang nhap thanh cong
    if (strncmp(buf, "230 ", 4) == 0) {
        puts("Dang nhap thanh cong.");
    } else {
        puts("Dang nhap that bai.");
        close(client);
        return 1;
    }

    // Dang nhap thanh cong. Lay ve danh sach thu muc va tap tin.
    // Gui lenh PASV
    send(client, "PASV\r\n", 6, 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    // Xu ly ket qua lenh PASV
    char *pos = strchr(buf, '('); 

    // 1. Lay ra dia chi IP
    char *p1 = strtok(pos, "(),");
    char *p2 = strtok(NULL, "(),");
    char *p3 = strtok(NULL, "(),");
    char *p4 = strtok(NULL, "(),");
    char IP[16];
    sprintf(IP, "%s.%s.%s.%s\n", p1, p2, p3, p4);
    printf("%s", IP);
    // 2. Lay ra gia tri cong
    char *i1 = strtok(NULL, "(),");
    char *i2 = strtok(NULL, "(),");
    unsigned short port = atoi(i1) * 256 + atoi(i2);
    printf("%d\n", port);

    // 3. Mo ket noi den dia chi tren server
    struct sockaddr_in new_addr;
    new_addr.sin_family = AF_INET;
    new_addr.sin_addr.s_addr = inet_addr(IP);
    new_addr.sin_port = htons(port);

    int data_client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    ret = connect(data_client, (struct sockaddr *) &new_addr, sizeof(new_addr));
    if (ret == -1) {
        perror("connect() failed");
        return 1;
    }

    printf("Ket noi thanh cong!\n");

    // Gui lenh LIST
    send(client, "LIST\r\n", 6, 0);

    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);
    // Nhan du lieu tren kenh du lieu
    // In ra man hinh

    while (1)
    {
        ret = recv(data_client, buf, sizeof(buf)-1, 0);
        if (ret <= 0) {
            close(data_client);
            return 1;
        }

        buf[ret] = 0;
        puts(buf);
    }
    
    ret = recv(client, buf, sizeof(buf), 0);
    if (ret <= 0) {
        close(client);
        return 1;
    }

    buf[ret] = 0;
    puts(buf);

    close(client);
}