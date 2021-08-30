//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

#include <string>
#include <AclAPI.h>
#include <sddl.h>
#include <appmodel.h>
#include <UserEnv.h>
#include <securityappcontainer.h>


using namespace UWPServer;

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
using namespace Windows::System::Threading;

MainPage::MainPage()
{
	InitializeComponent();
	_Status->Text = "Started";


	TimeSpan period;
	period.Duration = 60 * 10000000; // 10,000,000 ticks per second

}

#define BUFSIZE 256

void UWPServer::MainPage::OnCreatePipeClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	PSID mySid{};
	PSID otherSid{};
	UINT32 pfnLength{ MAX_PACKAGE_NAME };
	wchar_t pfn[MAX_PACKAGE_NAME]{ 0 };
	ULONG bnoLength{ MAX_PATH };
	wchar_t bno[MAX_PATH]{ 0 };
	ULONG dummy{ 0 };
	LPTSTR mySidString{ nullptr };
	LPTSTR otherSidString{ nullptr };
	CHAR TokenUserBuffer[TOKEN_USER_MAX_SIZE];
	PTOKEN_USER User = (PTOKEN_USER)TokenUserBuffer;
	DWORD ReturnLength;

	if (GetTokenInformation(GetCurrentProcessToken(), TokenUser, User, TOKEN_USER_MAX_SIZE, &ReturnLength) == false) {
		_Status->Text = "GetTokenInfromation failure";
		return;
	}

	auto r1 = GetCurrentPackageFamilyName(&pfnLength, pfn);
	auto r2 = DeriveAppContainerSidFromAppContainerName(pfn, &mySid);
	auto r3 = DeriveAppContainerSidFromAppContainerName(L"OtherApp_tp1tpcm9wdwpy", &otherSid);

	ConvertSidToStringSid(mySid, &mySidString);
	ConvertSidToStringSid(otherSid, &otherSidString);

	EXPLICIT_ACCESS access[3]{ 0 };
	access[0].grfAccessMode = GRANT_ACCESS;
	access[0].grfAccessPermissions = GENERIC_ALL;
	access[0].grfInheritance = NO_INHERITANCE;
	access[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	access[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
	access[0].Trustee.ptstrName = (LPWCH)User->User.Sid;

	access[1].grfAccessMode = GRANT_ACCESS;
	access[1].grfAccessPermissions = GENERIC_ALL;
	access[1].grfInheritance = NO_INHERITANCE;
	access[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	access[1].Trustee.TrusteeType = TRUSTEE_IS_USER;
	access[1].Trustee.ptstrName = (LPWCH)mySid;

	access[2].grfAccessMode = GRANT_ACCESS;
	access[2].grfAccessPermissions = GENERIC_ALL;
	access[2].grfInheritance = NO_INHERITANCE;
	access[2].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	access[2].Trustee.TrusteeType = TRUSTEE_IS_USER;
	access[2].Trustee.ptstrName = (LPWCH)otherSid;

	PACL acl{ 0 };
	auto r4 = SetEntriesInAcl(3, access, nullptr, &acl);

	SECURITY_DESCRIPTOR descriptor{ 0 };
	auto r5 = InitializeSecurityDescriptor(&descriptor, SECURITY_DESCRIPTOR_REVISION);
	auto r6 = SetSecurityDescriptorDacl(&descriptor, TRUE, acl, FALSE);

	SECURITY_ATTRIBUTES sa{ 0 };
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = FALSE;
	sa.lpSecurityDescriptor = &descriptor;

	//auto r7 = GetAppContainerNamedObjectPath(nullptr, nullptr, bnoLength, bno, &dummy);
	WCHAR lpszPipename[] = L"\\\\.\\pipe\\LOCAL\\MyTestSharedMemory";

	m_hPipe = CreateNamedPipe(
		lpszPipename,             // pipe name 
		PIPE_ACCESS_DUPLEX,       // read/write access 
		PIPE_TYPE_MESSAGE |       // message type pipe 
		PIPE_READMODE_MESSAGE |   // message-read mode 
		PIPE_WAIT,                // blocking mode 
		PIPE_UNLIMITED_INSTANCES, // max. instances  
		BUFSIZE,                  // output buffer size 
		BUFSIZE,                  // input buffer size 
		0,                        // client time-out 
		&sa);                    // default security attribute 

	DWORD lastError = 0;
	if (m_hPipe == INVALID_HANDLE_VALUE)
	{
		lastError = GetLastError();
		_Status->Text = "Pipe creation error";
	}
	else 
	{
		_Status->Text = "Pipe created";

		TimeSpan period;
		period.Duration = 1 * 10000000; // 10,000,000 ticks per second

		ThreadPoolTimer^ PeriodicTimer = ThreadPoolTimer::CreatePeriodicTimer(
			ref new TimerElapsedHandler([this](ThreadPoolTimer^ source)
				{
					//
					// TODO: Work
					//
					if (m_hPipe != INVALID_HANDLE_VALUE)
					{
						TCHAR                   szIn[80] = L"";
						TCHAR					szOut[80] = L"Server response string";
						DWORD                   cbRead = 0;
						DWORD					cbWritten = 0;
						HANDLE                  hEvents[2] = { NULL, NULL };
						DWORD                   dwWait;
						OVERLAPPED              os;
						memset(&os, 0, sizeof(OVERLAPPED));
						os.hEvent = hEvents[1];
						ResetEvent(hEvents[1]);


						bool bRet = ReadFile(
							m_hPipe,          // file to read from
							szIn,           // address of input buffer
							sizeof(szIn),   // number of bytes to read
							&cbRead,        // number of bytes read
							&os);           // overlapped stuff, not needed

						if (!bRet && (GetLastError() == ERROR_IO_PENDING))
						{
							dwWait = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
							if (dwWait != WAIT_OBJECT_0 + 1)
							{
								// not overlapped i/o event - error occurred,
								m_Quit = true;                           // or server stop signaled
							}
						}

						if (cbRead > 0)
						{
							String^ pipein = ref new String(szIn);
							//
							// Update the UI thread by using the UI core dispatcher.
							//
							_Status->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
								ref new Windows::UI::Core::DispatchedHandler([this, &pipein]()
									{
										_Status->Text = pipein;
									}));

							// send a response
							 //
							bRet = WriteFile(
								m_hPipe,          // file to write to
								szOut,          // address of output buffer
								sizeof(szOut),  // number of bytes to write
								&cbWritten,     // number of bytes written
								&os);           // overlapped stuff, not needed


							if (!bRet && (GetLastError() == ERROR_IO_PENDING))
							{
								dwWait = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
								if (dwWait != WAIT_OBJECT_0 + 1)     // not overlapped i/o event - error occurred,
								{
									// not overlapped i/o event - error occurred,
									m_Quit = true;                           // or server stop signaled
								}
							}
						}

					}


				}), period);


	}





	//auto mappingHandle = CreateFileMapping(INVALID_HANDLE_VALUE, &sa, PAGE_READWRITE, 0, 8, L"MyTestSharedMemory");
	//sharedBuffer = (long*)MapViewOfFile(mappingHandle, FILE_MAP_WRITE, 0, 0, 8);
	//*sharedBuffer = 12345678;

	//auto r7 = GetAppContainerNamedObjectPath(nullptr, nullptr, bnoLength, bno, &dummy);

	//txt_myPfn->Text = ref new Platform::String(pfn);
	//txt_mySid->Text = ref new Platform::String(mySidString);
	//txt_otherSid->Text = ref new Platform::String(otherSidString);
	//txt_objectName->Text = ref new Platform::String((std::wstring(bno) + L"\\MyTestSharedMemory").c_str());
	//txt_sharedValue->Text = ref new Platform::String(std::to_wstring(*sharedBuffer).c_str());

}

void UWPServer::MainPage::HandlePipe()
{
	_Status->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
		ref new Windows::UI::Core::DispatchedHandler([this]
			{
				_Status->Text = "Test";
			}));
}

