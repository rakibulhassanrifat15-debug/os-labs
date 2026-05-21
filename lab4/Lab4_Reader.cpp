#include "Lab4_RW_Common.h"

int main(int argc, char* argv[]) {
    int readerId = (argc > 1) ? atoi(argv[1]) : static_cast<int>(GetCurrentProcessId());
    int operationsCount = (argc > 2) ? atoi(argv[2]) : 5;

    std::string logFileName = "reader_" + std::to_string(readerId) + ".csv";
    std::ofstream log(logFileName, std::ios::out);

    log << "time_ms;process;id;state;page" << std::endl;

    SharedObjects shared = OpenSharedObjects();

    std::cout << "Reader started. ID = " << readerId << std::endl;
    std::cout << "Operations count = " << operationsCount << std::endl;

    for (int operation = 0; operation < operationsCount; ++operation) {
        WriteLog(log, "READER", readerId, "WAIT_READ_BEGIN", -1);

        WaitForSingleObject(shared.fullSemaphore, INFINITE);
        WaitForSingleObject(shared.bufferMutex, INFINITE);

        DWORD page = static_cast<DWORD>(shared.header->readIndex);
        shared.header->pageState[page] = PAGE_READING;

        char* pagePtr = GetPagePointer(shared, page);
        std::string message = pagePtr;

        WriteLog(log, "READER", readerId, "READING", static_cast<int>(page));

        DWORD duration = RandomOperationTime();
        Sleep(duration);

        ZeroMemory(pagePtr, shared.header->pageSize);
        shared.header->pageState[page] = PAGE_EMPTY;
        shared.header->readIndex = (shared.header->readIndex + 1) % PAGES_COUNT;

        WriteLog(log, "READER", readerId, "RELEASE_READ_BEGIN", static_cast<int>(page));

        ReleaseMutex(shared.bufferMutex);
        ReleaseSemaphore(shared.emptySemaphore, 1, NULL);

        WriteLog(log, "READER", readerId, "IDLE", -1);

        std::cout << "Reader " << readerId
            << " read page " << page
            << " during " << duration << " ms" << std::endl;
        std::cout << "Message: " << message << std::endl;

        Sleep(100);
    }

    CloseSharedObjects(shared);

    std::cout << "Reader finished. Log file: " << logFileName << std::endl;
    return 0;
}