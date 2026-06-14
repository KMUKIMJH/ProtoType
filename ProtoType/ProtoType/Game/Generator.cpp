#include "Generator.h"
#include "../Engine/Engine.h"
#include "../OpenGL/RGBA.h"
#include "../Engine/Texture.h"
#include "../Engine/Sprite.h"
#include <random>
#include <algorithm>

namespace CS230
{
    Generator::Generator(Math::vec2 pos) : GameObject(pos)
    {
        rect = Math::rect{ Math::vec2{ pos.x - 75.0, pos.y }, Math::vec2{ pos.x + 75.0, pos.y + 200.0 } };
        AddGOComponent(new CS230::Sprite("Assets/image/Objects/Generator.spt", this));
        allUpgrades = {
            // Movement
            { 1, "Movement Speed", "Movement Speed +10%", 7, true, -1, [](Player& p) { p.move.speedBonus += 0.1; } },
            { 2, "Jump Height", "Jump Height +10%", 7, true, -1, [](Player& p) { p.combat.jumpHeightBonus += 0.1; } },
            { 3, "Triple Jump", "Enable 3rd Jump", 13, false, -1, [](Player& p) { p.combat.tripleJump = true; } },

            // Hammer
            { 4, "Hammer Damage", "Hammer Damage +1", 12, true, static_cast<int>(WeaponType::Hammer), [](Player& p) { p.combat.hammerDamageBonus += 1; } },
            { 5, "Magnetic Charge", "Hammer Charge Pulls Enemies", 12, false, static_cast<int>(WeaponType::Hammer), [](Player& p) { p.combat.hammerPullOnCharge = true; } },
            { 6, "Extended Shaft", "Hammer Range +50%", 12, false, static_cast<int>(WeaponType::Hammer), [](Player& p) { p.combat.hammerRangeBonus += 0.5; } },

            // Arrow
            { 7, "Quiver Size", "Max Arrows +1", 13, true, static_cast<int>(WeaponType::Arrow), [](Player& p) { p.combat.arrowMaxAmmoBonus += 1; } },
            { 8, "Bow Melee Damage", "Bow Melee Damage +1", 11, true, static_cast<int>(WeaponType::Arrow), [](Player& p) { p.combat.arrowMeleeDamageBonus += 1; } },
            { 9, "Bow Melee Range", "Bow Melee Range +100%", 13, false, static_cast<int>(WeaponType::Arrow), [](Player& p) { p.combat.arrowMeleeRangeBonus += 1.0; } },

            // Sword
            { 10, "Extra Slash", "Sword Max Combo +1", 12, false, static_cast<int>(WeaponType::Sword), [](Player& p) { p.combat.swordComboBonus += 1; } },
            { 11, "Sword Damage", "Sword Damage +1", 12, true, static_cast<int>(WeaponType::Sword), [](Player& p) { p.combat.swordDamageBonus += 1; } },
            { 12, "Sword Range", "Sword Range +100%", 12, false, static_cast<int>(WeaponType::Sword), [](Player& p) { p.combat.swordRangeBonus += 1.0; } },

            // Misc
            { 13, "Slow-mo Duration", "Slow-mo Duration +2s", 7, true, -1, [](Player& p) { p.combat.slowMoDurationBonus += 2.0; } },
            { 14, "Parry Window", "Parry Window +0.5s", 7, true, -1, [](Player& p) { p.combat.parryWindowBonus += 0.5; } },
            { 15, "Gauge Efficiency", "Slow-mo Max Gauge -1 Slot", 7, true, -1, [](Player& p) { p.combat.slowMoGaugeReduction += 1; } }
        };
    }

    void Generator::Update(double dt)
    {
        GameObject::Update(dt);
    }

    void Generator::ToggleUI()
    {
        menuOpen = !menuOpen;
        if (menuOpen)
        {
            selectedIndex = 0;
            needsTextUpdate = true; 
        }
    }

    void Generator::RollUpgrades(Player* p)
    {
        currentOffers.clear();
        if (!p) return;

        int curWeap = p->GetWeapon() ? static_cast<int>(p->GetWeapon()->GetType()) : -1;

        std::vector<UpgradeOption> valid;
        for (const auto& up : allUpgrades)
        {
            if (up.reqWeapon != -1 && up.reqWeapon != curWeap) continue;

            if (!up.repeatable)
            {
                if (std::find(p->combat.purchasedUpgrades.begin(), p->combat.purchasedUpgrades.end(), up.id) != p->combat.purchasedUpgrades.end()) continue;
            }
            valid.push_back(up);
        }

        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(valid.begin(), valid.end(), g);

        for (size_t i = 0; i < 3 && i < valid.size(); ++i)
        {
            currentOffers.push_back(valid[i]);
        }

        needsTextUpdate = true; 
    }

    void Generator::UpdateUI(Player* p)
    {
        if (!menuOpen || !p) return;

        int previousIndex = selectedIndex;
        int previousOffersCount = static_cast<int>(currentOffers.size());

        if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::W))
        {
            selectedIndex--;
            if (selectedIndex < 0) selectedIndex = static_cast<int>(currentOffers.size()) - 1;
            Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
        }
        else if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::S))
        {
            selectedIndex++;
            if (selectedIndex >= static_cast<int>(currentOffers.size())) selectedIndex = 0;
            Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
        }
        else if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Enter) || Engine::GetInput().KeyJustPressed(CS230::Input::Keys::E))
        {
            if (currentOffers.empty())
            {
                menuOpen = false;
                return;
            }

            auto& sel = currentOffers[selectedIndex];
            if (p->GetScrap() >= sel.cost)
            {
                p->UseScrap(sel.cost);
                sel.applyFunc(*p);
                Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/coin.wav");

                if (!sel.repeatable) p->combat.purchasedUpgrades.push_back(sel.id);

                currentOffers.erase(currentOffers.begin() + selectedIndex);
                if (selectedIndex >= static_cast<int>(currentOffers.size()))
                {
                    selectedIndex = std::max(0, static_cast<int>(currentOffers.size()) - 1);
                }
            }
            else
            {
                Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/block.wav");
            }
        }
        else if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Escape))
        {
            menuOpen = false;
        }

        if (previousIndex != selectedIndex || previousOffersCount != static_cast<int>(currentOffers.size()) || p->GetScrap() != lastCachedScrap || needsTextUpdate)
        {
            RefreshCachedTexts(p);
            needsTextUpdate = false;
            lastCachedScrap = p->GetScrap();
        }
    }

    void Generator::Draw(Math::TransformationMatrix camM)
    {
        GameObject::Draw(camM);
    }

    void Generator::DrawUI(Math::TransformationMatrix ortho,[[maybe_unused]] Player* p)
    {
        if (!menuOpen) return;

        Engine::GetRenderer2D().DrawRectangle(ortho * Math::TranslationMatrix(Math::vec2{ 1920.0 / 2.0, 1080.0 / 2.0 }) * Math::ScaleMatrix(Math::vec2{ 1920.0, 1080.0 }), GAME200::BLACK & 0xC0FFFFFF, GAME200::BLACK & 0xC0FFFFFF, 0.0);

        if (texTitle) texTitle->Draw(ortho * Math::TranslationMatrix(Math::vec2{ 1920.0 / 2.0 - 300.0, 800.0 }) * Math::ScaleMatrix(Math::vec2{ 1.2, 1.2 }));

        if (texScrap) texScrap->Draw(ortho * Math::TranslationMatrix(Math::vec2{ 1920.0 / 2.0 - 800.0, 750.0 }) * Math::ScaleMatrix(Math::vec2{ 0.8, 0.8 }));

        if (currentOffers.empty())
        {
            if (texEmpty) texEmpty->Draw(ortho * Math::TranslationMatrix(Math::vec2{ 1920.0 / 2.0 - 150.0, 600.0 }));
            return;
        }

        double startY = 600.0;
        double startX = 1920.0 / 2.0 - 450.0;

        for (int i = 0; i < static_cast<int>(cachedOfferTexts.size()) && i < static_cast<int>(currentOffers.size()); ++i)
        {
            if (cachedOfferTexts[i].textTex) {
                cachedOfferTexts[i].textTex->Draw(ortho * Math::TranslationMatrix(Math::vec2{ startX, startY - i * 120.0 }) * Math::ScaleMatrix(Math::vec2{ 0.8, 0.8 }));
            }

            if (cachedOfferTexts[i].descTex) {
                cachedOfferTexts[i].descTex->Draw(ortho * Math::TranslationMatrix(Math::vec2{ startX + 50.0, startY - i * 120.0 - 40.0 }) * Math::ScaleMatrix(Math::vec2{ 0.5, 0.5 }));
            }
        }
    }
    void Generator::RefreshCachedTexts(Player* p)
    {
        if (!texTitle) {
            texTitle = Engine::GetFont(0).PrintToTexture("Terminal Upgrades", GAME200::YELLOW);
            texEmpty = Engine::GetFont(0).PrintToTexture("Sold Out!", GAME200::GRAY);
        }

        if (p) {
            texScrap = Engine::GetFont(0).PrintToTexture("My Scrap: " + std::to_string(p->GetScrap()), GAME200::GREEN);
        }

        cachedOfferTexts.clear();
        for (int i = 0; i < static_cast<int>(currentOffers.size()); ++i)
        {
            unsigned int color = (i == selectedIndex) ? GAME200::YELLOW : GAME200::WHITE;
            std::string repText = currentOffers[i].repeatable ? "(Repeatable)" : "(Once)";
            std::string text = currentOffers[i].name + " " + repText + " - " + std::to_string(currentOffers[i].cost) + " Scrap";

            OfferText ot;
            ot.textTex = Engine::GetFont(0).PrintToTexture(text, color);
            ot.descTex = Engine::GetFont(0).PrintToTexture(currentOffers[i].description, GAME200::GRAY);
            cachedOfferTexts.push_back(ot);
        }
    }
}
