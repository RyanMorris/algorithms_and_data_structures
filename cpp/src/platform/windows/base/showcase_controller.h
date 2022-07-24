#ifndef PLATFORM_WINDOWS_BASE_SHOWCASE_CONTROLLER_H
#define PLATFORM_WINDOWS_BASE_SHOWCASE_CONTROLLER_H

#include "platform/windows/graphics/d3d_renderer.h"
#include "platform/windows/graphics/d3d_resources.h"
#include "engine/input/keyboard.h"
#include "engine/input/mouse.h"
#include "engine/utilities/step_timer.h"

namespace winapp {

class ShowcaseController {
public:
	ShowcaseController(const std::shared_ptr<engine::D3DResources>& pD3DResources, const std::shared_ptr<engine::Keyboard>& pKeyboard, const std::shared_ptr<engine::Mouse>& pMouse);
	~ShowcaseController();
	ShowcaseController(const ShowcaseController&) = delete;
	ShowcaseController& operator=(const ShowcaseController&) = delete;
	void Initialize();
	void NotifyViewPortChange(int windowWidth, int windowHeight, bool isFullScreen);
	void CreateWindowSizeDependentResources();
	void Update();
	void Render();
	uint32_t GetTimerFps() const { return m_timer.GetFramesPerSecond(); }

	void OnDeviceLost();
	void OnDeviceRestored();
private:
	engine::StepTimer							m_timer;
	std::shared_ptr<engine::D3DResources>		m_pD3DResources;
	std::shared_ptr<engine::D3DRenderer>		m_pD3DRenderer;
	std::shared_ptr<engine::Keyboard>			m_pKeyboard;
	std::shared_ptr<engine::Mouse>				m_pMouse;
};

} // namespace winapp

#endif // PLATFORM_WINDOWS_BASE_SHOWCASE_CONTROLLER_H