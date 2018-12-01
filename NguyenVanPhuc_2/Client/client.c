/*
 * Hoten: Nguyen Van Phuc
 * MSSV: 16021019
 * Mota: Client nhập địa chỉ IP của server từ bàn phím và kết nối với server qua cổng của server(3001)
 *       Người dùng nhập tên file từ bàn phím
 *       Client gửi tên file cần download cho server
 *       Client đợi phản hồi, hiển thị thông tin file nhận được từ server
 *       Client lặp lại yêu cầu gửi tên file cần download cho server và tải file về
 *       Khi nhập ký tự đặc biệt ('QUIT'), client đóng kết nối
 */

#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include "../utils.h"

int main() {
	int sockfd;

	int family = AF_INET;
	int type = SOCK_STREAM;
	int protocol = 0;

	/* Setup socket */
	sockfd = socket(family, type, protocol);
	if (sockfd < 0) {
		error("socket");
	}


	char ip_input[15];
	int port_input;
	int buffer;
	printf("IP: ");
	scanf("%s", ip_input);
	printf("Port: ");
	scanf("%d", &port_input);
	printf("Buffer: ");
	scanf("%d", &buffer);
	getchar();

	if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &buffer, sizeof(int)) < 0)
		error("setsockopt sndbuf");
	if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF,  &buffer, sizeof(int)) < 0)
		error("setsockopt rcvbuf");

	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port_input);
	servaddr.sin_addr.s_addr = inet_addr(ip_input);
	/* servaddr.sin_port = htons(3001); */
	/* servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); */

	if (connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0) {
		error("connect");
	}

	char exitCommand[] = "QUIT";
	printf("Connected! (Enter '%s' to stop)\n", exitCommand);

	while(1) {
		char fileName[100];
		printf(">>> ");
		fgets(fileName, sizeof(fileName), stdin);
		unsigned int fileName_len = strlen(fileName);
		fileName[fileName_len - 1] = '\0';

		if (strcmp(fileName, exitCommand) == 0) {
			break;
		}

		struct timespec send_time, rcv_time;

		writeToSocket(sockfd, &fileName_len, sizeof(fileName_len));
		clock_gettime(CLOCK_MONOTONIC_RAW, &send_time);
		writeToSocket(sockfd, fileName, fileName_len);

		long fsize;
		if (readFromSocket(sockfd, &fsize, sizeof(fsize)) <= 0) {
			error("read");
		}
		if (fsize <= 0) {
			printf("No file on server.\n");
			continue;
		}
		printf("File size: %ldbytes\n", fsize);
		char *content = (char*) malloc(fsize);
		FILE *fp = fopen(fileName, "wb" );
		if(readFromSocket(sockfd, content, fsize) <= 0) {
			error("read file's content");
		}
		clock_gettime(CLOCK_MONOTONIC_RAW, &rcv_time);

		long time_total = (rcv_time.tv_sec - send_time.tv_sec) * 1e3 + (rcv_time.tv_nsec - send_time.tv_nsec) / 1e6;
		printf("Total time: %ldms\n", time_total);
		fwrite(content, fsize, 1, fp);
		fclose(fp);
		bzero(&content, sizeof(content));
	}

	close(sockfd);

	return 0;
}
