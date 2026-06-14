#pragma once
#include "../Engine/GameObject.h"
#include "Player.h"
#include "Weapon.h"
#include <vector>
#include <string>
#include <functional>
#include <memory>

namespace CS230
{
    class Texture;
    struct UpgradeOption
    {
        int id;
        std::string name;
        std::string description;
        int cost;
        bool repeatable;
        int reqWeapon;
        std::function<void(Player&)> applyFunc;
    };

    class Generator : public GameObject
    {
    public:
        Generator(Math::vec2 pos);
        void Update(double dt) override;
        void Draw(Math::TransformationMatrix camM) override;

        GameObjectTypes Type() override
        {
            return GameObjectTypes::Generator;
        }

        std::string TypeName() override
        {
            return "Generator";
        }

        const Math::rect& GetRect() const
        {
            return rect;
        }

        void RollUpgrades(Player* p);
        void ToggleUI();
        bool IsUIOpen() const
        {
            return menuOpen;
        }

        void UpdateUI(Player* p);
        void DrawUI(Math::TransformationMatrix ortho, Player* p);

    private:
        Math::rect rect;
        bool menuOpen{ false };
        int selectedIndex{ 0 };

        std::vector<UpgradeOption> allUpgrades;
        std::vector<UpgradeOption> currentOffers;
        struct OfferText {
            std::shared_ptr<Texture> textTex;
            std::shared_ptr<Texture> descTex;
        };

        std::shared_ptr<Texture> texTitle;
        std::shared_ptr<Texture> texScrap;
        std::shared_ptr<Texture> texEmpty;
        std::vector<OfferText> cachedOfferTexts;

        int lastCachedScrap{ -1 };
        bool needsTextUpdate{ false };

        void RefreshCachedTexts(Player* p);
    };
}