#ifndef CHATROOM_CLIENT_H
#define CHATROOM_CLIENT_H
#include"Common.h"
#include<string>
using namespace std;

class Client{
public:
    Client();
    void Start();
    void Close();
    void Connect();
    void Show_help();

private:
    string name;//  
    bool IsNickNameSet;
    int sock; //当前连接服务器的socket
    int pid;//current pid
    int epfd;//return of epoll_creat

    //创建管道
    int pip_fd[2];//pid_fd[0]:读管道, pid_fd[1]:写管道 
    struct sockaddr_in server;//连接的服务器
    char buff[BUF_SIZE];//信息缓冲区

    bool isClientWork;
};

#endif