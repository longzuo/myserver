#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <vector>
#include <map>
#include <iostream>

#define BUFFSIZE 100

using namespace std;
struct ClientEntry
{
    int sockFd;
    struct sockaddr_in addr;
};

class ChatServer
{
    public:
        ChatServer(int port=8123):_port(port){}
        ~ChatServer(){}

        void start();


    private:
        int _port;
        int _listenFd;
        fd_set _readFds, _writeFds;
        vector<ClientEntry> _clientVec;
        char _buff[BUFFSIZE];


        void checkSelectFds();
        int resetSelectFds();
};
void ChatServer::start()
{
   	struct sockaddr_in server_sock_addr;
    
	/* create the listen socket fd*/
	if ((_listenFd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		cout<<"create socket error"<<endl;
		return ;
	}	
	
	/* set the server_sock_addr*/
	server_sock_addr.sin_family = AF_INET;
	server_sock_addr.sin_port = htons(_port);
	server_sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(_listenFd, (struct sockaddr *)&server_sock_addr, sizeof(server_sock_addr)) < 0) {
		cout<<"bind error";
		return ;
	}

	if (listen(_listenFd, 1024) < 0) {
		cout<<"listen error\n";
		return ;
	}


     
    while(1)
    {
        // set the _listenFd to _readFds  client to the _writeFds
        int max;
        max = resetSelectFds();
        struct timeval timeout;
        timeout.tv_sec=3;                  
        timeout.tv_usec=0;

        // select waiting
        cout<<"listening:"<<_clientVec.size()+1<<endl;
        int ret;
        ret = select(max+1, &_readFds, NULL,NULL,&timeout);
        if(ret == 0) 
        {
           cout<<"time out";
           continue; 
         
        }
        else if (ret < 0)
        {
            cout<<"select error";
            continue;
        }

        
        // judge which is ready    
        checkSelectFds();
    }
    
}

int ChatServer::resetSelectFds()
{
    int max;
        
    max = 0;
    FD_ZERO(&_readFds);
    FD_SET(_listenFd,&_readFds);
    if(_listenFd > max) max = _listenFd;
    
    for(int i = 0; i < _clientVec.size(); i++)
    {
        FD_SET(_clientVec[i].sockFd,&_readFds);
        if(_clientVec[i].sockFd > max) max = _clientVec[i].sockFd;
    }
    return max;
}
void ChatServer::checkSelectFds()
{
    //check clientFd
    for (int i =0; i< _clientVec.size(); i++)
    {
        if (FD_ISSET(_clientVec[i].sockFd,&_readFds))
        {
            string receiveStr;
            int ret;
            do{
                ret = recv(_clientVec[i].sockFd, _buff,BUFFSIZE,0);    
                if (ret <=  0)
                {
                    cout<<"receive error"<<endl;    
                    //vector<ClientEntry>::iterator itr = _clientVec.begin();
                    //itr += i;
                    _clientVec.erase(_clientVec.begin()+i);
                    break;
                }
                receiveStr += string(_buff,ret);
            }while(ret == BUFFSIZE);
            char tmp[30];
            sprintf(tmp,"%s",inet_ntoa(_clientVec[i].addr.sin_addr));
            cout<<"receive form "<<tmp<<":"<<receiveStr<<endl; 
            

        }
    }

    //check listenFd
    if (FD_ISSET(_listenFd, &_readFds))
    {
        struct sockaddr_in client_sock_addr;
        int client_fd;
        int client_sock_addr_len = sizeof(client_sock_addr);
        
        client_fd = accept(_listenFd, (struct sockaddr *)&client_sock_addr, (socklen_t*)&client_sock_addr_len);
        if(client_fd <= 0)
        {
            cout<<"accept error"<<endl;    
            return ;
        }
                char tmp[30];
                sprintf(tmp,"%s:%d",inet_ntoa(client_sock_addr.sin_addr),client_sock_addr.sin_port);
                cout<<"new connection:"<<tmp<<endl; 
        ClientEntry newEntry;
        newEntry.sockFd = client_fd;
        memcpy(&newEntry.addr, &client_sock_addr, sizeof(struct sockaddr_in));
        _clientVec.push_back(newEntry);
    }
    
}

int main()
{
    ChatServer chatServer;
    chatServer.start();    
    return 0;
}



