#include "PlayerHUD.h"
#include "Player.h"
#include "Weapon.h"
#include "../Engine/Engine.h"
#include "../Engine/TextureManager.h"
#include "../OpenGL/DrawUtils2D.h"
#include <GL/glew.h>
#include <cstdio>
#include <algorithm>
#include <string>

void PlayerHUD::Draw(const CS230::Player& player)
{
    if (!hpIconTex) hpIconTex = Engine::GetTextureManager().Load("Assets/image/Interface/HP_Gage_.png");
    if (!hpFullTex) hpFullTex = Engine::GetTextureManager().Load("Assets/image/Interface/HP_Gage.png");
    if (!hpEmptyTex) hpEmptyTex = Engine::GetTextureManager().Load("Assets/image/Interface/HP_Gage_Gray.png");
    if (!gaugeBgTex) gaugeBgTex = Engine::GetTextureManager().Load("Assets/image/Interface/PR_Gage_Gray.png");
    if (!gaugeFillTex) gaugeFillTex = Engine::GetTextureManager().Load("Assets/image/Interface/PR_Gage.png");
    if (!scrapIconTex) scrapIconTex = Engine::GetTextureManager().Load("Assets/image/Objects/GearScrap.png");

    int scrap = player.GetScrap();
    if (scrap != lastScrap || !scrapTex)
    {
        lastScrap = scrap;
        char buf[32];
        sprintf_s(buf, "X %d", scrap);
        scrapTex = Engine::GetFont(0).PrintToTextureScaled(buf, 0xFFD700FF, 0.7);
    }

    int wt = -1;
    int currentAmmo = -1;
    int maxAmmo = -1;

    if (player.GetWeapon())
    {
        wt = static_cast<int>(player.GetWeapon()->GetType());
        if (wt == static_cast<int>(CS230::WeaponType::Arrow))
        {
            if (auto* arrow = dynamic_cast<CS230::ArrowWeapon*>(player.GetWeapon()))
            {
                currentAmmo = arrow->GetAmmo();
                maxAmmo = arrow->GetMaxAmmo();
            }
        }
    }

    bool needsWepTexUpdate = false;
    if (wt != lastWeaponType) needsWepTexUpdate = true;
    if (wt == static_cast<int>(CS230::WeaponType::Arrow) && currentAmmo != lastAmmoCount) needsWepTexUpdate = true;
    if (wt == static_cast<int>(CS230::WeaponType::Arrow) && !wepTex) needsWepTexUpdate = true;

    if (needsWepTexUpdate)
    {
        lastWeaponType = wt;
        lastAmmoCount = currentAmmo;

        if (wt == static_cast<int>(CS230::WeaponType::Arrow) && currentAmmo >= 0)
        {
            char wbuf[64];
            sprintf_s(wbuf, "Arrows : %d / %d", currentAmmo, maxAmmo);
            wepTex = Engine::GetFont(0).PrintToTextureScaled(wbuf, 0xFFFFFFFF, 0.7);
        }
        else if (wt == static_cast<int>(CS230::WeaponType::Sword))
        {
            wepTex = Engine::GetFont(0).PrintToTextureScaled("Weapon : Sword", 0xFFFFFFFF, 0.7);
        }
        else if (wt == static_cast<int>(CS230::WeaponType::Hammer))
        {
            wepTex = Engine::GetFont(0).PrintToTextureScaled("Weapon : Hammer", 0xFFFFFFFF, 0.7);
        }
        else
        {
            wepTex = nullptr;
        }
    }

    if (!rollLabelTex)
    {
        rollLabelTex = Engine::GetFont(0).PrintToTextureScaled("Roll:", 0xFFFFFFFF, 0.7);
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    double startX = 20.0;
    double scale = 1.0;

    double maxH = 0.0;
    if (hpIconTex && hpFullTex)
    {
        double iconYSize = static_cast<double>(hpIconTex->GetSize().y);
        double fullYSize = static_cast<double>(hpFullTex->GetSize().y);
        maxH = std::max(iconYSize, fullYSize) * scale;
    }

    double currentY = 1080.0 - 20.0 - maxH;

    double hpBarStartX = startX;
    double hpBarY = currentY;
    double iconY = currentY;

    if (hpIconTex && hpFullTex && hpEmptyTex)
    {
        Math::ivec2 iconSize = hpIconTex->GetSize();
        Math::ivec2 gageSize = hpFullTex->GetSize();

        iconY = currentY + (maxH - static_cast<double>(iconSize.y) * scale) / 2.0;
        hpBarY = currentY + (maxH - static_cast<double>(gageSize.y) * scale) / 2.0;

        hpIconTex->Draw(Math::TranslationMatrix(Math::vec2{ startX, iconY }) * Math::ScaleMatrix(Math::vec2{ scale, scale }));

        hpBarStartX = startX + (static_cast<double>(iconSize.x) * scale) - 20.0;
        double cursorX = hpBarStartX;

        int hp = player.GetHealth();
        int maxHP = player.combat.maxHealth;

        for (int i = 0; i < maxHP; ++i)
        {
            if (i < hp) hpFullTex->Draw(Math::TranslationMatrix(Math::vec2{ cursorX, hpBarY }) * Math::ScaleMatrix(Math::vec2{ scale, scale }));
            else hpEmptyTex->Draw(Math::TranslationMatrix(Math::vec2{ cursorX, hpBarY }) * Math::ScaleMatrix(Math::vec2{ scale, scale }));

            cursorX += (static_cast<double>(gageSize.x) * scale) - 40.0;
        }
    }

    if (gaugeBgTex && gaugeFillTex)
    {
        Math::ivec2 bgSize = gaugeBgTex->GetSize();
        Math::ivec2 fillSize = gaugeFillTex->GetSize();

        double parryY = hpBarY - (static_cast<double>(bgSize.y) * scale) + 0.0;
        double parryX = hpBarStartX;

        gaugeBgTex->Draw(Math::TranslationMatrix(Math::vec2{ parryX, parryY }) * Math::ScaleMatrix(Math::vec2{ scale, scale }));

        double timeRatio = player.GetTimeGaugeRatio();
        timeRatio = std::clamp(timeRatio, 0.0, 1.0);

        if (timeRatio > 0.0)
        {
            double offsetX = (static_cast<double>(bgSize.x) - static_cast<double>(fillSize.x)) * scale / 2.0;
            double offsetY = (static_cast<double>(bgSize.y) - static_cast<double>(fillSize.y)) * scale / 2.0;

            int drawWidth = static_cast<int>(static_cast<double>(fillSize.x) * timeRatio);

            if (drawWidth > 0)
            {
                unsigned int tintColor = player.IsSlowMoActive() ? 0xFF0000FF : 0xFFFFFFFF;
                gaugeFillTex->Draw(
                    Math::TranslationMatrix(Math::vec2{ parryX + offsetX, parryY + offsetY }) * Math::ScaleMatrix(Math::vec2{ scale, scale }),
                    Math::ivec2{ 0, 0 },
                    Math::ivec2{ drawWidth, fillSize.y },
                    tintColor
                );
            }
        }

        double lowestY = currentY;
        if (hpIconTex) lowestY = std::min(lowestY, iconY);
        lowestY = std::min(lowestY, parryY);

        currentY = lowestY - 45.0;
    }
    else
    {
        currentY -= 45.0;
    }

    if (scrapIconTex && scrapTex)
    {
        Math::ivec2 sSize = scrapIconTex->GetSize();
        double sScale = 30.0 / static_cast<double>(sSize.y);

        scrapIconTex->Draw(Math::TranslationMatrix(Math::vec2{ startX, currentY }) * Math::ScaleMatrix(Math::vec2{ sScale, sScale }));
        scrapTex->Draw(Math::TranslationMatrix(Math::vec2{ startX + (static_cast<double>(sSize.x) * sScale) + 15.0, currentY - 5.0 }));

        currentY -= 40.0;
    }

    if (wepTex)
    {
        wepTex->Draw(Math::TranslationMatrix(Math::vec2{ startX, currentY }));
        currentY -= 40.0;
    }

    auto drawBar = [&](const std::shared_ptr<CS230::Texture>& labelTex, double ratio, unsigned int fillColor, unsigned int backColor = 0x808080FF)
        {
            const double barW = 100.0;
            const double barH = 12.0;

            if (labelTex)
            {
                labelTex->Draw(Math::TranslationMatrix(Math::vec2{ startX, currentY }));

                const double barStartX = startX + static_cast<double>(labelTex->GetSize().x) + 10.0;
                const double barStartY = currentY + (static_cast<double>(labelTex->GetSize().y) * 0.5) - (barH * 0.5);

                Engine::GetRenderer2D().DrawRectangle(
                    Math::TranslationMatrix(Math::vec2{ barStartX + barW * 0.5, barStartY + barH * 0.5 }) * Math::ScaleMatrix(Math::vec2{ barW, barH }),
                    backColor, backColor, 0.0
                );

                const double fillW = barW * std::clamp(ratio, 0.0, 1.0);
                if (fillW > 0.0)
                {
                    Engine::GetRenderer2D().DrawRectangle(
                        Math::TranslationMatrix(Math::vec2{ barStartX + fillW * 0.5, barStartY + barH * 0.5 }) * Math::ScaleMatrix(Math::vec2{ fillW, barH }),
                        fillColor, fillColor, 0.0
                    );
                }
            }
            currentY -= 40.0;
        };

    double rollRatio = 1.0;
    if (player.GetRollCooldownMax() > 0.0)
    {
        rollRatio = 1.0 - (player.GetRollCooldown() / player.GetRollCooldownMax());
    }
    if (rollRatio < 1.0)
    {
        drawBar(rollLabelTex, rollRatio, 0xFFA500FF);
    }
}