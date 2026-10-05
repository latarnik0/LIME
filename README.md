# LIME: A Low-Level Linux Metrics

## Overview

**Lime** is a lightweight, terminal-based system monitor for Linux written from scratch in **C++**. 
**Lime interacts directly with the Linux Kernel via the `/proc` virtual filesystem.**
The goal of this project was to expand knowledge and the understanding of **Operating System architecture**, specifically how the Linux kernel exposes process and resource metrics to user space, and to implement efficient text parsing in C++.

## Key Features 
* **Static Info:** Lists info about system (OS, kernel version, host name) and static info about CPU like vendor, model name and more.
* **Memory:** Visualizes RAM and Swap usage by parsing `/proc/meminfo`.
* **CPU Usage:** Displays CPU usage in real time by calculating raw data from `/proc/stat`.
* **Network Stats:** Displays the KB/s received and transmitted using data from `proc/net/dev`.
* **Disks usage:** Dispalys disks memory usage and real-time read/write values.
* **Processess:** Lists active processes. Counts active User and Kernel threads. User can kill selected processess.
* **Uptime:** Displays uptime.

## Technical Highlights

* **Linux System Programming:**
    * Understanding the **`/proc` filesystem** structure. 
* **C++ Development:**
    * **Multithreading:** Implemented multithreading. One thread to gather and update data and main thread to read data and print results. Race condition is taken care of by using `std::mutex` and `std::atomic`.   
    * **Data processing:** Efficient parsing of complex, whitespace-delimited kernel data files and using `std::regex` to extract data.
    * **STL Containers:** Using `std::vector` and `std::map` for dynamic process lists.
    * **Work with filesystems:** Using `std::filesystem` to move through and check directories.
    * **Ncurses library:** Using `ncurses` to build intuitive terminal interface and manage keyboard and mouse input.

## Prerequisites
* Linux environment (Ubuntu/Debian/Arch/Fedora)
* C++ Compiler (g++ or clang++) supporting C++17
* Make or CMake
* Ncurses library (`libncurses-dev` or `ncurses-devel`)

## Additional Info
* Project start date: **january 2026**
* Last update: **03.03.2026**
* Status: **finished**
