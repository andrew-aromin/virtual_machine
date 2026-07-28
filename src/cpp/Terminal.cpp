#include "Terminal.hpp"
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <iostream>
#include <unistd.h>
#include <sys/select.h>

namespace lc3 {

Terminal* Terminal::s_instance = nullptr;

Terminal::Terminal() {
    s_instance = this;
    std::signal(SIGINT, Terminal::handle_interrupt);

    if (tcgetattr(STDIN_FILENO, &original_tio_) == 0) {
        struct termios new_tio = original_tio_;
        new_tio.c_lflag &= ~(ICANON | ECHO);
        if (tcsetattr(STDIN_FILENO, TCSANOW, &new_tio) == 0) {
            raw_mode_active_ = true;
        }
    }
}

Terminal::~Terminal() {
    restore();
    if (s_instance == this) {
        s_instance = nullptr;
    }
}

void Terminal::restore() {
    if (raw_mode_active_) {
        tcsetattr(STDIN_FILENO, TCSANOW, &original_tio_);
        raw_mode_active_ = false;
    }
}

void Terminal::handle_interrupt(int signal) {
    if (s_instance) {
        s_instance->restore();
    }
    std::cout << "\n";
    std::exit(-2);
}

bool Terminal::check_key() {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    return select(STDIN_FILENO + 1, &readfds, nullptr, nullptr, &timeout) != 0;
}

char Terminal::read_char() {
    return static_cast<char>(std::getchar());
}

void Terminal::write_char(char ch) {
    std::putc(ch, stdout);
    std::fflush(stdout);
}

} // namespace lc3
