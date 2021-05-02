#ifndef LINE_COUNTER_HPP
#define LINE_COUNTER_HPP

#include <iostream>
#include <fstream>
#include <exception>
#include <list>
#include <string>
#include <filesystem>

#include <boost/thread.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>
#include <boost/atomic/atomic.hpp>

namespace fs = std::filesystem;
namespace asio = boost::asio;

class LineCounter
{
public:
    LineCounter()
        : totalLines(0), stopPool(false)
    {
        this->GetFilesName();
        this->AsyncCountLinesNumber();
        this->SumLinesNumber();
    }

    void GetFilesName();
    void CountLinesNumber();
    void AsyncCountLinesNumber();
    void SumLinesNumber();

private:
    struct File
    {
        std::string fileName;
        std::size_t linesCount;

        File(std::string fileName) : fileName(fileName), linesCount(0) {}
    };

    std::size_t totalLines;

    std::list<File> filesInfo;
    std::list<File>::iterator listIter;

    boost::mutex mutexLineCounter;
    boost::atomic<bool> stopPool;
};

#endif // LINE_COUNTER_HPP