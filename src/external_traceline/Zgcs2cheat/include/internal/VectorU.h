#pragma once
#ifndef _F_VECTOR_U_H
#define _F_VECTOR_U_H

#include "fMath.h"
#include <string>
#include <vector>
#include <sstream>
#include <fstream>

bool hexStringToBytes(const std::string& hex, std::vector<unsigned char> & bytes);

bool parseVertices(const std::string& hexBlob, std::vector<Vector3>& vertices);

bool parseTriangles(const std::string& hexBlob, std::vector<Triangle>& triangles);

bool parseMaterials(const std::string& arrayContent, std::vector<int>& materials);

bool parseFloatArray(const std::string& arrayContent, std::vector<float>& values);

bool generateTrianglesFromAABB(const Vector3& min, const Vector3& max, int materialIndex, std::vector<TriangleCombined>& tris);

void SerializeVector3(std::ofstream& out, const Vector3& v);

void SerializeAABB(std::ofstream& out, const AABB& box);
#endif