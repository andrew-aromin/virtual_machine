#include <iostream>
#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <sys/select.h>
#include "LC3.hpp"

struct termios original_tio;

void disable_input_buffering() {
    tcgetattr(STDIN_FILENO, &original_tio);
    struct termios new_tio = original_tio;
    new_tio.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

void restore_input_buffering() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
}

void handle_interrupt(int signal) {
    restore_input_buffering();
    std::cout << "\n";
    exit(-2);
}

bool check_key() {
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    return select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout) != 0;
}

int main(int argc, char** argv) {
    signal(SIGINT, handle_interrupt);
    disable_input_buffering();

    LC3 vm;
    if (argc > 1) {
        if (vm.load_program(argv[1])) {
            vm.run();
        } else {
            std::cerr << "Failed to load image: " << argv[1] << std::endl;
        }
    }

    restore_input_buffering();
    return 0;
}


