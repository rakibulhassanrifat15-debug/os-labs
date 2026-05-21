#include <windows.h>
#include <iostream>
#include <string>

const wchar_t* PIPE_NAME = L"\\\\.\\pipe\\Rifat_Lab4_NamedPipe";
const DWORD BUFFER_SIZE = 512;

HANDLE pipeHandle = INVALID_HANDLE_VALUE;
HANDLE writeEvent = NULL;

void CreatePipeAndEvent() {
    if (pipeHandle != INVALID_HANDLE_VALUE) {
        std::cout << "Pipe already created." << std::endl;
        return;
    }

    pipeHandle = CreateNamedPipeW(
        PIPE_NAME,
        PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        1,
        BUFFER_SIZE,
        BUFFER_SIZE,
        0,
        NULL
    );

    if (pipeHandle == INVALID_HANDLE_VALUE) {
        std::cout << "CreateNamedPipe failed. Error: " << GetLastError() << std::endl;
        return;
    }

    writeEvent = CreateEventW(NULL, TRUE, FALSE, NULL);

    if (writeEvent == NULL) {
        std::cout << "CreateEvent failed. Error: " << GetLastError() << std::endl;
        CloseHandle(pipeHandle);
        pipeHandle = INVALID_HANDLE_VALUE;
        return;
    }

    std::cout << "Named pipe and event created successfully." << std::endl;
}

void ConnectClient() {
    if (pipeHandle == INVALID_HANDLE_VALUE) {
        std::cout << "Create pipe first." << std::endl;
        return;
    }

    std::cout << "Waiting for client..." << std::endl;

    BOOL connected = ConnectNamedPipe(pipeHandle, NULL)
        ? TRUE
        : (GetLastError() == ERROR_PIPE_CONNECTED);

    if (connected) {
        std::cout << "Client connected successfully." << std::endl;
    }
    else {
        std::cout << "ConnectNamedPipe failed. Error: " << GetLastError() << std::endl;
    }
}

void SendMessageAsync() {
    if (pipeHandle == INVALID_HANDLE_VALUE || writeEvent == NULL) {
        std::cout << "Pipe is not ready." << std::endl;
        return;
    }

    std::cout << "Enter message: ";
    std::string message;
    std::getline(std::cin, message);

    if (message.empty()) {
        message = "empty message";
    }

    OVERLAPPED overlapped{};
    overlapped.hEvent = writeEvent;

    ResetEvent(writeEvent);

    DWORD bytesWritten = 0;

    BOOL result = WriteFile(
        pipeHandle,
        message.c_str(),
        static_cast<DWORD>(message.size() + 1),
        &bytesWritten,
        &overlapped
    );

    if (!result) {
        DWORD error = GetLastError();

        if (error == ERROR_IO_PENDING) {
            std::cout << "Asynchronous WriteFile started. Waiting..." << std::endl;

            WaitForSingleObject(writeEvent, INFINITE);

            if (GetOverlappedResult(pipeHandle, &overlapped, &bytesWritten, FALSE)) {
                std::cout << "Message sent asynchronously. Bytes: " << bytesWritten << std::endl;
            }
            else {
                std::cout << "GetOverlappedResult failed. Error: " << GetLastError() << std::endl;
            }
        }
        else {
            std::cout << "WriteFile failed. Error: " << error << std::endl;
        }
    }
    else {
        std::cout << "Message sent immediately. Bytes: " << bytesWritten << std::endl;
    }
}

void DisconnectClient() {
    if (pipeHandle == INVALID_HANDLE_VALUE) {
        std::cout << "Pipe is not created." << std::endl;
        return;
    }

    if (DisconnectNamedPipe(pipeHandle)) {
        std::cout << "Client disconnected." << std::endl;
    }
    else {
        std::cout << "DisconnectNamedPipe failed. Error: " << GetLastError() << std::endl;
    }
}

void Cleanup() {
    if (pipeHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(pipeHandle);
        pipeHandle = INVALID_HANDLE_VALUE;
    }

    if (writeEvent != NULL) {
        CloseHandle(writeEvent);
        writeEvent = NULL;
    }
}

void ShowMenu() {
    std::cout << "\n===== Pipe Server Menu =====" << std::endl;
    std::cout << "1. Create named pipe and event" << std::endl;
    std::cout << "2. Connect client" << std::endl;
    std::cout << "3. Send message asynchronously" << std::endl;
    std::cout << "4. Disconnect client" << std::endl;
    std::cout << "5. Exit" << std::endl;
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
            CreatePipeAndEvent();
            break;
        case 2:
            ConnectClient();
            break;
        case 3:
            SendMessageAsync();
            break;
        case 4:
            DisconnectClient();
            break;
        case 5:
            Cleanup();
            break;
        default:
            std::cout << "Wrong choice." << std::endl;
            break;
        }
    } while (choice != 5);

    return 0;
}