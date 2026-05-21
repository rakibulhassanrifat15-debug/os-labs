#include "Lab4_RW_Common.h"

int main(int argc, char* argv[]) {
    int writerId = (argc > 1) ? atoi(argv[1]) : static_cast<int>(GetCurrentProcessId());
    int operationsCount = (argc > 2) ? atoi(argv[2]) : 5;

    std::string logFileName = "writer_" + std::to_string(writerId) + ".csv";
    std::ofstream log(logFileName, std::ios::out);

    log << "time_ms;process;id;state;page" << std::endl;

    SharedObjects shared = OpenSharedObjects();

    std::cout << "Writer started. ID = " << writerId << std::endl;
    std::cout << "Operations count = " << operationsCount << std::endl;

    for (int operation = 0; operation < operationsCount; ++operation) {
        WriteLog(log, "WRITER", writerId, "WAIT_WRITE_BEGIN", -1);

        WaitForSingleObject(shared.emptySemaphore, INFINITE);
        WaitForSingleObject(shared.bufferMutex, INFINITE);

        DWORD page = static_cast<DWORD>(shared.header->writeIndex);
        shared.header->pageState[page] = PAGE_WRITING;

        LONG writeNumber = InterlockedIncrement(&shared.header->globalWriteNumber);

        char* pagePtr = GetPagePointer(shared, page);
        ZeroMemory(pagePtr, shared.header->pageSize);

        sprintf_s(
            pagePtr,
            shared.header->pageSize,
            "Writer %d, write number %ld, time %u",
            writerId,
            writeNumber,
            timeGetTime()
        );

        WriteLog(log, "WRITER", writerId, "WRITING", static_cast<int>(page));

        DWORD duration = RandomOperationTime();
        Sleep(duration);

        shared.header->pageState[page] = PAGE_FULL;
        shared.header->writeIndex = (shared.header->writeIndex + 1) % PAGES_COUNT;

        WriteLog(log, "WRITER", writerId, "RELEASE_WRITE_BEGIN", static_cast<int>(page));

        ReleaseMutex(shared.bufferMutex);
        ReleaseSemaphore(shared.fullSemaphore, 1, NULL);

        WriteLog(log, "WRITER", writerId, "IDLE", -1);

        std::cout << "Writer " << writerId
            << " wrote page " << page
            << " during " << duration << " ms" << std::endl;

        Sleep(100);
    }

    CloseSharedObjects(shared);

    std::cout << "Writer finished. Log file: " << logFileName << std::endl;
    return 0;
}