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
#include <pthread.h>
#include "../utils.h"

/* New struct of socket's info */
typedef struct {
	int newfd;
	struct sockaddr_in cliaddr;
} socket_info;

/* Func as args of pthread_create() */
void *sendFile(void *args);

/* Total files server send to clients */
int ndone;
pthread_mutex_t	ndone_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Main method */
int main() {
	int sockfd;

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

	if (bind(sockfd, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) < 0)
		error("bind");

	if (listen(sockfd, 10) < 0) 
		error("listen");

	/* Socket info as args of sendFile() */
	socket_info *client_info;

	while(1) {
		client_info = (socket_info *) malloc(sizeof(socket_info));
		struct sockaddr_in cliaddr;
		int newfd;
		unsigned int cliaddr_len = sizeof(cliaddr);
		newfd = accept(sockfd, (struct sockaddr *) &cliaddr, &cliaddr_len);

		if (newfd < 0)  {
			error("accept");
			continue;
		}
		client_info->newfd = newfd;
		client_info->cliaddr = cliaddr;

		/* Create new thread */
		pthread_t tid;
		int thread = pthread_create(&tid, NULL, &sendFile, (void *) client_info);
		if (thread != 0) {
			printf("Create thread error.\n");
			close(client_info->newfd);
		}
	}

	return 0;
}

/* Func to send file from server to client */
void *sendFile(void *args) {
	/* Get client address */
	pthread_detach(pthread_self());
	socket_info client_info = *((socket_info *) args);
	int newfd = client_info.newfd;
	char *ip = inet_ntoa(client_info.cliaddr.sin_addr);
	uint16_t port = ntohs(client_info.cliaddr.sin_port);
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

		/* Get file size */
		fseek(fp, 0, SEEK_END);
		fsize = ftell(fp);
		fseek(fp, 0, SEEK_SET);  //same as rewind(fp);

		/* Send file to client */
		char *content = (char *) malloc(fsize + 1);
		fread(content, fsize, 1, fp);
		writeToSocket(newfd, &fsize, sizeof(fsize));
		writeToSocket(newfd, content, fsize);
		content[fsize] = 0;
		fclose(fp);

		/* Increase total file sent */
		pthread_mutex_lock(&ndone_mutex);
		ndone++;
		printf("Total files sent: %d\n", ndone);
		pthread_mutex_unlock(&ndone_mutex);
	}
	printf("Disconnected from %s:%u.\n", ip, port);
	return NULL;
}
