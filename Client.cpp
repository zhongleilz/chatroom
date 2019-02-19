#include"Client.h"
#include<iostream>
#include"unp.h"

using namespace std;
Client::Client(){
    //初始化要连接的服务器
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);//8888
    inet_pton(AF_INET, SERVER_IP, &server.sin_addr);

    sock = 0;
    pid = 0;
    epfd = 0;
    isClientWork = true;
    IsNickNameSet  = false;
}

void Client::Show_help(){
    std::cout << "Usage:" << std::endl;
    std::cout << "     <message>               : send message to all online users ." << std::endl;
    std::cout << "     > <username> <message>  : send a message to a single user ." << std::endl;
    std::cout << "     exit                    : disconnect to the server and leave."<<std::endl;
    std::cout << "     clear                   : clear the screen."<<std::endl;
    std::cout << "     $ show users            : show informations of all currently online users"<<std::endl;
    std::cout << "\033[33mNow choose a nickname to start:\033[0m";
}

void Client::Connect(){
     // Step 1：创建socket
    // 使用socket()

    // Step 2：连接服务端
    // connect()

    // Step 3：创建管道，其中fd[0]用于父进程读，fd[1]用于子进程写
    // 使用pipe()

    // Step 4：创建epoll
    // epoll_create()

    // Step 5：将sock和管道读端描述符都添加到内核事件表中
    // addfd()
    cout<<"Connect server "<<SERVER_IP<<":"<<SERVER_PORT<<endl;
    
    //creat socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        perror("creat sock error");
        exit(-1);
    }

    //connect server
    if(connect(sock, (struct sockaddr*) &server, sizeof(server))< 0){
        perror("connect error");
        exit(-1);
    }

    //creat pip
    if(pipe(pip_fd)<0){
        perror("pip error");
        exit(-1);
    }

    //creat epoll
    epfd = epoll_create(EPOLL_SIZE);

    if(epfd < 0){
        perror("epfd error");
        exit(-1);
    }

    //add sock and pip read to kernel
    addfd(epfd,sock,true);
    addfd(epfd, pip_fd[0],true);
}

void Client::Close(){
    
    if(pid){
        close(pip_fd[0]);//关闭父进程
        close(sock);
    }
    else{
        //关闭子进程
        close(pip_fd[1]);//
    }
}


// 启动客户端
void Client::Start(){
// Step 1：连接服务器
    // Connect()

    // Step 2：创建子进程
    // fork()

    // Step 3：进入子进程执行流程
    // 子进程负责收集用户输入的消息并写入管道
    // fgets() write(pipe_fd[1])

    // Step 4：进入父进程执行流程
    // 父进程负责读管道数据及epoll事件
    // 4.1 获取就绪事件
    // epoll_wait()
    // 4.2 处理就绪事件
    // 接收服务器端消息并显示 recv()
    // 读取管道消息并发给服务端 read() send()

    //epoll 事件队列
    static struct epoll_event events[2];

    Connect();

    pid = fork();

    //如果创建成功一个子进程,对于父进程来说是返回子进程的ID.
    //而对于子进程来说就是返回0.而返回-1代表创建子进程失败.
    
    if(pid < 0)
    {
        perror("fork error");
        close(sock);
        exit(-1);
    }
    else if(pid == 0){
        //进入子进程
        string nickname;
        close(pip_fd[0]);//关闭父进程 读端

        Show_help();

        cout<<"Input 'EXIT ' to exit this chat room"<<endl;
        while(isClientWork){
                bzero(&buff, BUF_SIZE);
                fgets(buff, BUF_SIZE,stdin);

                //client 的第一个输入为起的别名
                if(!IsNickNameSet){
                    IsNickNameSet = true;
                    name = static_cast<string>(buff);
                }

                //如果输入EXIT
                if(strncasecmp(buff, EXIT, strlen(EXIT))==0){
                    isClientWork = 0;
                }
                else if(strncasecmp(buff,"clear",strlen("clear")==0)){
                    system("clear");
                    continue;
                }
                
                if(write(pip_fd[1], buff,strlen(buff)-1  )<0){
                        perror("fork error");
                        exit(-1);
                    }
                
        }
    }
    else{
        //进入父进程 pid > 0
        //关闭写端
        close(pip_fd[1]);
        while(isClientWork){
            int epoll_events_count = epoll_wait(epfd, events, 2,-1);

            //处理就绪事件
            for(int i=0;i<epoll_events_count;i++){
                bzero(&buff, BUF_SIZE);

                //服务端来的消息
                if(events[i].data.fd == sock)
                {   //接受服务器
                    int ret = recv(sock, buff, BUF_SIZE, 0);

                    if(ret == 0){
                        cout<< "Server closed connection :"<<sock<<endl;
                        close(sock);
                        isClientWork = 0;
                    }
                    else{
                        cout<<buff<<endl;
                    }

                }
                //子进程写入事件发生， 父进程处理并发送到服务端
                else{
                    //父进程从管道读取数据
                    int ret  =read(events[i].data.fd, buff, BUF_SIZE);

                    //ret = 0
                    if(ret ==0)
                        isClientWork = 0;
                        else{
                            //将信息发送服务端
                            send(sock,  buff, BUF_SIZE, 0);
                 
                        }
                }
            }
        }

    }
    Close();
}