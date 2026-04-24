// All rights reserved by Daniil Grigoriev.
#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <functional>

using namespace std::filesystem;
using ProgressCallback = std::function<void(size_t, const std::string &)>;

void SplitString(const std::string &input, std::vector<std::string> &subStrings);

size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *userp);

void TranslatePO(const path &sourceFile, const path &sourceRoot, bool overwriteOriginal, const ProgressCallback &progressCallback = nullptr,const bool &bCancelled = false);

void FindAllPO(const path &sourceDir, bool overwriteOriginalFiles, const ProgressCallback &progressCallback = nullptr,const bool &bCancelled = false);

void DiscoverPOFiles(const path &sourceDir, std::vector<std::string> &poFiles);

void GetAllMsgstrs(const path &sourceFile, std::vector<std::string> &msgstrs);