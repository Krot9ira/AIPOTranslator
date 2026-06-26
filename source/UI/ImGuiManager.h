// All rights reserved by Daniil Grigoriev.
#pragma once

#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include "Config/Config.h"

class ImGuiManager
{
public:
    static ImGuiManager &Instance();
    ~ImGuiManager();

    void Initialize();
    void Shutdown();
    void RenderMainWindow();

    // Settings management
    void LoadConfig();
    void SaveConfig();
    void ApplyConfig();

    // Translation management
    void SelectFolder(const std::string &folderPath);
    void StartTranslation();
    void CancelTranslation();

    // XLSX translation management
    void SelectXLSXFile(const std::string &filePath);
    void StartXLSXTranslation();

    // Progress tracking
    void SetTranslationProgress(size_t progress);
    void AddLogMessage(const std::string &message);
    void ClearLog();

    // Getters
    const Config &GetCurrentConfig() const { return currentConfig; }
    const std::vector<std::string> &GetPOFiles() const { return poFiles; }
    const std::vector<std::string> &GetLogMessages() const { return logMessages; }
    size_t GetTranslationProgress() const { return translationProgress; }
    bool IsTranslating() const { return isTranslating; }
    std::string GetCurrentTranslationFile() const { return currentTranslationFile; }

private:
    ImGuiManager();
    ImGuiManager(const ImGuiManager &) = delete;
    ImGuiManager &operator=(const ImGuiManager &) = delete;

    void RenderSettingsTab();
    void RenderTranslationTab();
    void RenderXLSXTranslationTab();
    void RenderLogTab();
    void RenderAboutTab();

    void UpdateConfigUI();
    void UpdateTranslationUI();

    // Config
    Config currentConfig;
    Config defaultConfig;
    bool configChanged;

    bool isInitialized = false;

    // Translation state
    std::vector<std::string> poFiles;
    std::string selectedFolderPath;
    std::string selectedXLSXFile;
    size_t translationProgress;
    std::string currentTranslationFile;
    bool isTranslating;
    bool isTranslationCancelled;
    void UpdateTranslationProgress(size_t progress, const std::string &currentFile);
    // Log
    std::vector<std::string> logMessages;
    std::mutex logMutex;
    static const size_t MAX_LOG_MESSAGES = 1000;

    // UI state
    bool showConfigDialog;
    bool showAboutDialog;
};
