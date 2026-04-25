#include "FontManager.hpp"
#include <spdlog/spdlog.h>
#include <filesystem>

FontManager& FontManager::Instance()
{
    static FontManager instance;
    return instance;
}

void FontManager::Initialize()
{
    if (m_Initialized)
        return;

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();

    m_Fonts.clear();
    m_FontIndex.clear();

    m_Initialized = true;
    spdlog::info("FontManager initialized");
}

void FontManager::Shutdown()
{
    if (!m_Initialized)
        return;

    DestroyFontTexture();
    m_Fonts.clear();
    m_FontIndex.clear();
    m_Initialized = false;

    spdlog::info("FontManager shutdown");
}

ImFont* FontManager::AddFont(const std::string& name, const std::string& path, float size)
{
    return AddFont(name, path, size, true); // 默认加载中文字形
}

ImFont* FontManager::AddFont(const std::string& name, const std::string& path, float size, bool)
{
    if (!m_Initialized)
    {
        spdlog::error("FontManager not initialized");
        return nullptr;
    }

    // 检查文件是否存在
    if (!std::filesystem::exists(path))
    {
        spdlog::error("Font file not found: {}", path);
        return nullptr;
    }

    if (m_FontIndex.find(name) != m_FontIndex.end())
    {
        spdlog::warn("Font '{}' already exists", name);
        return m_Fonts[m_FontIndex[name]].font;
    }

    ImGuiIO& io = ImGui::GetIO();
    ImFontConfig font_config;
    font_config.OversampleH = 2;
    font_config.OversampleV = 2;
    font_config.PixelSnapH = true;

    // 中文字形范围
    static const ImWchar chinese_glyph_ranges[] = {
        0x0020, 0x007F, // Basic Latin
        0x00A0, 0x00FF, // Latin Extended
        0x2000, 0x206F, // General Punctuation
        0x3000, 0x30FF, // CJK Symbols
        0x4E00, 0x9FFF, // CJK Unified Ideographs
        0xFF00, 0xFFEF, // Halfwidth Forms
        0,
    };

    spdlog::info("Loading font: {} from {}", name, path);

    // 先加载一个简单的默认字体，确保 ImGui 能正常工作
    if (m_Fonts.empty())
    {
        ImFont* default_font = io.Fonts->AddFontDefault();
        if (default_font)
        {
            spdlog::info("Default font loaded");
        }
    }

    // 加载中文字体
    ImFont* font = io.Fonts->AddFontFromFileTTF(path.c_str(), size, &font_config, chinese_glyph_ranges);

    if (font == nullptr)
    {
        spdlog::error("Failed to load font: {} from {}", name, path);
        return nullptr;
    }

    FontInfo info;
    info.name = name;
    info.path = path;
    info.font = font;
    info.size = size;

    size_t index = m_Fonts.size();
    m_Fonts.push_back(info);
    m_FontIndex[name] = index;

    spdlog::info("Font loaded successfully: {} (glyphs loaded)", name);

    return font;
}

ImFont* FontManager::GetFont(const std::string& name)
{
    auto it = m_FontIndex.find(name);
    if (it == m_FontIndex.end())
    {
        spdlog::warn("Font '{}' not found", name);
        return nullptr;
    }
    return m_Fonts[it->second].font;
}

void FontManager::SetDefaultFont(const std::string& name)
{
    ImFont* font = GetFont(name);
    if (font != nullptr)
    {
        m_DefaultFontName = name;
        ImGui::GetIO().FontDefault = font;
        spdlog::info("Default font set to: {}", name);
    }
}

bool FontManager::BuildFontTexture(VkCommandBuffer)
{
    ImGuiIO& io = ImGui::GetIO();

    unsigned char* pixels = nullptr;
    int width = 0, height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    if (pixels == nullptr || width == 0 || height == 0)
    {
        spdlog::error("Font texture data is empty");
        return false;
    }

    spdlog::info("Font texture ready: {}x{}", width, height);
    return true;
}

void FontManager::DestroyFontTexture()
{
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->ClearTexData();
    spdlog::debug("Font texture destroyed");
}