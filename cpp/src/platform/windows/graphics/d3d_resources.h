#ifndef PLATFORM_WINDOWS_GRAPHICS_D3D_RESOURCES_H
#define PLATFORM_WINDOWS_GRAPHICS_D3D_RESOURCES_H

#include "engine/utilities/engine_interfaces.h"

namespace engine {

class D3DResources {
	//friend class RenderSystem;
public:
	D3DResources(HWND hWnd, int width, int height);
	~D3DResources();
	D3DResources(const D3DResources&) = delete;
	D3DResources& operator=(const D3DResources&) = delete;
	void SetLogicalSize(float width, float height, bool isFullScreen);
	//void SetCurrentOrientation(Windows::Graphics::Display::DisplayOrientations currentOrientation);
	//void SetDpi(float dpi);
	void ValidateDevice();
	void HandleDeviceLost();
	void RegisterDeviceNotify(IDeviceNotify* deviceNotify);
	void Trim();
	void Present();

	float										GetRenderTargetWidth() const { return m_d3dRenderTargetWidth; }
	float										GetRenderTargetHeight() const { return m_d3dRenderTargetHeight; }

	// The size of the render target, in pixels.
	//Windows::Foundation::Size	GetOutputSize() const						{ return m_outputSize; }
	float										GetOutputWidth() const { return m_outputWidth; }
	float										GetOutputHeight() const { return m_outputHeight; }

	// The size of the render target, in dips.
	//Windows::Foundation::Size	GetLogicalSize() const						{ return m_logicalSize; }
	float										GetLogicalWidth() const { return m_logicalWidth; }
	float										GetLogicalHeight() const { return m_logicalHeight; }

	float										GetDpi() const { return m_effectiveDpi; }

	// D3D Accessors.
	ID3D11Device5*								GetD3DDevice() const { return m_pD3dDevice.Get(); }
	ID3D11DeviceContext4*						GetD3DDeviceContext() const { return m_pD3dContext.Get(); }
	IDXGISwapChain4*							GetSwapChain() const { return m_pSwapChain.Get(); }
	D3D_FEATURE_LEVEL							GetDeviceFeatureLevel() const { return m_d3dFeatureLevel; }
	ID3D11RenderTargetView1*					GetBackBufferRenderTargetView() const { return m_pD3dRenderTargetView.Get(); }
	ID3D11DepthStencilView*						GetDepthStencilView() const { return m_pD3dDepthStencilView.Get(); }
	ID3D11RasterizerState2*						GetRasterizerState() const { return m_pD3dRasterizerState.Get(); }
	D3D11_VIEWPORT								GetScreenViewport() const { return m_screenViewport; }
	DirectX::XMFLOAT4X4							GetOrientationTransform3D() const { return m_orientationTransform3D; }

	// D2D Accessors.
	ID2D1Factory3*								GetD2DFactory() const { return m_pD2dFactory.Get(); }
	ID2D1Device2*								GetD2DDevice() const { return m_pD2dDevice.Get(); }
	ID2D1DeviceContext2*						GetD2DDeviceContext() const { return m_pD2dContext.Get(); }
	ID2D1Bitmap1*								GetD2DTargetBitmap() const { return m_pD2dTargetBitmap.Get(); }
	IDWriteFactory3*							GetDWriteFactory() const { return m_pDwriteFactory.Get(); }
	IWICImagingFactory2*						GetWicImagingFactory() const { return m_pWicFactory.Get(); }
	D2D1::Matrix3x2F							GetOrientationTransform2D() const { return m_orientationTransform2D; }

private:
	void CreateDeviceIndependentResources();
	void CreateDeviceResources();
	void CreateWindowSizeDependentResources();
	void UpdateRenderTargetSize();
	DXGI_MODE_ROTATION ComputeDisplayRotation();

	float			m_d3dRenderTargetWidth;
	float			m_d3dRenderTargetHeight;
	float			m_outputWidth;
	float			m_outputHeight;
	float			m_logicalWidth;
	float			m_logicalHeight;
	float			m_dpi;
	float			m_effectiveDpi;	// This is the DPI that will be reported back to the app. It takes into account whether the app supports high resolution screens or not.
	UINT			m_syncInterval;	// 0u for none, 1u for screen refrash rate? (block until VSync, putting the application to sleep until the next VSync) (1-4 is range for IDXGISwapChain::Present)

	// Direct3D objects.
	Microsoft::WRL::ComPtr<ID3D11Device5>			m_pD3dDevice;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext4>	m_pD3dContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain4>			m_pSwapChain;

	// Direct3D rendering objects. Required for 3D.
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView1>	m_pD3dRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	m_pD3dDepthStencilView;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState2>	m_pD3dRasterizerState;
	D3D11_VIEWPORT									m_screenViewport;

	// Direct2D drawing components.
	Microsoft::WRL::ComPtr<ID2D1Factory3>			m_pD2dFactory;
	Microsoft::WRL::ComPtr<ID2D1Device2>			m_pD2dDevice;
	Microsoft::WRL::ComPtr<ID2D1DeviceContext2>		m_pD2dContext;
	Microsoft::WRL::ComPtr<ID2D1Bitmap1>			m_pD2dTargetBitmap;

	// DirectWrite drawing components.
	Microsoft::WRL::ComPtr<IDWriteFactory3>			m_pDwriteFactory;
	Microsoft::WRL::ComPtr<IWICImagingFactory2>		m_pWicFactory;

	// Cached reference to the Window.
	HWND											m_hWnd;

	// Cached device properties.
	D3D_FEATURE_LEVEL									m_d3dFeatureLevel;

	// Transforms used for display orientation.
	D2D1::Matrix3x2F								m_orientationTransform2D;
	DirectX::XMFLOAT4X4								m_orientationTransform3D;

	// The IDeviceNotify can be held directly as it owns the DeviceResources.
	IDeviceNotify* m_pDeviceNotify;
};

} // namespace engine

#endif // PLATFORM_WINDOWS_GRAPHICS_D3D_RESOURCES_H