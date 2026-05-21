#pragma once

#define NOMINMAX

#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <random>
#include <ctime>
#include <cstdio>

#pragma comment(lib, "winmm.lib")

const DWORD STUDENT_TICKET_NUMBER = 431419;
const DWORD PAGES_COUNT = 18;
const DWORD MAGIC_VALUE = 0x43141904;

const wchar_t* MAP_NAME = L"Local\\Rifat_Lab4_RW_Mapping";
const wchar_t* INIT_MUTEX_NAME = L"Local\\Rifat_Lab4_RW_InitMutex";
const wchar_t* BUFFER_MUTEX_NAME = L"Local\\Rifat_Lab4_RW_BufferMutex";
const wchar_t* EMPTY_SEMAPHORE_NAME = L"Local\\Rifat_Lab4_RW_EMPTY";
const wchar_t* FULL_SEMAPHORE_NAME = L"Local\\Rifat_Lab4_RW_FULL";

enum PageState {
    PAGE_EMPTY = 0,
    PAGE_WRITING = 1,
    PAGE_FULL = 2,
    PAGE_READING = 3
};

struct SharedHeader {
    DWORD magic;
    DWORD pageSize;
    DWORD pagesCount;
    DWORD headerSize;
    DWORD totalSize;

    LONG writeIndex;
    LONG readIndex;
    LONG globalWriteNumber;

    LONG pageState[PAGES_COUNT];
};

struct SharedObjects {
    HANDLE initMutex;
    HANDLE mapping;
    HANDLE bufferMutex;
    HANDLE emptySemaphore;
    HANDLE fullSemaphore;

    LPVOID view;
    SharedHeader* header;
};

DWORD AlignToPage(DWORD value, DWORD pageSize) {
    return ((value + pageSize - 1) / pageSize) * pageSize;
}

void WriteLog(std::ofstream& log, const std::string& processName, int processId,
    const std::string& state, int pageNumber) {
    log << timeGetTime() << ";"
        << processName << ";"
        << processId << ";"
        << state << ";"
        << pageNumber << std::endl;
    log.flush();
}

DWORD RandomOperationTime() {
    static std::mt19937 generator(static_cast<unsigned int>(timeGetTime() ^ GetCurrentProcessId()));
    std::uniform_int_distribution<DWORD> distribution(500, 1500);
    return distribution(generator);
}

char* GetPagePointer(const SharedObjects& shared, DWORD pageNumber) {
    char* base = static_cast<char*>(shared.view);
    return base + shared.header->headerSize + pageNumber * shared.header->pageSize;
}

SharedObjects OpenSharedObjects() {
    SharedObjects shared{};
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    DWORD pageSize = sysInfo.dwPageSize;
    DWORD headerSize = AlignToPage(sizeof(SharedHeader), pageSize);
    DWORD totalSize = headerSize + pageSize * PAGES_COUNT;

    shared.initMutex = CreateMutexW(NULL, FALSE, INIT_MUTEX_NAME);
    if (shared.initMutex == NULL) {
        std::cout << "CreateMutex INIT failed. Error: " << GetLastError() << std::endl;
        ExitProcess(1);
    }

    WaitForSingleObject(shared.initMutex, INFINITE);

    shared.mapping = CreateFileMappingW(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        totalSize,
        MAP_NAME
    );

    if (shared.mapping == NULL) {
        std::cout << "CreateFileMapping failed. Error: " << GetLastError() << std::endl;
        ExitProcess(1);
    }

    bool alreadyExists = (GetLastError() == ERROR_ALREADY_EXISTS);

    shared.view = MapViewOfFile(
        shared.mapping,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        totalSize
    );

    if (shared.view == NULL) {
        std::cout << "MapViewOfFile failed. Error: " << GetLastError() << std::endl;
        ExitProcess(1);
    }

    shared.header = static_cast<SharedHeader*>(shared.view);

    shared.bufferMutex = CreateMutexW(NULL, FALSE, BUFFER_MUTEX_NAME);
    shared.emptySemaphore = CreateSemaphoreW(NULL, PAGES_COUNT, PAGES_COUNT, EMPTY_SEMAPHORE_NAME);
    shared.fullSemaphore = CreateSemaphoreW(NULL, 0, PAGES_COUNT, FULL_SEMAPHORE_NAME);

    if (shared.bufferMutex == NULL || shared.emptySemaphore == NULL || shared.fullSemaphore == NULL) {
        std::cout << "Synchronization object creation failed. Error: " << GetLastError() << std::endl;
        ExitProcess(1);
    }

    if (!alreadyExists || shared.header->magic != MAGIC_VALUE) {
        ZeroMemory(shared.view, totalSize);

        shared.header->magic = MAGIC_VALUE;
        shared.header->pageSize = pageSize;
        shared.header->pagesCount = PAGES_COUNT;
        shared.header->headerSize = headerSize;
        shared.header->totalSize = totalSize;
        shared.header->writeIndex = 0;
        shared.header->readIndex = 0;
        shared.header->globalWriteNumber = 0;

        for (DWORD i = 0; i < PAGES_COUNT; ++i) {
            shared.header->pageState[i] = PAGE_EMPTY;
        }

        std::cout << "Shared memory initialized." << std::endl;
    }

    ReleaseMutex(shared.initMutex);

    if (!VirtualLock(shared.view, totalSize)) {
        std::cout << "Warning: VirtualLock failed. Error: " << GetLastError() << std::endl;
    }
    else {
        std::cout << "VirtualLock completed successfully." << std::endl;
    }

    std::cout << "Page size: " << pageSize << " bytes" << std::endl;
    std::cout << "Pages count: " << PAGES_COUNT << std::endl;
    std::cout << "Total shared memory size: " << totalSize << " bytes" << std::endl;

    return shared;
}

void CloseSharedObjects(SharedObjects& shared) {
    if (shared.view != NULL) {
        VirtualUnlock(shared.view, shared.header->totalSize);
        UnmapViewOfFile(shared.view);
    }

    if (shared.mapping != NULL) CloseHandle(shared.mapping);
    if (shared.bufferMutex != NULL) CloseHandle(shared.bufferMutex);
    if (shared.emptySemaphore != NULL) CloseHandle(shared.emptySemaphore);
    if (shared.fullSemaphore != NULL) CloseHandle(shared.fullSemaphore);
    if (shared.initMutex != NULL) CloseHandle(shared.initMutex);
}