#ifndef TERMINAL_HPP
#define TERMINAL_HPP

#include <termios.h>

namespace lc3 {

class Terminal {
public:
    Terminal();
    ~Terminal();

    Terminal(const Terminal&) = delete;
    Terminal& operator=(const Terminal&) = delete;
    Terminal(Terminal&&) = delete;
    Terminal& operator=(Terminal&&) = delete;

    static bool check_key();
    static char read_char();
    static void write_char(char ch);

private:
    struct termios original_tio_{};
    bool raw_mode_active_{false};

    static void handle_interrupt(int signal);
    static Terminal* s_instance;
    void restore();
};

} // namespace lc3

#endif // TERMINAL_HPP
