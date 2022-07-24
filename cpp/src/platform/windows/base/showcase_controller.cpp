#include "pch.h"
#include "showcase_controller.h"

namespace winapp {

using namespace engine;

ShowcaseController::ShowcaseController(const std::shared_ptr<D3DResources>& pD3DResources, const std::shared_ptr<Keyboard>& pKeyboard, const std::shared_ptr<Mouse>& pMouse)
	: m_pD3DResources(pD3DResources), m_pKeyboard(pKeyboard), m_pMouse(pMouse) {
	m_pD3DRenderer = std::make_shared<D3DRenderer>(m_pD3DResources);
}

ShowcaseController::~ShowcaseController() {}

void ShowcaseController::Initialize() {
	//std::future<void> resources = std::async(std::launch::async, &D3DRenderer::CreateResourcesAsync, m_pD3DRenderer.get());
	//resources.get();
	m_pD3DRenderer->CreateResourcesAsync();

	// load settings and apply to components
	CameraSettingsComponent* cameraSettings = m_ecsWorld.GetCameraSettingsComponent();
	if (cameraSettings != nullptr) {
		cameraSettings->m_hasFixedPitchYawRoll = true;
		cameraSettings->m_fixedPitch = -engine::constants::enginePiDiv3;
		cameraSettings->m_fixedYaw = 0.0f;
	}

	CreateWindowSizeDependentResources();
	m_pD3DRenderer->CreateResourcesFinalStep(m_ecsWorld.GetCameraComponent()->GetProjectionMatrix());
}

void ShowcaseController::NotifyViewPortChange(int windowWidth, int windowHeight, bool isFullScreen) {
	ApplicationSettingsComponent* applicationSettings = m_ecsWorld.GetApplicationSettingsComponent();
	if (applicationSettings != nullptr) {
		applicationSettings->m_isFullScreen = isFullScreen;
		applicationSettings->m_windowWidth = windowWidth;
		applicationSettings->m_windowHeight = windowHeight;
	}
}

void ShowcaseController::CreateWindowSizeDependentResources() {
	CameraComponent* camera = m_ecsWorld.GetCameraComponent();
	camera->SetProjection(engine::constants::enginePiDiv4, m_pD3DResources->GetRenderTargetWidth() / m_pD3DResources->GetRenderTargetHeight(), engine::constants::defaultNearPlane, engine::constants::defaultFarPlane);
	m_pD3DRenderer->CreateWindowSizeDependentResources(camera->GetProjectionMatrix());
}

void ShowcaseController::Update() {
	m_ecsWorld.ProcessComponentActionQueue();
	m_ecsWorld.RemoveEntitiesInQueue();
	m_ecsWorld.CreateEntitiesInQueue();
	m_ecsWorld.ProcessEntityActionQueue();
	m_ecs.UpdateSystems(m_ecsWorld.GetInputSystemLayers().start, 0.0);	// InputSystem
	if (m_gameState == GameState::Running || m_gameState == GameState::PlayerDead) {
		//m_ecs.UpdateSystems(m_ecsWorld.GetInputSystemLayers().end, 0.0);	// PlayerMovementSystem (essentially another input step)
		m_timer.Tick([&]() {
			double timeLeft = m_timer.GetElapsedSeconds();
			for (LayerId i = m_ecsWorld.GetPreSimulationUpdateSystemLayers().start; i <= m_ecsWorld.GetPreSimulationUpdateSystemLayers().end; ++i) {
				// ActionCommandSystem in here, want to tick action queue, but don't want to raycast for every simulation step
				m_ecs.UpdateSystems(i, timeLeft);
			}
			double simulationTimeLeft = timeLeft;
			double elapsedFrameTime;
			while (simulationTimeLeft > 0.0) {
				elapsedFrameTime = std::min(simulationTimeLeft, engine::constants::physicsFrameTime);
				simulationTimeLeft -= elapsedFrameTime;
				for (LayerId i = m_ecsWorld.GetSimulationSystemLayers().start; i <= m_ecsWorld.GetSimulationSystemLayers().end; ++i) {
					m_ecs.UpdateSystems(i, elapsedFrameTime);
				}
			}
			for (LayerId i = m_ecsWorld.GetPostSimulationUpdateSystemLayers().start; i <= m_ecsWorld.GetPostSimulationUpdateSystemLayers().end; ++i) {
				m_ecs.UpdateSystems(i, timeLeft);
			}
			});
	}
}

void ShowcaseController::Render() {
	m_ecs.UpdateSystems(m_ecsWorld.GetRenderSystemLayers().start, 0.0);	// camera system

	m_pD3DRenderer->SetupRender(m_ecsWorld.GetCameraComponent()->GetViewMatrix());

	for (LayerId i = m_ecsWorld.GetRenderSystemLayers().start + 1; i <= m_ecsWorld.GetRenderSystemLayers().end; ++i) {
		m_ecs.UpdateSystems(i, 0.0);
	}
}

void ShowcaseController::OnDeviceLost() {
	m_pD3DRenderer->ReleaseDeviceDependentResources();
}

void ShowcaseController::OnDeviceRestored() {
	CameraComponent* camera = m_ecsWorld.GetCameraComponent();
	camera->SetProjection(engine::constants::enginePiDiv4, m_pD3DResources->GetRenderTargetWidth() / m_pD3DResources->GetRenderTargetHeight(), engine::constants::defaultNearPlane, engine::constants::defaultFarPlane);

	m_pD3DRenderer->CreateDeviceDependentResources();
	m_pD3DRenderer->CreateWindowSizeDependentResources(camera->GetProjectionMatrix());

	//std::future<void> resources = std::async(std::launch::async, &D3DRenderer::CreateResourcesAsync, m_pD3DRenderer.get());
	//resources.get();
	m_pD3DRenderer->CreateResourcesAsync();
	m_pD3DRenderer->CreateResourcesFinalStep(camera->GetProjectionMatrix());
}

} // namespace winapp