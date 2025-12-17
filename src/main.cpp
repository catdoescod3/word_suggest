#include "suffix_engine.hpp"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <ctype.h>
#include <mutex>
#include <print>
#include <string>
#include <termios.h>
#include <unistd.h>

#define CTRL_KEY(k) ((k) & 0x1f)
#define CLEAR_RETURN "\x1b[2K\r"

struct termios orig_termios;
std::mutex mutex;

suffix_search_result latest_search;
uint64_t current_input_version = 0;

// Clear everything below the current line
static void clear_below()
{
    write(STDOUT_FILENO, "\x1b[0J", 4);
    fflush(stdout);
}

// Clear the current input line
static void clear_return()
{
    write(STDOUT_FILENO, CLEAR_RETURN, 5);
    fflush(stdout);
}

static void die(const char* s)
{
    perror(s);
    exit(1);
}

static void disable_raw_mode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");

    clear_below();
    write(STDOUT_FILENO, CLEAR_RETURN, 5);
    fflush(stdout);
}

static void enable_raw_mode()
{
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        die("tcgetattr");

    atexit(disable_raw_mode);

    struct termios raw = orig_termios;
    raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= CS8;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

static char read_key()
{
    char c;
    int nread;
    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
    {
        if (nread == -1 && errno != EAGAIN)
            die("read");
    }
    return c;
}

std::uint64_t version;
void on_suffix_search(suffix_search_result result)
{
    std::lock_guard<std::mutex> lock(mutex);
    // Only keep results for the latest version
    if (version >= latest_search.version)
    {
        latest_search = std::move(result);
        latest_search.version = version;
    }
}

int get_cursor_pos(int* rows, int* cols)
{
    char buf[32];
    unsigned int i = 0;
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4)
        return -1;
    while (i < sizeof(buf) - 1)
    {
        if (read(STDIN_FILENO, &buf[i], 1) != 1)
            break;
        if (buf[i] == 'R')
            break;
        i++;
    }
    buf[i] = '\0';
    if (buf[0] != '\x1b' || buf[1] != '[')
        return -1;
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2)
        return -1;
    return 0;
}

// Render suggestions below the input line
void render_suggestions(const suffix_search_result& result)
{

    // Save cursor
    int row, column;
    get_cursor_pos(&row, &column);

    clear_below();
    write(STDOUT_FILENO, "\r\n", 2);

    for (const auto& s : result.suffixes)
    {
        write(STDOUT_FILENO, "\x1b[2K\r", 5);
        std::string line = result.prefix + s + "\n";
        write(STDOUT_FILENO, line.c_str(), line.size());
    }

    // Restore cursor
    char buf[32];
    int bytes = sprintf(buf, "\x1b[%d;%dH", row, column);
    write(STDIN_FILENO, buf, bytes);

    fflush(stdout);
}

// Handle keypresses, update input_word and version
void process_key(std::string& input_word, suffix_engine& engine)
{
    char c = read_key();

    if (iscntrl(c))
    {
        switch (c)
        {
            case '\n':
            case '\r':
                engine.add_word(input_word);
                input_word.clear();
                clear_below();
                clear_return();
                break;
            case 127: // Backspace
                if (!input_word.empty())
                {
                    input_word.pop_back();
                    clear_return();
                    write(STDOUT_FILENO, input_word.c_str(), input_word.size());
                    current_input_version++;
                    engine.search(input_word);
                }
                break;
            case CTRL_KEY('q'): exit(0);
        }
    }
    else
    {
        input_word.push_back(c);
        current_input_version++;
        engine.search(input_word); // pass input_word to engine

        clear_return();
        write(STDOUT_FILENO, input_word.c_str(), input_word.size());
    }

    fflush(stdout);
}

int main()
{
    enable_raw_mode();

    std::string input_word;
    suffix_engine engine(on_suffix_search);

    while (true)
    {
        process_key(input_word, engine);

        suffix_search_result snapshot;
        {
            std::lock_guard<std::mutex> lock(mutex);
            snapshot = latest_search;
        }

        // Render suggestions only if results match current input
        if (!input_word.empty() && input_word.starts_with(snapshot.prefix))
        {
            render_suggestions(snapshot);
        }
    }
}
