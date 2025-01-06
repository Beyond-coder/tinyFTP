#include    "server.h"
#include    "../common/database.h"

void * clientConnect(void * arg)
{
    // 动态申请的因此需要delete
    ThreadArg * pthreadArg = (ThreadArg *)arg;
    SrvPI srvPI(DBFILENAME, pthreadArg->fd);

    // 可以用一个eventloop来实现
    while (1)
    {
        srvPI.run();
    }

    delete pthreadArg;
    return(NULL);
}


int main(int argc, char **argv)
{
    {
        // 打开数据库文件
        Database db(DBFILENAME);

        // 初始化数据库，每个用户有自己的目录
        db.init();
    }
   
    
    // 最大切片数
    printf("MAXNSLICE: %lu\n", MAXNSLICE);
    // socket
    struct sockaddr_in  cliaddr;
    socklen_t len = sizeof(cliaddr);
    char buff[MAXLINE];
    int listenfd, srvConnfd;

    // 服务器监听套接字
    // 域名为NULL
    // 端口号为2121
    Socket listenSocket(SRV_SOCKET, NULL, CTRPORT);
    listenfd = listenSocket.init();

    std::cout << "Listen socket port: " << CTRPORT << std::endl;

    // 创建一个线程
    pthread_t tid;
    

    while (1)
    {  
        // 通过循环去建立连接,建立好连接之后再创建一个线程
        srvConnfd = listenSocket.tcpAccept(listenfd, (SA *) &cliaddr, &len);
        printf("connection from %s, port %d\n",
                inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, buff, sizeof(buff)), ntohs(cliaddr.sin_port));
        // 在线程中去处理连接
        ThreadArg * pthreadArg = new ThreadArg;
        pthreadArg->fd = srvConnfd;

        // 执行的函数是clientConnect，传递的参数是pthreadArg
        Pthread_create(&tid, NULL, &clientConnect, pthreadArg);
    }
    return 0;   
}


