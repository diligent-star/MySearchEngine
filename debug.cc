#include <iostream>
#include <string>
#include <sys/types.h>
#include "searcher.hpp"
#include "VitalPar.hpp"
#include "MySocket.hpp"
#include "Utility.hpp"
 #include "thread_pool.hpp"

const std::string WWWROOT = "./wwwroot";
const std::string HOME = "/index.html";

//创建 搜索引擎
mySearcher::Searcher searcher;

class SockTask
{
private:
    int _sock;

private:
    void WebHome()
    {
        //Web首页路径
        std::string htmlFile;
        htmlFile += WWWROOT + HOME;
        //响应web根目录首页信息
        std::string http_respond;
        http_respond += "http/1.0 200 OK\r\n";
        //http_respond += "Connection: keep-alive\n";
        http_respond += "Content-Type: text/html; charset=UTF-8\r\n";
        http_respond += "Content-Length: ";
        http_respond += std::to_string(myUtility::doFile::FileSize(htmlFile));
        http_respond += "\r\n";
        http_respond += "\r\n";//http 空行
        myUtility::doFile::ReadFile(htmlFile, &http_respond);
        
        //std::cout << http_respond << std::endl;//打印发送的数据
        
        send(_sock, http_respond.c_str(), http_respond.size(), 0);
        myUtility::MySocket::Close(_sock);
    } 
    void SendJson(std::string& jsonString)
    {
        std::string http_respond;
        http_respond += "http/1.0 200 OK\r\n";
        //http_respond += "Connection: keep-alive\n";
        http_respond += "Content-Type: application/json; charset=UTF-8\r\n";
        http_respond += "Content-Length: ";
        http_respond += std::to_string(jsonString.size());
        http_respond += "\r\n";
        http_respond += "\r\n";//http 空行
        http_respond += jsonString.c_str();
        //std::cout << http_respond << std::endl; //Debug 打印回传内容
        send(_sock, http_respond.c_str(), http_respond.size(), 0);
        myUtility::MySocket::Close(_sock);
    }
public:
    //次socketId可能违法使用
    SockTask()
        :_sock(-1)
    {}

    explicit SockTask(int sock)
        :_sock(sock)
    {}

    SockTask& operator=(const SockTask& rhs)
    {
        this->_sock = rhs._sock;
        return *this; 
    }

    void Run()
    {
        //对非法sock禁止运行
        if(_sock < 0)
        {
            std::cout << "_sock Error!" << std::endl;
            return;
        }

        //对_sock内容进行读取
        char buffer[4*1024];
        memset(buffer, 0, sizeof(buffer));
        ssize_t s = recv(_sock, buffer, sizeof(buffer), 0);
        /*if(s > 0)
        {
            //对读到数据进行打印 Debug
            //std::cout << buffer << std::endl;
        }*/
        //std::cout << "打印结束！" << std::endl; //debug

        //分离所需词汇 1.找到Content-Length
        std::string httpContent(buffer);
        std::string keyStr("Content-Length");
        size_t pos = httpContent.find(keyStr);
        if(pos != std::string::npos)
        {   
            //有正文请求处理正文请求
            size_t endPos = httpContent.find("\r\n", pos);  //从pos出找第一个换行
            std::string number = httpContent.substr(pos+16, endPos-pos-16);//截取数字字符串
            size_t contentLen = atoi(number.c_str());//内容的长度，字符串转为数字
            size_t lineBreakPos = httpContent.find("\r\n\r\n");
            if(contentLen == 0)
            {
                //std::cout << "1" << std::endl; //debug 观察webhome运行流
                WebHome();
            }
            
            //截取内容
            if(lineBreakPos != std::string::npos)
            {
                std::string content = httpContent.substr(lineBreakPos + 4, contentLen);//内容
                //找用户搜索关键词
                size_t begin = content.find("word=");
                std::string word = content.substr(begin+5);//搜索关键词

                //对关键词搜索
                std::string jsonString;
                ::searcher.Search(word, &jsonString);
                //返回json串
                SendJson(jsonString);
            }
        }
        else
        {
            //std::cout << "2" << std::endl; //debug 观察webhome运行流
            //没有正文，响应Web首页
            WebHome();
        }
    }
};

int main()
{
    ::searcher.InitSearcher(myPar::OUTPUT_PARSE_URL);
    //网络通信
    int listenSock = myUtility::MySocket::Socket();

    myUtility::MySocket::Bind(listenSock, 8081);
    std::cout << "绑定成功！" << std::endl; //debug 观察服务启动
    myUtility::MySocket::Listen(listenSock);
    std::cout << "监听成功！" << std::endl; //debug 观察服务启动    
    while(1)
    {
        int sock = myUtility::MySocket::Accept(listenSock);
        std::cout << "接受成功！" << sock << std::endl;  //debug 观察服务启动
        //创建线程处理 网络套接字
        MyUtility::ThreadPool<SockTask>::GetThreadPool()->PushTask(SockTask(sock));

    }

    return 0;
}