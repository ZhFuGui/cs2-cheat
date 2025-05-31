#include "../include/internal/FileU.h"

bool readFile(const std::string& filename, std::string& content) {

    content.clear();

    if (!std::filesystem::exists(filename)) {
        errno = ENOENT;
        return false;
    }

    // 检查是否为常规文件
    if (!std::filesystem::is_regular_file(filename)) {
        errno = EINVAL;
        return false;
    }

    // 获取文件大小以预分配内存
    std::uintmax_t fileSize = std::filesystem::file_size(filename);

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    // 使用RAII确保文件自动关闭
    struct FileCloser {
        std::ifstream* file;
        ~FileCloser() { if (file && file->is_open()) file->close(); }
    } fileCloser{ &file };

    // 预分配足够的内存
    content.reserve(fileSize);

    // 高效读取文件内容
    content.assign(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    // 检查读取过程中是否发生错误
    return !file.bad();
}
