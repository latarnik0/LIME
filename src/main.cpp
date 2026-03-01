#include <ncurses.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include "structs.hpp"
#include "lime_core.hpp"
#include "ui.hpp"

int main(){
    std::atomic<bool> run(true);
    std::mutex m;
    STATE state;
    
    std::thread t(gather_data, std::ref(state), std::ref(m), std::ref(run));

    initscr();
    cbreak();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(500); 
 
    int scroll_offset = 0;

    while(run){
        int current_max_size = 0;

        {
            std::lock_guard<std::mutex> lock(m);
            current_max_size = state.pCurr.size();

            if (current_max_size > 0 && scroll_offset >= current_max_size) {
                scroll_offset = current_max_size - 1;
            } else if (current_max_size == 0) {
                scroll_offset = 0;
            }

 
            print_res(state, scroll_offset);
            refresh();
            
        } 
        int ch = getch();

        
        if(ch == 'q' || ch == 'Q'){
            run = false;
        }
        else if(ch == KEY_UP){
            if(scroll_offset > 0){
                scroll_offset--;
            }
        }
        else if(ch == KEY_DOWN){
            
            if(current_max_size > 0 && scroll_offset < current_max_size - 1){
                scroll_offset++;
            }
        }
    }

    
    t.join(); 
    endwin();
    return 0;
}
