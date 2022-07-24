#include "pch.h"

// define which subsystem to use
#ifndef SUB_SYS_CONSOLE
#define SUB_SYS_CONSOLE		// console
#endif

//#ifndef SUB_SYS_WINDOWS
//#define SUB_SYS_WINDOWS		// windows desktop
//#endif


// console testing entry point
#ifdef SUB_SYS_CONSOLE

#include "tests/linked_list_tests.h"
#include "tests/doubly_linked_list_tests.h"

int main() {
	TestLinkedList_Float(std::cout);
	std::cout << std::endl;

	TestLinkedList_Class(std::cout);
	std::cout << std::endl;

	TestTrackedLinkedList(std::cout);
	std::cout << std::endl;

	TestDoublyLinkedList_Int(std::cout);
	std::cout << std::endl;
}

#endif	// SUB_SYS_CONSOLE


// windows desktop entry point
#ifdef SUB_SYS_WINDOWS

#include "platform/windows/base/application.h"
#include "platform/windows/utilities/co_initialize.h"

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd) {
	try {
		engine::CCoInitialize COMInit;	// this NEEDS to be before the declaration of application so that it is destructed after application
		if (SUCCEEDED(COMInit)) {
			winapp::Application application;
			if (application.Initialize(1920, 1080, (TCHAR*)_T("Showcase"), application.GetAppIcon(hInstance))) {
				return (int)application.Run();
			}
		} else {
			throw std::exception("failed to initialize COM");
		}
		return 0;
	}
	catch (const std::exception& ex) {
		size_t size = strlen(ex.what()) + 1;
		wchar_t* buffer = new wchar_t[size];
		size_t outSize;
		mbstowcs_s(&outSize, buffer, size, ex.what(), size - 1);
		MessageBox(nullptr, buffer, L"An error occurred", MB_OK | MB_ICONEXCLAMATION | MB_DEFAULT_DESKTOP_ONLY);
		delete[] buffer;
	}
	catch (...) {
		MessageBox(nullptr, L"unknown exception", L"An unknown error occurred", MB_OK | MB_ICONEXCLAMATION);
	}
	return -1;
}

#endif	// SUB_SYS_WINDOWS