#include "../include/LineOfSightChecker.h"

bool RayIntersectsTriangle(const Vector3& rayOrigin, const Vector3& rayDir,const TriangleCombined& triangle, float& t) {
    const float EPSILON = 1e-7f;
    Vector3 edge1 = triangle.v1 - triangle.v0;
    Vector3 edge2 = triangle.v2 - triangle.v0;
    Vector3 h = rayDir.cross(edge2);
    float a = edge1.dot(h);
    if (a > -EPSILON && a < EPSILON)
        return false;
    float f = 1.0f / a;
    Vector3 s = rayOrigin - triangle.v0;
    float u = f * s.dot(h);
    if (u < 0.0f || u > 1.0f)
        return false;
    Vector3 q = s.cross(edge1);
    float v = f * rayDir.dot(q);
    if (v < 0.0f || u + v > 1.0f)
        return false;
    t = f * edge2.dot(q);
    return (t > EPSILON);
}

void CollectIntersections(const Vector3& rayOrigin, const Vector3& rayDir,float maxDistance, std::vector<std::pair<float, int>>& intersections,const BVHNode* root) {
    std::function<void(const BVHNode*)> traverse = [&](const BVHNode* node) {
        if (!node || !node->bounds.RayIntersects(rayOrigin, rayDir))
            return;
        if (node->IsLeaf()) {
            for (const auto& tri : node->triangles) {
                float t;
                if (RayIntersectsTriangle(rayOrigin, rayDir, tri, t)) {
                    if (t > 1e-7f && t < maxDistance) {
                        intersections.push_back(std::make_pair(t, tri.materialIndex));
                    }
                }
            }
        }
        else {
            if (node->left)
                traverse(node->left.get());
            if (node->right)
                traverse(node->right.get());
        }
        };
    traverse(root); // 使用参数 root
}

BOOL IsPointVisible(const Vector3& point1, const Vector3& point2, const BVHNode* root,std::vector<std::tuple<float, int, float, int>>& result) {
    result.clear();

    Vector3 rayDir = { point2.x - point1.x, point2.y - point1.y, point2.z - point1.z };
    float totalDistance = std::sqrt(rayDir.dot(rayDir));

    // 检查射线长度是否有效
    if (totalDistance <= 0.0f) {
        return false;
    }

    rayDir = { rayDir.x / totalDistance, rayDir.y / totalDistance, rayDir.z / totalDistance };
    std::vector<std::pair<float, int>> hits;
    CollectIntersections(point1, rayDir, totalDistance, hits, root);

    std::sort(hits.begin(), hits.end(), [](const std::pair<float, int>& a, const std::pair<float, int>& b) {
        return a.first < b.first;
        });

    float prevExitT = 0.0f;
    for (size_t i = 0; i + 1 < hits.size(); i += 2) {
        float entryT = hits[i].first;
        int entryMat = hits[i].second;
        float exitT = hits[i + 1].first;
        int exitMat = hits[i + 1].second;

        // 验证交点顺序是否正确
        if (entryT > exitT) {
            return false;
        }

        float gap = entryT - prevExitT;
        float thickness = exitT - entryT;
        result.push_back(std::make_tuple(gap, entryMat, thickness, exitMat));
        prevExitT = exitT;
    }

    return true;
}