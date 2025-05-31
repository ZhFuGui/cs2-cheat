#ifndef _F_FILE_U_H
#define _F_FILE_U_H

#include <fstream>
#include <string>
#include <system_error>
#include <filesystem>

bool readFile(const std::string& filename, std::string& content);

#endif