#ifndef LINE_COUNTER_HPP
#define LINE_COUNTER_HPP

#include <iostream>
#include <fstream>
#include <exception>
#include <vector>
#include <string>
#include <filesystem>
#include <future>
#include <thread>

#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>

namespace fs = std::filesystem;
namespace asio = boost::asio;
struct File
{
    fs::path m_pathToFile;
    std::size_t m_linesCount;

    File(fs::path t_pathToFile) : m_pathToFile(t_pathToFile), m_linesCount(0) {}
};

void getFilesName(fs::path const &pathToDir);
std::size_t sumLinesNumber(std::vector<File> &filesInfo);
std::size_t countLines(fs::path const &pathToDir);
std::future<std::size_t> asyncCountLines(fs::path const &pathToDir);

#endif // LINE_COUNTER_HPP