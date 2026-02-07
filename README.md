# 🍋‍🟩LIME: A Low-Level Linux Metrics

## Overview

**LIME** is a lightweight, terminal-based system monitor for Linux written from scratch in **C++**. 
Unlike standard monitoring scripts that rely on high-level libraries (like Python's `psutil`) or command-line parsers (like calling `top` via subprocess), **LIME interacts directly with the Linux Kernel via the `/proc` virtual filesystem.**
The goal of this project was to expand knowledge and the understanding of **Operating System architecture**, specifically how the Linux kernel exposes process and resource metrics to user space, and to implement efficient text parsing in C++.

## Key Features

* **Real-time CPU Usage:** Calculates utilization percentage per core by analyzing raw 'jiffies' from `/proc/stat`.
* **Memory Management:** Visualizes RAM and Swap usage by parsing `/proc/meminfo`.
* **Process Viewer:** Lists active processes with PID, User, and Command, fetched by script that executes `ps aux`.
* **System Info:** Displays Kernel version, Uptime, and OS release information.
* **TUI Interface:** Clean, flicker-free terminal interface using `ncurses`.

## Technical Highlights

* **Linux System Programming:**
    * Understanding the **`/proc` filesystem** structure.
* **C++ Development:**
    * **String Manipulation:** Efficient parsing of complex, whitespace-delimited kernel data files.
    * **STL Containers:** Using `std::vector` and `std::map` for dynamic process lists.
* **System Architecture:**


## Prerequisites
* Linux environment (Ubuntu/Debian/Arch/Fedora)
* C++ Compiler (g++ or clang++) supporting C++17
* Make or CMake
* Ncurses library (`libncurses-dev` or `ncurses-devel`)

## Additional Info
* Project start date: **january 2026**
* Last update: **07.02.2026**
* Status: **under development**
