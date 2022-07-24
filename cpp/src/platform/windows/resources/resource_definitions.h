#ifndef PLATFORM_WINDOWS_RESOURCES_RESOURCE_DEFINITIONS_H
#define PLATFORM_WINDOWS_RESOURCES_RESOURCE_DEFINITIONS_H

namespace engine {
namespace constants {

#if defined(_DEBUG)
const std::wstring compiledShadersFilePath = L"../engine/compiled_shaders/";
#else
const std::wstring compiledShadersFilePath = L"compiled_shaders/";
#endif

} // namespace constants
} // namespace engine

#endif // PLATFORM_WINDOWS_RESOURCES_RESOURCE_DEFINITIONS_H