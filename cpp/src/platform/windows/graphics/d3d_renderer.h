#ifndef PLATFORM_WINDOWS_GRAPHICS_D3DRENDERER_H
#define PLATFORM_WINDOWS_GRAPHICS_D3DRENDERER_H

#include "d3d_resources.h"

namespace engine {

class D3DRenderer {
public:
	D3DRenderer(const std::shared_ptr<D3DResources>& pD3DResources);
	~D3DRenderer();
	D3DRenderer(const D3DRenderer&) = delete;
	D3DRenderer& operator=(const D3DRenderer&) = delete;
	void CreateDeviceDependentResources();
	void ReleaseDeviceDependentResources();
	void CreateWindowSizeDependentResources(const DirectX::XMMATRIX& projectionMatrix);
	void CreateResourcesAsync();
	void CreateResourcesFinalStep(const DirectX::XMMATRIX& projectionMatrix);
	void SetupRender(const DirectX::XMMATRIX& viewMatrix);

	ID3D11VertexShader* GetVertexShader() { return m_pVertexShader.Get(); }
	ID3D11PixelShader* GetPixelShader() { return m_pPixelShader.Get(); }
	ID3D11Buffer* GetConstantBufferPerObject() { return m_pConstantBufferPerObject.Get(); }
private:
	std::shared_ptr<D3DResources> m_pD3DResources;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> m_pVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pPixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> m_pVertexInputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pConstantBufferProjectionMatrix;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pConstantBufferViewMatrix;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_pConstantBufferPerObject;
};
} // namespace engine

#endif // PLATFORM_WINDOWS_GRAPHICS_D3DRENDERER_H