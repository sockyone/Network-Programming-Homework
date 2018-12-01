/*
 * Hoten: Nguyen Van Phuc
 * MSSV: 16021091
 * Mota: Server mở cổng 3001 chờ kết nối TCP từ client
 *       Server hiển thị địa chỉ IP và cổng của client
 *       Server nhận thông báo từ client, đổi các chữ cái trong thông báo nhận được sang chữ hoa và gửi lại thông báo cho Client
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
    if(sockfd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in sockaddr;
    bzero(&sockaddr, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(3001);
    sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sockfd, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) < 0) {
        perror("bind");
        return 1;
    }

    if (listen(sockfd, 10) < 0) {
        perror("listen");
        return 1;
    }

    /* Keep server alive */
    while(1) {
        printf("Waiting for connection... (Press Ctrl-c to stop)\n");
        struct sockaddr_in cliaddr;
        int newfd;
        unsigned int cliaddr_len = sizeof(cliaddr);
        newfd = accept(sockfd, (struct sockaddr*) &cliaddr, &cliaddr_len);
        if (newfd < 0)  {
            perror("accept");
            return 1;
        }

        char *ip = inet_ntoa(cliaddr.sin_addr);
        uint16_t port = ntohs(cliaddr.sin_port);
        printf("Client address: %s:%u\n", ip, port);

        /* Keep listening from client */
        while(1) {

            int length = 0;
            // Read header from client
            int status = readFromSocket(newfd, &length, sizeof(length));
            if (status == 0) {
                break;
            }
            char client_mess[length + 1];
            bzero(client_mess, sizeof(client_mess));
            // Read message from client
            readFromSocket(newfd, client_mess, length);
            printf("<<< %s\n", client_mess);

            toUpperCase(client_mess);

            writeToSocket(newfd, client_mess, length);
        }
    }

    return 0;
}
