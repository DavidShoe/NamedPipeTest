// NPClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <vector>
#include <string>
#include <sstream>

#define BUFSIZE 512

void printError(LPCTSTR msg)
{
    DWORD eNum;
    WCHAR sysMsg[256];
    WCHAR* p;

    eNum = GetLastError();
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, eNum,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        sysMsg, 256, NULL);

    // Trim the end of the line and terminate it with a null
    p = sysMsg;
    while ((*p > 31) || (*p == 9))
        ++p;
    do { *p-- = 0; } while ((p >= sysMsg) &&
        ((*p == '.') || (*p < 33)));

    // Display the message
    _tprintf(TEXT("\n  WARNING: %s failed with error %d (%s)"), msg, eNum, sysMsg);
}

BOOL GetAppContainerProcessTokens(std::vector<HANDLE>& Tokens)
{
    HANDLE hProcessSnap;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;
    HANDLE hProcessToken;
    ULONG ulIsAppContainer;
    DWORD dwReturnLength;

    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        printError(TEXT("CreateToolhelp32Snapshot (of processes)"));
        return(FALSE);
    }

    // Set the size of the structure before using it.
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process,
    // and exit if unsuccessful
    if (!Process32First(hProcessSnap, &pe32))
    {
        printError(TEXT("Process32First")); // show cause of failure
        CloseHandle(hProcessSnap);          // clean the snapshot object
        return(FALSE);
    }

    // Now walk the snapshot of processes, and
    // display information about each process in turn
    do
    {
        //if (_wcsicmp(L"UWPServer.exe", pe32.szExeFile) == 0) {

            hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);
            if (hProcess == NULL)
                continue;
            else
            {
                if (OpenProcessToken(hProcess, TOKEN_QUERY, &hProcessToken))
                {
                    if (GetTokenInformation(hProcessToken, TokenIsAppContainer, &ulIsAppContainer, sizeof(ulIsAppContainer), &dwReturnLength))
                    {
                        if (ulIsAppContainer)
                        {
                            Tokens.push_back(hProcessToken);
                        }
                    }
                }

                if (!ulIsAppContainer)
                {
                    CloseHandle(hProcessToken);
                }

                CloseHandle(hProcess);
            }

            if (ulIsAppContainer)
            {

                _tprintf(TEXT("\n\n====================================================="));
                _tprintf(TEXT("\nAPPCONTAINER PROCESS NAME:  %s"), pe32.szExeFile);
                _tprintf(TEXT("\n-------------------------------------------------------"));

                _tprintf(TEXT("\n  Process ID        = 0x%08X"), pe32.th32ProcessID);
                _tprintf(TEXT("\n  Thread count      = %d"), pe32.cntThreads);
                _tprintf(TEXT("\n  Parent process ID = 0x%08X"), pe32.th32ParentProcessID);
                _tprintf(TEXT("\n  Priority base     = %d\n\n"), pe32.pcPriClassBase);
            }
        //}
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);

    return(TRUE);
}


int main()
{
    HANDLE hPipe;
    WCHAR lpvMessage[] = L"Default message from client.";
    TCHAR  chBuf[BUFSIZE];
    BOOL   fSuccess = FALSE;
    DWORD  cbRead, cbToWrite, cbWritten, dwMode;
    std::wstring strPipeName;
    std::vector<HANDLE> Tokens;
    ULONG ulSessionId;
    WCHAR ObjectPath[1024];
    ULONG ulReturnLength;
    std::wstringstream stringStream;
    BOOL bContinue;

    // Try to open a named pipe; wait for it, if necessary. 

    if (!GetAppContainerProcessTokens(Tokens)) {
        return -1;
    }

    for (HANDLE hToken : Tokens) {

        if (!GetTokenInformation(hToken, TokenSessionId, &ulSessionId, sizeof(ulSessionId), &ulReturnLength))
        {
            printError(TEXT("Get Session Id"));
            continue;
        }

        stringStream.str(L"");
        stringStream << ulSessionId;

        strPipeName = L"\\\\.\\pipe\\Sessions\\";
        strPipeName += stringStream.str();
        strPipeName += L"\\";

        if (!GetAppContainerNamedObjectPath(hToken, NULL, sizeof(ObjectPath)/sizeof(WCHAR), ObjectPath, &ulReturnLength))
        {
            printError(TEXT("Get AppContainer Named Object Path"));
            continue;
        }

        strPipeName += ObjectPath;
        strPipeName += L"\\MyTestSharedMemory";

        bContinue = false;

        while (1)
        {
            _tprintf(TEXT("Attempting to open pipe: %s\n"), strPipeName.c_str());

            hPipe = CreateFile(
                strPipeName.c_str(),   // pipe name 
                GENERIC_READ |  // read and write access 
                GENERIC_WRITE,
                0,              // no sharing 
                NULL,           // default security attributes
                OPEN_EXISTING,  // opens existing pipe 
                0,              // default attributes 
                NULL);          // no template file 

          // Break if the pipe handle is valid. 

            if (hPipe != INVALID_HANDLE_VALUE)
            {
                _tprintf(TEXT("Successfully opened pipe!\n"));
                break;
            }

            // Exit if an error other than ERROR_PIPE_BUSY occurs. 

            if (GetLastError() != ERROR_PIPE_BUSY)
            {
                _tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
                bContinue = true;
                break;
            }

            // All pipe instances are busy, so wait for 20 seconds. 

            if (!WaitNamedPipe(strPipeName.c_str(), 20000))
            {
                printf("Could not open pipe: 20 second wait timed out.");
                bContinue = true;
                break;
            }
        }

        //
        // Did some error occur? Try the next one.
        //

        if (bContinue)
        {
            continue;
        }

        // The pipe connected; change to message-read mode. 

        dwMode = PIPE_READMODE_MESSAGE;
        fSuccess = SetNamedPipeHandleState(
            hPipe,    // pipe handle 
            &dwMode,  // new pipe mode 
            NULL,     // don't set maximum bytes 
            NULL);    // don't set maximum time 
        if (!fSuccess)
        {
            _tprintf(TEXT("SetNamedPipeHandleState failed. GLE=%d\n"), GetLastError());
            break;
        }

        // Send a message to the pipe server. 

        cbToWrite = (lstrlen(lpvMessage) + 1) * sizeof(TCHAR);
        _tprintf(TEXT("Sending %d byte message: \"%s\"\n"), cbToWrite, lpvMessage);

        fSuccess = WriteFile(
            hPipe,                  // pipe handle 
            lpvMessage,             // message 
            cbToWrite,              // message length 
            &cbWritten,             // bytes written 
            NULL);                  // not overlapped 

        if (!fSuccess)
        {
            _tprintf(TEXT("WriteFile to pipe failed. GLE=%d\n"), GetLastError());
            break;
        }

        printf("\nMessage sent to server, receiving reply as follows:\n");

        do
        {
            // Read from the pipe. 

            fSuccess = ReadFile(
                hPipe,    // pipe handle 
                chBuf,    // buffer to receive reply 
                BUFSIZE * sizeof(TCHAR),  // size of buffer 
                &cbRead,  // number of bytes read 
                NULL);    // not overlapped 

            if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
                break;

            _tprintf(TEXT("\"%s\"\n"), chBuf);
        } while (!fSuccess);  // repeat loop if ERROR_MORE_DATA 

        if (!fSuccess)
        {
            _tprintf(TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError());
            break;
        }

        printf("\n<End of message, press ENTER to terminate connection and exit>");
        _getch();

        CloseHandle(hPipe);
    }

    return 0;
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
