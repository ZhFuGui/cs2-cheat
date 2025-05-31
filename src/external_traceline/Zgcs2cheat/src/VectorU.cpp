#include "../include/internal/VectorU.h"


bool hexStringToBytes(const std::string& hex, std::vector<unsigned char>& bytes) {

    bytes.clear();

    // 检查十六进制字符串长度是否为偶数
    if (hex.size() % 2 != 0)
        return false;

    bytes.reserve(hex.size() / 2);

    for (size_t i = 0; i < hex.size(); i += 2) {
        std::string byteStr = hex.substr(i, 2);
        unsigned int byte;

        // 验证字符是否为合法的十六进制字符
        for (char c : byteStr) {
            if (!std::isxdigit(c))
                return false;
        }

        try {
            byte = std::stoul(byteStr, nullptr, 16);
            bytes.push_back(static_cast<unsigned char>(byte));
        }
        catch (...) {
            return false;
        }
    }

    return true;
}

bool parseVertices(const std::string& hexBlob, std::vector<Vector3>& vertices) {

    std::vector<unsigned char> bytes ;

    if (!hexStringToBytes(hexBlob, bytes)) {
        return false;
    }

    // 验证字节数是否为3个float的倍数
    if (bytes.size() % (3 * sizeof(float)) != 0) {
        return false;
    }

    size_t numVerts = bytes.size() / (3 * sizeof(float));
    vertices.resize(numVerts);

    for (size_t i = 0; i < numVerts; ++i) {
        float v[3];
        std::memcpy(v, &bytes[i * 12], 12);
        vertices[i] = { v[0], v[1], v[2] };
    }

    return true;
}

bool parseTriangles(const std::string& hexBlob, std::vector<Triangle>& triangles) {

    std::vector<unsigned char> bytes;

    if (!hexStringToBytes(hexBlob, bytes)) {
        return false;
    }

    // 验证字节数是否为3个int的倍数
    if (bytes.size() % (3 * sizeof(int)) != 0) {
        return false;
    }

    size_t numTris = bytes.size() / (3 * sizeof(int));
    triangles.resize(numTris);

    for (size_t i = 0; i < numTris; ++i) {
        int idx[3];
        std::memcpy(idx, &bytes[i * 12], 12);
        triangles[i] = { idx[0], idx[1], idx[2] };
    }

    return true;
}

bool parseMaterials(const std::string& arrayContent, std::vector<int>& materials) {
    materials.clear();

    std::istringstream iss(arrayContent);
    int num;
    bool valid = true;

    while (iss >> num) {
        materials.push_back(num);

        // 检查是否还有更多数据
        if (iss.peek() == ',') {
            iss.ignore();
        }
        else if (iss.peek() != EOF) {
            // 非预期字符，解析失败
            valid = false;
            break;
        }
    }

    // 验证是否完全解析或字符串为空
    return valid && (iss.eof() || arrayContent.empty());
}

bool parseFloatArray(const std::string& arrayContent, std::vector<float>& values) {
    values.clear();

    std::istringstream iss(arrayContent);
    float num;
    bool valid = true;

    while (iss >> num) {
        values.push_back(num);

        // 检查流状态和下一个字符
        if (iss.peek() == EOF) {
            break; // 已到达流末尾
        }
        else if (iss.peek() == ',') {
            iss.ignore(); // 跳过逗号
        }
        else if (std::isspace(iss.peek())) {
            // 跳过空白字符
            while (std::isspace(iss.peek())) {
                iss.ignore();
            }
            // 如果跳过空白后是EOF或逗号，继续解析
            if (iss.peek() == EOF || iss.peek() == ',') {
                if (iss.peek() == ',') iss.ignore();
            }
            else {
                valid = false; // 非预期字符
                break;
            }
        }
        else {
            valid = false; // 非预期字符
            break;
        }
    }

    // 验证是否完全解析或字符串为空
    return valid && (iss.eof() || arrayContent.empty());
}

bool generateTrianglesFromAABB(const Vector3& min, const Vector3& max, int materialIndex, std::vector<TriangleCombined>& tris) {
    tris.clear();

    // 验证输入是否有效
    if (min.x > max.x || min.y > max.y || min.z > max.z) {
        return false;
    }

    Vector3 v0 = { min.x, min.y, min.z };
    Vector3 v1 = { max.x, min.y, min.z };
    Vector3 v2 = { max.x, max.y, min.z };
    Vector3 v3 = { min.x, max.y, min.z };
    Vector3 v4 = { min.x, min.y, max.z };
    Vector3 v5 = { max.x, min.y, max.z };
    Vector3 v6 = { max.x, max.y, max.z };
    Vector3 v7 = { min.x, max.y, max.z };

    auto addFace = [&tris, materialIndex](const Vector3& a, const Vector3& b, const Vector3& c, const Vector3& d) {
        TriangleCombined t1, t2;
        t1.v0 = a; t1.v1 = b; t1.v2 = c; t1.materialIndex = materialIndex;
        t2.v0 = a; t2.v1 = c; t2.v2 = d; t2.materialIndex = materialIndex;
        tris.push_back(t1);
        tris.push_back(t2);
        };

    addFace(v0, v1, v2, v3);
    addFace(v4, v5, v6, v7);
    addFace(v0, v1, v5, v4);
    addFace(v3, v2, v6, v7);
    addFace(v0, v3, v7, v4);
    addFace(v1, v2, v6, v5);

    return true;
}

void SerializeVector3(std::ofstream& out, const Vector3& v) {
    out.write(reinterpret_cast<const char*>(&v.x), sizeof(float));
    out.write(reinterpret_cast<const char*>(&v.y), sizeof(float));
    out.write(reinterpret_cast<const char*>(&v.z), sizeof(float));
}

void SerializeAABB(std::ofstream& out, const AABB& box) {
    SerializeVector3(out, box.min);
    SerializeVector3(out, box.max);
}