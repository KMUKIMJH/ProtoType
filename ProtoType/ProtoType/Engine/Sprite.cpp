/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Sprite.cpp
Project:    CS230 Engine
Author:     Minchan Cho
Created:    March 11, 2025
*/

#include "Sprite.h"
#include "Engine.h"

CS230::Sprite::Sprite(const std::filesystem::path& sprite_file, GameObject* object) :object(object)
{
    Load(sprite_file);
}


CS230::Sprite::~Sprite()
{
	for (Animation* animation : animations) 
    {
		delete animation;
	}
	animations.clear();
}


Math::ivec2 CS230::Sprite::GetHotSpot(int index)
{
	if (index < 0 || index >= hotspots.size()) {
		Engine::GetLogger().LogError("Invalid hotspot index: " + std::to_string(index));
		return Math::ivec2(0, 0);
	}
	return hotspots[index];
}


void CS230::Sprite::Draw(Math::TransformationMatrix display_matrix, unsigned int tint) {
    texture->Draw(display_matrix * Math::TranslationMatrix(-GetHotSpot(0)), GetFrameTexel(animations[current_animation]->CurrentFrame()), GetFrameSize(), tint);
}


void CS230::Sprite::Load(const std::filesystem::path& sprite_file) {
    animations.clear();
    hotspots.clear();
    frame_texels.clear();

    if (sprite_file.extension() != ".spt") {
        throw std::runtime_error(sprite_file.generic_string() + " is not a .spt file");
    }
    std::ifstream in_file(sprite_file);

    if (in_file.is_open() == false) {
        throw std::runtime_error("Failed to load " + sprite_file.generic_string());
    }

    std::string text;
    in_file >> text;
    texture = Engine::GetTextureManager().Load(text);
    frame_size = texture->GetSize();

    in_file >> text;
    while (in_file.eof() == false) {
        if (text == "FrameSize") {
            in_file >> frame_size.x;
            in_file >> frame_size.y;
        }
        else if (text == "NumFrames") {
            int frame_count;
            in_file >> frame_count;
            for (int i = 0; i < frame_count; i++) {
                frame_texels.push_back({ frame_size.x * i, 0 });
            }
        }
        else if (text == "Frame") {
            int frame_location_x, frame_location_y;
            in_file >> frame_location_x;
            in_file >> frame_location_y;
            frame_texels.push_back({ frame_location_x, frame_location_y });
        }
        else if (text == "HotSpot") {
            int hotspot_x, hotspot_y;
            in_file >> hotspot_x;
            in_file >> hotspot_y;
            hotspots.push_back({ hotspot_x, hotspot_y });
        }
        else if (text == "Anim") {
            in_file >> text;
            animations.push_back(new Animation(text));
        }
        else if (text == "RectCollision") {
            Math::irect boundary;
            in_file >> boundary.point_1.x >> boundary.point_1.y >> boundary.point_2.x >> boundary.point_2.y;
            if (object == nullptr) {
                Engine::GetLogger().LogError("Cannot add collision to a null object");
            }
            else {
                object->AddGOComponent(new RectCollision(boundary, object));
            }
        }
        else if (text == "CircleCollision") {
            double radius;
            in_file >> radius;
            if (object == nullptr) {
                Engine::GetLogger().LogError("Cannot add collision to a null object");
            }
            else {
                object->AddGOComponent(new CircleCollision(radius, object));
            }
        }
        else {
            Engine::GetLogger().LogError("Unknown command: " + text);
        }
        in_file >> text;
    }

    if (frame_texels.empty() == true) {
        frame_texels.push_back({ 0,0 });
    }

    if (animations.empty()) {
        animations.push_back(new Animation());
        PlayAnimation(0);
    }
}

Math::ivec2 CS230::Sprite::GetFrameSize()
{
    return frame_size;
}


Math::ivec2 CS230::Sprite::GetFrameTexel(int index) const
{
   if (index < 0 || index >= frame_texels.size()) {
       Engine::GetLogger().LogError("Invalid frame texel index: " + std::to_string(index));
       return { 0, 0 };
   }
   return frame_texels[index];
}

void CS230::Sprite::PlayAnimation(int animation)
{
    if (animation < 0 || animation >= animations.size())
    {
        Engine::GetLogger().LogError("animation doesn't exist: " + std::to_string(animation));
        current_animation = 0;
        return;
    }
    else 
    {
        current_animation = animation;
        animations[animation]->Reset();
    }
}

bool CS230::Sprite::AnimationEnded()
{
    return animations[current_animation]->Ended();
}

void CS230::Sprite::Update(double dt)
{
    animations[current_animation]->Update(dt);
}

CS230::Sprite::Sprite(Sprite&& temporary) noexcept :
    texture(std::move(temporary.texture)),
    hotspots(std::move(temporary.hotspots)),
    current_animation(temporary.current_animation),
    frame_size(temporary.frame_size),
    frame_texels(std::move(temporary.frame_texels)),
    animations(std::move(temporary.animations))
{
}

CS230::Sprite& CS230::Sprite::operator=(Sprite&& temporary) noexcept {
    std::swap(texture, temporary.texture);
    std::swap(hotspots, temporary.hotspots);
    std::swap(current_animation, temporary.current_animation);
    std::swap(frame_size, temporary.frame_size);
    std::swap(frame_texels, temporary.frame_texels);
    std::swap(animations, temporary.animations);
    return *this;
}
