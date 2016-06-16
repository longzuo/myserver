#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
typedef struct sockaddr SA;

int main(int argc,char *argv[])
{
	struct sockaddr_in server_sock_addr;
	struct sockaddr_in client_sock_addr;
	int listen_fd;

	int client_fd;
	int client_sock_addr_len;

	char *client_ip=NULL;
	
	/* create the listen socket fd*/
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("create socket error\n");
		return -1;
	}	
	
	/* set the server_sock_addr*/
	server_sock_addr.sin_family = AF_INET;
	server_sock_addr.sin_port = htons(8123);
	server_sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listen_fd, (SA *)&server_sock_addr, sizeof(server_sock_addr)) < 0) {
		printf("bind error\n");
		return -1;
	}

	if (listen(listen_fd, 1024) < 0) {
		printf("listen error\n");
		return -1;
	}
	while (1) {
        printf("waiting connection...\n");
		client_fd = accept(listen_fd, (SA *)&client_sock_addr, &client_sock_addr_len);
		//client_ip = inet_ntoa(client_sock_addr.sin_addr);
		//printf("service request from %s\n", client_ip);
		write(client_fd, "hello", 6);
		close(client_fd);
	}
}
