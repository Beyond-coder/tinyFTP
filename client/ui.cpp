#include    "ui.h"

// command init
map<const string, const uint16_t> UI::cmdMap = {    {"USER",    USER},
                                                    {"PASS",    PASS},
                                                    {"USERADD", USERADD},
                                                    {"USERDEL", USERDEL},

                                                    {"GET",     GET},
                                                    {"PUT",     PUT},
                                                    {"LS",      LS},
                                                    {"LLS",     LLS},
                                                    {"CD",      CD},
                                                    {"LCD",     LCD},
                                                    {"RM",      RM},
                                                    {"LRM",     LRM},
                                                    {"PWD",     PWD},
                                                    {"LPWD",    LPWD},
                                                    {"MKDIR",   MKDIR},
                                                    {"LMKDIR",  LMKDIR},
                                                    {"QUIT",    QUIT},
                                                    {"HELP",    HELP},

                                                    {"MGET",    MGET},
                                                    {"MPUT",    MPUT},
                                                    {"RGET",    RGET},
                                                    {"RPUT",    RPUT},
                                                    {"RMDIR",   RMDIR},

                                                    {"SHELL",   SHELL},
                                                    {"LSHELL",  LSHELL},

                                                    {"BINARY",  BINARY},
                                                    {"ASCII",   ASCII}
                                                                        };

UI::UI(const char *host): cliPI(host)
{  

}

void UI::run()
{ 
	string word;
    // 用来存储获得的用户输入命令
	string inputline;

    // user validate commands
    //while (printf("\033[35mUsername for 'tinyFTP': \033[0m"), getline(std::cin, inputline))
    // 多个操作顺序执行，并返回最后一个操作的结果, 返回inputline
    // 进行用户验证登录
    while (printf("Username for 'tinyFTP': "), getline(std::cin, inputline))
    {
        // clear cmdVector each time when user input
        // clear: remove all elements, but keep capacity
        this->cmdVector.clear();
        
        // 将一行数据用作字符串流，随后处理每个单词
        std::istringstream is(inputline);
        while(is >> word)
            this->cmdVector.push_back(word);
        
        // if user enter nothing, assume special anonymous user
        // 如果用户什么都不输入,假定为匿名用户
        if (this->cmdVector.empty())
        {
            // 账户密码都是anonymous
            this->cmdVector.push_back("anonymous");
            this->cmdVector.push_back("anonymous"); 
            if (!cliPI.cmdPASS(this->cmdVector))
            {
                continue;
            } else {
                break;
            }
        }

        if (!cliPI.cmdUSER(this->cmdVector))
        {
            continue;
        } else {
            char *password = getpass("\033[35mPassword for 'tinyFTP': \033[0m");
            // printf("\033[35mPassword for 'tinyFTP': \033[0m");
            // getline(std::cin, inputline);
            // std::istringstream isPass(inputline);
            // while(isPass >> word)
            // {
            //      this->cmdVector.push_back(word);
            //      //std::cout << word << endl;
            // }
            this->cmdVector.push_back(password);  
            if (!cliPI.cmdPASS(this->cmdVector))
            {
                continue;
            } else {
                break;
            }
        }
    }                
    // 登录成功，获得用户名
    this->username = this->cmdVector[0];


    int         maxfdp1;
    fd_set      rset;
    int connfd = cliPI.getConnfd();

    FD_ZERO(&rset);

    printf("%s@tinyFTP> ", username.c_str());
    while(1) 
    {   
        // 清空输出缓冲区
        fflush(stdout);
        // 用IO多路复用
        // 将两个文件描述符加入文件描述符集合中
        FD_SET(connfd, &rset);
        FD_SET(fileno(stdin), &rset);
        maxfdp1 = connfd + 1;
        if (select(maxfdp1, &rset, NULL, NULL, NULL) < 0)
            Error::sys("select error");

        // 使用select,是包的话, 读包, 是命令的话去执行命令
        // 判断文件描述符是否可读，用来读取网络包
        // 判断标准输入是否可读，用来判断是否有新的用户命令
        if (FD_ISSET(connfd, &rset)) {  /* socket is readable */
            cliPI.recvOnePacket();
        }

        // 输入缓冲区是否可读，用来获得用户的指令
        if (FD_ISSET(fileno(stdin), &rset)) /* input is readable */
        {  
            getline(std::cin, inputline);
            cmdRun(inputline);
            printf("%s@tinyFTP> ", username.c_str());
        }
    }

	// other ftp commands: first cout prompt (use "," operator)
	// while (printf("%s@tinyFTP> ", username.c_str()), getline(std::cin, inputline))
	// {
	// 	cmdRun(inputline);
	// }                                                         
	
}
void UI::cmdRun(string & inputline)
{
    // clear cmdVector each time when user input
    this->cmdVector.clear();
    //std::cout << "inputline: " << inputline << inputline.size() << std::endl;

    // 将参数加入到vector中
    // split input string
    for (auto it = inputline.begin(); it < inputline.end(); ++it)
    {
        string param;
        for(; it < inputline.end(); ++it)
        {
            if ((*it) == ' ' || (*it) == '\t')
            {
                break;
            // 对空格进行特殊处理，如果用户输入是'\ ', 就解析为空格
            } else if ((*it) == '\\' && (it + 1) != inputline.end() && *(it + 1) == ' ')
            {
                param += ' ';
                ++it;
            } else {
                param += *it;
            }
        } 
        if (!param.empty())
        {
            this->cmdVector.push_back(param);
        }
    }

    // for (auto it = cmdVector.cbegin(); it != cmdVector.cend(); ++it)
    //    std::cout << it->size() << "cmdVector: " << *it << std::endl;

    // std::istringstream is(inputline);
    // while(is >> word)
    //  this->cmdVector.push_back(word);

    if (!cmdCheck())
    {
        return;
    } else {
        // remove command word, others are params
        // 删掉命令名字,只留下参数
        cmdVector.erase(cmdVector.begin());
        // vector中只剩下参数
        cliPI.run(this->cmdid, this->cmdVector);
    }

    // for (auto it = cmdVector.cbegin(); it != cmdVector.cend(); ++it)
    //     std::cout << "cmdVector" << *it << std::endl;
    // for (std::vector<string>::size_type i = 0; i < cmdVector.size(); i++)
    //     std::cout << cmdVecto
}
bool UI::cmdCheck()
{
    if (cmdVector.empty())
    {
        return false;
    }
    // 找到命令
	map<const string, const uint16_t >::iterator iter = cmdMap.find(toUpper(cmdVector[0]));
	if (iter != cmdMap.end())
	{
        // 获得cmdid即可
		this->cmdid = iter->second;
        return true;
	} else {
        // 命令错误
        std::cerr << cmdVector[0] << ": command not found"  << std::endl;
        return false;
	}
}

string UI::toUpper(string &s)
{
	string upperStr;
	for(string::size_type i=0; i < s.size(); i++)
		upperStr += toupper(s[i]);
	return upperStr;
}