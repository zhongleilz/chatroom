#ifndef CHATROOM_SERVER_H
#define CHATROOM_SERVER_H

#include"Common.h"
#include<string>
#include<map>
#include<ctime>
using namespace std;

//定义客户端的信息
typedef struct {
    string client_host;//客户端主机名
    string client_port;//客户端端口号
    string client_name;//客户端名称
    
    int connfd;//clientfd
    bool is_nickname_set;
    char * join_time;//加入时间
}client_info;


//服务器类
class Server{
public:
    Server();//构造函数  
    //~Server();
    void Init();//服务器初始化设置
    void Start();//服务器启动
    void Close();//关闭
    
    int GetMessageToClient(int connfd );//私聊

private:
    int listenfd;//创建监听的socket    
    int epfd;//epoll_create返回值    
    struct sockaddr_in server;//服务器serveraddr的信息
    void remove_clients(int connfd);
    int  SendBroadcasttoMessage(int connfd);//广播消息给客户端

    list<int> clients_list;//客户段列表
    map<int,client_info> clients_map;

    int ShowUserInfoToClient(int connfd);//发送当前已连接服务器客户端的信息
    char *current_time;//当前时间
};

#endif