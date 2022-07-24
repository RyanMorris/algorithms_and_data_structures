#ifndef PCH_H
#define PCH_H

#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
	#define STRICT

	#include <d2d1_3.h>
	#include <d2d1effects_2.h>
	#include <d3d11_4.h>
	#include <d3dcompiler.h>
	#include <DirectXMath.h>
	#include <dwrite_3.h>
	#include <dxgi1_4.h>
	#include <dxgidebug.h>
	#include <wincodec.h>
	#include <Windows.h>
	#include <wrl.h>

#elif defined(__ANDROID__)
	#define PLATFORM_NAME "android"

#elif defined(__linux__)			// Debian, Ubuntu, Gentoo, Fedora, openSUSE, RedHat, Centos and other
	#define PLATFORM_NAME "linux"

#elif defined(__unix__) || !defined(__APPLE__) && defined(__MACH__)
	#include <sys/param.h>
	#if defined(BSD)				// FreeBSD, NetBSD, OpenBSD, DragonFly BSD
		#define PLATFORM_NAME "bsd"
	#endif
#endif

#include <future>
#include <iostream>
#include <queue>
#include <string>
#include <tchar.h>
#include <vector>

#endif // PCH_H