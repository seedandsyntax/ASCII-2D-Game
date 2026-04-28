#include <curses.h>
#include <print>

int main(){
    initscr();
    cbreak();
    keypad(stdscr, 1);

    int input;
    size_t times {0};
    while ((input = getch()) != 'q'){ // press "q" to exit
        if (input == KEY_UP) {
            times++;
            printw("arrow up detected: %d\n", times);
            refresh();
            if (times % 10 == 0) clear();
        }

        if (input == 'c'){
            clear(); // clear terminal
        }
    }
    
    std::println("Goodbye!");
    endwin();
    return 0;
}
