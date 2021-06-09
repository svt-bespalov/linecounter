#include <iostream>

#include <fstream>
#include <exception>
#include <vector>
#include <string>
#include <thread>
#include <utility>
#include <boost/asio/post.hpp>

#include "lineCounter.hpp"

namespace
{
    struct File
    {
        fs::path m_pathToFile;
        std::size_t m_linesCount;

        File(fs::path t_pathToFile) : m_pathToFile(t_pathToFile), m_linesCount(0) {}
    };

    std::vector<File> GetFilesName(const fs::path &pathToDir, std::mutex &mutexLineCounter, errorFuncType &errorFunc)
    {
        std::vector<File> filesInfo;

        try
        {
            for (const auto &entry : fs::directory_iterator(pathToDir))
            {
                if (entry.is_regular_file())
                {
                    filesInfo.emplace_back(File(entry.path()));
                }
            }

        }
        catch(const std::system_error &except)
        {
            std::unique_lock<std::mutex> lock(mutexLineCounter);
            errorFunc(pathToDir, except.code());
        }

        return filesInfo;
    }

    std::size_t SumLinesNumber(std::vector<File> &filesInfo)
    {
        std::size_t totalLines = 0;

        for (File element : filesInfo)
        {
            totalLines += element.m_linesCount;
        }

        return totalLines;
    }

    void CountLines(std::vector<File> &filesInfo, std::vector<File>::iterator &vectorIter, std::mutex &mutexLineCounter, errorFuncType &errorFunc)
    {
        auto isVectorEnd = [&]() -> bool
        {
            std::unique_lock<std::mutex> lock(mutexLineCounter);
            return vectorIter == filesInfo.end();
        };

        auto getFileInfo = [&]() -> File&
        {
            std::unique_lock<std::mutex> lock(mutexLineCounter);
            return *(vectorIter++);
        };

        while (!isVectorEnd())
        {
            std::ifstream fin;
            std::string line;

            auto &fileInfo = getFileInfo();

            try
            {
                fin.open(fileInfo.m_pathToFile.string());
                if (!fin.is_open())
                {
                    throw std::system_error(std::make_error_code(std::errc::no_such_file_or_directory));
                }

                while (std::getline(fin, line))
                {
                    ++(fileInfo.m_linesCount);
                }

                fin.close();
            }
            catch (const std::system_error &except)
            {
                std::unique_lock<std::mutex> lock(mutexLineCounter);
                errorFunc(fileInfo.m_pathToFile, except.code());
            }
        }
    }
}

std::future<std::size_t> AsyncCountLines(const fs::path &pathToDir, asio::thread_pool &threadPool, errorFuncType errorFunc)
{
    std::promise<std::size_t> promiseCountLines;
    std::future<std::size_t> futureCountLines = promiseCountLines.get_future();

    asio::post(threadPool, [&pathToDir, &threadPool, promiseCountLines = std::move(promiseCountLines), &errorFunc]() mutable
    {
        std::mutex mutexLineCounter;
        std::condition_variable conditionVar;

        auto filesInfo = GetFilesName(pathToDir, mutexLineCounter, errorFunc);
        auto vectorIter = filesInfo.begin();
        
        const std::size_t tasksNumber = std::min(filesInfo.size(), static_cast<size_t>(std::thread::hardware_concurrency()));
        std::size_t tasksCount = tasksNumber;

        for (size_t i = 0; i < tasksNumber; ++i)
        {
            asio::post(threadPool, [&]()
            {
                CountLines(filesInfo, vectorIter, mutexLineCounter, errorFunc);

                std::unique_lock<std::mutex> lock(mutexLineCounter);
                if (--tasksCount == 0)
                {
                    conditionVar.notify_one();
                }
            });
        }

        std::unique_lock<std::mutex> lock(mutexLineCounter);
        conditionVar.wait(lock, [&]() { return tasksCount == 0; });

        promiseCountLines.set_value(SumLinesNumber(filesInfo));
    });


    return futureCountLines;
}