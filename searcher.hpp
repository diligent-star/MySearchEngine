#pragma once
#include <algorithm>
#include <jsoncpp/json/json.h>
#include <tr1/unordered_map>

#include "Utility.hpp"
#include "Index.hpp"

namespace mySearcher
{
    class Searcher
    {
    public:
        Searcher(){}
        ~Searcher(){}

    public:
        void InitSearcher(const std::string& input)
        {
            //创建索引
            _pIndex = myIndex::Index::GetInstace();
            std::cout << "索引开始建立！" << std::endl;

            //建立索引
            _pIndex->BuildIndex(input);

            std::cout << "索引建立成功！" << std::endl;
        }

        void Search(const std::string& inputForSearch, std::string* jsonString)
        {
            //将搜索关键字进行分词
            std::vector<std::string> words;
            myUtility::myJieba::CutString(inputForSearch, &words);

            //根据分词内容 进行 倒排索引查找
            myIndex::Index::InvertedChain items;
            std::unordered_map<uint64_t, myIndex::InvertedNode_t> deduplication;
            std::vector<std::string>::iterator endW = words.end();
            for(std::vector<std::string>::iterator it = words.begin(); it != endW; ++it)
            {
                myUtility::doString::ToLower(*it);
                myIndex::Index::InvertedChain* pCur = _pIndex->GetInvertedIndex(*it);
                if(pCur == nullptr)
                {
                    continue;
                }

                //将所有 分词的倒排拉链去重合并
                for(const auto& elem : *pCur)
                {
                    auto iteratorD = deduplication.find(elem._docID);
                    if(iteratorD == deduplication.end())
                    {
                        //没找到，插入
                        deduplication.insert(std::make_pair(elem._docID, elem));
                    }
                    else
                    {
                        //找到了，取权重大的保留
                        if(iteratorD->second._weight > elem._weight)
                        {
                            ;
                        }
                        else
                        {
                            deduplication.erase(iteratorD);
                            deduplication.insert(std::make_pair(elem._docID, elem));
                        }
                    }
                }
                //items.insert(items.end(), pCur->begin(), pCur->end());
            }
            //将去重后的结果给items
            for(const auto& elem : deduplication)
            {
                items.push_back(elem.second);
            }
            deduplication.clear();

            //对倒排拉链进行排序
            std::sort(items.begin(), items.end(), 
                        [](const myIndex::InvertedNode& lhs, const myIndex::InvertedNode& rhs)->bool{
                            return lhs._weight > rhs._weight;
                        });            
            
            //搜索结果 构建json串
            Json::Value root; //json串 总输出
            myIndex::Index::InvertedChain::iterator endI = items.end();
            for(myIndex::Index::InvertedChain::iterator it = items.begin(); it != endI; ++it)
            {
                myIndex::DocInfo_t* pDoc = _pIndex->GetForwordIndex(it->_docID);
                if(pDoc == nullptr)
                {
                    continue;
                }
                Json::Value subRoot;
                subRoot["title"] = pDoc->_title;
                subRoot["abstract"] = GetAbstract(pDoc->_content, it->_word);    //该内容暂时是去标签的结果，并不是最终要的 content
                subRoot["url"] = pDoc->_url;

                root.append(subRoot);
            }

            Json::StyledWriter writer;
            *jsonString = writer.write(root);           
        }

    private:
        std::string GetAbstract(const std::string& htmlContent, const std::string& word)
        {
            const std::size_t FORWORD = 50;
            const std::size_t BACK = 50;
            
            //找到 word 在 htmlContent 中的首次出现
            //std::size_t pos = htmlContent.find(word);
            //if(pos == std::string::npos)
            //{
            //    return "NONE ERROR!";   //不可能出现该情况！
            //}
            //不适用 string.find的原因：此find查询是区分大小写的(大小写敏感的)，我们的搜索是大小写不敏感的
            //而原 htmlContent中的内容是既有大写也有小写的

            std::string::const_iterator iterator = std::search(htmlContent.cbegin(), htmlContent.cend(), word.cbegin(), word.cend(), [](int x, int y)->bool{
                                return std::tolower(x) == std::tolower(y);
                            });
            if(iterator == htmlContent.cend())
            {
                return "NONE ERROR!"; 
            }

            std::size_t pos = std::distance(htmlContent.cbegin(), iterator);

            std::size_t start = 0;
            std::size_t end = htmlContent.size()-1;
            if(start + FORWORD < pos) 
            {
                start = pos - FORWORD;
            }
            else
            {
                ;//start 就是开头
            }

            if(end >= BACK && (end - BACK) > (pos+word.size()))
            {
                end = pos + BACK;
            }
            else
            {
                ;//end 就是结尾
            }
            if(start < end)
            {
                return "..." + htmlContent.substr(start, end-start) + "...";
            }
            else
            {
                return "NONE!";     //几乎不可能发生
            }  
        }

    private:
        myIndex::Index* _pIndex; 
    };
}