#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <memory.h>
#include <pthread.h>

typedef struct
{
    int pipe[2];
}StateLink;

typedef struct sockaddr SA;
int pnum = 2;
int stop_flag = 0;
int childPid[10];
int listen_fd;
int pipeFd1[2];// parents read 
int pipeFd2[2];// parents write

int g_requestNum;// can them be local var?
int g_totalTime;


// signal function excuted when sigint received
void stopSignal()
{
    int i = 0;
    while(childPid[i] != 0)
    {
        kill(childPid[i], SIGTERM);
        i++;
    }
    stop_flag = 1;
}
int createListenFd(int port)
{
	struct sockaddr_in server_sock_addr;
	int listenfd;
    
	/* create the listen socket fd*/
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("create socket error\n");
		return -1;
	}	
	
	/* set the server_sock_addr*/
	server_sock_addr.sin_family = AF_INET;
	server_sock_addr.sin_port = htons(port);
	server_sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listenfd, (SA *)&server_sock_addr, sizeof(server_sock_addr)) < 0) {
		printf("bind error\n");
		return -1;
	}

	if (listen(listenfd, 1024) < 0) {
		printf("listen error\n");
		return -1;
	}
    return listenfd;
}

void *collectStats(void *p)
{
    /*
    sigset_t newmask, oldmask;
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGINT);
    if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) < 0)
    {
        printf("sigprocmask error");    
    }
    */
    char runTime[2];
    //fcntl(pipeFd1[0], F_SETFL,  O_NONBLOCK);
    while(stop_flag == 0)
    {
        //printf("thread running\n");
        while((read(pipeFd1[0], runTime, sizeof(runTime))) > 0)
        {
            printf("thread read:%d", runTime[0]);
            g_requestNum++;
            g_totalTime += runTime[0];
            printf("num:%d\n",g_requestNum);
            char count[10];
            sprintf(count, "%d", g_requestNum);
            write(pipeFd2[1], count, sizeof(count));

        }
        sleep(1);
    }
}

int main(int argc,char *argv[])
{

    //used for collect info from child
    if(pipe(pipeFd1) == -1)
    {
        printf("pipe error\n");
    }
    if(pipe(pipeFd2) == -1)
    {
        printf("pipe error\n");
    }
    fcntl(pipeFd1[0], F_SETFL,  O_NONBLOCK);
    fcntl(pipeFd1[1], F_SETFL,  O_NONBLOCK);
    fcntl(pipeFd2[0], F_SETFL,  O_NONBLOCK);
    fcntl(pipeFd2[1], F_SETFL,  O_NONBLOCK);

    if ((listen_fd = createListenFd(8123)) == -1)
    {
        printf("create listenFd error");
        exit(-1);
    }
    
    pthread_t statsThreadId;
    statsThreadId = pthread_create(&statsThreadId, NULL, collectStats, NULL);
    if(statsThreadId != 0)
    {
        printf("create pthread error");
        exit(-1);
    }

    while (pnum -- > 0)
    {
        pid_t workerPid = fork();
        if (workerPid == -1)
        {
            printf("fork error");
            exit(0);
        }
        else if (workerPid == 0)
        {
            worker(); 
        }
        else if (workerPid > 0 )
        {
            int i = 0;
            while(childPid[i] != 0)i++;
            childPid[i] = workerPid;
            printf("parents:%d is working\n", workerPid);
        }
    }
    signal(SIGINT, stopSignal);//when ctrl+c send pipe command to children and switch stop_flag
    close(pipeFd1[1]);//close the write end;
    close(pipeFd2[0]);//close the read end;
    while (stop_flag == 0)
    {
        waitpid(-1, NULL,WNOHANG);
        sleep(1);
    }
    close(pipeFd1[0]);
    close(pipeFd2[1]);
    //pthread_join(statsThreadId, NULL);
    close(listen_fd);
    printf("parents out\n");
    exit(0);
}

