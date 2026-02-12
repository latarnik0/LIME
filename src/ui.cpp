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
	init_pair(8, COLOR_GREEN, COLOR_WHITE);


//----------------------------------STATIC INFO----------------------------------------------------------
	attron(COLOR_PAIR(2));
	attron(A_BOLD);
	attron(A_STANDOUT);
	mvprintw(0, 0, "STATIC INFO          ");
	attroff(COLOR_PAIR(2));
	attroff(A_BOLD);
	attroff(A_STANDOUT);

	attron(COLOR_PAIR(3));
	attron(A_BOLD);
	attron(A_STANDOUT);
	mvprintw(1, 0, "# System");
	attroff(COLOR_PAIR(3));
	attroff(A_BOLD);
	attroff(A_STANDOUT);
	
	attron(A_BOLD);
	mvprintw(2, 0, "OS: ");
	attroff(A_BOLD);
	printw("%s", state.sys.os.c_str());
	attron(A_BOLD);
    	mvprintw(3, 0, "Kernel version: ");
	attroff(A_BOLD);
        printw("%s", state.sys.kver.c_str());
	attron(A_BOLD);
    	mvprintw(4, 0, "Host name: ");
	attroff(A_BOLD);
	printw("%s", state.sys.hostname.c_str());
	
	attron(COLOR_PAIR(3));
        attron(A_BOLD);
        attron(A_STANDOUT);
        mvprintw(6, 0, "# CPU");
        attroff(COLOR_PAIR(3));
        attroff(A_BOLD);
        attroff(A_STANDOUT);

	attron(A_BOLD);
	mvprintw(7, 0, "Vendor: ");
    	attroff(A_BOLD);
	printw("%s", state.cpus.vendor.c_str());
	
	attron(A_BOLD);
	mvprintw(8, 0, "Family: ");
    	attroff(A_BOLD);
	printw("%s", state.cpus.cpufamily.c_str());
	
	attron(A_BOLD);
	mvprintw(9, 0, "Model Name: ");
    	attroff(A_BOLD);
	printw("%s", state.cpus.modelname.c_str() );

	attron(A_BOLD);
	mvprintw(10, 0, "Cores: ");
    	attroff(A_BOLD);
	printw("%s", state.cpus.cores.c_str());

	attron(A_BOLD);
	mvprintw(11, 0, "Clock speed: ");
	attroff(A_BOLD);
	printw("%s", state.cpus.mhz.c_str());
	printw(" MHz");


//----------------------------------RESOURCES----------------------------------------------------------------
	attron(COLOR_PAIR(1));
	attron(A_BOLD);
	attron(A_STANDOUT);
	mvprintw(14, 0, "RESOURCES            ");
	attroff(COLOR_PAIR(1));
    	attroff(A_BOLD);
    	attroff(A_STANDOUT);
	
	attron(COLOR_PAIR(3));
        attron(A_BOLD);
        attron(A_STANDOUT);
        mvprintw(15, 0, "# Memory & CPU");
        attroff(COLOR_PAIR(3));
        attroff(A_BOLD);
        attroff(A_STANDOUT);

	// RAM
	attron(A_BOLD); 
    	mvprintw(16, 0, "RAM total: ");
	attroff(A_BOLD);
	printw("%.2f", static_cast<float> (state.mem.tot / 1000.0));
	printw(" MB");

	attron(A_BOLD);
    	mvprintw(17, 0, "RAM available: ");
	attroff(A_BOLD);
	printw("%.2f", static_cast<float> (state.mem.av / 1000.0));
	printw(" MB");
		
	// swap mem
	attron(A_BOLD);
	mvprintw(18, 0, "Swap total: ");
	attroff(A_BOLD);	
	printw("%.2f", static_cast<float> (state.mem.swapt / 1000.0));
        printw(" MB");

	attron(A_BOLD);
    	mvprintw(19, 0, "Swap available: "); 
	attroff(A_BOLD);
	printw("%.2f", static_cast<float> (state.mem.swapf / 1000.0));
        printw(" MB");
	

	attron(A_BOLD);
        mvprintw(20, 0, "RAM usage: ");
        attroff(A_BOLD);
        printw("%.2f", static_cast<float> (((state.mem.tot/1000.0) - (state.mem.av/1000.0) ) / (state.mem.tot/1000.0) * 100.0));
        printw("%%");

	attron(A_BOLD);
        mvprintw(21, 0, "Swap usage: ");
	attroff(A_BOLD);
	printw("%.2f", static_cast<float> (((state.mem.swapt/1000.0) - (state.mem.swapf/1000.0) ) / (state.mem.swapt/1000.0) * 100.0));
        printw("%%");
	attron(A_BOLD);
        mvprintw(22, 0, "CPU usage: ");
        attroff(A_BOLD);
	printw("%.2f", state.curr.usage);
        printw(" %%");


	mvprintw(23, 0, "[");
        for(int i=0; i<20; i++){
        if(i<state.mem.usg/4){
            if(i<=10){
                attron(COLOR_PAIR(4));
                printw("|");
                attroff(COLOR_PAIR(4));
            }
                else if(i>10 && i<=15){
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
        mvprintw(23, 20, "RAM");
	printw("]");

	
	//swap show
mvprintw(24, 0, "[");
        for(int i=0; i<20; i++){
        if(i<state.mem.swapusg/4){
            if(i<=10){
                attron(COLOR_PAIR(4));
                printw("|");
                attroff(COLOR_PAIR(4));
            }
                else if(i>10 && i<=15){
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
        mvprintw(24, 20, "Swap");
	printw("]");


	//CPU show
	mvprintw(25, 0, "[");

	for(int i=0; i<20; i++){
                if(i<state.curr.usageInt/4){
			if(i<=10){
                        	attron(COLOR_PAIR(4));
                        	printw("|");
                        	attroff(COLOR_PAIR(4));
                	}
                	else if(i>10 && i<=15){
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
        mvprintw(25, 20, "CPU");
	printw("]");

attron(COLOR_PAIR(8));
        attron(A_BOLD);
        attron(A_STANDOUT);
        mvprintw(28, 0, "NETWORK              ");
        attroff(COLOR_PAIR(8));
        attroff(A_BOLD);
        attroff(A_STANDOUT);

	attron(COLOR_PAIR(3));
        attron(A_BOLD);
        attron(A_STANDOUT);
        mvprintw(29, 0, "# Download/Upload");
        attroff(COLOR_PAIR(3));
        attroff(A_BOLD);
        attroff(A_STANDOUT);

	attron(A_BOLD);
	mvprintw(30, 0, "Received: ");
	attroff(A_BOLD);
	printw("%ld", state.netcur.rxDynamic);
        printw(" B");

	attron(A_BOLD);
        mvprintw(31, 0, "Transmitted: ");
	attroff(A_BOLD);
	printw("%ld", state.netcur.txDynamic);
        printw(" B");



	attron(COLOR_PAIR(4) | A_BOLD);
	mvprintw(0, 140, "Uptime: ");
	attroff(COLOR_PAIR(4) | A_BOLD);
	printw("%02d:%02d:%02d", state.proc.uptimeH, state.proc.uptimeM, state.proc.uptimeS);

	attron(A_BOLD);
	mvprintw(2, 140, "Kernel threads: ");
	attroff(A_BOLD);
	printw("%d", state.psaux.kthrd);

	attron(A_BOLD);
	mvprintw(3, 140, "User threads: ");
	attroff(A_BOLD);
	printw("%d", state.psaux.uthrd);
}
