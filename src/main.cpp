#include "suffix_engine.hpp"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <print>
#include <termios.h>

#define STDIN 0

static termios original_terminal_settings;

void die(const char* die_message)
{
    std::perror(die_message);
    std::exit(1);
}

void disable_raw_mode()
{
    if (tcsetattr(STDIN, TCSAFLUSH, &original_terminal_settings) == -1)
    {
        die("tcsetattr");
    }
}

void enable_raw_mode()
{
    tcgetattr(STDIN, &original_terminal_settings);
    atexit(disable_raw_mode);

    termios raw = original_terminal_settings;
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP | IXON | ICRNL);
    raw.c_cflag |= CS8;
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    tcsetattr(STDIN, TCSAFLUSH, &raw);
}

char read_key()
{
    char read_char;
    while (int read_bytes = read(STDIN, &read_char, 1) != 1)
    {
        if (read_bytes == -1 && errno != EAGAIN)
            die("read");
    }

    return read_char;
}

bool process_key_press()
{
    char key = read_key();

    switch (key)
    {
        case 'q': return false;
    }

    return true;
}

void process_suffixes(suffix_search_result results)
{
    for (auto suffix : results.suffixes)
    {
        std::println("{}{}", results.prefix, suffix);
    }
}

int main()
{
    suffix_engine suffix_engine(process_suffixes);
    suffix_engine.search("prefix");
    std::cin.get();
}
