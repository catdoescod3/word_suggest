#ifndef H_TRIE
#define H_TRIE

#include <array>
#include <string>
#include <vector>

struct trie_node
{
    trie_node(char _character);

    char character;
    bool is_terminal;
    std::array<trie_node*, 26> children;

    void add_word(std::string& word);
    std::vector<std::string> fetch_suffixes(const std::string& prefix);
};

#endif
