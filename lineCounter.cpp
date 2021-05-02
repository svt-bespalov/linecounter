#include "lineCounter.hpp"

void LineCounter::GetFilesName()
{
    for (const auto &entry : fs::directory_iterator(fs::current_path()))
    {
        if (entry.is_regular_file())
        {
            filesInfo.push_back(File(entry.path().filename()));
        }
    }

    this->listIter = filesInfo.begin();
}

void LineCounter::CountLinesNumber()
{
    boost::unique_lock<boost::mutex> lock{this->mutexLineCounter, boost::defer_lock};
    std::list<File>::iterator iter;
    std::ifstream fin;
    std::string line;

    lock.lock();

    if (this->listIter != this->filesInfo.end())
    {
        iter = (this->listIter)++;
    }
    else
    {
        this->stopPool = true;
        return;
    }

    lock.unlock();
    
    try
    {
        fin.open(iter->fileName);
        if (!fin.is_open())
        {
            throw std::runtime_error("Unable open file");
        }

        while (std::getline(fin, line))
        {
            ++(iter->linesCount);
        }

        fin.close();
    }
    catch (const std::runtime_error &exception)
    {
        lock.lock();

        std::cerr << "EXCEPTION" << std::endl;
        std::cerr << "Thread id: " << boost::this_thread::get_id() << std::endl;
        std::cerr << "File: " << iter->fileName << std::endl;
        std::cerr << "What: " << exception.what() << std::endl;

        iter = filesInfo.erase(iter);

        lock.unlock();
    }
}

void LineCounter::AsyncCountLinesNumber()
{
    asio::thread_pool pool(boost::thread::hardware_concurrency());

    while (!this->stopPool)
    {
        asio::post(pool, [&]() { CountLinesNumber(); });
    }

    pool.stop();
    pool.join();
}

void LineCounter::SumLinesNumber()
{
    for (File element : this->filesInfo)
    {
        this->totalLines += element.linesCount;
    }

    std::cout << "Total lines: " << this->totalLines << std::endl;
}