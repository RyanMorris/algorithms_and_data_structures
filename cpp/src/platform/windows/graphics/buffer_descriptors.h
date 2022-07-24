#ifndef ENGINE_GRAPHICS_BUFFER_DESCRIPTORS_H
#define ENGINE_GRAPHICS_BUFFER_DESCRIPTORS_H

namespace engine {

struct PNVertex {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
};

static D3D11_INPUT_ELEMENT_DESC PNVertexLayout[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

struct PCVertex {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 color;
};

static D3D11_INPUT_ELEMENT_DESC PCVertexLayout[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"COLOR",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
};

struct PNTVertex {
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 textureCoordinate;
};

static D3D11_INPUT_ELEMENT_DESC PNTVertexLayout[] =
{
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
};

struct ConstantBufferProjectionMatrix {
	DirectX::XMFLOAT4X4 projectionMatrix;
};

struct ConstantBufferViewMatrix {
	DirectX::XMFLOAT4X4 viewMatrix;
};

struct ConstantBufferPerObject {
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4 meshColor;
};

} // namespace engine

#endif // ENGINE_GRAPHICS_BUFFER_DESCRIPTORS_H