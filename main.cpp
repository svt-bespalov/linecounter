#include <functional>
#include <boost/program_options.hpp>

#include "lineCounter.hpp"

namespace po = boost::program_options;

int main(int argc, char **argv)
{
    po::variables_map varsMap;
    po::variables_map::iterator varsMapIter;

    po::options_description desc("Options");
    desc.add_options()
        ("help", "Show current message")
        ("path", po::value<std::string>(), "Path to a directory");
    
    po::store(po::command_line_parser(argc, argv).options(desc).allow_unregistered().run(), varsMap);
    po::notify(varsMap);

    std::function showHelp = [&desc]()
    {
        std::cout << desc << std::endl;
        std::exit(EXIT_FAILURE);
    };

    varsMapIter = varsMap.find("path");
    if (varsMapIter != varsMap.end())
    {
        fs::current_path(fs::path(varsMapIter->second.as<std::string>()));
        std::cout << "Current path: " << fs::current_path() << std::endl;
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

    LineCounter lineCounter;

    return 0;
}