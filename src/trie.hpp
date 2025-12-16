#ifndef H_TRIE
#define H_TRIE

#include <array>
#include <string>
#include <vector>

struct trie_node
{
    char character;
    std::array<trie_node*, 26> nodes;
};

void trie_add_word(trie_node& trie, const std::string& word);
std::vector<std::string> trie_fetch_suffixes(trie_node& trie,
                                             const std::string& prefix);

#endif
