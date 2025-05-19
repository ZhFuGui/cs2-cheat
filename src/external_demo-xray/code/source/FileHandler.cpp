#include "../headers/FileHandler.h"
#include <Windows.h>
#include <fstream>
#include <algorithm>
#include <iostream>

// ���ı��ļ�
void FileHandler::OpenTextFile(const std::string& filePath) {
    ShellExecuteA(nullptr, "open", filePath.c_str(), nullptr, nullptr, SW_SHOW);
}

// �� cs2Xray.ini �ļ���ȡ������
char* FileHandler::ReadSignature() {

    // ��ȡ�û������ļ�·��
    char* userProfile = nullptr;
    size_t len = 0;
    if (_dupenv_s(&userProfile, &len, "USERPROFILE") != 0 || !userProfile) {
        MessageBoxW(nullptr, L"�޷���ȡ�û������ļ�·����", L"����", 0);
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
            MessageBoxW(nullptr, L"�ļ�Ϊ�գ�", L"����", 0);
            OpenTextFile(filePath);
            return nullptr;
        }

        // �Ƴ����пո�
        fileContent.erase(std::remove(fileContent.begin(), fileContent.end(), ' '), fileContent.end());

        // ��̬�����ڴ�
        char* result = new char[fileContent.size() + 1];
        if (!result) {
            MessageBoxW(nullptr, L"�ڴ����ʧ�ܣ�", L"����", 0);
            return nullptr;
        }

        std::copy(fileContent.begin(), fileContent.end(), result);
        result[fileContent.size()] = '\0';
        return result;
    }
    else {
        // ���û������ļ��򴴽����ļ�
        std::ofstream outFile(filePath);
        if (outFile.is_open()) {
            outFile.close();
        }
        MessageBoxW(nullptr, L"�ļ������ڣ��Ѵ��� cs2Xray.ini������д��������������С�", L"����", 0);
        OpenTextFile(filePath);
        return nullptr;
    }
}