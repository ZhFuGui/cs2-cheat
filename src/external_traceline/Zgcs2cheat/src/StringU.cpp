#include "../include/internal/StringU.h"

bool findMeshesBlock(const std::string& text, std::string& output) {
    size_t startPos = text.find("m_meshes");
    if (startPos == std::string::npos)
        return false;

    startPos = text.find('[', startPos);
    if (startPos == std::string::npos)
        return false;

    int braceDepth = 1;
    size_t contentStart = startPos + 1;
    size_t currentPos = contentStart;

    while (currentPos < text.size() && braceDepth > 0) {
        if (text[currentPos] == '[')
            braceDepth++;
        else if (text[currentPos] == ']')
            braceDepth--;
        currentPos++;
    }

    // 检查是否正常结束
    if (braceDepth != 0)
        return false;

    output = text.substr(contentStart, currentPos - contentStart - 1);
    return true;
}

bool extractStructs(const std::string& block, std::vector<std::string>& outputStructs) {
    outputStructs.clear(); // 清空输出缓冲区

    int braceDepth = 0;
    size_t structStartPos = std::string::npos;

    for (size_t i = 0; i < block.size(); ++i) {
        if (block[i] == '{') {
            if (braceDepth == 0)
                structStartPos = i;
            braceDepth++;
        }
        else if (block[i] == '}') {
            braceDepth--;
            if (braceDepth == 0 && structStartPos != std::string::npos) {
                outputStructs.push_back(block.substr(structStartPos, i - structStartPos + 1));
                structStartPos = std::string::npos;
            }
        }
    }

    // 检查括号是否完全匹配
    return braceDepth == 0;
}

bool parseCollisionAttributes(const std::string& pfileText, std::vector<int>& defaultIndices) {
    defaultIndices.clear();

    if (pfileText.empty()) {
        return false;
    }

    size_t startPos = pfileText.find("m_collisionAttributes");
    if (startPos == std::string::npos) {
        return false;
    }

    startPos = pfileText.find('[', startPos);
    if (startPos == std::string::npos)
        return false;

    int braceDepth = 1;
    size_t contentStart = startPos + 1;
    size_t currentPos = contentStart;

    while (currentPos < pfileText.size() && braceDepth > 0) {
        if (pfileText[currentPos] == '[')
            braceDepth++;
        else if (pfileText[currentPos] == ']')
            braceDepth--;
        currentPos++;
    }

    if (braceDepth != 0)
        return false;

    std::string collisionBlock = pfileText.substr(contentStart, currentPos - contentStart - 1);
    std::vector<std::string> collisionStructs;
    if (!extractStructs(collisionBlock, collisionStructs)) {
        return false;
    }

    for (size_t idx = 0; idx < collisionStructs.size(); idx++) {
        size_t strPos = collisionStructs[idx].find("m_CollisionGroupString");
        if (strPos != std::string::npos) {
            size_t eqPos = collisionStructs[idx].find("=", strPos);
            if (eqPos == std::string::npos)
                continue;
            eqPos++;
            while (eqPos < collisionStructs[idx].size() &&
                (std::isspace(collisionStructs[idx][eqPos]) || collisionStructs[idx][eqPos] == '"'))
                eqPos++;
            size_t endPos = collisionStructs[idx].find_first_of("\"", eqPos);
            if (endPos == std::string::npos)
                continue;
            std::string groupStr = collisionStructs[idx].substr(eqPos, endPos - eqPos);
            std::transform(groupStr.begin(), groupStr.end(), groupStr.begin(), ::tolower);
            if (groupStr == "default") {
                defaultIndices.push_back(static_cast<int>(idx));
            }
        }
    }

    return true;
}

bool findParam(const std::string& block, const std::string& key, int& value) {
    size_t keyPos = block.find(key);
    if (keyPos == std::string::npos)
        return false;

    size_t eqPos = block.find("=", keyPos);
    if (eqPos == std::string::npos)
        return false;

    eqPos++; // 跳过等号

    // 跳过空白字符
    while (eqPos < block.size() && std::isspace(block[eqPos]))
        eqPos++;

    // 解析整数值
    size_t endPos = eqPos;
    while (endPos < block.size() && std::isdigit(block[endPos]))
        endPos++;

    if (endPos == eqPos) // 没有找到数字
        return false;

    try {
        value = std::stoi(block.substr(eqPos, endPos - eqPos));
        return true;
    }
    catch (...) {
        return false;
    }
}

bool extractInnerMesh(const std::string& meshBlock, std::string& output) {
    size_t keyPos = meshBlock.find("m_Mesh");
    if (keyPos == std::string::npos)
        return false;

    size_t bracePos = meshBlock.find('{', keyPos);
    if (bracePos == std::string::npos)
        return false;

    int braceDepth = 0;
    size_t startPos = bracePos;
    size_t currentPos = bracePos;

    for (; currentPos < meshBlock.size(); ++currentPos) {
        if (meshBlock[currentPos] == '{')
            braceDepth++;
        else if (meshBlock[currentPos] == '}')
            braceDepth--;

        if (braceDepth == 0)
            break;
    }

    // 检查是否正常结束
    if (currentPos >= meshBlock.size())
        return false;

    output = meshBlock.substr(startPos, currentPos - startPos + 1);
    return true;
}

bool extractHexBlob(const std::string& block, const std::string& key, std::string& hexBlob) {
    size_t keyPos = block.find(key);
    if (keyPos == std::string::npos)
        return false;

    size_t startPos = block.find("#[", keyPos);
    if (startPos == std::string::npos)
        return false;

    startPos += 2; // 跳过 "#["

    size_t endPos = block.find(']', startPos);
    if (endPos == std::string::npos)
        return false;

    hexBlob = block.substr(startPos, endPos - startPos);

    // 移除空白字符
    hexBlob.erase(std::remove_if(hexBlob.begin(), hexBlob.end(),
        [](unsigned char c) { return std::isspace(c); }),
        hexBlob.end());

    return true;
}

bool extractArrayContent(const std::string& block, const std::string& key, std::string& output) {
    size_t keyPos = block.find(key);
    if (keyPos == std::string::npos)
        return false;

    size_t startPos = block.find('[', keyPos);
    if (startPos == std::string::npos)
        return false;

    size_t endPos = block.find(']', startPos);
    if (endPos == std::string::npos)
        return false;

    output = block.substr(startPos + 1, endPos - startPos - 1);
    return true;
}

bool findHullsBlock(const std::string& text, std::string& output) {
    size_t keyPos = text.find("m_hulls");
    if (keyPos == std::string::npos)
        return false;

    size_t startPos = text.find('[', keyPos);
    if (startPos == std::string::npos)
        return false;

    int braceDepth = 1;
    size_t contentStart = startPos + 1;
    size_t currentPos = contentStart;

    while (currentPos < text.size() && braceDepth > 0) {
        if (text[currentPos] == '[')
            braceDepth++;
        else if (text[currentPos] == ']')
            braceDepth--;
        currentPos++;
    }

    // 检查是否正常结束
    if (braceDepth != 0)
        return false;

    output = text.substr(contentStart, currentPos - contentStart - 1);
    return true;
}

bool extractInnerBlock(const std::string& block, const std::string& key, std::string& output) {
    size_t keyPos = block.find(key);
    if (keyPos == std::string::npos)
        return false;

    size_t bracePos = block.find('{', keyPos);
    if (bracePos == std::string::npos)
        return false;

    int braceDepth = 1; // 从1开始，因为已经找到第一个 '{'
    size_t startPos = bracePos;
    size_t currentPos = bracePos + 1; // 从 '{' 后面开始

    while (currentPos < block.size() && braceDepth > 0) {
        if (block[currentPos] == '{')
            braceDepth++;
        else if (block[currentPos] == '}')
            braceDepth--;
        currentPos++;
    }

    // 检查是否正常结束
    if (braceDepth != 0)
        return false;

    output = block.substr(startPos, currentPos - startPos);
    return true;
}