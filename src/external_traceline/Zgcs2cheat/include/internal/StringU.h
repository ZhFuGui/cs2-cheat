#ifndef _F_STRING_U_H
#define _F_STRING_U_H

#include <string>
#include <filesystem>


bool findMeshesBlock(const std::string& text, std::string& output);

bool extractStructs(const std::string& block, std::vector<std::string>& outputStructs);

bool parseCollisionAttributes(const std::string& pfileText, std::vector<int>& defaultIndices);

bool findParam(const std::string& block, const std::string& key, int& value);

bool extractInnerMesh(const std::string& meshBlock, std::string& output);

bool extractHexBlob(const std::string& block, const std::string& key, std::string& hexBlob);

bool extractArrayContent(const std::string& block, const std::string& key, std::string& output);

bool findHullsBlock(const std::string& text, std::string& output);

bool extractInnerBlock(const std::string& block, const std::string& key, std::string& output);
#endif