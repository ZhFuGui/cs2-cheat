#pragma once
#include <string>

// �ļ������࣬�����ȡ�ʹ������ļ�
class FileHandler {
public:
    // ���ı��ļ�
    static void OpenTextFile(const std::string& filePath);

    // �� cs2Xray.ini �ļ���ȡ������
    static char* ReadSignature();
};