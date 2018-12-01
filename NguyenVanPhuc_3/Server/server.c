/*
 * Hoten: Nguyen Van Phuc
 * MSSV: 16021091
 * Mota: Server mở cổng 3001 chờ kết nối TCP từ Client.
 *       Server hiển thị địa chỉ IP và cổng của Client khi Client kết nối tới.
 *       Server nhận tên file từ Client, Server gửi lại file trong thư mục của Server cho Client.
 */

#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include "../utils.h"

void sig_chld() {
	pid_t pid;
	int status;

	while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
		printf("Child %d terminated.\n", pid);
	}
	return;
}

int main() {
	int sockfd;
	pid_t childpid;

	int family = AF_INET;
	int type = SOCK_STREAM;
	int protocol = 0;

	sockfd = socket(family, type, protocol);
	if (sockfd < 0) {
		error("socket");
	}

	/* Setup socket */
	int reuseaddr = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &reuseaddr, sizeof(int)) < 0)
		error("setsockopt reuseaddr");

	struct sockaddr_in sockaddr;
	bzero(&sockaddr, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(3001);
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockfd, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) < 0) {
		error("bind");
	}

	if (listen(sockfd, 10) < 0) {
		error("listen");
	}

	signal(SIGCHLD, sig_chld);

	while(1) {
		struct sockaddr_in cliaddr;
		int newfd;
		unsigned int cliaddr_len = sizeof(cliaddr);
		newfd = accept(sockfd, (struct sockaddr*) &cliaddr, &cliaddr_len);
		if (newfd < 0)  {
			if (errno == EINTR) {
				continue;
			} else {
				error("accept");
			}
		}

		childpid = fork();
		if (childpid == 0) {
			close(sockfd);
			char *ip = inet_ntoa(cliaddr.sin_addr);
			uint16_t port = ntohs(cliaddr.sin_port);
			printf("Client address: %s:%u\n", ip, port);

			while(1) {
				int length = 0;

				int status = readFromSocket(newfd, &length, sizeof(length));
				if (status == 0) {
					close(newfd);
					break;
				}
				char fileName[length + 1] ;
				bzero(fileName, sizeof(fileName));

				if(readFromSocket(newfd, fileName, length)< 0){
					error("read fileName");
				}
				printf("File's name: %s\n", fileName);

				FILE *fp;
				fp = fopen(fileName, "rb");
				long fsize;
				if (fp == NULL) {
					fsize = -1;
					writeToSocket(newfd, &fsize, sizeof(fsize));
					printf("File not found.\n");
					continue;
				}

				fseek(fp, 0, SEEK_END);
				fsize = ftell(fp);
				fseek(fp, 0, SEEK_SET);  //same as rewind(fp);

				char *content = (char*) malloc(fsize + 1);
				fread(content, fsize, 1, fp);
				writeToSocket(newfd, &fsize, sizeof(fsize));
				writeToSocket(newfd, content, fsize);
				content[fsize] = 0;
				fclose(fp);
			}
			exit(0);
		}
		close(newfd);
	}

	return 0;
}
