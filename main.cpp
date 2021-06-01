#include <functional>
#include <boost/program_options.hpp>

#include "lineCounter.hpp"

namespace po = boost::program_options;

int main(int argc, char **argv)
{
    po::options_description desc("Options");
    desc.add_options()
        ("help", "Show current message")
        ("path", po::value<std::string>(), "Path to a directory");
    
    po::variables_map varsMap;
    po::store(po::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), varsMap);
    po::notify(varsMap);

    std::function showHelp = [&desc]()
    {
        std::cout << desc << std::endl;
        std::exit(EXIT_FAILURE);
    };

    po::variables_map::iterator varsMapIter;
    varsMapIter = varsMap.find("path");

    fs::path pathToDir;
    if (varsMapIter != varsMap.end())
    {
        pathToDir = varsMapIter->second.as<std::string>();
    }
    else
    {
        showHelp();
    }

    varsMapIter = varsMap.find("help");
    if (varsMapIter != varsMap.end())
    {
        showHelp();
    }

    asio::thread_pool threadPool(std::thread::hardware_concurrency());

    auto futureCountLines = asyncCountLines(pathToDir, threadPool);

    // Do something else

    auto totalLines = futureCountLines.get();
    std::cout << "Total lines: " << totalLines << std::endl;

    threadPool.join();
    return 0;
}