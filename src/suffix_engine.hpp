#ifndef H_WORD_SUGGEST
#define H_WORD_SUGGEST

#include "trie.hpp"
#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>
#include <thread>

struct suffix_search_result
{
    std::string prefix;
    std::vector<std::string> suffixes;
};

class suffix_engine
{
    using search_callback = void (*)(suffix_search_result);

    public:
    suffix_engine(search_callback _callback);
    ~suffix_engine();

    suffix_engine(const suffix_engine&) = delete;
    suffix_engine& operator=(const suffix_engine&) = delete;

    void add_word(std::string word);
    void search(const std::string& prefix);

    private:
    void thread_main();

    enum class task_type
    {
        add_word,
        search,
        stop
    };

    struct task
    {
        task_type type;
        std::string data;
    };

    trie_node trie_root;
    std::string prefix;

    std::mutex mutex;
    std::condition_variable condition_variable;
    std::deque<task> task_queue;

    bool stop_search;
    search_callback callback;
    std::thread engine_thread;
};

#endif
