#ifndef PLATFORM_WINDOWS_BASE_APPLICATION_H
#define PLATFORM_WINDOWS_BASE_APPLICATION_H

#include "engine/input/keyboard.h"
#include "engine/input/mouse.h"
#include "engine/utilities/engine_interfaces.h"
#include "platform/windows/base/showcase_controller.h"
#include "platform/windows/graphics/d3d_resources.h"

namespace winapp {

class Application : public engine::IDeviceNotify {
public:
	Application();
	~Application();
	bool									Initialize(int width, int height, TCHAR* windowName, HICON icon);
	void									SetTitle(const LPCWSTR& title);
	HICON									GetAppIcon(HINSTANCE hInstance);
	WPARAM									Run();

	// IDeviceNotify functions
	void									OnDeviceLost() override;
	void									OnDeviceRestored() override;
private:
	void									InitializeWindow(int width, int height, TCHAR* windowName, HICON icon);
	static LRESULT WINAPI					WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
	void									OnDisplayChange(int width, int height, int colorDepth);
	void									OnWindowResize(int width, int height, bool isMaximized);
	LRESULT									OnSysCommand(WPARAM wParam, LPARAM lParam);
	LRESULT									OnAltEnter();

	HWND									m_hWnd;
	HINSTANCE								m_hInstance;
	LPCWSTR									m_windowClassName;
	bool									m_isInitialized = false;
	bool									m_isFullScreen = false;
	int										m_windowWidth;
	int										m_windowHeight;
	std::vector<BYTE>						m_rawInputBuffer;
	std::unique_ptr<ShowcaseController>		m_pController;
	std::shared_ptr<engine::D3DResources>	m_pD3DResources;
	std::shared_ptr<engine::Keyboard>		m_pKeyboard;
	std::shared_ptr<engine::Mouse>			m_pMouse;
};
} // namespace winapp

#endif // PLATFORM_WINDOWS_BASE_APPLICATION_H