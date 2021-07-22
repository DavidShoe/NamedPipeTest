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

MainPage::MainPage()
{
	InitializeComponent();
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

	auto r1 = GetCurrentPackageFamilyName(&pfnLength, pfn);
	auto r2 = DeriveAppContainerSidFromAppContainerName(pfn, &mySid);
	auto r3 = DeriveAppContainerSidFromAppContainerName(L"OtherApp_tp1tpcm9wdwpy", &otherSid);

	ConvertSidToStringSid(mySid, &mySidString);
	ConvertSidToStringSid(otherSid, &otherSidString);

	EXPLICIT_ACCESS access[2]{ 0 };
	access[0].grfAccessMode = GRANT_ACCESS;
	access[0].grfAccessPermissions = GENERIC_ALL;
	access[0].grfInheritance = NO_INHERITANCE;
	access[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	access[0].Trustee.TrusteeType = TRUSTEE_IS_USER;
	access[0].Trustee.ptstrName = (LPWCH)mySid;

	access[1].grfAccessMode = GRANT_ACCESS;
	access[1].grfAccessPermissions = GENERIC_ALL;
	access[1].grfInheritance = NO_INHERITANCE;
	access[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
	access[1].Trustee.TrusteeType = TRUSTEE_IS_USER;
	access[1].Trustee.ptstrName = (LPWCH)otherSid;

	PACL acl{ 0 };
	auto r4 = SetEntriesInAcl(2, access, nullptr, &acl);

	SECURITY_DESCRIPTOR descriptor{ 0 };
	auto r5 = InitializeSecurityDescriptor(&descriptor, SECURITY_DESCRIPTOR_REVISION);
	auto r6 = SetSecurityDescriptorDacl(&descriptor, TRUE, acl, FALSE);

	SECURITY_ATTRIBUTES sa{ 0 };
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = FALSE;
	sa.lpSecurityDescriptor = &descriptor;

	auto r7 = GetAppContainerNamedObjectPath(nullptr, nullptr, bnoLength, bno, &dummy);
	auto lpszPipename = ref new Platform::String((L"\\\\.\\pipe\\" + std::wstring(bno) + L"\\MyTestSharedMemory").c_str());

	m_hPipe = CreateNamedPipe(
		lpszPipename->Data(),             // pipe name 
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
