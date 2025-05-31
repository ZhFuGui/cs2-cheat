#include "../include/BvhProcessor.h"
#include "../include/internal/FileU.h"
#include "../include/internal/StringU.h"
#include "../include/internal/fMath.h"
#include "../include/internal/VectorU.h"

const size_t LEAF_THRESHOLD = 4;

std::unique_ptr<BVHNode> BuildBVH(const std::vector<TriangleCombined>& tris) {
	auto node = std::make_unique<BVHNode>();
	if (tris.empty())
		return node;
	AABB bounds = tris[0].ComputeAABB();
	for (size_t i = 1; i < tris.size(); ++i) {
		AABB tAABB = tris[i].ComputeAABB();
		bounds.min.x = std::min(bounds.min.x, tAABB.min.x);
		bounds.min.y = std::min(bounds.min.y, tAABB.min.y);
		bounds.min.z = std::min(bounds.min.z, tAABB.min.z);
		bounds.max.x = std::max(bounds.max.x, tAABB.max.x);
		bounds.max.y = std::max(bounds.max.y, tAABB.max.y);
		bounds.max.z = std::max(bounds.max.z, tAABB.max.z);
	}
	node->bounds = bounds;
	if (tris.size() <= LEAF_THRESHOLD) {
		node->triangles = tris;
		return node;
	}
	Vector3 diff = bounds.max - bounds.min;
	int axis = (diff.x > diff.y && diff.x > diff.z) ? 0 : ((diff.y > diff.z) ? 1 : 2);
	std::vector<TriangleCombined> sortedTris = tris;
	std::sort(sortedTris.begin(), sortedTris.end(), [axis](const TriangleCombined& a, const TriangleCombined& b) {
		AABB aAABB = a.ComputeAABB();
		AABB bAABB = b.ComputeAABB();
		float centerA, centerB;
		if (axis == 0) {
			centerA = (aAABB.min.x + aAABB.max.x) / 2.0f;
			centerB = (bAABB.min.x + bAABB.max.x) / 2.0f;
		}
		else if (axis == 1) {
			centerA = (aAABB.min.y + aAABB.max.y) / 2.0f;
			centerB = (bAABB.min.y + bAABB.max.y) / 2.0f;
		}
		else {
			centerA = (aAABB.min.z + aAABB.max.z) / 2.0f;
			centerB = (bAABB.min.z + bAABB.max.z) / 2.0f;
		}
		return centerA < centerB;
		});
	size_t mid = sortedTris.size() / 2;
	std::vector<TriangleCombined> leftTris(sortedTris.begin(), sortedTris.begin() + mid);
	std::vector<TriangleCombined> rightTris(sortedTris.begin() + mid, sortedTris.end());
	node->left = BuildBVH(leftTris);
	node->right = BuildBVH(rightTris);
	return node;
}

static void SerializeTriangle(std::ofstream& out, const TriangleCombined& tri) {
	SerializeVector3(out, tri.v0);
	SerializeVector3(out, tri.v1);
	SerializeVector3(out, tri.v2);
	out.write(reinterpret_cast<const char*>(&tri.materialIndex), sizeof(int));
}

bool SaveBVH(const BVHNode* node, std::ofstream& out) {
	if (!node)
		return false;
	SerializeAABB(out, node->bounds);
	bool isLeaf = node->IsLeaf();
	out.write(reinterpret_cast<const char*>(&isLeaf), sizeof(bool));
	if (isLeaf) {
		size_t triCount = node->triangles.size();
		out.write(reinterpret_cast<const char*>(&triCount), sizeof(size_t));
		for (const auto& tri : node->triangles) {
			SerializeTriangle(out, tri);
		}
	}
	else {
		SaveBVH(node->left.get(), out);
		SaveBVH(node->right.get(), out);
	}
	return true;
}

WINCSAPI
BOOL
WINAPI
GenerateBvhFile(
    _In_ const std::string& inputPath,
    _In_ const std::string& outputPath
)
{
    std::vector<TriangleCombined> combinedTriangles;

    // 读取输入文件内容
    std::string fileContent;
    if (!readFile(inputPath, fileContent)) {
        return FALSE;
    }

    // 提取网格数据块
    std::string meshesBlock;
    if (!findMeshesBlock(fileContent, meshesBlock)) {
        return FALSE;
    }

    // 解析网格结构
    std::vector<std::string> meshStructs;
    if (!extractStructs(meshesBlock, meshStructs)) {
        return FALSE;
    }

    // 获取默认碰撞索引
    std::vector<int> defaultCollisionIndices;
    if (!parseCollisionAttributes(fileContent, defaultCollisionIndices)) {
        return FALSE;
    }

    // 处理每个网格结构
    for (const auto& meshStruct : meshStructs) {
        // 检查碰撞属性
        int collisionAttribute = 0;
        if (!findParam(meshStruct, "m_nCollisionAttributeIndex", collisionAttribute)) {
            continue;
        }

        // 过滤不匹配的碰撞属性
        if (std::find(defaultCollisionIndices.begin(), defaultCollisionIndices.end(), collisionAttribute) == defaultCollisionIndices.end()) {
            continue;
        }

        // 获取表面属性
        int surfaceProperty = 0;
        if (!findParam(meshStruct, "m_nSurfacePropertyIndex", surfaceProperty)) {
            continue;
        }

        // 提取内部网格数据
        std::string innerMesh;
        if (!extractInnerMesh(meshStruct, innerMesh) || innerMesh.empty()) {
            continue;
        }

        // 提取顶点和三角形数据
        std::string verticesHex;
        if (!extractHexBlob(innerMesh, "m_Vertices", verticesHex) || verticesHex.empty()) {
            continue;
        }

        std::string trianglesHex;
        if (!extractHexBlob(innerMesh, "m_Triangles", trianglesHex) || trianglesHex.empty()) {
            continue;
        }

        // 解析顶点和三角形
        std::vector<Vector3> vertices;
        std::vector<Triangle> triangles;
        if (!parseVertices(verticesHex, vertices) || !parseTriangles(trianglesHex, triangles)) {
            return FALSE;
        }

        // 解析材质
        std::string materialsContent;
        std::vector<int> materials;
        bool hasMaterials = extractArrayContent(innerMesh, "m_Materials", materialsContent);

        if (hasMaterials && !materialsContent.empty() && !parseMaterials(materialsContent, materials)) {
            return FALSE;
        }

        // 构建组合三角形
        for (size_t i = 0; i < triangles.size(); ++i) {
            TriangleCombined tc;
            const Triangle& t = triangles[i];

            // 验证顶点索引有效性
            if (t.a >= vertices.size() || t.b >= vertices.size() || t.c >= vertices.size()) {
                continue;
            }

            tc.v0 = vertices[t.a];
            tc.v1 = vertices[t.b];
            tc.v2 = vertices[t.c];

            // 设置材质索引
            tc.materialIndex = hasMaterials ?
                (i < materials.size() ? materials[i] : 15) :
                surfaceProperty;

            combinedTriangles.push_back(tc);
        }
    }

    // 处理包围盒数据
    std::string hullsBlock;
    if (!findHullsBlock(fileContent, hullsBlock) || hullsBlock.empty()) {
        return FALSE;
    }

    std::vector<std::string> hullStructs;
    if (!extractStructs(hullsBlock, hullStructs) || hullStructs.empty()) {
        return FALSE;
    }

    int overallMinSurface = INT_MAX;
    int overallMaxSurface = INT_MIN;
    int totalHullTriangles = 0;

    // 处理每个包围盒结构
    for (const auto& hullStruct : hullStructs) {
        // 获取表面属性索引
        int surfacePropertyIndex;
        if (!findParam(hullStruct, "m_nSurfacePropertyIndex", surfacePropertyIndex)) {
            return FALSE;
        }

        // 更新表面属性范围
        overallMinSurface = std::min(overallMinSurface, surfacePropertyIndex);
        overallMaxSurface = std::max(overallMaxSurface, surfacePropertyIndex);

        // 提取内部包围盒数据
        std::string innerHull;
        if (!extractInnerBlock(hullStruct, "m_Hull", innerHull) || innerHull.empty()) {
            continue;
        }

        // 提取边界框数据
        std::string boundsBlock;
        if (!extractInnerBlock(innerHull, "m_Bounds", boundsBlock) || boundsBlock.empty()) {
            continue;
        }

        // 提取最小和最大边界值
        std::string minBoundsStr;
        std::string maxBoundsStr;
        if (!extractArrayContent(boundsBlock, "m_vMinBounds", minBoundsStr) || minBoundsStr.empty() ||
            !extractArrayContent(boundsBlock, "m_vMaxBounds", maxBoundsStr) || maxBoundsStr.empty()) {
            continue;
        }

        // 解析边界值数组
        std::vector<float> minValues;
        std::vector<float> maxValues;
        if (!parseFloatArray(minBoundsStr, minValues) || minValues.size() < 3 ||
            !parseFloatArray(maxBoundsStr, maxValues) || maxValues.size() < 3) {
            continue;
        }

        // 构建边界框向量
        Vector3 vmin = { minValues[0], minValues[1], minValues[2] };
        Vector3 vmax = { maxValues[0], maxValues[1], maxValues[2] };

        // 从AABB生成三角形
        std::vector<TriangleCombined> hullTriangles;
        generateTrianglesFromAABB(vmin, vmax, surfacePropertyIndex, hullTriangles);

        // 添加到组合三角形集合
        totalHullTriangles += hullTriangles.size();
        combinedTriangles.insert(combinedTriangles.end(), hullTriangles.begin(), hullTriangles.end());
    }

    // 构建BVH树并保存到文件
    std::unique_ptr<BVHNode> bvhRoot = BuildBVH(combinedTriangles);

    std::ofstream outfile(outputPath, std::ios::binary);
    if (outfile.good()) {
        SaveBVH(bvhRoot.get(), outfile);
        outfile.close();
        return TRUE;
    }

    return FALSE;
}

static Vector3 DeserializeVector3(std::ifstream& in) {
	float x, y, z;
	in.read(reinterpret_cast<char*>(&x), sizeof(float));
	in.read(reinterpret_cast<char*>(&y), sizeof(float));
	in.read(reinterpret_cast<char*>(&z), sizeof(float));
	return Vector3(x, y, z);
}

static AABB DeserializeAABB(std::ifstream& in) {
	AABB box;
	box.min = DeserializeVector3(in);
	box.max = DeserializeVector3(in);
	return box;
}

static TriangleCombined DeserializeTriangle(std::ifstream& in) {
	TriangleCombined tri;
	tri.v0 = DeserializeVector3(in);
	tri.v1 = DeserializeVector3(in);
	tri.v2 = DeserializeVector3(in);
	in.read(reinterpret_cast<char*>(&tri.materialIndex), sizeof(int));
	return tri;
}

std::unique_ptr<BVHNode> LoadBVH(std::ifstream& in) {
	if (in.eof())
		return nullptr;
	auto node = std::make_unique<BVHNode>();
	node->bounds = DeserializeAABB(in);
	bool isLeaf = false;
	in.read(reinterpret_cast<char*>(&isLeaf), sizeof(bool));
	if (isLeaf) {
		size_t triCount = 0;
		in.read(reinterpret_cast<char*>(&triCount), sizeof(size_t));
		node->triangles.resize(triCount);
		for (size_t i = 0; i < triCount; ++i) {
			node->triangles[i] = DeserializeTriangle(in);
		}
	}
	else {
		node->left = LoadBVH(in);
		node->right = LoadBVH(in);
	}
	return node;
}

WINCSAPI
BOOL
WINAPI
ParseBvhFile(
    _In_ const std::string& filePath,
    _Out_ std::unique_ptr<BVHNode>& outData
)
{
    outData = nullptr;

	std::ifstream infile(filePath, std::ios::binary);
	if (!infile.good()) {
		return FALSE;
	}

	try {
		outData = LoadBVH(infile);
		return TRUE;
	}
	catch (const std::exception& e) {
        outData = nullptr;
		return FALSE;
	}
}