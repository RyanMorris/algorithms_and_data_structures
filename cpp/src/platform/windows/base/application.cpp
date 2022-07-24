#include "pch.h"
#include "application.h"
//#include "../resource.h"

namespace winapp {

Application* g_pApplication = nullptr;

Application::Application()
	: m_hWnd(nullptr), m_hInstance(nullptr), m_windowClassName(nullptr), m_windowWidth(0), m_windowHeight(0) {
	g_pApplication = this;
}

Application::~Application() {
	if (m_pD3DResources)
		m_pD3DResources->RegisterDeviceNotify(nullptr);		// deregister device notification

	ShowCursor(true);
	ClipCursor(nullptr);

	// fix the display settings if leaving full screen mode.
	if (m_isFullScreen) {
		ChangeDisplaySettings(NULL, 0);
	}

	DestroyWindow(m_hWnd);
	UnregisterClass(m_windowClassName, m_hInstance);
	m_hWnd = nullptr;
}

bool Application::Initialize(int width, int height, TCHAR* windowName, HICON icon) {
	InitializeWindow(width, height, windowName, icon);

	m_pD3DResources = std::make_shared<engine::D3DResources>(m_hWnd, width, height);
	m_pD3DResources->RegisterDeviceNotify(this);
	m_pKeyboard = std::make_shared<engine::Keyboard>();
	m_pMouse = std::make_shared<engine::Mouse>(m_hWnd);
	m_pController = std::make_unique<ShowcaseController>(m_pD3DResources, m_pKeyboard, m_pMouse);
	m_pController->Initialize();
	m_pController->NotifyViewPortChange(m_windowWidth, m_windowHeight, m_isFullScreen);

	m_isInitialized = true;

	// re-adjust to full screen now that it's initialized
	if (m_isFullScreen) {
		// get screen size
		HMONITOR Hmon = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFOEX info;
		info.cbSize = sizeof(info);
		if (!GetMonitorInfoW(Hmon, &info)) {
			throw std::exception("Application::InitializeWindow : GetMonitorInfoW full-screen error");
		}
		m_windowWidth = (info.rcMonitor.right - info.rcMonitor.left);
		m_windowHeight = (info.rcMonitor.bottom - info.rcMonitor.top);
		OnWindowResize(m_windowWidth, m_windowHeight, true);
	}

	return true;
}

void Application::InitializeWindow(int width, int height, TCHAR* windowName, HICON icon) {
	m_windowWidth = width;
	m_windowHeight = height;
	m_hInstance = GetModuleHandle(nullptr);
	m_windowClassName = windowName;

	WNDCLASSEX windowsClass = {};
	windowsClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	windowsClass.lpfnWndProc = WindowProc;
	windowsClass.cbClsExtra = 0;
	windowsClass.cbWndExtra = 0;
	windowsClass.hInstance = m_hInstance;
	windowsClass.hIcon = icon;
	windowsClass.hIconSm = icon;
	windowsClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowsClass.hbrBackground = nullptr;
	windowsClass.lpszMenuName = nullptr;
	windowsClass.lpszClassName = m_windowClassName;
	windowsClass.cbSize = sizeof(WNDCLASSEX);

	if (!RegisterClassEx(&windowsClass)) {
		throw std::runtime_error("Application::InitializeWindow : RegisterClassEx() failed");
	}

	// get the resolution of the client desktop
	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	int posX, posY;

	if (m_isFullScreen) {
		posX = posY = 0;
	} else {
		// place the window in the middle of the screen.
		posX = (screenWidth - m_windowWidth) / 2;
		posY = (screenHeight - m_windowHeight) / 2;
	}

	RECT winRect = { 0, 0, m_windowWidth, m_windowHeight };
	AdjustWindowRect(&winRect, WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU, FALSE);
	// create window & get handle
	m_hWnd = CreateWindowEx(
		WS_EX_APPWINDOW,
		m_windowClassName,
		m_windowClassName,
		WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU,
		posX,
		posY,
		winRect.right - winRect.left,
		winRect.bottom - winRect.top,
		nullptr,
		nullptr,
		m_hInstance,
		this
	);

	if (m_hWnd == nullptr) {
		throw std::exception("Application::InitializeWindow : error creating window");
	}

	// register mouse raw input device
	RAWINPUTDEVICE rawMouse;
	rawMouse.usUsagePage = 0x01; // mouse page
	rawMouse.usUsage = 0x02; // mouse usage
	rawMouse.dwFlags = 0;
	rawMouse.hwndTarget = nullptr;
	if (RegisterRawInputDevices(&rawMouse, 1, sizeof(rawMouse)) == FALSE) {
		throw std::exception("Application::InitializeWindow : failed to register raw input device");
	}

	// bring the window up on the screen and set it as main focus.
	ShowWindow(m_hWnd, SW_SHOW);
	SetForegroundWindow(m_hWnd);
	SetFocus(m_hWnd);
}

void Application::SetTitle(const LPCWSTR& title) {
	if (SetWindowText(m_hWnd, title) == 0) {
		throw std::exception("Application::SetTitle : unable to set window text");
	}
}

HICON Application::GetAppIcon(HINSTANCE hInstance) {
	//return LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	return nullptr;
}

void Application::OnDeviceLost() {
	m_pController->OnDeviceLost();
}

void Application::OnDeviceRestored() {
	m_pController->OnDeviceRestored();
}

WPARAM Application::Run() {
	MSG msg;
	while (true) {
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				return msg.wParam;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		m_pController->Update();
		m_pController->Render();
		m_pD3DResources->Present();
	}
}

LRESULT WINAPI Application::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
	switch (msg) {
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}
	case WM_CLOSE: {
		// WM_CLOSE is treated the same as WM_DESTROY since there is only one window running and it will call the destructor
		PostQuitMessage(0);
		return 0;
	}
	case WM_KILLFOCUS: {
		// clear keystate when window loses focus to prevent input getting stuck
		if (g_pApplication->m_pMouse != nullptr && g_pApplication->m_pKeyboard != nullptr) {
			g_pApplication->m_pKeyboard->ClearKeyStates();
			g_pApplication->m_pMouse->FreeCursor();
			g_pApplication->m_pMouse->ShowCursor();
		}
		break;
	}
	case WM_ACTIVATE: {
		// confine/free cursor on window to foreground/background if cursor disabled
		if (g_pApplication->m_pMouse != nullptr && g_pApplication->m_pKeyboard != nullptr) {
			if (!g_pApplication->m_pMouse->IsCursorEnabled()) {
				if (wParam & WA_ACTIVE) {
					g_pApplication->m_pMouse->ConfineCursor();
					g_pApplication->m_pMouse->HideCursor();
				} else {
					g_pApplication->m_pMouse->FreeCursor();
					g_pApplication->m_pMouse->ShowCursor();
				}
			}
		}
		break;
	}
	case WM_DISPLAYCHANGE: {
		int colorDepth = (int)wParam;
		int width = (int)(short)LOWORD(lParam);
		int height = (int)(short)HIWORD(lParam);
		g_pApplication->OnDisplayChange(width, height, colorDepth);
		break;
	}
	case WM_SIZE: {
		bool isMaximized = ((int)wParam == SIZE_MAXIMIZED);
		int width = (int)(short)LOWORD(lParam);
		int height = (int)(short)HIWORD(lParam);
		g_pApplication->OnWindowResize(width, height, isMaximized);
		break;
	}
	case WM_MENUCHAR: {
		// alt + <key> press
		// a menu is active and the user presses a key that does not correspond to any mnemonic or accelerator key, so just ignore and don't beep
		return MAKELRESULT(0, MNC_CLOSE);
	}
	case WM_SYSCOMMAND: {
		LRESULT result = g_pApplication->OnSysCommand(wParam, lParam);
		/*if (result) {
			*pbNoFurtherProcessing = true;
		}*/
		break;
	}
	case WM_KEYDOWN: {
		if (!(lParam & 0x40000000) || g_pApplication->m_pKeyboard->IsAutoRepeatKeysEnabled()) {
			g_pApplication->m_pKeyboard->OnKeyDown(static_cast<unsigned char>(wParam));
		}
		break;
	}
	case WM_SYSKEYDOWN: {
		// handle the alt and f10 keys
		if (wParam == VK_MENU || wParam == VK_F10) {
			// bit 30, the previous key state. The value is 1 if the key is down before the message is sent, or it is zero if the key is up.
			if (!(lParam & 0x40000000) || g_pApplication->m_pKeyboard->IsAutoRepeatKeysEnabled()) {
				g_pApplication->m_pKeyboard->OnKeyDown(static_cast<unsigned char>(wParam));
			}
			return true; // default eat them
		} else if (wParam == VK_RETURN) {
			return g_pApplication->OnAltEnter();
		} else {
			// if alt + some key, push the other key into m_pKeyboard
			if (!(lParam & 0x40000000) || g_pApplication->m_pKeyboard->IsAutoRepeatKeysEnabled()) {
				g_pApplication->m_pKeyboard->OnKeyDown(static_cast<unsigned char>(wParam));
			}
		}
		break;
	}
	case WM_KEYUP: {
		g_pApplication->m_pKeyboard->OnKeyUp(static_cast<unsigned char>(wParam));
		break;
	}
	case WM_SYSKEYUP: {
		if (wParam == VK_MENU || wParam == VK_F10) {
			g_pApplication->m_pKeyboard->OnKeyUp(static_cast<unsigned char>(wParam));
		}
		break;
	}
	case WM_CHAR: {
		if (!(lParam & 0x40000000) || g_pApplication->m_pKeyboard->IsAutoRepeatCharsEnabled()) {
			g_pApplication->m_pKeyboard->OnChar(static_cast<unsigned char>(wParam));
		}
		break;
	}
	case WM_SYSCHAR: {
		// handle the alt and f10 keys
		// also enter, don't want to handle alt enter here?
		if (wParam == VK_MENU || wParam == VK_F10 || wParam == VK_RETURN) {
			// bit 30, the previous key state. The value is 1 if the key is down before the message is sent, or it is zero if the key is up.
			if (!(lParam & 0x40000000) || g_pApplication->m_pKeyboard->IsAutoRepeatCharsEnabled()) {
				g_pApplication->m_pKeyboard->OnChar(static_cast<unsigned char>(wParam));
			}
			return true; // default eat them
		}
		/*else if (wParam == VK_RETURN) {
			return g_pApplication->OnAltEnter();
		}*/
		break;
	}
	case WM_LBUTTONDOWN: {
		g_pApplication->m_pMouse->OnLeftDown(LOWORD(lParam), HIWORD(lParam));
		break;
	}
	case WM_LBUTTONUP: {
		g_pApplication->m_pMouse->OnLeftUp(LOWORD(lParam), HIWORD(lParam));
		break;
	}
	case WM_RBUTTONDOWN: {
		g_pApplication->m_pMouse->OnRightDown(LOWORD(lParam), HIWORD(lParam));
		break;
	}
	case WM_RBUTTONUP: {
		g_pApplication->m_pMouse->OnRightUp(LOWORD(lParam), HIWORD(lParam));
		break;
	}
	case WM_MBUTTONDOWN: {
		g_pApplication->m_pMouse->OnMiddleDown(LOWORD(lParam), HIWORD(lParam));
		break;
	}
	case WM_MBUTTONUP: {
		g_pApplication->m_pMouse->OnMiddleUp(LOWORD(lParam), HIWORD(lParam));
		break;
	}
	case WM_MOUSEWHEEL: {
		if (GET_WHEEL_DELTA_WPARAM(wParam) < 0) {
			g_pApplication->m_pMouse->OnWheelDown(LOWORD(lParam), HIWORD(lParam));
		} else if (GET_WHEEL_DELTA_WPARAM(wParam) > 0) {
			g_pApplication->m_pMouse->OnWheelUp(LOWORD(lParam), HIWORD(lParam));
		}
		break;
	}
	case WM_MOUSEMOVE: {
		g_pApplication->m_pMouse->OnMouseMove(LOWORD(lParam), HIWORD(lParam));
		break;
	}
	case WM_INPUT: {
		if (!g_pApplication->m_pMouse->IsRawEnabled()) {
			break;
		}
		UINT dataSize = 0;
		GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, NULL, &dataSize, sizeof(RAWINPUTHEADER));
		if (dataSize > 0) {
			g_pApplication->m_rawInputBuffer.resize(dataSize);
			if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, g_pApplication->m_rawInputBuffer.data(), &dataSize, sizeof(RAWINPUTHEADER)) == dataSize) {
				RAWINPUT* raw = reinterpret_cast<RAWINPUT*>(g_pApplication->m_rawInputBuffer.data());
				if (raw->header.dwType == RIM_TYPEMOUSE) {
					g_pApplication->m_pMouse->OnMouseMoveRaw(raw->data.mouse.lLastX, raw->data.mouse.lLastY);
				}
			}
		}
		// make sure to call DefWindowProc for WM_INPUT messages ?
		break;
	}
	default: break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void Application::OnDisplayChange(int width, int height, int colorDepth) {
	/*desktopRect_.left = 0;
	desktopRect_.top = 0;
	desktopRect_.right = width;
	desktopRect_.bottom = height;
	colorDepth_ = colorDepth;*/
}

void Application::OnWindowResize(int width, int height, bool isMaximized) {
	m_windowWidth = width;
	m_windowHeight = height;
	if (m_isInitialized) {
		// isMaximized == SIZE_MAXIMIZED == the window has been maximized, which is different than full screen
		m_pD3DResources->SetLogicalSize((float)width, (float)height, m_isFullScreen);
	}
	if (m_pController) {
		m_pController->CreateWindowSizeDependentResources();
		m_pController->NotifyViewPortChange(m_windowWidth, m_windowHeight, m_isFullScreen);
	}
}

LRESULT Application::OnSysCommand(WPARAM wParam, LPARAM lParam) {
	switch (wParam) {
	case SC_CLOSE: {
		return 0;
	}
	case SC_MAXIMIZE: {
		if (!m_isFullScreen) {
			// TODO: turn maximize into FULLSCREEN toggle?
			//m_pD3DRenderer->ResizeScreen(true);
			//m_isFullScreen = true;
		}
		return 0;
	}
	default:
		// return non-zero if we didn't process the SYSCOMMAND message
		return DefWindowProc(m_hWnd, WM_SYSCOMMAND, wParam, lParam);
	}
}

LRESULT Application::OnAltEnter() {
	m_isFullScreen = !m_isFullScreen;
	if (m_isFullScreen) {
		// get screen size
		HMONITOR Hmon = MonitorFromWindow(m_hWnd, MONITOR_DEFAULTTOPRIMARY);
		MONITORINFOEX info;
		info.cbSize = sizeof(info);
		if (!GetMonitorInfoW(Hmon, &info)) {
			return EXIT_FAILURE;
		}
		m_pD3DResources->SetLogicalSize((float)(info.rcMonitor.right - info.rcMonitor.left), (float)(info.rcMonitor.bottom - info.rcMonitor.top), m_isFullScreen);
	} else {
		m_pD3DResources->SetLogicalSize((float)m_windowWidth, (float)m_windowHeight, m_isFullScreen);
	}
	if (m_pController) {
		m_pController->CreateWindowSizeDependentResources();
	}
	return 0;
}
} // namespace winapp