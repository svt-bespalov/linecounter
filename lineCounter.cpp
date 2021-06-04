#include <iostream>
#include <fstream>
#include <exception>
#include <vector>
#include <string>
#include <thread>
#include <latch>
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

    std::vector<File> getFilesName(fs::path const &pathToDir)
    {
        std::vector<File> filesInfo;
        std::cout << "Current path: " << pathToDir.string() << std::endl;

        for (const auto &entry : fs::directory_iterator(pathToDir))
        {
            if (entry.is_regular_file())
            {
                filesInfo.emplace_back(File(entry.path()));
            }
        }

        return filesInfo;
    }

    std::size_t sumLinesNumber(std::vector<File> &filesInfo)
    {
        std::size_t totalLines = 0;

        for (File element : filesInfo)
        {
            totalLines += element.m_linesCount;
        }

        return totalLines;
    }

    void countLines(std::vector<File> &filesInfo, std::vector<File>::iterator &vectorIter, std::mutex &mutexLineCounter, errorFuncType &errorFunc)
    {
        auto isVectorEnd = [&]() -> bool
        {
            std::unique_lock<std::mutex> lock{mutexLineCounter};
            return vectorIter == filesInfo.end();
        };

        auto getFileInfo = [&]() -> File&
        {
            std::unique_lock<std::mutex> lock{mutexLineCounter};
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
                    throw ENOENT;
                }

                while (std::getline(fin, line))
                {
                    ++(fileInfo.m_linesCount);
                }

                fin.close();
            }
            catch (int errCode)
            {
                std::unique_lock<std::mutex> lock{mutexLineCounter};
                errorFunc(fileInfo.m_pathToFile, std::error_code(errCode, std::generic_category()));
            }
        }
    }
}

std::future<std::size_t> asyncCountLines(fs::path const &pathToDir, asio::thread_pool &threadPool, errorFuncType errorFunc)
{
    std::promise<std::size_t> promiseCountLines;
    std::future<std::size_t> futureCountLines = promiseCountLines.get_future();

    asio::post(threadPool, [&pathToDir, &threadPool, promiseCountLines = std::move(promiseCountLines), &errorFunc]() mutable
    {
        std::mutex mutexLineCounter;
        auto filesInfo = getFilesName(pathToDir);
        auto vectorIter = filesInfo.begin();
        
        std::size_t tasksNumber = std::min(filesInfo.size(), static_cast<size_t>(std::thread::hardware_concurrency()));
        std::latch latches(tasksNumber);

        for (size_t i = 0; i < tasksNumber; ++i)
        {
            asio::post(threadPool, [&]()
            {
                countLines(filesInfo, vectorIter, mutexLineCounter, errorFunc);
                latches.count_down();
            });
        }

        latches.wait();

        promiseCountLines.set_value(sumLinesNumber(filesInfo));
    });


    return futureCountLines;
}