#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <vector>
#include <map>
#include <iostream>
#include <sstream>

#include "todeService.h"

#define BUFFSIZE 100

using namespace std;
struct ClientEntry
{
    int sockFd;
    struct sockaddr_in addr;
};

class Tiny
{
    public:
        Tiny(int port=8124):_port(port){}
        ~Tiny(){}

        void start();


    private:
        int _port;
        int _listenFd;
        fd_set _readFds, _writeFds;
        vector<ClientEntry> _clientVec;
        char _buff[BUFFSIZE];


        void checkSelectFds();
        int resetSelectFds();

        void parseInput(string& str, string& method, string& url, string& version,string& content);
        void doFd(int i);
};
void Tiny::parseInput(string& str, string& method, string& url, string& version,string& content)
{
   //format HTTP /index.html HTTP/1.1\r\nXXXX\r\n\r\nkey=value
   istringstream iss(str);
   iss>>method>>url>>version; 
   size_t pos = str.find("\r\n\r\n");
   if (pos != string::npos)
    content = str.substr(pos+4);
}
void Tiny::doFd(int i)
{
    int fd = _clientVec[i].sockFd;

    string receiveStr;
    int ret;
    do{
        ret = recv(fd, _buff,BUFFSIZE,0);    
        if (ret <=  0)
        {
            cout<<"receive error"<<endl;    
            _clientVec.erase(_clientVec.begin()+i);
            close(fd);
            return;
        }
        receiveStr += string(_buff,ret);
    }while(ret == BUFFSIZE);
    string method;
    string url;
    string version;
    string content;
    parseInput(receiveStr,method, url, version,content);
    
    char tmp[30];
    sprintf(tmp,"%s",inet_ntoa(_clientVec[i].addr.sin_addr));
    cout<<"receive form "<<tmp<<":"<<receiveStr<<"|"<<endl; 
    cout<<"content:"<<content<<"|"<<endl;
    size_t spos = content.find('=');
    if (spos != string::npos)
    {
        string input = content.substr(spos+1);
        string output;
        todeService todeservice;
        todeservice.process(input, output); 
        cout<<"output:"<<output<<endl;
    }
    
    url = "."+url;
    if(url.find("cgi") != string::npos)
    {
        int rtn;
        if(fork() == 0)
        {
            dup2(fd,STDOUT_FILENO);    
            if(execlp(url.c_str(),url.c_str(),NULL) <0) cout<<"execlp error"<<endl; 
        }
        wait(&rtn);
    }
    else
    {
        int fileFd = open(url.c_str(),O_RDONLY);
        if(fileFd <= 0) cout<<"open error"<<endl;
        struct stat stat_buf;
        fstat(fileFd, &stat_buf);
        //sleep(3);

        sendfile(fd, fileFd, NULL, stat_buf.st_size);
    } 
    
    //cout<<method<<url<<version;
    /*
    ret = send(fd, receiveStr.c_str(), receiveStr.size(), 0);
    if (ret <= 0)
    {
        cout<<"send error"<<endl;    
    }
    else
    {
        cout<<"send ok!"<<endl;    
    }
    */
    _clientVec.erase(_clientVec.begin()+i);
    close(fd);
    
}
void Tiny::start()
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
           //cout<<"time out";
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

int Tiny::resetSelectFds()
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
void Tiny::checkSelectFds()
{
    //check clientFd
    for (int i =0; i< _clientVec.size(); i++)
    {
        if (FD_ISSET(_clientVec[i].sockFd,&_readFds))
        {
            doFd(i);
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
    Tiny chatServer;
    chatServer.start();    
    return 0;
}



