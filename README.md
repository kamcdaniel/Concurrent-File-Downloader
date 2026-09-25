# Concurrent File Downloader

## Overview

A concurrent file downloader written in C. The program reads download requests from an input file and uses multiple child processes to download files concurrently with `curl`.

## Features

* Creates a separate process for each download request using `fork()`
* Limits the number of simultaneous download processes
* Uses `exec()` to execute `curl`
* Supports optional download time limits
* Tracks child processes and properly reaps terminated processes
* Reports the process ID and input line number for each download
* Uses a Makefile for compilation
* Designed to produce a clean Valgrind report

## Download Functionality

The program accepts an input file containing an output filename, URL, and optional maximum download time:

```text
filename url [seconds]
```

For example:

```text
file.txt https://example.com/file.txt 10
```

The program executes the corresponding `curl` command and allows additional downloads to begin as existing processes finish.

## Technologies

* C
* UNIX/Linux system calls
* `fork()`
* `exec()`
* `wait()` / `waitpid()`
* Process management
* `curl`
* Valgrind
* Make
