#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
typedef struct sockaddr SA;
int pnum = 4;
int stop_flag = 0;

void stopSignal();
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
        while (pnum -- > 0)
        {
            printf("waiting connection...\n");
            pid_t workerPid = fork();
            if (workerPid == -1)
            {
                printf("fork error");
                exit(0);
            }
            else if (workerPid > 0 )
            {
                signal(SIGINT, stopSignal);
                printf("parents:%d is working\n", workerPid);
            }
            else if (workerPid == 0)
            {
                pid_t self = getpid();
                //while(stop_flag == 0)
                while(1)
                {
	    	        client_fd = accept(listen_fd, (SA *)&client_sock_addr, &client_sock_addr_len);
                    printf("child :%d serving,stop_flag:%d\n", self, stop_flag);
		            write(client_fd, "hello", 6);
                }
		        close(client_fd);
                close(listen_fd);

                printf("child:%d out\n",self);
                exit(0);
            }
		    //client_ip = inet_ntoa(client_sock_addr.sin_addr);
		    //printf("service request from %s\n", client_ip);
        }
        while (stop_flag == 0)
        {
            sleep(1);
            printf("parents\n");
        }
        printf("parents out\n");
        close(listen_fd);
        exit(0);
	}
}
void stopSignal()
{
    stop_flag = 1;
}
