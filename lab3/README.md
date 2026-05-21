# Lab 3 — Processes and Threads

## Task 3.1 — Win32 API Threads

This program calculates pi using numerical integration.

Parameters:
- N = 100000000
- Student ticket number = 431419
- Block size = 10 × 431419 = 4314190
- Tested thread counts: 1, 2, 4, 8, 12, 16

Used Win32 API functions:
- CreateThread
- ResumeThread
- SuspendThread

## Task 3.2 — OpenMP

This program calculates pi using OpenMP parallel for.

Parameters:
- N = 100000000
- Student ticket number = 431419
- Block size = 4314190
- Schedule: dynamic
- Tested thread counts: 1, 2, 4, 8, 12, 16

Used OpenMP:
- #pragma omp parallel for schedule(dynamic, BLOCK_SIZE)
- reduction(+:sum)
