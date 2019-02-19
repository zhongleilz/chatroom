#include"Common.h"
#include"unp.h"
#include"Server.h"
#include<list>
#include<ctime>
#include<cstdlib>
#include<cstdio>
using namespace std;

Server :: Server(){

    //初始化server端口
    inet_pton(AF_INET, SERVER_IP, &server.sin_addr);
    server.sin_family  = AF_INET;
    server.sin_port = htons(SERVER_PORT);//8888

    //socket init
    listenfd = 0;

    //epoll fd
    epfd = 0;
}

void Server::Init() 
{
    cout<<"Init server"<<endl;

    //creat  socket
    listenfd =  socket(AF_INET, SOCK_STREAM,0);
    if(listenfd < 0){
        perror("listener");
        exit(-1);
    }

    //bind socket
    if(bind(listenfd,( struct sockaddr*) &server, sizeof(server))< 0){
        perror("bind error");
        exit(-1);
    }

    //
    if(listen(listenfd, LISTENQ) < 0){
        perror("listen error");
        exit(-1);
    }

    //创建事件表
    epfd = epoll_create(EPOLL_SIZE);

    if(epfd < 0){
        perror("epoll error");
        exit(-1);
    }

    //往事件表中添加监听事件
    addfd(epfd, listenfd, true);
}


void Server::Close(){//OK

    //close socket
    close(listenfd);

    //close epoll
    close(epfd);
}

int Server::ShowUserInfoToClient(int connfd)//OK
{
    char message[BUF_SIZE];//保存格式化的信息
    int len = clients_map.size();
    sprintf(message,"\033[31mHere are %d users online now!",len);
    sprintf(message,"%s\nHost        Port        Join_time                User_name",message);
    map<int,client_info>::iterator iter;
    for(iter = clients_map.begin();iter != clients_map.end();iter++)
    {
        //iter->second.client_name;
        sprintf(message,"%s\n%s     %s        %s        %s",message, iter->second.client_host.c_str(), 
        iter->second.client_port.c_str(), iter->second.join_time, iter->second.client_name.c_str());
       
    } 
    sprintf(message,"%s\033[0m\n",message);

    if(send(connfd,message,strlen(message),0)< 0)
            return -1;

    return len;
}
 
int Server::SendBroadcasttoMessage(int connfd){//OK
    //接受缓冲区
    char buff[BUF_SIZE];

    //保存格式化的消息
    char message[BUF_SIZE];

    bzero(buff,BUF_SIZE);
    bzero(message,BUF_SIZE);

    //receive message
    cout<<"read message from clientID#"<<connfd<<")"<<endl;

    int len = recv(connfd, buff, BUF_SIZE,0);//接收信息

    //get the information of client
    client_info &current_client = clients_map[connfd];
    
    //new client join the chatroom,send information to other clients
    if(!current_client.is_nickname_set)
    {
        current_client.is_nickname_set = true;
        current_client.client_name = static_cast<string> (buff);
        sprintf(message, SERVER_WELCOME,current_client.client_name.c_str());

        for(auto it = clients_map.cbegin(); it!= clients_map.cend();it++)
        {
            if(it->first!= connfd){
                if(send(it->first,message,BUF_SIZE,0)<0)
                    return -1;
            }
        }
        return len;
    }

    //如果 client close
    if(len == 0){
        close(connfd);
        //clients_list.remove(connfd);
        clients_map.erase(connfd);
        cout<<"clientID#"<<connfd<<"closed.\n"<<"now there are "<<clients_map.size()
        <<" in this chat room"<<endl;

        //board leave information to other client
        sprintf(message,"%s  leave this chatroom",current_client.client_name.c_str());
        for(auto it = clients_map.cbegin();it!= clients_map.cend();it++)
        {
            if(it->first!=connfd)
            {
                if(send(it->first,message,BUF_SIZE,0)<0)
                    return -1;
            }
        }
        return len;
    }//发送广播给所以客户端
    else{
        if(clients_map.size()==1){
                //发送提示只有一人消息
                send(connfd,CAUTION,strlen(CAUTION),0);
                return len;
        }
        //inshow all client information
        if(strncasecmp(buff, "$ please show users",strlen("$ please show users"))== 0)
            return ShowUserInfoToClient(connfd);
        else if(buff[0] == '>')
        {
            string command = static_cast<string>(buff);
            
            size_t start=  command.find_first_of(' ');
            size_t last = command.find_last_of(' ');

            //get the name and message
            string name = command.substr(start+1,last-1);
            string msg = command.substr(last+1);

            char name_c[name.size()],msg_c[msg.size()];
            sub_space(name.c_str(),name_c);//去除多余的空格
            sub_space(msg.c_str(),msg_c);

            //私聊 发给特定的人
            for(auto it = clients_map.begin();it!=clients_map.end();it++){
                if(it->second.client_name == (static_cast<string>(name_c)) && (it->first!= connfd))
                {
                    if(send(it->first,msg_c,sizeof(msg_c),0)<0)
                        return -1;
                }
            }
            return len;

        }
        //将要发送的内容格式化保存到message
        sprintf(message, SERVER_MESSAGE, current_client.client_name.c_str(), buff); //有error
        cout<<"Client  "<<current_client.client_name<<"say>>"<<buff<<endl;

        map<int, client_info>::iterator iter;
        //将消息发给除了来源客户端以外的客户端
        for(iter = clients_map.begin(); iter!=clients_map.end();iter++){
            if(iter->first !=  connfd){
                //int s = send(*iter, message, BUF_SIZE,0);
                if(send(iter->first, message, BUF_SIZE,0)< 0)
                    return -1;
            }
        }
    }
    return len;
}


void Server::Start(){
    // Step 1：初始化服务端
    // Init()

    // Step 2：进入主循环

    // Step 3：获取就绪的事件
    // epoll_wait()

    // Step 4：循环处理所有就绪的事件
    // 4.1 如果是新连接则接受连接并将连接添加到epoll fd
    // accept() addfd()

    // 4.2 如果是新消息则广播给其他客户端
    // SendBroadcastMessage

    /*
    struct epoll_event {
    __uint32_t events;  Epoll events 
    epoll_data_t data;  User data variable 
    };
    */ 
        
        Init();
        //epoll  事件队列
        static struct epoll_event events[EPOLL_SIZE];

        while(1){
            int epoll_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
            
            if(epoll_count < 0){
                perror("epoll failure");
                break;
            }

            cout<<"the number of  epoll_count is "<<epoll_count<<endl;

            //处理epoll_event 中就绪的事件
            for(int i=0; i< epoll_count;i++){
                //新用户连接
                if(events[i].data.fd ==  listenfd){

                        struct sockaddr_in client;
                        socklen_t client_len = sizeof(sockaddr_in);//socklen_t unsigned int
                    
                        int clientfd = accept(listenfd, (sockaddr*) &client, &client_len);//get the ready client fd

                        cout<<"client connection from: "<<inet_ntoa(client.sin_addr)<<":"
                        <<ntohs(client.sin_port)<<",clientfd =  "<<clientfd<<endl;

                        //add the client fd to kernel event table
                        addfd(epfd, clientfd, true);

                        //服务端用list保存用户连接
                        //clients_list.push_back(clientfd);

                        //collect the information of client and add to clients_map
                        client_info add_client;
                        add_client.client_host = static_cast<string>( inet_ntoa(client.sin_addr));
                        //add_client.client_port = static_cast<string>(ntohs(client.sin_port));
                        uint16_t port_number = ntohs(client.sin_port);
                        char c[BUF_SIZE];
                        int length  = sprintf(c,"%X",port_number);
                        add_client.client_port = static_cast<string>(c);
                        add_client.connfd = clientfd;
                        time_t now = time(0);
                        add_client.join_time = ctime(&now);
                        add_client.join_time[strlen(add_client.join_time - 1)] = '\0';
                        add_client.is_nickname_set = false;
                        clients_map.insert(pair<int,client_info>(clientfd,add_client));
                    
                        cout<<"add new clientfd= "<<clientfd<<" to epoll"<<endl;
                        cout<<"Now there are  "<<clients_map.size()<<"in the chatroom"<<endl;
                        /*//服务端发送欢迎信息
                        char buff[BUF_SIZE];
                        bzero(buff,BUF_SIZE);
                        sprintf(buff,  SERVER_WELCOME, clientfd);
                        
                        int ret = send(clientfd, buff,BUF_SIZE,0);
                        if(ret < 0){
                            perror("send error");
                            Close();
                            exit(-1);
                        }
                        */
                }
                else
                //處理其他用戶法來的消息，並廣播給其他用戶
                {
                    int ret = SendBroadcasttoMessage(events[i].data.fd);
                    if(ret < 0){
                        perror("error");
                        Close();
                        exit(-1);
                    }
                }
            }

        }
        //關閉服務
    Close ();
}