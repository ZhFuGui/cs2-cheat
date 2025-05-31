#include "../include/internal/FileU.h"

bool readFile(const std::string& filename, std::string& content) {

    content.clear();

    if (!std::filesystem::exists(filename)) {
        errno = ENOENT;
        return false;
    }

    // ����Ƿ�Ϊ�����ļ�
    if (!std::filesystem::is_regular_file(filename)) {
        errno = EINVAL;
        return false;
    }

    // ��ȡ�ļ���С��Ԥ�����ڴ�
    std::uintmax_t fileSize = std::filesystem::file_size(filename);

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // ʹ��RAIIȷ���ļ��Զ��ر�
    struct FileCloser {
        std::ifstream* file;
        ~FileCloser() { if (file && file->is_open()) file->close(); }
    } fileCloser{ &file };

    // Ԥ�����㹻���ڴ�
    content.reserve(fileSize);

    // ��Ч��ȡ�ļ�����
    content.assign(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    // ����ȡ�������Ƿ�������
    return !file.bad();
}
