#pragma once

#include <future>
#include <filesystem>
#include <functional>
#include <system_error>
#include <boost/asio/thread_pool.hpp>

namespace fs = std::filesystem;
namespace asio = boost::asio;

using errorFuncType = std::function<void(fs::path const &pathToDir, std::error_code errorCode)>;

std::future<std::size_t> asyncCountLines(fs::path const &pathToDir, asio::thread_pool &threadPool, errorFuncType errorFunc);