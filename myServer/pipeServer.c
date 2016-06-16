#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
typedef struct sockaddr SA;
int pnum = 2;
int stop_flag = 0;

int pipeFd[2];
// signal function excuted when sigint received
void stopSignal()
{
    stop_flag = 1;
    char s[]="stop";
    write(pipeFd[1], s, sizeof(s));
}
//check stop flag to decide whether to continue 
// only used by children
//only excuted after a request is accepted
// if the child is hanged at the accepted, stop flag will not changed
// when more than one child is forked, only one would read the message
// writed by the parent, so it can't be used to stop the child by this case
void checkStop()
{
    char s[10];
    int readSize;
    //set the read end to nonblock mod 
    fcntl(pipeFd[0], F_SETFL, O_NONBLOCK);
    if((readSize = read(pipeFd[0], s, sizeof(s))) > 0)
    {
        stop_flag = 1;
    }
}

int createListenFd(int port)
{
	struct sockaddr_in server_sock_addr;
	int listen_fd;

	/* create the listen socket fd*/
	if ((listen_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("create socket error\n");
		return -1;
	}	
	
	/* set the server_sock_addr*/
	server_sock_addr.sin_family = AF_INET;
	server_sock_addr.sin_port = htons(port);
	server_sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listen_fd, (SA *)&server_sock_addr, sizeof(server_sock_addr)) < 0) {
		printf("bind error\n");
		return -1;
	}

	if (listen(listen_fd, 1024) < 0) {
		printf("listen error\n");
		return -1;
	}
    return listen_fd;
}

int main(int argc,char *argv[])
{
    int listen_fd;
    if ((listen_fd = createListenFd(8123)) == -1)
    {
        printf("create listenFd error");
        exit(-1);
    }
   if(pipe(pipeFd) != 0)
    {
        printf("create pipe error");
        exit(-1);
    }

    while (pnum -- > 0)
    {
        printf("waiting connection...\n");
        pid_t workerPid = fork();
        if (workerPid == -1)
        {
            printf("fork error");
            exit(0);
        }
        else if (workerPid == 0)
        {
            pid_t self = getpid();
            close(pipeFd[1]);//close the write fd
            signal(SIGINT, SIG_IGN);
            struct sockaddr_in client_sock_addr;
            int client_fd;
            int client_sock_addr_len;
            char *client_ip=NULL;
            while(stop_flag == 0)
            //while(1)
            {
                client_fd = accept(listen_fd, (SA *)&client_sock_addr, &client_sock_addr_len);
                printf("child :%d serving,stop_flag:%d\n", self, stop_flag);
                //client_ip = inet_ntoa(client_sock_addr.sin_addr);
                //printf("service request from %s\n", client_ip);
                write(client_fd, "hello", 6);
                checkStop();
            }
            close(client_fd);
            close(listen_fd);
            close(pipeFd[0]);

            printf("child:%d out\n",self);
            exit(0);
        }
        else if (workerPid > 0 )
        {
            close(pipeFd[0]);//close the read fd
            signal(SIGINT, stopSignal);//when ctrl+c send pipe command to children
            printf("parents:%d is working\n", workerPid);
        }
    }
    while (stop_flag == 0)
    {
        sleep(1);
    }
    printf("parents out\n");
    close(listen_fd);
    close(pipeFd[1]);//close the write end
    exit(0);
}

