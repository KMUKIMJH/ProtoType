/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Background.h
Project:    CS230 Engine
Author:     Minchan Cho
Created:    March 31, 2023
*/

#pragma once
#include <filesystem>
#include "../Engine/Texture.h"
#include "../Engine/Camera.h"
#include "../Engine/Component.h"

class Background : public CS230::Component {
public:
    void Add(const std::filesystem::path& texture_path, Math::vec2 speed);
    void Unload();
    void Draw(const CS230::Camera& camera);
   // void Draw(Math::TransformationMatrix& camera_matrix);
    Math::ivec2 GetSize();
private:
    struct ParallaxLayer {
        std::shared_ptr<CS230::Texture> texture;
        Math::vec2 speed{1.0, 1.0};
    };

    std::vector<ParallaxLayer> backgrounds;
};
