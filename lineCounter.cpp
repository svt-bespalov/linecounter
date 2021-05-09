#include "lineCounter.hpp"

LineCounter::LineCounter(std::string t_pathToDir)
    : m_pathToDir(t_pathToDir), m_totalLines(0), m_stopPool(false)
    {
        fs::current_path(m_pathToDir);
        std::cout << "Current path: " << fs::current_path() << std::endl;

        getFilesName();
        asyncCountLinesNumber();
        sumLinesNumber();
    }

void LineCounter::getFilesName()
{
    for (const auto &entry : fs::directory_iterator(fs::current_path()))
    {
        if (entry.is_regular_file())
        {
            m_filesInfo.push_back(File(entry.path().filename()));
        }
    }

    m_vectorIter = m_filesInfo.begin();
}

void LineCounter::countLinesNumber()
{
   auto isVectorEnd = [&]() -> bool
   {
       boost::unique_lock<boost::mutex> lock{m_mutexLineCounter};
       return m_vectorIter == m_filesInfo.end();
   };

   auto getFileInfo = [&]() -> File&
   {
       boost::unique_lock<boost::mutex> lock{m_mutexLineCounter};
       return *(m_vectorIter++);
   };

   while (!isVectorEnd())
   {
       std::ifstream fin;
       std::string line;

       auto &fileInfo = getFileInfo();

       try
       {
           fin.open(fileInfo.m_fileName);
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
           boost::unique_lock<boost::mutex> lock{m_mutexLineCounter};

           std::cerr << "EXCEPTION" << std::endl;
           std::cerr << "Thread id: " << boost::this_thread::get_id() << std::endl;
           std::cerr << "File: " << fileInfo.m_fileName << std::endl;
           std::cerr << "What: " << exception.what() << std::endl;
       }
   }

   m_stopPool = true;
}

void LineCounter::asyncCountLinesNumber()
{
    asio::thread_pool pool(boost::thread::hardware_concurrency());

    for (size_t i = 0; i < std::min(m_filesInfo.size(), static_cast<size_t>(boost::thread::hardware_concurrency())); ++i)
    {
        asio::post(pool, [&]() { countLinesNumber(); });
    }

    pool.stop();
    pool.join();
}

void LineCounter::sumLinesNumber()
{
    for (File element : m_filesInfo)
    {
        m_totalLines += element.m_linesCount;
    }

    std::cout << "Total lines: " << m_totalLines << std::endl;
}