#include "suffix_engine.hpp"
#include "trie.hpp"
#include <mutex>

suffix_engine::suffix_engine(search_callback _callback)
    : stop_search(false),
      callback(_callback),
      engine_thread(&suffix_engine::thread_main, this)
{}

suffix_engine::~suffix_engine()
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        stop_search = true;
    }

    condition_variable.notify_one();
    engine_thread.join();
}

void suffix_engine::search(const std::string& search_prefix)
{
    if (search_prefix.empty())
        return;

    {
        std::lock_guard<std::mutex> lock(mutex);
        prefix = search_prefix;
    }

    condition_variable.notify_one();
}

void suffix_engine::thread_main()
{
    while (true)
    {
        suffix_search_result result;
        {
            std::unique_lock<std::mutex> lock(mutex);
            condition_variable.wait(
                lock, [&] { return stop_search || !prefix.empty(); });

            if (stop_search)
                return;

            result.prefix = std::move(prefix);
            prefix.clear();
        }

        result.suffixes = trie_fetch_suffixes(trie_root, result.prefix);
        callback(std::move(result));
    }
}
