
#pragma once
#include <filesystem>
#include <memory>
#include <unordered_map>
#include <vector>

namespace CS230
{
    class Texture;


    class TextureManager
    {
    public:

        [[nodiscard]] std::shared_ptr<Texture> Load(const std::filesystem::path& file_name);


        void Unload();

        void StartRenderTextureMode(int width, int height);

    
        std::shared_ptr<Texture> EndRenderTextureMode();

    private:
        std::unordered_map<std::filesystem::path, std::shared_ptr<Texture>> textures;
    };
}
