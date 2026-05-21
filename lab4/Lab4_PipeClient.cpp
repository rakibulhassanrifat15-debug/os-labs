#include <windows.h>
#include <iostream>

const wchar_t* PIPE_NAME = L"\\\\.\\pipe\\Rifat_Lab4_NamedPipe";
const DWORD BUFFER_SIZE = 512;

HANDLE pipeHandle = INVALID_HANDLE_VALUE;
char readBuffer[BUFFER_SIZE];
OVERLAPPED readOverlapped{};
volatile bool readCompleted = false;

VOID CALLBACK ReadCompletionRoutine(
    DWORD errorCode,
    DWORD numberOfBytesTransfered,
    LPOVERLAPPED overlapped
) {
    if (errorCode != 0) {
        std::cout << "ReadFileEx completed with error: " << errorCode << std::endl;
    }
    else {
        std::cout << "Read completed. Bytes: " << numberOfBytesTransfered << std::endl;
        std::cout << "Received message: " << readBuffer << std::endl;
    }

    readCompleted = true;
}

void ConnectToPipe() {
    if (pipeHandle != INVALID_HANDLE_VALUE) {
        std::cout << "Client already connected." << std::endl;
        return;
    }

    pipeHandle = CreateFileW(
        PIPE_NAME,
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        NULL
    );

    if (pipeHandle == INVALID_HANDLE_VALUE) {
        std::cout << "CreateFile failed. Error: " << GetLastError() << std::endl;
        return;
    }

    std::cout << "Connected to named pipe successfully." << std::endl;
}

void ReadMessageAsync() {
    if (pipeHandle == INVALID_HANDLE_VALUE) {
        std::cout << "Connect to pipe first." << std::endl;
        return;
    }

    ZeroMemory(readBuffer, BUFFER_SIZE);
    ZeroMemory(&readOverlapped, sizeof(readOverlapped));
    readCompleted = false;

    BOOL result = ReadFileEx(
        pipeHandle,
        readBuffer,
        BUFFER_SIZE,
        &readOverlapped,
        ReadCompletionRoutine
    );

    if (!result) {
        std::cout << "ReadFileEx failed. Error: " << GetLastError() << std::endl;
        return;
    }

    std::cout << "Asynchronous ReadFileEx started. Waiting..." << std::endl;

    while (!readCompleted) {
        SleepEx(INFINITE, TRUE);
    }
}

void Disconnect() {
    if (pipeHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(pipeHandle);
        pipeHandle = INVALID_HANDLE_VALUE;
        std::cout << "Client disconnected." << std::endl;
    }
}

void ShowMenu() {
    std::cout << "\n===== Pipe Client Menu =====" << std::endl;
    std::cout << "1. Connect to named pipe" << std::endl;
    std::cout << "2. Read message asynchronously" << std::endl;
    std::cout << "3. Disconnect" << std::endl;
    std::cout << "4. Exit" << std::endl;
    std::cout << "Choose: ";
}

int main() {
    int choice = 0;

    do {
        ShowMenu();
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
        case 1:
            ConnectToPipe();
            break;
        case 2:
            ReadMessageAsync();
            break;
        case 3:
            Disconnect();
            break;
        case 4:
            Disconnect();
            break;
        default:
            std::cout << "Wrong choice." << std::endl;
            break;
        }
    } while (choice != 4);

    return 0;
}