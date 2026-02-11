#include "ui.hpp"
#include <ncurses.h>

// PRINT STATE
void print_res(STATE &state){

	int max_x, max_y;
	getmaxyx(stdscr, max_y, max_x);

	start_color();
    use_default_colors();

    init_pair(1, COLOR_RED, COLOR_WHITE);
    init_pair(2, COLOR_BLUE, COLOR_WHITE);
    init_pair(3, COLOR_WHITE, -1);
	init_pair(4, COLOR_GREEN, COLOR_BLACK);
	init_pair(5, COLOR_YELLOW, COLOR_BLACK);
	init_pair(6, COLOR_RED, COLOR_BLACK);
	init_pair(7, COLOR_BLACK, COLOR_BLACK);


//----------------------------------STATIC INFO----------------------------------------------------------
    attron(COLOR_PAIR(2));
	attron(A_BOLD);
	attron(A_STANDOUT);
	mvprintw(0, 20, "STATIC INFO");
	attroff(COLOR_PAIR(2));
	attroff(A_BOLD);
	attroff(A_STANDOUT);

	attron(COLOR_PAIR(3));
	attron(A_BOLD);
	attron(A_STANDOUT);
	mvprintw(3, 24, "SYSTEM");
	attroff(COLOR_PAIR(3));
	attroff(A_BOLD);
	attroff(A_STANDOUT);

	mvprintw(4, 0, "OS: %s", state.sys.os.c_str());
    	mvprintw(5, 0, "Kernel version: %s", state.sys.kver.c_str());
    	mvprintw(6, 0, "Host name: %s", state.sys.hostname.c_str());
	
	attron(COLOR_PAIR(3));
        attron(A_BOLD);
        attron(A_STANDOUT);
        mvprintw(8, 24, "CPU");
        attroff(COLOR_PAIR(3));
        attroff(A_BOLD);
        attroff(A_STANDOUT);

	mvprintw(9, 0, "Vendor: %s" ,state.cpus.vendor.c_str());
    	mvprintw(10, 0, "Family: %s ", state.cpus.cpufamily.c_str());
    	mvprintw(11, 0, "Model Name: %s ", state.cpus.modelname.c_str());
    	mvprintw(12, 0, "Cores: %s ", (state.cpus.cores).c_str());
    	mvprintw(13, 0, "Clock speed: %s ", (state.cpus.mhz).c_str());
	printw(" MHz");



	attron(COLOR_PAIR(3));
    	attron(A_BOLD);
    	attron(A_STANDOUT);
    	mvprintw(15, 20, "Disks Usage");
    	attroff(COLOR_PAIR(3));
    	attroff(A_BOLD);
    	attroff(A_STANDOUT);




//----------------------------------RESOURCES----------------------------------------------------------------
	attron(COLOR_PAIR(1));
	attron(A_BOLD);
	attron(A_STANDOUT);
	mvprintw(0, 80, "RESOURCES");
	attroff(COLOR_PAIR(1));
    attroff(A_BOLD);
    attroff(A_STANDOUT);

	attron(COLOR_PAIR(3));
    attron(A_BOLD);
    attron(A_STANDOUT);
    mvprintw(3, 83, "RAM");
    attroff(COLOR_PAIR(3));
    attroff(A_BOLD);
    attroff(A_STANDOUT);
	
    	
	// RAM 
    mvprintw(4, 70, "RAM total: %.2f", static_cast<float> (state.mem.tot / 1000.0));
	printw(" MB");
    mvprintw(5, 70, "RAM available: %.2f", static_cast<float> (state.mem.av / 1000.0));
	printw(" MB");
	mvprintw(6, 70, "Usage: %.2f", static_cast<float> (((state.mem.tot/1000.0) - (state.mem.av/1000.0) ) / (state.mem.tot/1000.0) * 100.0));
	printw("%%");
	
	// swap mem
	mvprintw(8, 70, "Swap total: %.2f", static_cast<float> (state.mem.swapt / 1000.0));
        printw(" MB");
    	mvprintw(9, 70, "Swap free: %.2f", static_cast<float> (state.mem.swapf / 1000.0));
        printw(" MB");
        mvprintw(10, 70, "Usage: %.2f", static_cast<float> (((state.mem.swapt/1000.0) - (state.mem.swapf/1000.0) ) / (state.mem.swapt/1000.0) * 100.0));
        printw("%%");

	mvprintw(12, 70, "RAM");
	mvprintw(13, 70, "[");
	for(int i=0; i<50; i++){
        if(i<state.mem.usg/2){
            if(i<=30){
                attron(COLOR_PAIR(4));
                printw("|");
                attroff(COLOR_PAIR(4));
            }
        	else if(i>30 && i<=40){
                attron(COLOR_PAIR(5));
                printw("|");
                attroff(COLOR_PAIR(5));
        	}
        else{
            attron(COLOR_PAIR(6));
            printw("|");
            attroff(COLOR_PAIR(6));
        	}
        }
    	else{
                        attron(COLOR_PAIR(7));
                        printw(" ");
                        attroff(COLOR_PAIR(7));
                }
        }

	mvprintw(13, 120, "]");


	attron(COLOR_PAIR(3));
        attron(A_BOLD);
        attron(A_STANDOUT);
        mvprintw(15, 80, "CPU Usage");
        attroff(COLOR_PAIR(3));
        attroff(A_BOLD);
        attroff(A_STANDOUT);

        mvprintw(16, 70, "All cores: %.2f", state.curr.usage);
        printw(" %%");

	mvprintw(18, 70, "[");

	for(int i=0; i<50; i++){
                if(i<state.curr.usageInt/2){
			if(i<=30){
                        	attron(COLOR_PAIR(4));
                        	printw("|");
                        	attroff(COLOR_PAIR(4));
                	}
                	else if(i>30 && i<=40){
                        	attron(COLOR_PAIR(5));
                        	printw("|");
                        	attroff(COLOR_PAIR(5));
                	}
                	else{
                        	attron(COLOR_PAIR(6));
                        	printw("|");
                        	attroff(COLOR_PAIR(6));
                	}
		}
		else{
			attron(COLOR_PAIR(7));
        		printw(" ");
			attroff(COLOR_PAIR(7));
		}
        }
        mvprintw(18, 120, "]");



	mvprintw(0, 140, "Uptime: %02d:%02d:%02d", state.proc.uptimeH, state.proc.uptimeM, state.proc.uptimeS);
	mvprintw(2, 140, "Kernel threads: %d", state.psaux.kthrd);
	mvprintw(3, 140, "User threads: %d", state.psaux.uthrd);
	mvprintw(4, 140, "Receive: %ld", state.net.rx/1000);
	printw("kB");
	mvprintw(5, 140, "Transmit: %ld", state.net.tx/1000);
	printw("kB");
}
