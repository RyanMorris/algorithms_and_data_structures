#include "pch.h"
#include "d3d_renderer.h"
#include "platform/windows/graphics/buffer_descriptors.h"
#include "platform/windows/resources/resource_definitions.h"
#include "platform/windows/resources/resource_loader.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d2d1")

namespace engine {

D3DRenderer::D3DRenderer(const std::shared_ptr<D3DResources>& pD3DResources)
	: m_pD3DResources(pD3DResources) {
	CreateDeviceDependentResources();
}

D3DRenderer::~D3DRenderer() {}

void D3DRenderer::CreateDeviceDependentResources() {
}

void D3DRenderer::ReleaseDeviceDependentResources() {
	m_pConstantBufferProjectionMatrix.Reset();	// all the device resources are invalid, release now so it won't be pointlessly updated before it's recreated
}

void D3DRenderer::CreateWindowSizeDependentResources(const DirectX::XMMATRIX& projectionMatrix) {
	// *note* this function is kind of pointless until more stuff is added, like HUD or something
	if (m_pConstantBufferProjectionMatrix) {
		// check if the pointer has been set because the first exection of this it won't be yet, which this step will get handled in CreateResourcesFinalStep
		// this update is for when screen changes or device restored
		DirectX::XMFLOAT4X4 orientation = m_pD3DResources->GetOrientationTransform3D();
		ConstantBufferProjectionMatrix cbuffer;
		DirectX::XMStoreFloat4x4(&cbuffer.projectionMatrix, DirectX::XMMatrixTranspose(projectionMatrix * DirectX::XMMatrixTranspose(XMLoadFloat4x4(&orientation))));
		m_pD3DResources->GetD3DDeviceContext()->UpdateSubresource(m_pConstantBufferProjectionMatrix.Get(), 0u, nullptr, &cbuffer, 0u, 0u);
	}
}

void D3DRenderer::CreateResourcesAsync() {
	ID3D11Device5* d3dDevice = m_pD3DResources->GetD3DDevice();

	// create the constant buffers
	D3D11_BUFFER_DESC bdDefault = {0};
	bdDefault.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bdDefault.Usage = D3D11_USAGE_DEFAULT;
	bdDefault.CPUAccessFlags = 0u;

	HRESULT hr;
	bdDefault.ByteWidth = (sizeof(ConstantBufferProjectionMatrix) + 15) / 16 * 16;
	m_pConstantBufferProjectionMatrix = nullptr;
	if (FAILED(hr = d3dDevice->CreateBuffer(&bdDefault, nullptr, &m_pConstantBufferProjectionMatrix))) {
		throw std::exception("D3DRenderer::CreateResourcesAsync : failed to create constant buffer for projection matrix");
	}

	bdDefault.ByteWidth = (sizeof(ConstantBufferViewMatrix) + 15) / 16 * 16;
	m_pConstantBufferViewMatrix = nullptr;
	if (FAILED(hr = d3dDevice->CreateBuffer(&bdDefault, nullptr, &m_pConstantBufferViewMatrix))) {
		throw std::exception("D3DRenderer::CreateResourcesAsync : failed to create constant buffer for view matrix");
	}

	bdDefault.ByteWidth = ((sizeof(ConstantBufferPerObject) + 15) / 16) * 16;
	m_pConstantBufferPerObject = nullptr;
	if (FAILED(hr = d3dDevice->CreateBuffer(&bdDefault, nullptr, &m_pConstantBufferPerObject))) {
		throw std::exception("D3DRenderer::CreateResourcesAsync : failed to create constant buffer for render objects");
	}

	ResourceLoader resourceLoader {d3dDevice};
	//uint32_t numElements = ARRAYSIZE(PCVertexLayout);
	//std::vector<std::future<void>> futures;
	//futures.emplace_back(std::async(std::launch::async, &ResourceLoader::LoadVertexShaderAsync, &resourceLoader,
	//								(constants::compiledShadersFilePath + L"vertex_solid.cso"),
	//								PCVertexLayout,
	//								numElements,
	//								m_pVertexShader.GetAddressOf(),
	//								m_pVertexInputLayout.GetAddressOf()));
	//futures.emplace_back(std::async(std::launch::async, &ResourceLoader::LoadPixelShaderAsync, &resourceLoader,
	//								(constants::compiledShadersFilePath + L"pixel_solid.cso"),
	//								m_pPixelShader.GetAddressOf()));
	uint32_t numElements = ARRAYSIZE(PNTVertexLayout);
	std::vector<std::future<void>> futures;
	futures.emplace_back(std::async(std::launch::async, &ResourceLoader::LoadVertexShaderAsync, &resourceLoader,
									(constants::compiledShadersFilePath + L"vertex_texture.cso"),
									PNTVertexLayout,
									numElements,
									m_pVertexShader.GetAddressOf(),
									m_pVertexInputLayout.GetAddressOf()));
	futures.emplace_back(std::async(std::launch::async, &ResourceLoader::LoadPixelShaderAsync, &resourceLoader,
									(constants::compiledShadersFilePath + L"pixel_texture.cso"),
									m_pPixelShader.GetAddressOf()));
	//futures.emplace_back(std::async(std::launch::async, [&] {std::this_thread::sleep_for(std::chrono::seconds(3)); }));
	for (std::future<void>& f : futures) {
		f.get();
	}

	// non async
	/*resourceLoader.LoadVertexShader(
		(constants::compiledShadersFilePath + L"vertex_texture.cso"),
		PNTVertexLayout,
		numElements,
		m_pVertexShader.GetAddressOf(),
		m_pVertexInputLayout.GetAddressOf());

	resourceLoader.LoadPixelShader(
		(constants::compiledShadersFilePath + L"pixel_texture.cso"),
		m_pPixelShader.GetAddressOf()
	);*/
}

void D3DRenderer::CreateResourcesFinalStep(const DirectX::XMMATRIX& projectionMatrix) {
	DirectX::XMFLOAT4X4 orientation = m_pD3DResources->GetOrientationTransform3D();
	ConstantBufferProjectionMatrix cbuffer;
	DirectX::XMStoreFloat4x4(&cbuffer.projectionMatrix, DirectX::XMMatrixTranspose(projectionMatrix * DirectX::XMMatrixTranspose(XMLoadFloat4x4(&orientation))));
	m_pD3DResources->GetD3DDeviceContext()->UpdateSubresource(m_pConstantBufferProjectionMatrix.Get(), 0u, nullptr, &cbuffer, 0u, 0u);
}

void D3DRenderer::SetupRender(const DirectX::XMMATRIX& viewMatrix) {
	ID3D11DeviceContext4* d3dContext {m_pD3DResources->GetD3DDeviceContext()};
	ID2D1DeviceContext2* d2dContext {m_pD3DResources->GetD2DDeviceContext()};

	// https://stackoverflow.com/questions/53195870/how-to-use-dxgi-swap-effect-flip-sequential-correctly
	// "In general you should set your render targets, viewport, and scissor rectangles every frame, typically right after you clear the render target and depth/stencil buffer."
	ID3D11RenderTargetView* const targets[1] = {m_pD3DResources->GetBackBufferRenderTargetView()};
	d3dContext->OMSetRenderTargets(1u, targets, m_pD3DResources->GetDepthStencilView());
	d3dContext->ClearDepthStencilView(m_pD3DResources->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0u);
	d2dContext->SetTarget(m_pD3DResources->GetD2DTargetBitmap());

	const float color[4] = {0.230f, 0.148f, 0.289f, 1.0f};
	d3dContext->ClearRenderTargetView(m_pD3DResources->GetBackBufferRenderTargetView(), color);

	d3dContext->RSSetState(m_pD3DResources->GetRasterizerState());	// do every frame if just one?

	// configure viewport
	// https://github.com/microsoft/DirectXTK/wiki/The-basic-game-loop
	// "previously, the viewport was set in ResizeScreen and assumed to stay set for the remainder of the program execution or until
	// the window was resized. This approach is outdated, however, as the viewport could be overwritten or cleared by ClearState. When dealing with deferred contexts,
	// Xbox One fast semantics, or the Direct3D 12 API, assumptions of device state persisting from frame-to-frame without being reset is a likely source of rendering bugs.
	// Therefore, this template uses the best practice of resetting the viewport state at the start of each frame."
	/*D3D11_VIEWPORT viewPort;
	viewPort.Width = m_outputWidth;
	viewPort.Height = m_outputHeight;
	viewPort.MinDepth = 0.0f;
	viewPort.MaxDepth = 1.0f;
	viewPort.TopLeftX = 0.0f;
	viewPort.TopLeftY = 0.0f;
	m_pDeviceContext->RSSetViewports(1u, &viewPort);*/


	ConstantBufferViewMatrix constantBufferViewMatrix;
	DirectX::XMStoreFloat4x4(&constantBufferViewMatrix.viewMatrix, DirectX::XMMatrixTranspose(viewMatrix));
	d3dContext->UpdateSubresource(m_pConstantBufferViewMatrix.Get(), 0u, nullptr, &constantBufferViewMatrix, 0u, 0u);

	d3dContext->IASetInputLayout(m_pVertexInputLayout.Get());

	//ID3D11Buffer* constantBufferNeverChanges {m_constantBufferNeverChanges.get()};
	//d3dContext->VSSetConstantBuffers(0, 1, &constantBufferNeverChanges);

	// TODO: pre multiply view and projection, send one less buffer to gpu

	//ID3D11Buffer* pConstantBufferProjectionMatrix { m_pD3DResources->GetConstantBufferProjectionMatrix() };
	d3dContext->VSSetConstantBuffers(1u, 1u, m_pConstantBufferProjectionMatrix.GetAddressOf());

	//ID3D11Buffer* constantBufferViewMatrix {m_constantBufferChangesEveryFrame.get()};
	d3dContext->VSSetConstantBuffers(2u, 1u, m_pConstantBufferViewMatrix.GetAddressOf());

	//ID3D11Buffer* pConstantBufferPerObject { m_pD3DResources->GetConstantBufferPerObject() };
	d3dContext->VSSetConstantBuffers(3u, 1u, m_pConstantBufferPerObject.GetAddressOf());

	d3dContext->PSSetConstantBuffers(0u, 1u, m_pConstantBufferPerObject.GetAddressOf());
}
} // namespace engine