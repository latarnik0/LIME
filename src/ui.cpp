#include "ui.hpp"
#include <ncurses.h>

// PRINT STATE
void print_res(STATE &state, int scroll_offset){
	erase();
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
	init_pair(9, COLOR_MAGENTA, COLOR_WHITE);


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
	printw("%10.2f MB", (state.mem.tot / 1024.0f));

	attron(A_BOLD);
    	mvprintw(17, 0, "RAM available: ");
	attroff(A_BOLD);
	printw("%10.2f MB",(state.mem.av / 1024.0f));
		
	// swap mem
	attron(A_BOLD);
	mvprintw(18, 0, "Swap total: ");
	attroff(A_BOLD);	
	printw("%8.2f MB", (state.mem.swapt / 1024.0f));

	attron(A_BOLD);
    	mvprintw(19, 0, "Swap available: "); 
	attroff(A_BOLD);
	printw("%8.2f MB", (state.mem.swapf / 1024.0f));
	

	attron(A_BOLD);
        mvprintw(20, 0, "RAM usage: ");
        attroff(A_BOLD);
        printw("%10.2f %%", state.mem.usg);

	attron(A_BOLD);
        mvprintw(21, 0, "Swap usage: ");
	attroff(A_BOLD);
	printw("%10.2f %%", state.mem.swapusg);
	
	attron(A_BOLD);
        mvprintw(22, 0, "CPU usage: ");
        attroff(A_BOLD);
	printw("%10.2f %%", state.cpud.smoothUsage);


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
                if(i<(int)state.cpud.usage/4){
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
	printw("%13.2f KB/s", state.net.rxDiff/1024.0f);

	attron(A_BOLD);
        mvprintw(31, 0, "Transmitted: ");
	attroff(A_BOLD);
	printw("%10.2f KB/s", state.net.txDiff/1024.0f);


	attron(COLOR_PAIR(9));
        attron(A_BOLD);
        attron(A_STANDOUT);
        mvprintw(34, 0, "DISKS                ");
        attroff(COLOR_PAIR(9));
        attroff(A_BOLD);
        attroff(A_STANDOUT);


	attron(COLOR_PAIR(3));
        attron(A_BOLD);
        attron(A_STANDOUT);
        mvprintw(35, 0, "# Space usage");
        attroff(COLOR_PAIR(3));
        attroff(A_BOLD);
        attroff(A_STANDOUT);

	mvprintw(36, 0, "Device");
	mvprintw(36, 12, "Total");
	mvprintw(36, 19, "Free");
	mvprintw(36, 26, "Usage");

	int row=37;
	for(const auto& fs : state.disks.filesystems) {
            mvprintw(row, 0, "%s", fs.device.c_str());
	    mvprintw(row, 12, "%.1fGB", (fs.total/(1024.0f * 1024.0f * 1024.0f)));
	    mvprintw(row, 19, "%.1fGB", (fs.available/(1024.0f * 1024.0f * 1024.0f)));
	    mvprintw(row, 26, "%.1f%%", fs.usage);
	    row++;
        }
	
	
	attron(COLOR_PAIR(3));
        attron(A_BOLD);
        attron(A_STANDOUT);
        mvprintw(40, 0, "# Read/Write");
        attroff(COLOR_PAIR(3));
        attroff(A_BOLD);
        attroff(A_STANDOUT);

	mvprintw(41, 0, "Read: %8.2f KB/s", static_cast<float>(state.disks.readDiff/1024.0f));
	mvprintw(42, 0, "Write: %8.2f KB/s", static_cast<float>(state.disks.writeDiff/1024.0f));

	attron(COLOR_PAIR(4) | A_BOLD);
	mvprintw(max_y-3, 0, "Uptime: ");
	attroff(COLOR_PAIR(4) | A_BOLD);
	printw("%02d:%02d:%02d", state.proc.uptimeH, state.proc.uptimeM, state.proc.uptimeS);
	
	attron(A_BOLD);
        mvprintw(max_y-2, 0, "Kernel threads: ");
        attroff(A_BOLD);
        printw("%d", state.psaux.kthrd);

        attron(A_BOLD);
        mvprintw(max_y-1, 0, "User threads: ");
        attroff(A_BOLD);
        printw("%d", state.psaux.uthrd);



	int pstarty = 0;
	int pstartx = 59;
	
	attron(A_REVERSE);
	for(int i=pstarty;i<max_y;i++){
		mvprintw(i, pstartx-2, "  ");
	}
	mvprintw(pstarty, pstartx, "%-8s %-12s %-8s %-8s %-8s %-10s %s", 
             "PID", "USER", "PR", "NICE", "STATE", "MEM", "COMMAND");
	for(int i = getcurx(stdscr); i < max_x; i++) addch(' ');
	attroff(A_REVERSE);

	int current_row = pstarty+1;
	
	for (size_t i = scroll_offset; i < state.pCurr.size(); ++i) {
                if (current_row >= max_y) {
                        break;
                }
                const auto& p = state.pCurr[i];
                std::string user = p.user.empty() ? "N/A" : p.user; 
		std::string cmd = p.command.empty() ? "[Kernel Thread]" : p.command;
		char state_char = (p.RSZ == 0) ? '?' : p.RSZ;

                int cmd_max_len = max_x - pstartx - 50; 
                
                if (cmd_max_len > 0 && cmd.length() > cmd_max_len) {
                        cmd = cmd.substr(0, cmd_max_len - 3);
                }
                

                mvprintw(current_row, pstartx, "%-8d %-12.12s %-8d %-8d %-8c %-10llu %-48.48s",
                        p.pid,
                        user.c_str(),
                        p.priority,
                        p.nice,
                        state_char,
                        p.memResident,
                        cmd.c_str());

  

                current_row++;
        }
        refresh();
}
