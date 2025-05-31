#ifndef _F_LOSCHECKER_
#define _F_LOSCHECKER_

#ifdef ZG_CS2CHEAT_EXPORTS
#define WINBASEAPI __declspec(dllexport)
#else
#define WINBASEAPI __declspec(dllimport)
#endif

#ifndef WINCSAPI
#define WINCSAPI extern "C" WINBASEAPI
#endif

#include "./internal/fMath.h"
#include <utility>
#include <functional>
#define NOMINMAX
#include <Windows.h>

WINCSAPI
BOOL
WINAPI
IsPointVisible(
	const Vector3& point1, 
	const Vector3& point2, 
	const BVHNode* root,
	std::vector<std::tuple<float, int, float, int>>& result);

#endif