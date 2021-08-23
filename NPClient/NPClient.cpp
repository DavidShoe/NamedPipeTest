// NPClient.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <TlHelp32.h>

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
BOOL GetProcessList()
{
    HANDLE hProcessSnap;
    HANDLE hProcess;
    PROCESSENTRY32 pe32;
    DWORD dwPriorityClass;

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
        if (pe32.szExeFile[0] == 'A')
        {
            _tprintf(TEXT("\n\n====================================================="));
            _tprintf(TEXT("\nPROCESS NAME:  %s"), pe32.szExeFile);
            _tprintf(TEXT("\n-------------------------------------------------------"));

            // Retrieve the priority class.
            dwPriorityClass = 0;
            hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
            if (hProcess == NULL)
                printError(TEXT("OpenProcess"));
            else
            {
                dwPriorityClass = GetPriorityClass(hProcess);
                if (!dwPriorityClass)
                    printError(TEXT("GetPriorityClass"));
                CloseHandle(hProcess);
            }

            _tprintf(TEXT("\n  Process ID        = 0x%08X"), pe32.th32ProcessID);
            _tprintf(TEXT("\n  Thread count      = %d"), pe32.cntThreads);
            _tprintf(TEXT("\n  Parent process ID = 0x%08X"), pe32.th32ParentProcessID);
            _tprintf(TEXT("\n  Priority base     = %d"), pe32.pcPriClassBase);
            if (dwPriorityClass)
                _tprintf(TEXT("\n  Priority class    = %d"), dwPriorityClass);

            // List the modules and threads associated with this process
            //ListProcessModules(pe32.th32ProcessID);
            //ListProcessThreads(pe32.th32ProcessID);
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    return(TRUE);
}

int main()
{

    GetProcessList();


    //DWORD ReturnLength;
    //ULONG SessionId;

    //if (GetTokenInformation(GetCurrentProcessToken(), TokenSessionId, &SessionId, sizeof(SessionId), &ReturnLength) != FALSE) {
    //    std::cout << "Token Session Id: " << SessionId << std::endl;
    //}



    HANDLE hPipe;
    WCHAR lpvMessage[] = L"Default message from client.";
    TCHAR  chBuf[BUFSIZE];
    BOOL   fSuccess = FALSE;
    DWORD  cbRead, cbToWrite, cbWritten, dwMode;
    //WCHAR lpszPipename[] = TEXT("\\\\.\\pipe\\mynamedpipe");
    
    //WCHAR lpszPipename[] = TEXT("\\\\.\\pipe\\LOCAL\\foo");
    WCHAR lpszPipename[] = TEXT("AppContainerNamedObjects\\S-1-15-2-1852439461-2950083545-1915713802-4291091965-3023447011-4182446531-3405235729\\foo");
    //AppContainerNamedObjects\\S-1-15-2-1852439461-2950083545-1915713802-4291091965-3023447011-4182446531-3405235729
        hPipe = CreateFile(
            lpszPipename,   // pipe name 
            GENERIC_READ |  // read and write access 
            GENERIC_WRITE,
            0,              // no sharing 
            NULL,           // default security attributes
            OPEN_EXISTING,  // opens existing pipe 
            0,              // default attributes 
            NULL);          // no template file 

      // Break if the pipe handle is valid. 

        if (hPipe == INVALID_HANDLE_VALUE)
            return -1;

        // Exit if an error other than ERROR_PIPE_BUSY occurs. 

        if (GetLastError() != ERROR_PIPE_BUSY)
        {
            _tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
            return -1;
        }

        // All pipe instances are busy, so wait for 20 seconds. 

        if (!WaitNamedPipe(lpszPipename, 20000))
        {
            printf("Could not open pipe: 20 second wait timed out.");
            return -1;
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
        return -1;
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
        return -1;
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
        return -1;
    }

    printf("\n<End of message, press ENTER to terminate connection and exit>");
    _getch();

    CloseHandle(hPipe);

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
