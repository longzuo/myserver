#include "worker.h"
extern int listen_fd;
extern int pipeFd1[2];
extern int pipeFd2[2];
extern int stop_flag;

void worker()
{
    stop_flag = 0;
    int self_count = 0;
    
    close(pipeFd1[0]);
    close(pipeFd2[1]);

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
        self_count++;
        //client_ip = inet_ntoa(client_sock_addr.sin_addr);
        char tmp[30];
        sprintf(tmp,"%s",inet_ntoa(client_sock_addr.sin_addr));
        printf("service request from %s:%d\n", tmp,client_sock_addr.sin_port);
        char buf[100];
        char stat[2];
        char count[10];
        memset(buf, 0, sizeof(buf));
        memset(stat, 0, sizeof(stat));
        memset(count, 0, sizeof(count));
        stat[0] = 5;
        //notify the controller to plus one
        write(pipeFd1[1],stat,sizeof(stat));
        //get the plus result
        read(pipeFd2[0], count, sizeof(count));

        sprintf(buf, "%d servering ,serve %d/%s times", self,self_count, count );
        write(client_fd, buf, sizeof(buf));
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

