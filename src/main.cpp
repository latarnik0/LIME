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
        timeout(500);

	while(run){
		{	
			std::lock_guard<std::mutex> lock(m);
			print_res(state);
		}
		
		refresh();
		int quit = getch();
		if(quit == 'q' || quit == 'Q'){
			run = false;
		}
	
	}

	t.join();
	endwin();
	return 0;
}
