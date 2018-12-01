/*
 * Hoten: Nguyen Van Phuc
 * MSSV: 16021091
 * Mota: Server mở cổng 3001 chờ kết nối TCP từ client
 *       Server hiển thị địa chỉ IP và cổng của client
 *       Server nhận tên file từ client, Server gửi lại file trong thư mục của server cho client
 */

#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <errno.h>
#include "../utils.h"

int main() {
	int sockfd;
	struct sockaddr_in cliaddr;
	unsigned int cliaddr_len;

	int family = AF_INET;
	int type = SOCK_STREAM;
	int protocol = 0;

	int fd;
	int n_files = 0;
	int n_clients = 0;
	fd_set rfds;
	/* fd_set wfds; */
	/* fd_set efds; */
	fd_set afds;

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

	if (bind(sockfd, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) < 0)
		error("bind");

	if (listen(sockfd, 10) < 0)
		error("listen");

	FD_ZERO(&afds);
	FD_SET(sockfd, &afds);

	while(1) {
		memcpy(&rfds, &afds, sizeof(rfds));
		/* memcpy(&wfds, &afds, sizeof(wfds)); */
		/* memcpy(&efds, &afds, sizeof(efds)); */

		/* if (select(FD_SETSIZE, &rfds, &wfds, &efds, (struct timeval *) 0) < 0) */
		if (select(FD_SETSIZE, &rfds, (fd_set *) 0, (fd_set *) 0, (struct timeval *) 0) < 0)
			error("select");

		if (FD_ISSET(sockfd, &rfds)) {
			int newfd;
			cliaddr_len = sizeof(cliaddr);
			newfd = accept(sockfd, (struct sockaddr*) &cliaddr, &cliaddr_len);
			if (newfd < 0)  
				error("accept");

			char *ip = inet_ntoa(cliaddr.sin_addr);
			uint16_t port = ntohs(cliaddr.sin_port);
			printf("Client address: %s:%u\n", ip, port);

			FD_SET(newfd, &afds);
			n_clients++;
			printf("Total Clients: %d\n", n_clients);
		}

		for (fd = 0; fd < FD_SETSIZE; fd++) {
			if (fd != sockfd && FD_ISSET(fd, &rfds)) {
				int length = 0;

				int status = readFromSocket(fd, &length, sizeof(length));
				if (status == 0) {
					FD_CLR(fd, &afds);
					n_clients--;
					printf("Total Clients: %d\n", n_clients);
					close(fd);
					break;
				}
				char fileName[length + 1] ;
				bzero(fileName, sizeof(fileName));

				if(readFromSocket(fd, fileName, length)< 0)
					error("read fileName");
				printf("File's name: %s\n", fileName);

				FILE *fp;
				fp = fopen(fileName, "rb");
				long fsize;
				if (fp == NULL) {
					fsize = -1;
					writeToSocket(fd, &fsize, sizeof(fsize));
					printf("File not found.\n");
					continue;
				}

				fseek(fp, 0, SEEK_END);
				fsize = ftell(fp);
				fseek(fp, 0, SEEK_SET);  //same as rewind(fp);

				char *content = (char*) malloc(fsize + 1);
				fread(content, fsize, 1, fp);
				writeToSocket(fd, &fsize, sizeof(fsize));
				writeToSocket(fd, content, fsize);
				content[fsize] = 0;
				fclose(fp);

				n_files++;
				printf("Total files: %d\n", n_files);
			}
		}
	}

	return 0;
}
