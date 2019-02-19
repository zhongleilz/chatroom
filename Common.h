#ifndef CHARTROOM_COMMON_H
#define CHARTROOM_COMMON_H

#include<iostream>
#include<list>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/epoll.h>
#include<errno.h>
#include<fcntl.h>
#include<unistd.h>
#include<cstdio>
#include<stdlib.h>
#include<string.h>

#define SERVER_IP "127.0.0.1"//默认服务器IP地址
#define SERVER_PORT 8888//服务器端口号
#define EPOLL_SIZE 5000 //为epoll支持的最大句柄数
#define BUF_SIZE 0xFFFF //缓冲区大小
//#define SERVER_WELCOME "welcom you join to our chatroom. You chat ID is Client #%s"
#define SERVER_WELCOME "Weclome  %s  join our chatroom ! "
#define SERVER_MESSAGE "Client [%s] say >>%s" //收到消息的前缀
#define  EXIT "EXIT"//退出系统
#define CAUTION "There is only one in this chat room wow!"
#define LISTENQ 1024 // 

void setnonblocking(int sock);//设置为非阻塞

static void sub_space(const char *strIn, char *strOut)
{

    int i = 0;
    int j = strlen(strIn)-1;

    while(strIn[i] == ' ')
        ++i;

    while(strIn[j] == ' ')
        --j;
    strncpy(strOut, strIn + i , j - i + 1);
    strOut[j - i + 1] = '\0';
}

static void addfd(int epollfd, int fd, bool enable_et)
{
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN;
    if(enable_et)
        event.events = EPOLLIN | EPOLLET;//EPOLLIN 为数据可读 EPOLET 为ET工作方式
    
    //epoll事件注册函数

    epoll_ctl(epollfd, EPOLL_CTL_ADD,fd,&event);
    //setnonblocking(fd);//
    fcntl(fd, F_SETFL,fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
    //printf("fd added to epoll\n\n");
}

/*
void setnonblocking(int sock){
    int opts;
    opts = fcntl( sock, F_GETFD);
    if(opts < 0){
        perror("fcntl(sock,GETFD");
    }

    opts = opts | O_NONBLOCK;
    if(fcntl(sock,F_SETFL,opts<0))
    {
        perror("fctnl(sock, SETFL)");
        exit(1);
    } 
}   
*/

#endif