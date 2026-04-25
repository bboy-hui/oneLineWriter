#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <imgui.h>
#include <vulkan/vulkan.h>

class FontManager
{
public:
    struct FontInfo
    {
        std::string name;
        std::string path;
        ImFont* font = nullptr;
        float size = 18.0f;
    };

    static FontManager& Instance();

    void Initialize();
    void Shutdown();

    ImFont* AddFont(const std::string& name, const std::string& path, float size = 18.0f);
    ImFont* AddFont(const std::string& name, const std::string& path, float size, bool chinese_only);
    ImFont* GetFont(const std::string& name);
    void SetDefaultFont(const std::string& name);

    const std::vector<FontInfo>& GetAllFonts() const { return m_Fonts; }
    const std::string& GetDefaultFontName() const { return m_DefaultFontName; }

    // Build fonts texture for Vulkan
    bool BuildFontTexture(VkCommandBuffer command_buffer);
    void DestroyFontTexture();

private:
    FontManager() = default;
    ~FontManager() = default;
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    std::vector<FontInfo> m_Fonts;
    std::unordered_map<std::string, size_t> m_FontIndex;
    std::string m_DefaultFontName;
    bool m_Initialized = false;
};
