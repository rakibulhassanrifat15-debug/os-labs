# Lab 4 — Interprocess Communication

## Task 4.1 — Readers and Writers

Programs:
- Lab4_Reader.cpp
- Lab4_Writer.cpp
- Lab4_RW_Common.h

Used Win32 API:
- CreateFileMappingW
- MapViewOfFile
- VirtualLock
- CreateMutexW
- CreateSemaphoreW
- WaitForSingleObject
- ReleaseMutex
- ReleaseSemaphore
- timeGetTime

Parameters:
- Student ticket number: 431419
- Number of buffer pages: 18
- Page size: 4096 bytes
- Total processes in experiment: 19
- Readers: 9
- Writers: 10

## Task 4.2 — Named Pipes

Programs:
- Lab4_PipeServer.cpp
- Lab4_PipeClient.cpp

Used Win32 API:
- CreateNamedPipe
- ConnectNamedPipe
- DisconnectNamedPipe
- CreateEvent
- WriteFile
- WaitForSingleObject
- CreateFile
- ReadFileEx
- SleepEx

Test message:
Hello from server!
