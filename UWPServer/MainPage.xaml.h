//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
using namespace Windows::System::Threading;


namespace UWPServer
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

	private:
		void OnCreatePipeClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void OnClosePipeClick(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

		void TimerHandler(ThreadPoolTimer^ timer);
		void TimerCanceledHandler(ThreadPoolTimer^ timer);

		bool m_Quit = false;

		ThreadPoolTimer^ m_PeriodicTimer;

		HANDLE m_hPipe = INVALID_HANDLE_VALUE;
	};
}
