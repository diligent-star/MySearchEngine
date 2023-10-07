#pragma once
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>

#include "jiebaParticiple/Jieba.hpp"

namespace myUtility
{
    class doFile
    {
    public:
        static bool ReadFile(const std::string& url, std::string* outPut)
        {
            std::ifstream srcFile(url.c_str(), std::ifstream::in);
            if(!srcFile.is_open())
            {
                std::cerr << "OPEN " << url << " ERROR!" << std::endl;
                return false;
            }

            std::string tmp;
            while(std::getline(srcFile, tmp))
            {
                *outPut += tmp;
            }

            srcFile.close();
            return true;
        }

        static int FileSize(const std::string& fileUrl)
        {
            boost::filesystem::path filePath(fileUrl);
            if(boost::filesystem::exists(filePath))
            {
                return boost::filesystem::file_size(filePath);
            }
            else
            {
                std::cerr << "File Not Exits: " << fileUrl << std::endl;
                return 0;
            }
        }
    };

    class doString
    {
    public:
        //将line中字符 按照sep为分隔符切分 切分内容依次放入output中
        static void Split(const std::string& line, std::vector<std::string>* outPut, const std::string& sep)
        {
            boost::split(*outPut, line, boost::is_any_of(sep.c_str()), boost::token_compress_on);
        }

        static void ToLower(std::string& src)
        {
            boost::to_lower(src);
        }

    };

    //词库
    const char* const DICT_PATH = "./dict/jieba.dict.utf8";
    const char* const HMM_PATH = "./dict/hmm_model.utf8";
    const char* const USER_DICT_PATH = "./dict/user.dict.utf8";
    const char* const IDF_PATH = "./dict/idf.utf8";
    const char* const STOP_WORD_PATH = "./dict/stop_words.utf8";

    class myJieba
    {
    private:
        static cppjieba::Jieba _jieba;
    public: 
        static void CutString(const std::string& src, std::vector<std::string>* out)
        {
            _jieba.CutForSearch(src, *out);
        }
    };
    cppjieba::Jieba myJieba::_jieba(DICT_PATH, HMM_PATH, USER_DICT_PATH, IDF_PATH, STOP_WORD_PATH);
}
