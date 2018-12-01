/*
 * Hoten: Nguyen Van Phuc
 * MSSV: 16021019
 * Mota: Client nhập địa chỉ IP của server từ bàn phím và kết nối với server qua cổng của server(3001)
 *       Người dùng nhập chuỗi ký tự bất kỳ từ bàn phím
 *       Client đọc chuỗi ký tự nhập vào từ bàn phím và gửi thông báo này cho server
 *       Client đợi phản hồi, hiển thị thông báo nhận được từ server
 *       Client lặp lại việc đọc thông báo từ bàn phím và gửi thông báo cho server
 *       Khi nhập ký tự đặc biệt ('exit'), client đóng kết nối
 */

#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include "../utils.h"

int main() {
    int sockfd;

    int family = AF_INET;
    int type = SOCK_STREAM;
    int protocol = 0;

    sockfd = socket(family, type, protocol);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    char ip_input[15];
    int port_input;
    printf("IP: ");
    scanf("%s", ip_input);
    printf("Port: ");
    scanf("%d", &port_input);
    getchar();

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port_input);
    servaddr.sin_addr.s_addr = inet_addr(ip_input);
    /* servaddr.sin_port = htons(3001); */
    /* servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); */

    if(connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect");
        return 1;
    }

    printf("Connected! (Enter 'exit' to stop)\n");

    char exitCommand[] = "exit";

    while(1) {
        char client_mess[1000];
        printf(">>> ");
        fgets(client_mess, sizeof(client_mess), stdin);
        unsigned int client_mess_len = strlen(client_mess);
        client_mess[client_mess_len - 1] = '\0';

        if (strcmp(client_mess, exitCommand) == 0) {
            break;
        }

        // Send header to server (length of message)
        writeToSocket(sockfd, &client_mess_len, sizeof(client_mess_len));

        // Send message to server
        writeToSocket(sockfd, client_mess, client_mess_len);

        char server_mess[client_mess_len + 1];
        bzero(server_mess, sizeof(server_mess));
        readFromSocket(sockfd, server_mess, client_mess_len);

        printf("<<< %s\n", server_mess);
    }

    close(sockfd);

    return 0;
}
