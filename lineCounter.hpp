#ifndef LINE_COUNTER_HPP
#define LINE_COUNTER_HPP

#include <iostream>
#include <fstream>
#include <exception>
#include <vector>
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
    LineCounter(std::string t_pathToDir);

    void getFilesName();
    void asyncCountLinesNumber();
    void sumLinesNumber();

private:
    void countLinesNumber();

    struct File
    {
        std::string m_fileName;
        std::size_t m_linesCount;

        File(std::string t_fileName) : m_fileName(t_fileName), m_linesCount(0) {}
    };

    std::string m_pathToDir;
    std::size_t m_totalLines;

    std::vector<File> m_filesInfo;
    std::vector<File>::iterator m_vectorIter;

    boost::mutex m_mutexLineCounter;
    boost::atomic<bool> m_stopPool;
};

#endif // LINE_COUNTER_HPP