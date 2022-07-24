#include "pch.h"
#include "d3d_resources.h"
#include "platform/windows/resources/resource_loader.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

namespace engine {

namespace DisplayMetrics {
// High resolution displays can require a lot of GPU and battery power to render.
// High resolution phones, for example, may suffer from poor battery life if
// games attempt to render at 60 frames per second at full fidelity.
// The decision to render at full fidelity across all platforms and form factors
// should be deliberate.
static const bool SupportHighResolutions = false;

// The default thresholds that define a "high resolution" display. If the thresholds
// are exceeded and SupportHighResolutions is false, the dimensions will be scaled
// by 50%.
static const float DpiThreshold = 192.0f;		// 200% of standard desktop display.
static const float WidthThreshold = 1920.0f;	// 1080p width.
static const float HeightThreshold = 1080.0f;	// 1080p height.
};

// Constants used to calculate screen rotations
namespace ScreenRotation {
// 0-degree Z-rotation
static const DirectX::XMFLOAT4X4 Rotation0(
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
);

// 90-degree Z-rotation
static const DirectX::XMFLOAT4X4 Rotation90(
	0.0f, 1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
);

// 180-degree Z-rotation
static const DirectX::XMFLOAT4X4 Rotation180(
	-1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, -1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
);

// 270-degree Z-rotation
static const DirectX::XMFLOAT4X4 Rotation270(
	0.0f, -1.0f, 0.0f, 0.0f,
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
);
};

#if defined(_DEBUG)
// Check for SDK Layer support.
inline bool SdkLayersAvailable() {
	HRESULT hr = D3D11CreateDevice(
		nullptr,
		D3D_DRIVER_TYPE_NULL,       // There is no need to create a real hardware device.
		0,
		D3D11_CREATE_DEVICE_DEBUG,  // Check for the SDK layers.
		nullptr,                    // Any feature level will do.
		0,
		D3D11_SDK_VERSION,          // Always set this to D3D11_SDK_VERSION for Microsoft Store apps.
		nullptr,                    // No need to keep the D3D device reference.
		nullptr,                    // No need to know the feature level.
		nullptr                     // No need to keep the D3D device context reference.
	);
	return SUCCEEDED(hr);
}
#endif

inline float ConvertDipsToPixels(float dips, float dpi) {
	static const float dipsPerInch = 96.0f;
	return floorf(dips * dpi / dipsPerInch + 0.5f); // Round to nearest integer.
}

D3DResources::D3DResources(HWND hWnd, int width, int height)
	: m_d3dRenderTargetWidth((float)width),
	m_d3dRenderTargetHeight((float)height),
	m_outputWidth((float)width),
	m_outputHeight((float)height),
	m_logicalWidth((float)width),
	m_logicalHeight((float)height),
	m_dpi(-1.0f),
	m_effectiveDpi(-1.0f),
	m_syncInterval(1u),
	m_screenViewport(),
	m_d3dFeatureLevel(D3D_FEATURE_LEVEL_11_0),
	//m_d3dRenderTargetSize(),
	//m_outputSize(),
	//m_logicalSize(),
	//m_nativeOrientation(DisplayOrientations::None),
	//m_currentOrientation(DisplayOrientations::None),
	m_pDeviceNotify(nullptr) {
	m_hWnd = hWnd;
	CreateDeviceIndependentResources();
	CreateDeviceResources();
	CreateWindowSizeDependentResources();

	//CreateDeviceDependentResources();
}

D3DResources::~D3DResources() {
	if (m_pSwapChain)
		m_pSwapChain->SetFullscreenState(FALSE, nullptr);
	/*if (m_pD3dContext) {
		m_pD3dContext->ClearState();
		m_pD3dContext->Flush();
	}*/
}

// Configures resources that don't depend on the Direct3D device.
void D3DResources::CreateDeviceIndependentResources() {
	HRESULT hr;
	// Initialize Direct2D resources.
	D2D1_FACTORY_OPTIONS options;
	ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(_DEBUG)
	// If the project is in a debug build, enable Direct2D debugging via SDK Layers.
	options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

	// Initialize the Direct2D Factory.
	hr = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		__uuidof(ID2D1Factory3),
		&options,
		&m_pD2dFactory
	);
	if (FAILED(hr)) {
		throw std::exception("D3DResources::CreateDeviceIndependentResources : D2D1CreateFactory failed");
	}

	// Initialize the DirectWrite Factory.
	hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory3),
		&m_pDwriteFactory
	);
	if (FAILED(hr)) {
		throw std::exception("D3DResources::CreateDeviceIndependentResources : DWriteCreateFactory failed");
	}

	// Initialize the Windows Imaging Component (WIC) Factory.
	hr = CoCreateInstance(
		CLSID_WICImagingFactory2,
		nullptr,
		CLSCTX_INPROC_SERVER,
		IID_PPV_ARGS(&m_pWicFactory)
	);
	if (FAILED(hr)) {
		throw std::exception("D3DResources::CreateDeviceIndependentResources : CoCreateInstance failed");
	}
}

// Configures the Direct3D device, and stores handles to it and the device context.
void D3DResources::CreateDeviceResources() {
	// This flag adds support for surfaces with a different color channel ordering
	// than the API default. It is required for compatibility with Direct2D.
	UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(_DEBUG)
	if (SdkLayersAvailable()) {
		// If the project is in a debug build, enable debugging via SDK Layers with this flag.
		creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
	}
#endif

	// This array defines the set of DirectX hardware feature levels this app will support.
	// Note the ordering should be preserved.
	// Don't forget to declare your application's minimum required feature level in its
	// description.  All applications are assumed to support 9.1 unless otherwise stated.
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	// Create the Direct3D 11 API device object and a corresponding context.
	Microsoft::WRL::ComPtr<ID3D11Device> device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> context;

	HRESULT hr = D3D11CreateDevice(
		nullptr,					// Specify nullptr to use the default adapter.
		D3D_DRIVER_TYPE_HARDWARE,	// Create a device using the hardware graphics driver.
		0,							// Should be 0 unless the driver is D3D_DRIVER_TYPE_SOFTWARE.
		creationFlags,				// Set debug and Direct2D compatibility flags.
		featureLevels,				// List of feature levels this app can support.
		ARRAYSIZE(featureLevels),	// Size of the list above.
		D3D11_SDK_VERSION,			// Always set this to D3D11_SDK_VERSION for Microsoft Store apps.
		&device,					// Returns the Direct3D device created.
		&m_d3dFeatureLevel,			// Returns feature level of device created.
		&context					// Returns the device immediate context.
	);

	if (FAILED(hr)) {
		// If the initialization fails, fall back to the WARP device.
		// For more information on WARP, see: 
		// https://go.microsoft.com/fwlink/?LinkId=286690
		hr = D3D11CreateDevice(
			nullptr,
			D3D_DRIVER_TYPE_WARP, // Create a WARP device instead of a hardware device.
			0,
			creationFlags,
			featureLevels,
			ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION,
			&device,
			&m_d3dFeatureLevel,
			&context
		);
	}
	if (FAILED(hr)) {
		throw std::exception("D3DResources::CreateDeviceResources : failed to create ID3D11Device");
	}

	// Store pointers to the Direct3D 11.3 API device and immediate context.
	hr = device.As(&m_pD3dDevice);
	if (FAILED(hr)) {
		throw std::exception("D3DResources::CreateDeviceResources : failed to create Direct3D 11.4 API device");
	}

	hr = context.As(&m_pD3dContext);
	if (FAILED(hr)) {
		throw std::exception("D3DResources::CreateDeviceResources : failed to create Direct3D 11.3 API context");
	}

	// Create the Direct2D device object and a corresponding context.
	Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDevice;
	hr = m_pD3dDevice.As(&dxgiDevice);
	if (FAILED(hr)) {
		throw std::exception("D3DResources::CreateDeviceResources : failed to create IDXGIDevice3");
	}

	hr = m_pD2dFactory->CreateDevice(dxgiDevice.Get(), &m_pD2dDevice);
	if (FAILED(hr)) {
		throw std::exception("D3DResources::CreateDeviceResources : failed to create D2D device");
	}

	hr = m_pD2dDevice->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
		&m_pD2dContext
	);
	if (FAILED(hr)) {
		throw std::exception("D3DResources::CreateDeviceResources : failed to create D2D context");
	}
}

// These resources need to be recreated every time the window size is changed.
void D3DResources::CreateWindowSizeDependentResources() {
	// Clear the previous window size specific context.
	ID3D11RenderTargetView* nullViews[] = { nullptr };
	m_pD3dContext->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, nullptr);
	m_pD3dRenderTargetView = nullptr;
	m_pD2dContext->SetTarget(nullptr);
	m_pD2dTargetBitmap = nullptr;
	m_pD3dDepthStencilView = nullptr;
	m_pD3dContext->Flush1(D3D11_CONTEXT_TYPE_ALL, nullptr);

	UpdateRenderTargetSize();

	// The width and height of the swap chain must be based on the window's
	// natively-oriented width and height. If the window is not in the native
	// orientation, the dimensions must be reversed.
	DXGI_MODE_ROTATION displayRotation = ComputeDisplayRotation();
	bool swapDimensions = displayRotation == DXGI_MODE_ROTATION_ROTATE90 || displayRotation == DXGI_MODE_ROTATION_ROTATE270;
	m_d3dRenderTargetWidth = swapDimensions ? m_outputHeight : m_outputWidth;
	m_d3dRenderTargetHeight = swapDimensions ? m_outputWidth : m_outputHeight;

	HRESULT hr;
	if (m_pSwapChain != nullptr) {
		// If the swap chain already exists, resize it.
		hr = m_pSwapChain->ResizeBuffers(
			2, // Double-buffered swap chain.
			lround(m_d3dRenderTargetWidth),
			lround(m_d3dRenderTargetHeight),
			DXGI_FORMAT_B8G8R8A8_UNORM,
			0
		);

		if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
			// If the device was removed for any reason, a new device and swap chain will need to be created.
			HandleDeviceLost();
			// Everything is set up now. Do not continue execution of this method. HandleDeviceLost will reenter this method 
			// and correctly set up the new device.
			return;
		} else {
			if (FAILED(hr)) {
				throw std::exception("D3DResources::CreateWindowSizeDependentResources : device lost, could not recover");
			}
		}
	} else {
		// Otherwise, create a new one using the same adapter as the existing Direct3D device.

		/*
		BufferCount is the number of buffers in the swap chain. With flip model swap chains, the operating system may lock one buffer for an entire vsync
		interval while it is displayed, so the number of buffers available to the application to write is actually BufferCount-1. If BufferCount = 2,
		then there is only one buffer to write to until the OS releases the second one at the next vsync. A consequence of this is that the frame rate cannot exceed the refresh rate.
		When BufferCount >= 3, there are at least 2 buffers available to the application which it can cycle between (assuming SyncInterval=0), which allows the frame rate to be unlimited.
		https://software.intel.com/content/www/us/en/develop/articles/sample-application-for-direct3d-12-flip-model-swap-chains.html
		*/
		DXGI_SCALING scaling = DisplayMetrics::SupportHighResolutions ? DXGI_SCALING_NONE : DXGI_SCALING_STRETCH;
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
		swapChainDesc.Width = lround(m_d3dRenderTargetWidth);			// Match the size of the window.
		swapChainDesc.Height = lround(m_d3dRenderTargetHeight);
		swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;				// This is the most common swap chain format.
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;								// Don't use multi-sampling.
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 2;									// Use double-buffering to minimize latency.
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;	// All Microsoft Store apps must use this SwapEffect.
		swapChainDesc.Flags = 0;
		swapChainDesc.Scaling = scaling;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

		// This sequence obtains the DXGI factory that was used to create the Direct3D device above.
		Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDevice;
		hr = m_pD3dDevice.As(&dxgiDevice);
		if (FAILED(hr)) {
			throw std::exception("D3DResources::CreateWindowSizeDependentResources : failed to create IDXGIDevice3");
		}

		Microsoft::WRL::ComPtr<IDXGIAdapter> dxgiAdapter;
		hr = dxgiDevice->GetAdapter(&dxgiAdapter);
		if (FAILED(hr)) {
			throw std::exception("D3DResources::CreateWindowSizeDependentResources : failed to get IDXGIAdapter");
		}

		Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;
		hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
		if (FAILED(hr)) {
			throw std::exception("D3DResources::CreateWindowSizeDependentResources : dxgiAdapter GetParent failed");
		}

		Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain;
		hr = dxgiFactory->CreateSwapChainForHwnd(m_pD3dDevice.Get(), m_hWnd, &swapChainDesc, nullptr, nullptr, &swapChain);
		if (FAILED(hr)) {
			throw std::exception("D3DResources::CreateWindowSizeDependentResources : failed to create IDXGISwapChain4");
		}

		hr = swapChain.As(&m_pSwapChain);
		if (FAILED(hr)) {
			throw std::exception("D3DResources::CreateWindowSizeDependentResources : failed to query interface swap chain");
		}

		// Ensure that DXGI does not queue more than one frame at a time. This both reduces latency and
		// ensures that the application will only render after each VSync, minimizing power consumption.
		hr = dxgiDevice->SetMaximumFrameLatency(1);
		if (FAILED(hr)) {
			throw std::exception("D3DResources::CreateWindowSizeDependentResources : failed to SetMaximumFrameLatency");
		}
	}

	// Set the proper orientation for the swap chain, and generate 2D and
	// 3D matrix transformations for rendering to the rotated swap chain.
	// Note the rotation angle for the 2D and 3D transforms are different.
	// This is due to the difference in coordinate spaces.  Additionally,
	// the 3D matrix is specified explicitly to avoid rounding errors.

	switch (displayRotation) {
	case DXGI_MODE_ROTATION_IDENTITY:
		m_orientationTransform2D = D2D1::Matrix3x2F::Identity();
		m_orientationTransform3D = ScreenRotation::Rotation0;
		break;

	case DXGI_MODE_ROTATION_ROTATE90:
		m_orientationTransform2D =
			D2D1::Matrix3x2F::Rotation(90.0f) *
			D2D1::Matrix3x2F::Translation(m_logicalHeight, 0.0f);
		m_orientationTransform3D = ScreenRotation::Rotation270;
		break;

	case DXGI_MODE_ROTATION_ROTATE180:
		m_orientationTransform2D =
			D2D1::Matrix3x2F::Rotation(180.0f) *
			D2D1::Matrix3x2F::Translation(m_logicalWidth, m_logicalHeight);
		m_orientationTransform3D = ScreenRotation::Rotation180;
		break;

	case DXGI_MODE_ROTATION_ROTATE270:
		m_orientationTransform2D =
			D2D1::Matrix3x2F::Rotation(270.0f) *
			D2D1::Matrix3x2F::Translation(0.0f, m_logicalWidth);
		m_orientationTransform3D = ScreenRotation::Rotation90;
		break;

	default:
		throw std::exception("D3DResources::CreateWindowSizeDependentResources : DXGI_MODE_ROTATION invalid");
	}
	hr = m_pSwapChain->SetRotation(displayRotation);
	if (FAILED(hr)) {
		throw std::exception("D3DResources::CreateWindowSizeDependentResources : failed to SetRotation");
	}

	// Create a render target view of the swap chain back buffer.
	Microsoft::WRL::ComPtr<ID3D11Texture2D1> backBuffer;
	hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	if (FAILED(hr)) {
		throw std::exception("D3DResources::CreateWindowSizeDependentResources : failed to get ID3D11Texture2D1 back buffer");
	}

	hr = m_pD3dDevice->CreateRenderTargetView1(
		backBuffer.Get(),
		nullptr,
		&m_pD3dRenderTargetView
	);
	if (FAILED(hr)) {
		throw std::exception("D3DResources::CreateWindowSizeDependentResources : failed to get CreateRenderTargetView1");
	}

	// create the depth stencil buffer using a texture resource
	CD3D11_TEXTURE2D_DESC1 depthStencilDesc = {};
	depthStencilDesc.Width = lround(m_d3dRenderTargetWidth);
	depthStencilDesc.Height = lround(m_d3dRenderTargetHeight);
	depthStencilDesc.MipLevels = 1u;
	depthStencilDesc.ArraySize = 1u;
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.SampleDesc.Count = 1u;
	depthStencilDesc.SampleDesc.Quality = 0u;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	Microsoft::WRL::ComPtr<ID3D11Texture2D1> depthStencil;
	hr = m_pD3dDevice->CreateTexture2D1(
		&depthStencilDesc,
		nullptr,
		&depthStencil
	);
	if (FAILED(hr)) {
		throw std::exception("D3DResources::CreateWindowSizeDependentResources : failed to create depth stencil buffer");
	}

	CD3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc = {};
	depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0u;
	hr = m_pD3dDevice->CreateDepthStencilView(
		depthStencil.Get(),
		&depthStencilViewDesc,
		&m_pD3dDepthStencilView
	);
	if (FAILED(hr)) {
		throw std::exception("D3DResources::CreateWindowSizeDependentResources : failed to create depth stencil view");
	}

	// Set the 3D rendering viewport to target the entire window.
	m_screenViewport = CD3D11_VIEWPORT(
		0.0f,
		0.0f,
		m_d3dRenderTargetWidth,
		m_d3dRenderTargetHeight
	);

	m_pD3dContext->RSSetViewports(1u, &m_screenViewport);

	// setup rasterizer (is this screen size dependent?)
	/*D3D11_RASTERIZER_DESC2 rasterDesc = {};
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.SlopeScaledDepthBias = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.ScissorEnable = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.ForcedSampleCount = 0;
	rasterDesc.ConservativeRaster = D3D11_CONSERVATIVE_RASTERIZATION_MODE_OFF;
	hr = m_pD3dDevice->CreateRasterizerState2(
		&rasterDesc,
		&m_pD3dRasterizerState
	);
	if (FAILED(hr)) {
		throw std::exception("D3DResources::CreateWindowSizeDependentResources : failed to create rasterizer state");
	}*/


	// Create a Direct2D target bitmap associated with the
	// swap chain back buffer and set it as the current target.
	D2D1_BITMAP_PROPERTIES1 bitmapProperties =
		D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
			m_dpi,
			m_dpi
		);

	Microsoft::WRL::ComPtr<IDXGISurface2> dxgiBackBuffer;
	hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
	if (FAILED(hr)) {
		throw std::exception("D3DResources::CreateWindowSizeDependentResources : failed to get dxgiBackBuffer");
	}

	hr = m_pD2dContext->CreateBitmapFromDxgiSurface(
		dxgiBackBuffer.Get(),
		&bitmapProperties,
		&m_pD2dTargetBitmap
	);
	if (FAILED(hr)) {
		throw std::exception("D3DResources::CreateWindowSizeDependentResources : failed to CreateBitmapFromDxgiSurface");
	}

	m_pD2dContext->SetTarget(m_pD2dTargetBitmap.Get());
	m_pD2dContext->SetDpi(m_effectiveDpi, m_effectiveDpi);

	// Grayscale text anti-aliasing is recommended for all Microsoft Store apps.
	m_pD2dContext->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
}

// Determine the dimensions of the render target and whether it will be scaled down.
void D3DResources::UpdateRenderTargetSize() {
	//DPI_AWARENESS_CONTEXT dpiAwarenessContext = GetThreadDpiAwarenessContext();
	//DPI_AWARENESS dpiAwareness = GetAwarenessFromDpiAwarenessContext(dpiAwarenessContext);
	m_dpi = GetDpiForWindow(m_hWnd);
	m_effectiveDpi = m_dpi;

	// To improve battery life on high resolution devices, render to a smaller render target
	// and allow the GPU to scale the output when it is presented.
	if (!DisplayMetrics::SupportHighResolutions && m_dpi > DisplayMetrics::DpiThreshold) {
		float width = ConvertDipsToPixels(m_logicalWidth, m_dpi);
		float height = ConvertDipsToPixels(m_logicalHeight, m_dpi);

		// When the device is in portrait orientation, height > width. Compare the
		// larger dimension against the width threshold and the smaller dimension
		// against the height threshold.
		if (std::max(width, height) > DisplayMetrics::WidthThreshold && std::min(width, height) > DisplayMetrics::HeightThreshold) {
			// To scale the app we change the effective DPI. Logical size does not change.
			m_effectiveDpi /= 2.0f;
		}
	}

	// Calculate the necessary render target size in pixels.
	m_outputWidth = ConvertDipsToPixels(m_logicalWidth, m_effectiveDpi);
	m_outputHeight = ConvertDipsToPixels(m_logicalHeight, m_effectiveDpi);

	// Prevent zero size DirectX content from being created.
	m_outputWidth = std::max(m_outputWidth, 1.0f);
	m_outputHeight = std::max(m_outputHeight, 1.0f);
}

// This method is called when the CoreWindow is created (or re-created).
//void D3DResources::SetWindow(CoreWindow^ window) {
//	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();
//
//	m_window = window;
//	m_logicalSize = Windows::Foundation::Size(window->Bounds.Width, window->Bounds.Height);
//	m_nativeOrientation = currentDisplayInformation->NativeOrientation;
//	m_currentOrientation = currentDisplayInformation->CurrentOrientation;
//	m_dpi = currentDisplayInformation->LogicalDpi;
//	m_d2dContext->SetDpi(m_dpi, m_dpi);
//
//	CreateWindowSizeDependentResources();
//}

// This method is called in the event handler for the SizeChanged event.
void D3DResources::SetLogicalSize(float width, float height, bool isFullScreen) {
	if (m_logicalWidth != width || m_logicalHeight != height) {
		m_logicalWidth = width;
		m_logicalHeight = height;
		m_pSwapChain->SetFullscreenState(isFullScreen, nullptr);
		CreateWindowSizeDependentResources();
	}
}

// This method is called in the event handler for the DpiChanged event.
//void D3DResources::SetDpi(float dpi) {
//	if (dpi != m_dpi) 	{
//		m_dpi = dpi;
//
//		// When the display DPI changes, the logical size of the window (measured in Dips) also changes and needs to be updated.
//		m_logicalSize = Windows::Foundation::Size(m_window->Bounds.Width, m_window->Bounds.Height);
//
//		m_pD2dContext->SetDpi(m_dpi, m_dpi);
//		CreateWindowSizeDependentResources();
//	}
//}

// This method is called in the event handler for the OrientationChanged event.
//void D3DResources::SetCurrentOrientation(DisplayOrientations currentOrientation) {
//	if (m_currentOrientation != currentOrientation) 	{
//		m_currentOrientation = currentOrientation;
//		CreateWindowSizeDependentResources();
//	}
//}

// This method is called in the event handler for the DisplayContentsInvalidated event.
void D3DResources::ValidateDevice() {
	// The D3D Device is no longer valid if the default adapter changed since the device
	// was created or if the device has been removed.

	// First, get the information for the default adapter from when the device was created.
	HRESULT hr;
	Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDevice;
	if (FAILED(hr = m_pD3dDevice.As(&dxgiDevice))) {
		throw std::exception("D3DResources::ValidateDevice : QueryInterface IDXGIDevice3");
	}

	Microsoft::WRL::ComPtr<IDXGIAdapter> deviceAdapter;
	if (FAILED(hr = dxgiDevice->GetAdapter(&deviceAdapter))) {
		throw std::exception("D3DResources::ValidateDevice : GetAdapter IDXGIAdapter");
	}

	Microsoft::WRL::ComPtr<IDXGIFactory4> deviceFactory;
	if (FAILED(hr = deviceAdapter->GetParent(IID_PPV_ARGS(&deviceFactory)))) {
		throw std::exception("D3DResources::ValidateDevice : GetParent IDXGIFactory4 deviceFactory");
	}

	Microsoft::WRL::ComPtr<IDXGIAdapter1> previousDefaultAdapter;
	if (FAILED(hr = deviceFactory->EnumAdapters1(0, &previousDefaultAdapter))) {
		throw std::exception("D3DResources::ValidateDevice : EnumAdapters1 IDXGIAdapter1 previousDefaultAdapter");
	}

	DXGI_ADAPTER_DESC1 previousDesc;
	if (FAILED(hr = previousDefaultAdapter->GetDesc1(&previousDesc))) {
		throw std::exception("D3DResources::ValidateDevice : GetDesc1 DXGI_ADAPTER_DESC1 previousDesc");
	}

	// Next, get the information for the current default adapter.

	Microsoft::WRL::ComPtr<IDXGIFactory4> currentFactory;
	if (FAILED(hr = CreateDXGIFactory1(IID_PPV_ARGS(&currentFactory)))) {
		throw std::exception("D3DResources::ValidateDevice : CreateDXGIFactory1 IDXGIFactory4 currentFactory");
	}

	Microsoft::WRL::ComPtr<IDXGIAdapter1> currentDefaultAdapter;
	if (FAILED(hr = currentFactory->EnumAdapters1(0, &currentDefaultAdapter))) {
		throw std::exception("D3DResources::ValidateDevice : EnumAdapters1 IDXGIAdapter1 currentDefaultAdapter");
	}

	DXGI_ADAPTER_DESC1 currentDesc;
	if (FAILED(hr = currentDefaultAdapter->GetDesc1(&currentDesc))) {
		throw std::exception("D3DResources::ValidateDevice : GetDesc1 DXGI_ADAPTER_DESC1 currentDesc");
	}

	// If the adapter LUIDs don't match, or if the device reports that it has been removed,
	// a new D3D device must be created.

	if (previousDesc.AdapterLuid.LowPart != currentDesc.AdapterLuid.LowPart ||
		previousDesc.AdapterLuid.HighPart != currentDesc.AdapterLuid.HighPart ||
		FAILED(m_pD3dDevice->GetDeviceRemovedReason())) {
		// Release references to resources related to the old device.
		dxgiDevice = nullptr;
		deviceAdapter = nullptr;
		deviceFactory = nullptr;
		previousDefaultAdapter = nullptr;

		// Create a new device and swap chain.
		HandleDeviceLost();
	}
}

// Recreate all device resources and set them back to the current state.
void D3DResources::HandleDeviceLost() {
	m_pSwapChain = nullptr;

	if (m_pDeviceNotify != nullptr) {
		m_pDeviceNotify->OnDeviceLost();
	}

	CreateDeviceResources();
	m_pD2dContext->SetDpi(m_dpi, m_dpi);
	CreateWindowSizeDependentResources();

	if (m_pDeviceNotify != nullptr) {
		m_pDeviceNotify->OnDeviceRestored();
	}
}

// Register our DeviceNotify to be informed on device lost and creation.
void D3DResources::RegisterDeviceNotify(IDeviceNotify* deviceNotify) {
	m_pDeviceNotify = deviceNotify;
}

// Call this method when the app suspends. It provides a hint to the driver that the app 
// is entering an idle state and that temporary buffers can be reclaimed for use by other apps.
void D3DResources::Trim() {
	Microsoft::WRL::ComPtr<IDXGIDevice3> dxgiDevice;
	m_pD3dDevice.As(&dxgiDevice);
	dxgiDevice->Trim();
}

// Present the contents of the swap chain to the screen.
void D3DResources::Present() {
	// The first argument instructs DXGI to block until VSync, putting the application
	// to sleep until the next VSync. This ensures we don't waste any cycles rendering
	// frames that will never be displayed to the screen.
	DXGI_PRESENT_PARAMETERS parameters = { 0 };
	HRESULT hr = m_pSwapChain->Present1(m_syncInterval, 0u, &parameters);

	// Discard the contents of the render target.
	// This is a valid operation only when the existing contents will be entirely
	// overwritten. If dirty or scroll rects are used, this call should be removed.
	m_pD3dContext->DiscardView1(m_pD3dRenderTargetView.Get(), nullptr, 0);

	// Discard the contents of the depth stencil.
	m_pD3dContext->DiscardView1(m_pD3dDepthStencilView.Get(), nullptr, 0);

	// If the device was removed either by a disconnection or a driver upgrade, we 
	// must recreate all device resources.
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
		HandleDeviceLost();
	} else {
		if (FAILED(hr)) {
			throw std::exception("D3DResources::Present : swapchain failed to present");
		}
	}
}

// This method determines the rotation between the display device's native orientation and the current display orientation.
DXGI_MODE_ROTATION D3DResources::ComputeDisplayRotation() {
	//DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_UNSPECIFIED;
	DXGI_MODE_ROTATION rotation = DXGI_MODE_ROTATION_IDENTITY;

	// Note: NativeOrientation can only be Landscape or Portrait even though
	// the DisplayOrientations enum has other values.
	/*switch (m_nativeOrientation) 	{
		case DisplayOrientations::Landscape:
			switch (m_currentOrientation) 		{
				case DisplayOrientations::Landscape:
					rotation = DXGI_MODE_ROTATION_IDENTITY;
					break;

				case DisplayOrientations::Portrait:
					rotation = DXGI_MODE_ROTATION_ROTATE270;
					break;

				case DisplayOrientations::LandscapeFlipped:
					rotation = DXGI_MODE_ROTATION_ROTATE180;
					break;

				case DisplayOrientations::PortraitFlipped:
					rotation = DXGI_MODE_ROTATION_ROTATE90;
					break;
			}
			break;

		case DisplayOrientations::Portrait:
			switch (m_currentOrientation) 		{
				case DisplayOrientations::Landscape:
					rotation = DXGI_MODE_ROTATION_ROTATE90;
					break;

				case DisplayOrientations::Portrait:
					rotation = DXGI_MODE_ROTATION_IDENTITY;
					break;

				case DisplayOrientations::LandscapeFlipped:
					rotation = DXGI_MODE_ROTATION_ROTATE270;
					break;

				case DisplayOrientations::PortraitFlipped:
					rotation = DXGI_MODE_ROTATION_ROTATE180;
					break;
			}
			break;
	}*/
	return rotation;
}

} // namespace engine