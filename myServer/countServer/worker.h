#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <memory.h>
#include <arpa/inet.h>

typedef struct sockaddr SA;
void termSignal();

void worker();
