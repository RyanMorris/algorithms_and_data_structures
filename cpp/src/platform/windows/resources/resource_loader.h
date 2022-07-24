#ifndef PLATFORM_WINDOWS_RESOURCES_RESOURCE_LOADER_H
#define PLATFORM_WINDOWS_RESOURCES_RESOURCE_LOADER_H

namespace engine {

class ResourceLoader {
public:
    ResourceLoader(Microsoft::WRL::ComPtr<ID3D11Device> pDevice);
    void LoadVertexShader(const std::wstring& filename, const D3D11_INPUT_ELEMENT_DESC inputElementDesc[], uint32_t inputElementDescNumElements, ID3D11VertexShader** shader, ID3D11InputLayout** inputLayout);
    void LoadVertexShaderAsync(std::wstring filename, const D3D11_INPUT_ELEMENT_DESC inputElementDesc[], uint32_t inputElementDescNumElements, ID3D11VertexShader** shader, ID3D11InputLayout** inputLayout);
    void LoadPixelShader(const std::wstring& filename, ID3D11PixelShader** shader);
    void LoadPixelShaderAsync(std::wstring filename, ID3D11PixelShader** shader);
private:
    Microsoft::WRL::ComPtr<ID3D11Device> m_pDevice;
};

} // namespace engine

#endif // PLATFORM_WINDOWS_RESOURCES_RESOURCE_LOADER_H