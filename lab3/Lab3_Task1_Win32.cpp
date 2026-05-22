#include <windows.h>
#include <iostream>
#include <vector>
#include <iomanip>
#include <cstdint>

using namespace std;

const uint64_t N = 100000000ULL;
const uint64_t STUDENT_TICKET_NUMBER = 431419;
const uint64_t BLOCK_SIZE = 10ULL * STUDENT_TICKET_NUMBER; // 4314190

struct ThreadData {
    int id;
    HANDLE threadHandle;
    HANDLE startEvent;
    HANDLE finishedEvent;

    uint64_t start;
    uint64_t end;

    volatile LONG stop;
};

CRITICAL_SECTION sumLock;

double globalSum = 0.0;
uint64_t nextBlockStart = 0;

double CalculateBlock(uint64_t start, uint64_t end) {
    double sum = 0.0;

    for (uint64_t i = start; i < end; ++i) {
        double x = (static_cast<double>(i) + 0.5) / static_cast<double>(N);
        sum += 4.0 / (1.0 + x * x);
    }

    return sum;
}

DWORD WINAPI WorkerThread(LPVOID param) {
    ThreadData* data = (ThreadData*)param;

    while (true) {
        WaitForSingleObject(data->startEvent, INFINITE);

        if (data->stop) {
            return 0;
        }

        double localSum = CalculateBlock(data->start, data->end);

        EnterCriticalSection(&sumLock);
        globalSum += localSum;
        LeaveCriticalSection(&sumLock);

        SetEvent(data->finishedEvent);
    }
}

bool AssignBlock(ThreadData& data) {
    if (nextBlockStart >= N) {
        return false;
    }

    uint64_t start = nextBlockStart;
    uint64_t end = start + BLOCK_SIZE;

    if (end > N) {
        end = N;
    }

    nextBlockStart = end;

    data.start = start;
    data.end = end;

    return true;
}

double RunCalculation(int threadCount, double& piValue) {
    InitializeCriticalSection(&sumLock);

    globalSum = 0.0;
    nextBlockStart = 0;

    vector<ThreadData> data(threadCount);
    vector<bool> active(threadCount, false);
    vector<bool> suspended(threadCount, true);

    for (int i = 0; i < threadCount; ++i) {
        data[i].id = i;
        data[i].start = 0;
        data[i].end = 0;
        data[i].stop = 0;

        data[i].startEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        data[i].finishedEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        data[i].threadHandle = CreateThread(
            NULL,
            0,
            WorkerThread,
            &data[i],
            CREATE_SUSPENDED,
            NULL
        );

        if (data[i].threadHandle == NULL) {
            cout << "CreateThread failed. Error: " << GetLastError() << endl;
            exit(1);
        }
    }

    LARGE_INTEGER freq, t1, t2;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&t1);

    int activeThreads = 0;

    for (int i = 0; i < threadCount; ++i) {
        if (AssignBlock(data[i])) {
            ResetEvent(data[i].finishedEvent);
            SetEvent(data[i].startEvent);

            ResumeThread(data[i].threadHandle);
            suspended[i] = false;
            active[i] = true;
            activeThreads++;
        }
    }

    while (activeThreads > 0) {
        for (int i = 0; i < threadCount; ++i) {
            if (active[i]) {
                DWORD result = WaitForSingleObject(data[i].finishedEvent, 0);

                if (result == WAIT_OBJECT_0) {
                    SuspendThread(data[i].threadHandle);
                    suspended[i] = true;

                    active[i] = false;
                    activeThreads--;

                    if (AssignBlock(data[i])) {
                        ResetEvent(data[i].finishedEvent);
                        SetEvent(data[i].startEvent);

                        ResumeThread(data[i].threadHandle);
                        suspended[i] = false;
                        active[i] = true;
                        activeThreads++;
                    }
                }
            }
        }

        Sleep(0);
    }

    QueryPerformanceCounter(&t2);

    piValue = globalSum / static_cast<double>(N);

    for (int i = 0; i < threadCount; ++i) {
        data[i].stop = 1;
        SetEvent(data[i].startEvent);

        if (suspended[i]) {
            ResumeThread(data[i].threadHandle);
            suspended[i] = false;
        }
    }

    vector<HANDLE> handles(threadCount);
    for (int i = 0; i < threadCount; ++i) {
        handles[i] = data[i].threadHandle;
    }

    WaitForMultipleObjects(threadCount, handles.data(), TRUE, INFINITE);

    for (int i = 0; i < threadCount; ++i) {
        CloseHandle(data[i].threadHandle);
        CloseHandle(data[i].startEvent);
        CloseHandle(data[i].finishedEvent);
    }

    DeleteCriticalSection(&sumLock);

    return (double)(t2.QuadPart - t1.QuadPart) / freq.QuadPart;
}

int main() {
    cout << "========== Lab 3.1 Win32 Threads ==========\n";
    cout << "Formula: pi = (sum(4 / (1 + x_i^2))) / N\n";
    cout << "x_i = (i + 0.5) / N\n";
    cout << "N = " << N << endl;
    cout << "Student ticket number = " << STUDENT_TICKET_NUMBER << endl;
    cout << "Block size = 10 * student ticket number = " << BLOCK_SIZE << endl;
    cout << "Threads to test: 1, 2, 4, 8, 12, 16\n";
    cout << "===========================================\n\n";

    int threadCounts[] = { 1, 2, 4, 8, 12, 16 };

    cout << left << setw(12) << "Threads"
        << setw(18) << "Time, sec"
        << setw(20) << "Pi value"
        << endl;

    cout << "-------------------------------------------------\n";

    for (int tc : threadCounts) {
        double pi = 0.0;
        double timeSec = RunCalculation(tc, pi);

        cout << left << setw(12) << tc
            << setw(18) << fixed << setprecision(6) << timeSec
            << setw(20) << setprecision(12) << pi
            << endl;
    }

    cout << "\nDone.\n";
    return 0;
}