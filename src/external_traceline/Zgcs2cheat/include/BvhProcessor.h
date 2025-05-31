#ifndef _F_BVHPROC_
#define _F_BVHPROC_

#ifdef ZG_CS2CHEAT_EXPORTS
#define WINBASEAPI __declspec(dllexport)
#else
#define WINBASEAPI __declspec(dllimport)
#endif

#ifndef WINCSAPI
#define WINCSAPI extern "C" WINBASEAPI
#endif

#define NOMINMAX
#include <Windows.h>
#include <iostream>
#include "./internal/fMath.h"

WINCSAPI
BOOL
WINAPI
GenerateBvhFile(
	_In_ const std::string& inputPath, 
	_In_ const std::string& outputPath
);

WINCSAPI
BOOL
WINAPI
ParseBvhFile(
	_In_ const std::string& filePath,
	_Out_ std::unique_ptr<BVHNode>& outData
);
#endif