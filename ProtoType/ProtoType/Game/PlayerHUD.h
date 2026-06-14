#pragma once
#include <memory>

namespace CS230
{
    class Player;
    class Texture;
}

class PlayerHUD
{
public:
    void Draw(const CS230::Player& player);

private:
    int lastHP{ -1 };
    int lastScrap{ -1 };
    int lastWeaponType{ -1 };
    int lastAmmoCount{ -1 };

    std::shared_ptr<CS230::Texture> hpTex;
    std::shared_ptr<CS230::Texture> scrapTex;
    std::shared_ptr<CS230::Texture> wepTex;
    std::shared_ptr<CS230::Texture> rollLabelTex;
    std::shared_ptr<CS230::Texture> parryLabelTex;

    std::shared_ptr<CS230::Texture> hpIconTex;
    std::shared_ptr<CS230::Texture> hpFullTex;
    std::shared_ptr<CS230::Texture> hpEmptyTex;
    std::shared_ptr<CS230::Texture> gaugeBgTex;
    std::shared_ptr<CS230::Texture> gaugeFillTex;

    std::shared_ptr<CS230::Texture> scrapIconTex;
};