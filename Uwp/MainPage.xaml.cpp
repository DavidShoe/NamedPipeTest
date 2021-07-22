//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace Uwp;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

#define BUFSIZE 512

#include <windows.h> 
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

MainPage::MainPage()
{
	m_hPipe = INVALID_HANDLE_VALUE;
	InitializeComponent();
}

MainPage::~MainPage()
{
	CloseHandle(m_hPipe);
	m_hPipe = INVALID_HANDLE_VALUE;
}

void Uwp::MainPage::OnCreatePipeClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

	//HANDLE CreateNamedPipeA(
	//	LPCSTR                lpName,
	//	DWORD                 dwOpenMode,
	//	DWORD                 dwPipeMode,
	//	DWORD                 nMaxInstances,
	//	DWORD                 nOutBufferSize,
	//	DWORD                 nInBufferSize,
	//	DWORD                 nDefaultTimeOut,
	//	LPSECURITY_ATTRIBUTES lpSecurityAttributes
	//);

	//SECURITY_ATTRIBUTES sa;
	//sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	//sa.

	//m_hPipe = CreateNamedPipe(lpszPipename, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_WAIT | PIPE_ACCEPT_REMOTE_CLIENTS, 1, 256, 256, 0, nullptr);

	//if (m_hPipe == INVALID_HANDLE_VALUE)
	//{
	//	DWORD lastError = GetLastError();
	//	// Something went wrong
	//	if (lastError != ERROR_PIPE_BUSY)
	//	{
	//		lastError = GetLastError();
	//	}
	//}

    HANDLE hPipe;
    WCHAR lpvMessage[] = L"Default message from client.";
    TCHAR  chBuf[BUFSIZE];
    BOOL   fSuccess = FALSE;
    DWORD  cbRead, cbToWrite, cbWritten, dwMode;
    WCHAR lpszPipename[] = TEXT("\\\\.\\pipe\\mynamedpipe");

//
//    SECURITY_DESCRIPTOR descriptor{ 0 };
//    auto r5 = InitializeSecurityDescriptor(&descriptor, SECURITY_DESCRIPTOR_REVISION);
//    auto r6 = SetSecurityDescriptorDacl(&descriptor, TRUE, acl, FALSE);
//    SECURITY_ATTRIBUTES sa{ 0 };
//    sa.nLength = sizeof(sa);
//    sa.bInheritHandle = FALSE;
//    sa.lpSecurityDescriptor = &descriptor;
//
//
//    // Try to open a named pipe; wait for it, if necessary. 
//
//    while (1)
//    {
//
//        hPipe = CreateFile(
//            lpszPipename,   // pipe name 
//            GENERIC_READ |  // read and write access 
//            GENERIC_WRITE,
//            0,              // no sharing 
//            NULL,           // default security attributes
//            OPEN_EXISTING,  // opens existing pipe 
//            0,              // default attributes 
//            NULL);          // no template file 
//
//      // Break if the pipe handle is valid. 
//
//        if (hPipe != INVALID_HANDLE_VALUE)
//            break;
//
//        // Exit if an error other than ERROR_PIPE_BUSY occurs. 
//
//        if (GetLastError() != ERROR_PIPE_BUSY)
//        {
//            _tprintf(TEXT("Could not open pipe. GLE=%d\n"), GetLastError());
//            return ;
//        }
//
//        // All pipe instances are busy, so wait for 20 seconds. 
//
//        if (!WaitNamedPipe(lpszPipename, 20000))
//        {
//            printf("Could not open pipe: 20 second wait timed out.");
//            return ;
//        }
//    }
//
//    // The pipe connected; change to message-read mode. 
//
//    dwMode = PIPE_READMODE_MESSAGE;
//    fSuccess = SetNamedPipeHandleState(
//        hPipe,    // pipe handle 
//        &dwMode,  // new pipe mode 
//        NULL,     // don't set maximum bytes 
//        NULL);    // don't set maximum time 
//    if (!fSuccess)
//    {
//        _tprintf(TEXT("SetNamedPipeHandleState failed. GLE=%d\n"), GetLastError());
//        return ;
//    }
//
//    // Send a message to the pipe server. 
//
//    cbToWrite = (lstrlenW(lpvMessage) + 1) * sizeof(TCHAR);
//    _tprintf(TEXT("Sending %d byte message: \"%s\"\n"), cbToWrite, lpvMessage);
//
//    fSuccess = WriteFile(
//        hPipe,                  // pipe handle 
//        lpvMessage,             // message 
//        cbToWrite,              // message length 
//        &cbWritten,             // bytes written 
//        NULL);                  // not overlapped 
//
//    if (!fSuccess)
//    {
//        _tprintf(TEXT("WriteFile to pipe failed. GLE=%d\n"), GetLastError());
//        return ;
//    }
//
//    printf("\nMessage sent to server, receiving reply as follows:\n");
//
//    do
//    {
//        // Read from the pipe. 
//
//        fSuccess = ReadFile(
//            hPipe,    // pipe handle 
//            chBuf,    // buffer to receive reply 
//            BUFSIZE * sizeof(TCHAR),  // size of buffer 
//            &cbRead,  // number of bytes read 
//            NULL);    // not overlapped 
//
//        if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
//            break;
//
//        _tprintf(TEXT("\"%s\"\n"), chBuf);
//    } while (!fSuccess);  // repeat loop if ERROR_MORE_DATA 
//
//    if (!fSuccess)
//    {
//        _tprintf(TEXT("ReadFile from pipe failed. GLE=%d\n"), GetLastError());
//        return ;
//    }
//
//    printf("\n<End of message, press ENTER to terminate connection and exit>");
////    _getch();
//
//    CloseHandle(hPipe);
//
//    return ;
}




void Uwp::MainPage::OnSendHelloClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}
