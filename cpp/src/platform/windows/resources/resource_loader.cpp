#include "pch.h"
#include "resource_loader.h"

namespace engine {

ResourceLoader::ResourceLoader(Microsoft::WRL::ComPtr<ID3D11Device> pDevice)
    : m_pDevice(std::move(pDevice)) {
}

void ResourceLoader::LoadVertexShader(const std::wstring& filename, const D3D11_INPUT_ELEMENT_DESC inputElementDesc[], uint32_t inputElementDescNumElements, ID3D11VertexShader** shader, ID3D11InputLayout** inputLayout) {
    HRESULT hr;
    Microsoft::WRL::ComPtr<ID3DBlob> pVertexShaderBlob;
    if (FAILED(hr = D3DReadFileToBlob(filename.data(), &pVertexShaderBlob))) {
        throw std::exception("ResourceLoader::LoadVertexShader : failed reading vertex shader file");
    }
    if (FAILED(hr = m_pDevice->CreateVertexShader(pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(), nullptr, shader))) {
        throw std::exception("ResourceLoader::LoadVertexShader : failed creating vertex shader");
    }
    if (inputLayout != nullptr) {
        if (FAILED(hr = m_pDevice->CreateInputLayout(inputElementDesc, inputElementDescNumElements, pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(), inputLayout))) {
            throw std::exception("ResourceLoader::LoadVertexShader : failed creating input layout");
        }
    }
}

void ResourceLoader::LoadVertexShaderAsync(std::wstring filename, const D3D11_INPUT_ELEMENT_DESC inputElementDesc[], uint32_t inputElementDescNumElements, ID3D11VertexShader** shader, ID3D11InputLayout** inputLayout) {
    // this method assumes that the lifetime of input arguments may be shorter than the duration of this operation
    // in order to ensure accurate results, a copy of all arguments passed by pointer must be made
    // the method then ensures that the lifetime of the copied data exceeds that of the coroutine

    // Create copies of the layoutDesc array as well as the SemanticName strings, both of which are pointers to data whose lifetimes may be shorter than that of this method's coroutine
    std::vector<D3D11_INPUT_ELEMENT_DESC> inputElementDescCopy;
    std::vector<std::string> layoutDescSemanticNamesCopy;
    if (inputElementDesc != nullptr) {
        inputElementDescCopy = { inputElementDesc, inputElementDesc + inputElementDescNumElements };
        layoutDescSemanticNamesCopy.reserve(inputElementDescNumElements);
        for (auto&& desc : inputElementDescCopy) {
            desc.SemanticName = layoutDescSemanticNamesCopy.emplace(layoutDescSemanticNamesCopy.end(), desc.SemanticName)->c_str();
        }
    }

    HRESULT hr;
    Microsoft::WRL::ComPtr<ID3DBlob> pVertexShaderBlob;
    // TODO: make this read async?
    if (FAILED(hr = D3DReadFileToBlob(filename.data(), &pVertexShaderBlob))) {
        // get current dir of exe
        //WCHAR path[MAX_PATH];
        //GetModuleFileNameW(NULL, path, MAX_PATH);
        //char tmp[256];
        //sprintf_s(tmp, "%ls", path);
        //throw std::exception(tmp);

        std::string s = "ResourceLoader::LoadVertexShaderAsync : failed reading vertex shader file: ";
        throw std::exception(s.c_str());
    }
    if (FAILED(hr = m_pDevice->CreateVertexShader(pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(), nullptr, shader))) {
        throw std::exception("ResourceLoader::LoadVertexShaderAsync : failed creating vertex shader");
    }
    if (inputLayout != nullptr) {
        if (FAILED(hr = m_pDevice->CreateInputLayout((inputElementDesc == nullptr ? nullptr : inputElementDescCopy.data()), inputElementDescNumElements, pVertexShaderBlob->GetBufferPointer(), pVertexShaderBlob->GetBufferSize(), inputLayout))) {
            throw std::exception("ResourceLoader::LoadVertexShader : failed creating input layout");
        }
    }
}

void ResourceLoader::LoadPixelShader(const std::wstring& filename, ID3D11PixelShader** shader) {
    HRESULT hr;
    Microsoft::WRL::ComPtr<ID3DBlob> pPixelShaderBlob;
    if (FAILED(hr = D3DReadFileToBlob(filename.data(), &pPixelShaderBlob))) {
        throw std::exception("ResourceLoader::LoadPixelShader : failed reading pixel shader file");
    }
    if (FAILED(hr = m_pDevice->CreatePixelShader(pPixelShaderBlob->GetBufferPointer(), pPixelShaderBlob->GetBufferSize(), nullptr, shader))) {
        throw std::exception("ResourceLoader::LoadPixelShader : failed creating pixel shader");
    }
}

void ResourceLoader::LoadPixelShaderAsync(std::wstring filename, ID3D11PixelShader** shader) {
    // this method assumes that the lifetime of input arguments may be shorter than the duration of this operation
    // in order to ensure accurate results, a copy of all arguments passed by pointer must be made
    // the method then ensures that the lifetime of the copied data exceeds that of the coroutine
    HRESULT hr;
    Microsoft::WRL::ComPtr<ID3DBlob> pPixelShaderBlob;
    if (FAILED(hr = D3DReadFileToBlob(filename.data(), &pPixelShaderBlob))) {
        throw std::exception("ResourceLoader::LoadPixelShaderAsync : failed reading pixel shader file");
    }
    if (FAILED(hr = m_pDevice->CreatePixelShader(pPixelShaderBlob->GetBufferPointer(), pPixelShaderBlob->GetBufferSize(), nullptr, shader))) {
        throw std::exception("ResourceLoader::LoadPixelShaderAsync : failed creating pixel shader");
    }
}

} // namespace engine