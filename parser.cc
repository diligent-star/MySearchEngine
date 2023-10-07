#include <iostream>
#include <vector>
#include <string>
#include <boost/filesystem.hpp>
#include <utility>
#include "Utility.hpp"
#include "VitalPar.hpp"


typedef struct DocInfo
{
    std::string _title;
    std::string _content;
    std::string _url;
}DocInfo_t;

bool GetHtmlsURL(const std::string& dataURL, std::vector<std::string>* pvURL);
bool ParseHtmls(const std::vector<std::string>& vURl, std::vector<DocInfo_t>* vItems);
bool SaveItems(const std::vector<DocInfo_t>& vItems, const std::string& destURL);

int main()
{
    //管理 数据来源的URL
    std::vector<std::string> vURL;

    //根据路径 递归获取 所有html文件 URL
    if(!GetHtmlsURL(myPar::DATA_URL, &vURL))
    {
        std::cerr << "GET URL ERROR!" << std::endl;
        return 1;
    }

    /*
    //Debug 打印html文件URL
    std::vector<std::string>::iterator begin = vURL.begin();
    while(begin != vURL.end())
    {
        std::cout << *begin << std::endl;
        ++begin;
    }
    */

    //对所有URL中内容进行清洗
    std::vector<DocInfo_t> vItems;
    
    if(!ParseHtmls(vURL, &vItems))
    {
        std::cerr << "PARSE HTML ERROR!" << std::endl;
        return 2;
    }
    
    /*
    //Debug 打印DocInfo_t 内容
    std::vector<DocInfo_t>::iterator begin = vItems.begin();
    while(begin != vItems.end())
    {
        std::cout << begin->_title << std::endl << begin->_content << std::endl << begin->_url << std::endl;
        ++begin;
    }
    */

    //保存数据
    if(!SaveItems(vItems, myPar::OUTPUT_PARSE_URL))
    {
        std::cerr << "SAVE ITEMS ERROR!" << std::endl;
        return 3;
    }

    return 0;
}


bool GetHtmlsURL(const std::string& dataURL, std::vector<std::string>* pvURL)
{
    boost::filesystem::path rootURL(dataURL);
    if(!boost::filesystem::exists(rootURL))
    {
        std::cerr << "rootURL NOT EXIST!" << std::endl;
    }

    //所找文件的起始目录存在
    boost::filesystem::recursive_directory_iterator end;
    for(boost::filesystem::recursive_directory_iterator it(rootURL); it != end; it++)
    {
        if(!boost::filesystem::is_regular_file(*it))
        {
             //不是普通文件查找下一个
             continue;
        }

        if(it->path().extension() != ".html")
        {
            //不是html文档查找下一个
            continue;
        }

        //一定是html文档URL
        pvURL->push_back(it->path().c_str());
    }

    return true;
}

static bool ParseTitle(const std::string& htmlAll, std::string* outPut)
{
    size_t titleBegin = htmlAll.find("<title>");
    if(titleBegin == std::string::npos)
    {
        //没找到，返回false
        return false;
    }    
    titleBegin += std::string("<title>").size();
    size_t titleEnd = htmlAll.find("</title>");
    if(titleEnd == std::string::npos)
    {
        //没找到，返回false
        return false;
    }

    if(titleBegin > titleEnd)
    {
        //不合法的开始与结束
        return false;
    }

    *outPut = htmlAll.substr(titleBegin, titleEnd - titleBegin);
    return true;
}

static bool ParseContent(const std::string& htmlAll, std::string* outPut)
{
    enum Status
    {
        LABLE,
        CONTENT
    };

    enum Status status = LABLE;
    std::string::const_iterator end = htmlAll.cend();
    for(std::string::const_iterator begin = htmlAll.cbegin(); begin != end; ++begin)
    {
        switch(status)
        {
            case LABLE:
                    if(*begin == '>')
                    {
                        status = CONTENT;
                    }
                break;
            case CONTENT:
                    if(*begin == '<')
                    {
                        status = LABLE;
                    }
                    else
                    {
                        if(*begin == '\n')
                        {
                            outPut->push_back(' ');
                        }
                        else
                        {
                            outPut->push_back(*begin);
                        }
                    }
                break;
            defualt:break;
        }
    }
    return true;
}

static bool ParseURL(const std::string& url, std::string* outPut)
{
    std::string urlHead = "https://www.boost.org/doc/libs/1_81_0/doc/html";
    std::string urlTail = url.substr(myPar::DATA_URL.size());
    *outPut += urlHead;
    *outPut += urlTail;
    return true;
}



bool ParseHtmls(const std::vector<std::string>& vURL, std::vector<DocInfo_t>* vItems)
{
    std::vector<std::string>::const_iterator end = vURL.cend();
    for(std::vector<std::string>::const_iterator it = vURL.cbegin(); it != end; ++it)
    {   
        std::string htmlAll;
        //打开URL 获取URL中html文档所有内容
        if(!myUtility::doFile::ReadFile(*it, &htmlAll))
        {
            //读取失败，读取下一个
            continue;      
        }

        DocInfo_t Doc;
        //title提取
        if(!ParseTitle(htmlAll, &Doc._title))
        {
            //信息不完全，读取下一个
            continue;
        }

        //content提取
        if(!ParseContent(htmlAll, &Doc._content))
        {
            continue;
        }

        //URL提取
        if(!ParseURL(*it, &Doc._url))
        {
            continue;   
        }
        vItems->push_back(std::move(Doc));   
    }
    return true;
}

bool SaveItems(const std::vector<DocInfo_t>& vItems, const std::string& destURL)
{
    std::ofstream parse_html(destURL, std::ofstream::trunc | std::ofstream::out | std::ofstream::binary);
    if(!parse_html.is_open())
    {
        std::cerr << "SAVE ERROR! " << destURL << std::endl;
        return false;
    }

//const char SEP = '\3';
    //文档结构 以'/3'间隔 文档间用'\n'间隔
    std::vector<DocInfo_t>::const_iterator end = vItems.end();
    for(std::vector<DocInfo_t>::const_iterator it = vItems.begin(); it != end; ++it)
    {
        std::string tmp;
        tmp += it->_title + myPar::SEP;    //title
        tmp += it->_content + myPar::SEP;  //content
        tmp += it->_url + '\n';     //url
        parse_html.write(tmp.c_str(), tmp.size());
    }

    parse_html.close();
    return true;
}
