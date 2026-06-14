
#pragma once

#include <filesystem>
#include <optional>
#include <SDL2/SDL.h>

namespace assets
{
    namespace detail
    {
        inline std::optional<std::filesystem::path> try_get_asset_path(const std::filesystem::path& starting_directory)
        {
            namespace fs = std::filesystem;
            fs::path       assets_parent = fs::absolute(starting_directory);
            const fs::path root = assets_parent.root_path();

            do
            {
                const fs::path assets_folder = assets_parent / "Assets";
                if (fs::is_directory(assets_folder))
                {
                    return assets_parent;
                }
                assets_parent = assets_parent.parent_path();
            } while (assets_parent != root);

            return std::nullopt;
        }
    }

    inline std::filesystem::path get_base_path()
    {
        namespace fs = std::filesystem;
        static fs::path assets_folder = []
        {
            if (auto result = detail::try_get_asset_path(fs::current_path()))
                return result.value();
            const auto base_path = SDL_GetBasePath();
            auto result = detail::try_get_asset_path(base_path);
            SDL_free(base_path);
            if (result)
                return result.value();
            throw std::runtime_error{ "Failed to find Assets folder in parent folders" };
        }();
        return assets_folder;
    }

    inline std::filesystem::path locate_asset(const std::filesystem::path& asset_path)
    {
        std::filesystem::path asset_filepath = asset_path;
        if (!std::filesystem::exists(asset_filepath))
        {
            asset_filepath = get_base_path() / asset_filepath;
            if (!std::filesystem::exists(asset_filepath))
            {
                throw std::runtime_error("Failed to locate asset: " + asset_path.string());
            }
        }
        return asset_filepath;
    }
}
