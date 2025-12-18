# word_suggest


https://github.com/user-attachments/assets/89b5c344-ad6d-4d47-9c7e-f5f2a988787e


## Build

Requirements
- CMake version 4.0 or higher

Steps
- Clone the repository:
`git clone git@github.com:catdoescod3/word_suggest.git word_suggest && cd word_suggest`

- Run CMake:
`cmake -S . -B build`

- Compile the project:
`cd build && make`

- Run the program:
`./word_suggest`

## Usage

The program starts off with no stored words.

To add a word, simply type out the word and press **Enter**.
When you start typing the same word again, it will appear in the list **below
the input line**

## Project Analysis

The main components of this project are:
1. UI 
2. Suffix engine
3. Trie data structure

<img width="542" height="523" alt="image" src="https://github.com/user-attachments/assets/2095503b-e481-4b55-866f-061b5b4900a7" />


The UI is simple but demonstrates the capabilities of the engine. 
You could attach the engine to any client needing this functionality.

The **suffix engine** acts as an interface for querying the trie via an 
asynchronous callback. This design ensures a smooth UI by offloading 
search work to a worker thread.

The trie is used for efficiently searching words by prefix. Adding a 
word or checking if a word exists in a trie is O(n), where n is the 
length of the word. In this project, the trie is primarily used to 
return all words matching a given prefix for the suggestion list.

The algorithm for getting matching words is as follows:

1. Traverse the trie to the end of the prefix.

2. Perform a depth-first search (DFS) on each child node, keeping track of the string history.

3. When a node marked as terminal is found, append the string history to a list of suffixes.

4. Combine the prefix with each suffix when rendering.

<img width="403" height="441" alt="image" src="https://github.com/user-attachments/assets/606e285a-ab01-49f9-90a5-dc041b4fd2d5" />

## Pitfalls

### Trie

- For better cache locality, the trie could use a dynamic arena for node allocation.
- This would improve performance if pre-loading a large number of words.
- Cache efficiency also depends on insertion order of words.

### Engine

- The callback interface could be improved.
- Overall, the engine is stable, and this project provided valuable hands-on experience with concurrency.

### UI
- The UI was not the primary focus.
- The code is messy and could be refactored for clarity and maintainability.

## Lessons Learned
- Running the UI and engine on separate threads required careful synchronization of search results.
- Using `std::mutex` and `std::condition_variable` gave me hands-on experience beyond a top-level understanding.
- I gained a practical understanding of designing asynchronous systems while keeping the UI responsive.
