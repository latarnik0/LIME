# 🍋‍🟩LIME: A Low-Level Linux Metrics

## Overview

**Lime** is a lightweight, terminal-based system monitor for Linux written from scratch in **C++**. 
**Lime interacts directly with the Linux Kernel via the `/proc` virtual filesystem.**
The goal of this project was to expand knowledge and the understanding of **Operating System architecture**, specifically how the Linux kernel exposes process and resource metrics to user space, and to implement efficient text parsing in C++.

## Key Features 
* **Static Info:** Lists info about system (OS, kernel version, host name) and static info about CPU like vendor, model name and more.
* **Memory:** Visualizes RAM and Swap usage by parsing `/proc/meminfo`.
* **CPU Usage:** Displays CPU usage in real time by calculating raw data from `/proc/stat`.
* **Network Stats:** Displays the amount of bytes received and transmitted using data from `proc/net/dev`.
* **Processess:** Counts active User and Kernel threads.
* **Uptime:** Displays uptime.
* **TUI Interface:** Intuitive terminal interface using `ncurses`.

## Technical Highlights

* **Linux System Programming:**
    * Understanding the **`/proc` filesystem** structure. 
* **C++ Development:**
    * **Multithreading:** Implemented multithreading. One thread to gather and update data and main thread to read data and print results. Race condition is taken care of by using `std::mutex` and `std::atomic`.   
    * **String Manipulation:** Efficient parsing of complex, whitespace-delimited kernel data files.
    * **STL Containers:** Using `std::vector` and `std::map` for dynamic process lists.
    * **Work with filesystem:** Using `std::filesystem` to move through directories.

## Prerequisites
* Linux environment (Ubuntu/Debian/Arch/Fedora)
* C++ Compiler (g++ or clang++) supporting C++17
* Make or CMake
* Ncurses library (`libncurses-dev` or `ncurses-devel`)

## Additional Info
* Project start date: **january 2026**
* Last update: **12.02.2026**
* Status: **under development**
