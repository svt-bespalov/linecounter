#include "lineCounter.hpp"

void getFilesName(fs::path const &pathToDir, std::vector<File> &filesInfo)
{
    std::cout << "Current path: " << pathToDir.string() << std::endl;

    for (const auto &entry : fs::directory_iterator(pathToDir))
    {
        if (entry.is_regular_file())
        {
            filesInfo.push_back(File(entry.path()));
        }
    }
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

std::size_t countLines(fs::path const &pathToDir)
{
    std::vector<File> filesInfo;
    getFilesName(pathToDir, filesInfo);
    std::vector<File>::iterator vectorIter = filesInfo.begin();
    
    std::mutex mutexLineCounter;
    asio::thread_pool threadPool(std::thread::hardware_concurrency());

    for (size_t i = 0; i < std::min(filesInfo.size(), static_cast<size_t>(std::thread::hardware_concurrency())); ++i)
    {
        asio::post(threadPool, [&]()
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
                        throw std::runtime_error("Unable open file");
                    }

                    while (std::getline(fin, line))
                    {
                        ++(fileInfo.m_linesCount);
                    }

                    fin.close();
                }
                catch (const std::runtime_error &exception)
                {
                    std::unique_lock<std::mutex> lock{mutexLineCounter};

                    std::cerr << "EXCEPTION" << std::endl;
                    std::cerr << "Thread id: " << std::this_thread::get_id() << std::endl;
                    std::cerr << "File: " << fileInfo.m_pathToFile << std::endl;
                    std::cerr << "What: " << exception.what() << std::endl;
                }
            }
        });
    }

    threadPool.join();
    std::size_t totalLines = sumLinesNumber(filesInfo);

    return totalLines;
}

std::future<std::size_t> asyncCountLines(fs::path const &pathToDir)
{
   return std::async(countLines, std::ref(pathToDir));
}