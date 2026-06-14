/*
Copyright (C) 2023 DigiPen Institute of Technology
Reproduction or distribution of this file or its contents without
prior written consent is prohibited
File Name:  Background.cpp
Project:    CS230 Engine
Author:     Minchan Cho
Created:    March 31, 2023
*/
#include "Background.h"
#include "../Engine/Engine.h"

void Background::Add(const std::filesystem::path& texture_path, Math::vec2 speed)
{
    backgrounds.push_back(ParallaxLayer{ Engine::GetTextureManager().Load(texture_path), speed });
}

void Background::Unload()
{
    backgrounds.clear();
}

void Background::Draw(const CS230::Camera& camera)
{
    Math::vec2 camPos = camera.GetPosition();

    Math::ivec2 win = Engine::GetWindow().GetSize();
    Math::vec2 winD((double)win.x, (double)win.y);

    for (ParallaxLayer& layer : backgrounds)
    {
        auto tex = layer.texture;
        if (!tex)
        {
            continue;
        }

        Math::ivec2 texSize = tex->GetSize();
        Math::vec2 texSzD((double)texSize.x, (double)texSize.y);

        Math::vec2 scale{ 1.0, 1.0 };
        if (texSzD.x != 0 && texSzD.y != 0)
        {
            scale.x = winD.x / texSzD.x;
            scale.y = winD.y / texSzD.y;
        }

        Math::vec2 desired = -Math::vec2{ camPos.x * layer.speed.x, camPos.y * layer.speed.y };

        Math::vec2 scaledTex{ texSzD.x * scale.x, texSzD.y * scale.y };
        Math::vec2 maxNeg = -Math::vec2{ scaledTex.x - winD.x, scaledTex.y - winD.y };

        if (scaledTex.x <= winD.x)
        {
            desired.x = (winD.x - scaledTex.x) * 0.5;
        }
        else
        {
            if (desired.x < maxNeg.x)
            {
                desired.x = maxNeg.x;
            }
            if (desired.x > 0)
            {
                desired.x = 0;
            }
        }

        if (scaledTex.y <= winD.y)
        {
            desired.y = (winD.y - scaledTex.y) * 0.5;
        }
        else
        {
            if (desired.y < maxNeg.y)
            {
                desired.y = maxNeg.y;
            }
            if (desired.y > 0)
            {
                desired.y = 0;
            }
        }

        Math::TransformationMatrix mat = Math::TranslationMatrix(desired) * Math::ScaleMatrix(scale);
        tex->Draw(mat);
    }
}

Math::ivec2 Background::GetSize()
{
    if (backgrounds.empty())
    {
        return {0,0};
    }
    return backgrounds[backgrounds.size() - 1].texture->GetSize();
}