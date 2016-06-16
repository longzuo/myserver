#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

int main()
{
	int client_fd;
	struct sockaddr_in server_sock_addr;

	char buff[100];

	if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	server_sock_addr.sin_addr.s_addr = inet_addr("10.6.157.125");
	server_sock_addr.sin_port = htons(8123);
	server_sock_addr.sin_family = AF_INET;

	if (connect(client_fd, (struct sockaddr  *)&server_sock_addr, sizeof(server_sock_addr)) < 0)
	{
		printf("connection error\n");
		return -1;
	}
	
	if (read(client_fd, buff, 100)) printf("%s\n", buff);
	close(client_fd);
	
}

