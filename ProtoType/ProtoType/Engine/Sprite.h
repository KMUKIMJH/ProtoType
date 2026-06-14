/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Sprite.h
Project:    CS230 Engine
Author:     Minchan Cho
Created:    April 14, 2025
*/

#pragma once
#include <string>
#include "Vec2.h"
#include "Matrix.h"
#include "Texture.h"
#include "Animation.h"
#include "Component.h"
#include "GameObject.h"

namespace CS230 {
    class GameObject;

    class Sprite : public Component {
    public:

        Sprite(const std::filesystem::path& sprite_file, GameObject* object);
        ~Sprite();

        Sprite(const Sprite&) = delete;
        Sprite& operator=(const Sprite&) = delete;

        Sprite(Sprite&& temporary) noexcept;
        Sprite& operator=(Sprite&& temporary) noexcept;

        void Update(double dt) override;
        void Load(const std::filesystem::path& sprite_file);
        void Draw(Math::TransformationMatrix display_matrix, unsigned int tint = 0xFFFFFFFFu);
        int CurrentAnimation() { return current_animation; }
        Math::ivec2 GetHotSpot(int index);
        Math::ivec2 GetFrameSize();
        std::shared_ptr<CS230::Texture> GetTexture() const { return texture; }
        Math::ivec2 GetFrameTexel(int index) const;
        int GetCurrentFrame() const { return animations[current_animation]->CurrentFrame(); }

        void PlayAnimation(int animation);
        bool AnimationEnded();
    private:
        std::shared_ptr<CS230::Texture> texture;
        GameObject* object;
        std::vector<Math::ivec2> hotspots;

        int current_animation;
        Math::ivec2 frame_size;
        std::vector<Math::ivec2> frame_texels;
        std::vector<Animation*> animations;
    };
}
