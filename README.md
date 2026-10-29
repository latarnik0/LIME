# LIME: A Low-Level Linux Metrics

## Overview

**Lime** is a lightweight system monitor for Linux written from scratch in **C++**. 
**Lime interacts directly with the Linux Kernel via the `/proc` and `/sys` virtual filesystems.** 

The goal of this project was to expand knowledge and the understanding of **Operating System architecture**, specifically how the Linux kernel exposes process and hardware resource metrics to user space, and to implement efficient text parsing in C++. 

**Project Evolution:** Lime originally started as a strictly terminal-based utility using `ncurses`. It is currently undergoing a major architectural expansion to replace the terminal UI with a modern, graphical desktop interface built with **Qt Quick and QML**, while simultaneously expanding the backend capabilities to include deeper hardware metrics.

## Key Features 
- **Static Info:** Lists info about system (OS, kernel version, host name) and static info about CPU like vendor, model name and more.
- **Memory:** Visualizes RAM and Swap usage by parsing `/proc/meminfo`.
- **CPU Usage:** Displays total and **per-core** CPU usage (in percentages) in real time by calculating raw data from `/proc/stat`.
- **Temperatures:** Monitors real-time hardware temperatures by reading data from the `/sys` filesystem.
- **Network Stats:** Displays the KB/s received and transmitted using data from `/proc/net/dev`.
- **Disks usage:** Displays disks memory usage and real-time read/write values.
- **Processes:** Lists active processes. Counts active User and Kernel threads. User can kill selected processes.
- **Uptime:** Displays system uptime.

## Technical Highlights

- **Linux System Programming:**
    * Understanding and traversing the **`/proc` and `/sys` filesystems** structures to extract software and hardware metrics.
- **Modern GUI (Current Focus):**
    * **Qt Quick / QML:** Building a responsive, modern desktop graphical user interface.
    * **C++/QML Integration:** Exposing low-level backend data to the QML frontend efficiently.
- **C++ Backend Development:**
    * **Multithreading:** Implemented multithreading. One thread to gather and update data and the main thread to handle the UI. Race conditions are prevented using `std::mutex` and `std::atomic`.   
    * **Data processing:** Efficient parsing of complex, whitespace-delimited kernel data files using `std::regex` to extract data.
    * **Filesystem:** Using `std::filesystem` to move through and check directories (e.g., iterating through `/sys/class/thermal` or `/sys/class/hwmon`).
- **Legacy Terminal UI (v1.0):**
    * **Ncurses library:** The original version utilized `ncurses` to build an intuitive terminal interface and manage keyboard/mouse input.

## Prerequisites
- Linux environment (Ubuntu/Debian/Arch/Fedora)
- C++ Compiler (g++ or clang++) supporting C++17 or higher
- CMake
- **Qt 6 (or Qt 5)** development packages and QML modules
- *(Legacy)* Ncurses library (`libncurses-dev` or `ncurses-devel`) if building the older terminal version

## Additional Info
- Project start date: **January 2026**
- Last update: **29.10.2026**
- Status: **In active development (transitioning from Terminal UI to Qt/QML GUI; backend expanding to support `/sys` sensors and per-core metrics)**
