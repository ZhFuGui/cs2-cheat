#include "../headers/FileHandler.h"
#include <Windows.h>
#include <fstream>
#include <algorithm>
#include <iostream>

// 打开文本文件
void FileHandler::OpenTextFile(const std::string& filePath) {
    ShellExecuteA(nullptr, "open", filePath.c_str(), nullptr, nullptr, SW_SHOW);
}

// 从 cs2Xray.ini 文件读取特征码
char* FileHandler::ReadSignature() {

    // 获取用户配置文件路径
    char* userProfile = nullptr;
    size_t len = 0;
    if (_dupenv_s(&userProfile, &len, "USERPROFILE") != 0 || !userProfile) {
        MessageBoxW(nullptr, L"无法获取用户配置文件路径！", L"错误", 0);
        return nullptr;
    }

    std::string filePath = std::string(userProfile) + "\\Documents\\cs2Xray.ini";
    free(userProfile);

    std::ifstream inFile(filePath);
    std::string fileContent;

    if (inFile.is_open()) {
        std::getline(inFile, fileContent);
        inFile.close();

        if (fileContent.empty()) {
            MessageBoxW(nullptr, L"文件为空！", L"错误", 0);
            OpenTextFile(filePath);
            return nullptr;
        }

        // 移除所有空格
        fileContent.erase(std::remove(fileContent.begin(), fileContent.end(), ' '), fileContent.end());

        // 动态分配内存
        char* result = new char[fileContent.size() + 1];
        if (!result) {
            MessageBoxW(nullptr, L"内存分配失败！", L"错误", 0);
            return nullptr;
        }

        std::copy(fileContent.begin(), fileContent.end(), result);
        result[fileContent.size()] = '\0';
        return result;
    }
    else {
        // 如果没有这个文件则创建新文件
        std::ofstream outFile(filePath);
        if (outFile.is_open()) {
            outFile.close();
        }
        MessageBoxW(nullptr, L"文件不存在，已创建 cs2Xray.ini。请填写特征码后重新运行。", L"错误", 0);
        OpenTextFile(filePath);
        return nullptr;
    }
}