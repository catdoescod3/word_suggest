#include "trie.hpp"
#include <algorithm>

trie_node::trie_node(char _character)
    : character(_character), is_terminal(false)
{
    for (trie_node*& node : children)
    {
        node = nullptr;
    }
}

static void fetch_suffix_recursive(trie_node* root,
                                   std::vector<std::string>& suffixes,
                                   std::string& character_history)
{
    if (root->is_terminal)
    {
        suffixes.emplace_back(character_history);
    }

    for (trie_node* node : root->children)
    {
        if (node != nullptr)
        {
            character_history.push_back(node->character);
            fetch_suffix_recursive(node, suffixes, character_history);
            character_history.pop_back();
        }
    }
}

std::vector<std::string> trie_node::fetch_suffixes(const std::string& prefix)
{
    std::vector<std::string> suffixes;

    trie_node* prefix_end = this;
    for (char character : prefix)
    {
        int character_index = character - 'a';

        // Prefix isn't in the trie
        if (prefix_end->children[character_index] == nullptr)
        {
            return suffixes;
        }

        prefix_end = prefix_end->children[character_index];
    }

    std::string history = "";
    fetch_suffix_recursive(prefix_end, suffixes, history);
    return suffixes;
}

void trie_node::add_word(std::string& word)
{
    std::transform(word.begin(), word.end(), word.begin(), ::tolower);

    trie_node* current = this;
    for (char character : word)
    {
        int character_index = character - 'a';
        if (current->children[character_index] == nullptr)
        {
            trie_node* new_node = new trie_node(character);
            current->children[character_index] = new_node;
        }

        current = current->children[character_index];
    }

    current->is_terminal = true;
}
