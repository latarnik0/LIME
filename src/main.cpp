#include <ncurses.h>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <csignal>
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
    mousemask(ALL_MOUSE_EVENTS, NULL);
    
    int max_X, max_Y;
    getmaxyx(stdscr, max_Y, max_X);
    
    timeout(500); 
 
    int selectedPID = -1;
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

 
            print_res(state, scroll_offset, selectedPID);
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
	else if (ch == KEY_MOUSE) {
		MEVENT event;
    		if (getmouse(&event) == OK){
        		if (event.bstate & BUTTON1_CLICKED || event.bstate & BUTTON1_PRESSED) {
            			int pstarty = 0; 
            
            			if (event.y > pstarty && event.y < max_Y) {
                			int clicked_row_index = event.y - (pstarty + 1);
                			int target_vector_index = scroll_offset + clicked_row_index;

                			std::lock_guard<std::mutex> lock(m);
                			if (target_vector_index >= 0 && target_vector_index < state.pCurr.size()) {
                    
                    				selectedPID = state.pCurr[target_vector_index].pid;
                			}
            			}
        		}
		
                	#if NCURSES_MOUSE_VERSION > 1
                	else if (event.bstate & BUTTON4_PRESSED) {
                    		if (scroll_offset > 0) {
                        		scroll_offset--;
                    		}
                	}
                	else if (event.bstate & BUTTON5_PRESSED) {
                    		if (current_max_size > 0 && scroll_offset < current_max_size - 1) {
                        		scroll_offset++;
                    		}
                	}
                	#endif
    		}
	}
	else if (ch == 'k' || ch == 'K') {
    		if (selectedPID != -1) {
                	kill(selectedPID, SIGTERM); 
    		}
	}
    }

    
    t.join(); 
    endwin();
    return 0;
}
