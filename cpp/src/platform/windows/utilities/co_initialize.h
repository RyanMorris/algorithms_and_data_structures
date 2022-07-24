#ifndef PLATFORM_WINDOWS_UTILITIES_CO_INITIALIZE_H
#define PLATFORM_WINDOWS_UTILITIES_CO_INITIALIZE_H

namespace engine {

// class to handle the COM RAII
class CCoInitialize {
public:
	CCoInitialize() : m_hr(CoInitialize(NULL)) {}
	~CCoInitialize() { if (SUCCEEDED(m_hr)) CoUninitialize(); }
	operator HRESULT() const { return m_hr; }
	HRESULT m_hr;
};

} // namespace engine

#endif // PLATFORM_WINDOWS_UTILITIES_CO_INITIALIZE_H