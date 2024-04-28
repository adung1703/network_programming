#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/select.h>
#include <stdbool.h>

char buf[256];
int accepted[1024] = {0}; // Danh dau client da dang nhap
char welcome[] = "Dang nhap thanh cong\n";
char try_again[] = "Ten dang nhap hoac mat khau chua dung!\n";

// Kiem tra tai khoan co dung hay khong
bool check_account(char *user, char *pass, FILE *data) {
    char acc[128];
    while (fgets(acc, 128, data)) {
        char cur_user[64], cur_pass[64]; 
        sscanf(acc, "%s %s", cur_user, cur_pass);

        if (!strcmp(cur_user, user) && 
            !strcmp(cur_pass, pass)) {
                rewind(data);
                return true;
            }
    }
    rewind(data);
    return false;
}

// Dang nhap
void signin(int client, fd_set fdread, FILE *accs) {
    char login[128];
    char username[64], password[64];
    do {
        int ret = recv(client, login, sizeof(login), 0);
        if (ret <= 0) {
            accepted[client] = 0;
            close(client);
            FD_CLR(client, &fdread);
        } else {
            login[ret] = 0;
            printf("Received from %d: %s\n", client, login);
        }
        sscanf(login,"%s %s", username, password);

        if (check_account(username, password, accs)) {
            send(client, welcome, sizeof(welcome), 0);
            break;
        } else {
            send(client, try_again, sizeof(try_again), 0);
        }
    } while(1);
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

    FILE *accs = fopen("accounts.txt", "r");

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

                    // Neu chua dang nhap
                    if (!accepted[i]) {
                        printf("Moi dang nhap:\n");
                        signin(client, fdread, accs);
                        accepted[i] = 1;
                    } else {
                        // Nhan lenh tu client
                        ret = recv(client, buf, sizeof(buf), 0);               
                        if (ret <= 0) {
                            accepted[i] = 0;
                            close(client);
                            FD_CLR(client, &fdread);
                        } else {
                            buf[ret-1] = '\0'; // Do du lieu nhan co ca ki tu xuong dong
                            printf("Received from %d: %s\n", client, buf);
                            strcat(buf, " > out.txt");
                            printf("%s\n", buf);
                            system(buf);
                        }
                    } 
                }
            }
    }

    fclose(accs);

    return 0;
}