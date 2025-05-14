#pragma once
#include <string>

// 文件处理类，负责读取和打开配置文件
class FileHandler {
public:
    // 打开文本文件
    static void OpenTextFile(const std::string& filePath);

    // 从 cs2Xray.ini 文件读取特征码
    static char* ReadSignature();
};