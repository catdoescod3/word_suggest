#include "suffix_engine.hpp"
#include "trie.hpp"
#include <mutex>

suffix_engine::suffix_engine(search_callback _callback)
    : stop_search(false),
      callback(_callback),
      engine_thread(&suffix_engine::thread_main, this),
      trie_root('\0')
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

void suffix_engine::add_word(std::string word)
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        task_queue.push_back({task_type::add_word, std::move(word)});
    }
    condition_variable.notify_one();
}

void suffix_engine::search(const std::string& search_prefix)
{
    {
        std::lock_guard<std::mutex> lock(mutex);
        task_queue.push_back({task_type::search, std::move(search_prefix)});
    }
    condition_variable.notify_one();
}

void suffix_engine::thread_main()
{
    while (true)
    {
        task task;
        {
            std::unique_lock<std::mutex> lock(mutex);
            condition_variable.wait(lock, [&] { return !task_queue.empty(); });
            task = std::move(task_queue.front());
            task_queue.pop_front();
        }

        switch (task.type)
        {
            case task_type::add_word: trie_root.add_word(task.data); break;
            case task_type::search:
            {
                auto search_result = trie_root.fetch_suffixes(task.data);
                callback({task.data, std::move(search_result)});
                break;
            }
            case task_type::stop: return;
        }
    }
}
