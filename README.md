# Library Management System

A C console application for students, books, authors and lending records. I built it to practise linked lists, dynamic memory, function-pointer callbacks and file persistence.

[Türkçe](README.tr.md)

The source is `src/kutuphane.c`. Students use a doubly linked list; books, authors, copies and borrowing records use singly linked lists. A dynamic array holds book-author relationships. Shared CSV routines take callbacks instead of repeating each record-processing loop.

## Build

Use a C compiler such as GCC:

```sh
gcc -std=c99 -Wall -Wextra src/kutuphane.c -o kutuphane
./kutuphane
```

The program reads and writes its CSV files relative to the working directory. **This repository does not include the example CSV data directory described by the older README.** Keep any existing records backed up before running the menu against them. File names are defined near the top of the source.

The menu supports student/book/author operations, borrowing and returns, overdue checks and score-based penalties. Borrowing dates drive the overdue calculations.

## Limits

This is a single-file coursework application, not a database-backed library service. The repository does not contain an automated regression suite. The 30 October 2026 CV review inspected the source but did not compile or interactively test this C program; the command above is the documented build command, not a new successful build report.
