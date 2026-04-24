// All rights reserved by Daniil Grigoriev.
#include "UI/ImGuiManager.h"
#include "Config/Config.h"
#include "Utility.h"
#include "POTranlator.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <Shlobj.h>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <thread>
#include "ImGuiManager.h"

ImGuiManager::ImGuiManager()
    : configChanged(false), translationProgress(0), isTranslating(false), isTranslationCancelled(false), showConfigDialog(false), showAboutDialog(false)
{
    // Load default config
    defaultConfig = getDefaultConfig();
    currentConfig = defaultConfig;
}

ImGuiManager &ImGuiManager::Instance()
{
    static ImGuiManager instance;
    return instance;
}

ImGuiManager::~ImGuiManager()
{
    Shutdown();
}

void ImGuiManager::Initialize()
{
    // Load config on startup
    LoadConfig();

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    isInitialized = true;
}

void ImGuiManager::Shutdown()
{
    if (isInitialized)
    {
        isInitialized = false;
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}

void ImGuiManager::LoadConfig()
{
    try
    {
        currentConfig = loadConfig("config.json");
        configChanged = false;
        AddLogMessage("Config loaded successfully");
    }
    catch (const std::exception &ex)
    {
        currentConfig = defaultConfig;
        configChanged = false;
        AddLogMessage(std::string("Failed to load config, using defaults: ") + ex.what());
    }
}

void ImGuiManager::SaveConfig()
{
    try
    {
        saveConfig(currentConfig, "config.json");
        configChanged = false;
        AddLogMessage("Config saved successfully");
    }
    catch (const std::exception &ex)
    {
        AddLogMessage(std::string("Failed to save config: ") + ex.what());
    }
}

void ImGuiManager::ApplyConfig()
{
    currentConfig = defaultConfig;
    configChanged = false;
    AddLogMessage("Config reset to defaults");
}

void ImGuiManager::RenderMainWindow()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGui::Begin("AIPO Translator", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    static ImGuiTabBarFlags tabFlags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("##Tabs", tabFlags))
    {
        if (ImGui::BeginTabItem("Settings"))
        {
            RenderSettingsTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Translation"))
        {
            RenderTranslationTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Log"))
        {
            RenderLogTab();
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("About"))
        {
            RenderAboutTab();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void ImGuiManager::RenderSettingsTab()
{
    UpdateConfigUI();

    ImGui::SeparatorText("API Settings");

    ImGui::Text("API URL:");
    ImGui::SameLine();
    ImGui::PushItemWidth(400);
    if (ImGui::InputText("##APIUrl", currentConfig.apiUrl.data(), currentConfig.apiUrl.size() + 1, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        // Note: This is a simplified version - in production, use proper string handling
        configChanged = true;
    }
    ImGui::PopItemWidth();

    ImGui::Text("Model:");
    ImGui::SameLine();
    ImGui::PushItemWidth(400);
    if (ImGui::InputText("##Model", currentConfig.model.data(), currentConfig.model.size() + 1, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        configChanged = true;
    }
    ImGui::PopItemWidth();

    ImGui::Text("Prompt:");
    ImGui::SameLine();
	ImGui::PushItemWidth(400);
    if (ImGui::InputTextMultiline("##Prompt", currentConfig.prompt.data(),currentConfig.prompt.capacity() + 1,ImVec2(-FLT_MIN, 150), ImGuiInputTextFlags_WordWrap))
    {
        configChanged = true;
    }
	ImGui::PopItemWidth();

    ImGui::Text("Overwrite Original Files:");
    ImGui::SameLine();
    ImGui::PushItemWidth(400);
    ImGui::Checkbox("##Overwrite", &currentConfig.overwriteOriginalFiles);
    ImGui::PopItemWidth();
    configChanged = true;

    ImGui::Separator();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.2f, 1.0f));
    if (ImGui::Button("Save Settings", ImVec2(150, 0)))
    {
        SaveConfig();
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
    if (ImGui::Button("Reset to Defaults", ImVec2(150, 0)))
    {
        ApplyConfig();
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();

    if (ImGui::Button("Load Settings", ImVec2(150, 0)))
    {
        LoadConfig();
    }

    if (configChanged)
    {
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "* Unsaved changes");
    }
}

void ImGuiManager::RenderTranslationTab()
{
    UpdateTranslationUI();

    ImGui::SeparatorText("Folder Selection");

    ImGui::PushItemWidth(400);
    if (ImGui::Button("Select Folder", ImVec2(150, 0)))
    {
        // Open folder picker dialog
        BROWSEINFOW bi = {0};
        wchar_t path[MAX_PATH];

        bi.lpszTitle = L"Select folder containing PO files";
        bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
        bi.pszDisplayName = path;

        LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
        if (pidl != NULL)
        {
            if (SHGetPathFromIDListW(pidl, path))
            {
                // Convert wchar_t to std::string
                char charPath[MAX_PATH];
                WideCharToMultiByte(CP_UTF8, 0, path, -1, charPath, MAX_PATH, NULL, NULL);
                SelectFolder(std::string(charPath));
            }
            CoTaskMemFree(pidl);
        }
    }
    ImGui::PopItemWidth();

    if (!selectedFolderPath.empty())
    {
        ImGui::Text("Selected: %s", selectedFolderPath.c_str());
        ImGui::Text("Found PO files: %zu", poFiles.size());
    }

    ImGui::Separator();

    if (!poFiles.empty())
    {
        ImGui::Text("PO Files:");
        ImGui::PushItemWidth(400);
        ImGui::BeginChild("POFilesList", ImVec2(0, 200), true);
        for (const auto &file : poFiles)
        {
            ImGui::Text("  %s", (std::filesystem::path(file).parent_path().filename().string() + "/" + std::filesystem::path(file).filename().string()).c_str());
        }
        ImGui::EndChild();
        ImGui::PopItemWidth();
    }

    ImGui::Separator();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f));
    if (ImGui::Button("Start Translation", ImVec2(200, 0)))
    {
        StartTranslation();
    }
    ImGui::PopStyleColor();

    ImGui::SameLine();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    if (ImGui::Button("Cancel", ImVec2(100, 0)))
    {
        CancelTranslation();
    }
    ImGui::PopStyleColor();

    // Progress bar
    if (isTranslating)
    {
        ImGui::Text("Progress: %zu%%", translationProgress);
        ImGui::ProgressBar(static_cast<float>(translationProgress) / 100.0f, ImVec2(0, 0),
                           (currentTranslationFile.empty() ? "Translating..." : (std::string("Translating: ") + currentTranslationFile).c_str()));
    }
    else if (translationProgress >= 100)
    {
        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "Translation completed!");
        translationProgress = 0;
    }
}

void ImGuiManager::RenderLogTab()
{
    ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 100);
    if (ImGui::Button("Clear Log", ImVec2(100, 0)))
    {
        ClearLog();
    }

    ImGui::SameLine();
    static bool autoScroll = true;
    ImGui::Checkbox("Auto-scroll", &autoScroll);
    ImGui::PopItemWidth();

    ImGui::Separator();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));
    ImGui::BeginChild("LogWindow", ImVec2(0, 0), true,
                      ImGuiWindowFlags_HorizontalScrollbar);

    std::lock_guard<std::mutex> lock(logMutex);
    for (const auto &message : logMessages)
    {
        ImGui::TextUnformatted(message.c_str());
    }

    if (autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void ImGuiManager::RenderAboutTab()
{
    ImGui::Text("AIPO Translator");
    ImGui::Separator();
    ImGui::Text("Version 1.0");
    ImGui::Text("A GUI application for translating .po files using AI");

    ImGui::Separator();
    ImGui::Text("Features:");
    ImGui::BulletText("Translate .po files using Ollama AI models");
    ImGui::BulletText("Support for multiple languages");
    ImGui::BulletText("Progress tracking and logging");
    ImGui::BulletText("Configurable API settings");

    ImGui::Separator();
    ImGui::Text("Requires:");
    ImGui::BulletText("Ollama running at localhost:11434");
    ImGui::BulletText("qwen3.5:35b model (or other compatible model)");
}

void ImGuiManager::UpdateConfigUI()
{
    // This function can be used to update UI elements based on config state
    // Currently empty as we handle updates directly in RenderSettingsTab
}

void ImGuiManager::UpdateTranslationUI()
{
    // This function can be used to update UI elements based on translation state
    // Currently empty as we handle updates directly in RenderTranslationTab
}

void ImGuiManager::UpdateTranslationProgress(size_t progress, const std::string &currentFile)
{

    if (isTranslationCancelled)
    {
        AddLogMessage("Translation cancelled by user");
        return;
    }

    translationProgress = progress;

    

    if (!isTranslationCancelled && translationProgress >= 100)
    {
        translationProgress = 100;
        AddLogMessage("Completed: " + currentFile);
    }
}
void ImGuiManager::SelectFolder(const std::string &folderPath)
{
    selectedFolderPath = folderPath;
    poFiles.clear();

    try
    {
        DiscoverPOFiles(folderPath, poFiles);
        AddLogMessage(std::string("Found ") + std::to_string(poFiles.size()) + " PO file(s) in " + folderPath);
    }
    catch (const std::exception &ex)
    {
        AddLogMessage(std::string("Error scanning folder: ") + ex.what());
    }
}

void ImGuiManager::StartTranslation()
{
    if (poFiles.empty() || selectedFolderPath.empty())
    {
        AddLogMessage("Error: No PO files found. Please select a folder first.");
        return;
    }

    if (isTranslating)
    {
        AddLogMessage("Translation already in progress");
        return;
    }

    isTranslating = true;
    isTranslationCancelled = false;
    translationProgress = 0;
    currentTranslationFile.clear();

    AddLogMessage("Starting translation of " + std::to_string(poFiles.size()) + " file(s)");

    // Start translation in a separate thread to avoid blocking UI
    std::thread([this]()
                {
        try
        {
            FindAllPO(std::filesystem::path(selectedFolderPath), currentConfig.overwriteOriginalFiles, [this](size_t progress, const std::string &currentFile) {UpdateTranslationProgress(progress, currentFile);}, isTranslationCancelled);
        }
        catch (const std::exception& ex)
        {
            AddLogMessage(std::string("Translation error: ") + ex.what());
        }

        isTranslating = false; })
        .detach();
}

void ImGuiManager::CancelTranslation()
{
    if (isTranslating)
    {
        isTranslationCancelled = true;
        AddLogMessage("Cancelling translation...");
    }
}

void ImGuiManager::SetTranslationProgress(size_t progress)
{
    translationProgress = progress;
}

void ImGuiManager::AddLogMessage(const std::string &message)
{
    std::lock_guard<std::mutex> lock(logMutex);
    logMessages.push_back(message);

    if (logMessages.size() > MAX_LOG_MESSAGES)
    {
        logMessages.erase(logMessages.begin());
    }
}

void ImGuiManager::ClearLog()
{
    std::lock_guard<std::mutex> lock(logMutex);
    logMessages.clear();
    AddLogMessage("Log cleared");
}
