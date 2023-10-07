#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <tr1/unordered_map>
#include <mutex>
#include <utility>

#include "Utility.hpp"
#include "VitalPar.hpp"

namespace myIndex
{
    //html文档结构体
    typedef struct DocInfo
    {
        std::string _title;
        std::string _content;
        std::string _url;
        uint64_t _docID;
    } DocInfo_t;

    //倒排节点
    typedef struct InvertedNode
    {
        uint64_t _docID;
        std::string _word;
        int _weight; //权重
    } InvertedNode_t;

    class Index
    {
    private:
        static Index* _pInstance;
        static std::mutex _mtx;
        Index(){}
        Index(const Index&) = delete;
        Index& operator=(const Index&) = delete;
    public:
        static Index* GetInstace()
        {   
            if(_pInstance == nullptr)
            {
                _mtx.lock();
                if(_pInstance == nullptr)
                {
                    _pInstance = new Index();
                }
                _mtx.unlock();
            }
            return _pInstance;
        }

    public:
        typedef std::vector<DocInfo_t> ForwardIndex;                          //正排索引
        typedef std::vector<InvertedNode_t> InvertedChain;                    //倒排拉链
        typedef std::unordered_map<std::string, InvertedChain> InvertedIndex; //倒排索引
    private:
        ForwardIndex _forwardIndex;
        InvertedIndex _invertedIndex;

    public:
        ~Index() {}

        //根据DOCID 在正排索引中找到文档
        DocInfo *GetForwordIndex(const uint64_t &docID)
        {
            if (docID >= _forwardIndex.size())
            {
                std::cerr << docID << " NOT EXISTs!" << std::endl;
                return nullptr;
            }
            return &_forwardIndex[docID];
        }

        //根据关键字 在倒排索引中找到倒排拉链
        InvertedChain *GetInvertedIndex(const std::string &keyWord)
        {
            InvertedIndex::iterator it = _invertedIndex.find(keyWord);
            if (it == _invertedIndex.end())
            {
                std::cerr << keyWord << " NOT Find!" << std::endl;
                return nullptr;
            }
            return &(it->second);
        }

        //根据parse进程清洗的文档内容，构建正排与倒排索引
        bool BuildIndex(const std::string &srcHtmlUrl)
        {
            std::ifstream srcHtmls(srcHtmlUrl, std::ifstream::in | std::ifstream::binary);
            if (!srcHtmls.is_open())
            {
                std::cerr << srcHtmlUrl << " Open ERROR!" << std::endl;
                return false;
            }
            std::string doc;
            int count = 0;
            while(std::getline(srcHtmls, doc))
            {
                DocInfo* newForwardIndexDoc = BuildForwardIndex(doc);
                if(newForwardIndexDoc == nullptr)
                {
                    std::cerr << "Build " << doc << " ERROR!" << std::endl; //for debug
                    continue;
                }

                if(!BuildInvertedIndex(*newForwardIndexDoc))
                {
                    std::cerr << "Build InvertedIndex DOCID-> " << newForwardIndexDoc->_docID << " ERROR!" << std::endl; //for debug
                }

                ++count;
                if(count % 100 == 0)
                {
                        std::cout << "建立索引中... " << count << std::endl;
                }
            }
            srcHtmls.close();
            return true;
        }

    private:
        //成功返回插入元素的指针，失败返回nullptr
        DocInfo_t* BuildForwardIndex(const std::string& line)
        {
            //解析line 字符串切分 -> title, content, url
            std::vector<std::string> parts;
            myUtility::doString::Split(line, &parts, myPar::SEP);
            if(parts.size() != 3)
            {
                return nullptr;
            }

            DocInfo_t doc;
            doc._title = parts[0];
            doc._content = parts[1];
            doc._url = parts[2];
            doc._docID = _forwardIndex.size();  //从0开始的docID
            //3.插入正排索引的vector
            _forwardIndex.push_back(std::move(doc));
            return &_forwardIndex.back();
        }

        //根据DocInfo_t构建一个倒排索引
        bool BuildInvertedIndex(const DocInfo_t& doc)
        {
            //建立倒排索引
            //1.词频统计
            //2.倒排拉链建立

            typedef struct worldFrequencyNode
            {
                int _titleNum;
                int _contentNum;

                worldFrequencyNode()
                    :_titleNum(0)
                    ,_contentNum(0)
                {}
            }worldFrequencyNode_t;
            typedef std::unordered_map<std::string, worldFrequencyNode_t> WordFrequencey;
            WordFrequencey wordMap;

            //标题分词
            std::vector<std::string> titleWords;
            myUtility::myJieba::CutString(doc._title, &titleWords);
            
            //标题词频统计
            std::vector<std::string>::iterator endT = titleWords.end();
            for(std::vector<std::string>::iterator it = titleWords.begin(); it!=endT; ++it)
            {
                myUtility::doString::ToLower(*it);
                wordMap[*it]._titleNum++;
            }

            //内容分词
            std::vector<std::string> contentWords;
            myUtility::myJieba::CutString(doc._content, &contentWords);

            //内容词频统计
            std::vector<std::string>::iterator endC = contentWords.end();
            for(std::vector<std::string>::iterator it = contentWords.begin(); it!=endC; ++it)
            {
                myUtility::doString::ToLower(*it);
                wordMap[*it]._contentNum++;
            }

            //构建倒排拉链
            WordFrequencey::iterator endMap = wordMap.end();
            for(WordFrequencey::iterator it = wordMap.begin(); it != endMap; ++it)
            {
                InvertedNode_t node;
                node._docID = doc._docID;
                node._word = it->first;
                node._weight = 10 * it->second._titleNum + 1 * it->second._contentNum;  //相关性，权重计算->扩展阅读
                
                InvertedChain& invertedChain = _invertedIndex[it->first];
                invertedChain.push_back(std::move(node));
            }  

            return true;
        }
    };
    Index* Index::_pInstance = nullptr;
    std::mutex Index::_mtx;
}