#include "worker.h"
extern int listen_fd;
extern int pipeFd[2];
extern int stop_flag;

void worker()
{
    stop_flag = 0;
    
    close(pipeFd[0]);

    pid_t self = getpid();
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM,termSignal);
    struct sockaddr_in client_sock_addr;
    int client_fd;
    int client_sock_addr_len = sizeof(client_sock_addr);
    char *client_ip=NULL;
    while(stop_flag == 0)
    {
        client_fd = accept(listen_fd, (struct sockaddr *)&client_sock_addr, &client_sock_addr_len);
        //client_ip = inet_ntoa(client_sock_addr.sin_addr);
        char tmp[30];
        sprintf(tmp,"%s",inet_ntoa(client_sock_addr.sin_addr));
        printf("service request from %s:%d\n", tmp,client_sock_addr.sin_port);
        char buf[20];
        char stat[2];
        memset(buf, 0, sizeof(buf));
        sprintf(buf, "%d say hello", self);
        write(client_fd, buf, sizeof(buf));
        stat[0] = 5;
        write(pipeFd[1],stat,sizeof(stat));
    }
    close(client_fd);
    //close(listen_fd);

    printf("child:%d out\n",self);
    exit(0);

}
void termSignal()
{
   stop_flag = 1;
   if(listen_fd != 0)
   {
       printf("child receive term\n");
       if(close(listen_fd) == -1)
       {
           printf("close listenfd error\n");
       }
   }
}

