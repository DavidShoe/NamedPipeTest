//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

namespace Uwp
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

	virtual ~MainPage();

	private:
		void OnCreatePipeClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OnSendHelloClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		HANDLE m_hPipe;

		LPCTSTR lpszPipename = TEXT("\\\\.\\pipe\\pipetest");
	};
}
