#pragma once
#include "../Engine/GameState.h"
#include "../Engine/Texture.h"
#include "../Engine/Sprite.h"
#include <memory>

class Credits : public CS230::GameState {
public:
    Credits();
    void Load() override;
    void Update([[maybe_unused]] double dt) override;
    void Unload() override;
    void Draw() override;

    std::string GetName() override { return "Credits"; }

private:
    std::shared_ptr<CS230::Texture> Credit;
    std::shared_ptr<CS230::Texture> Tech;
    std::shared_ptr<CS230::Texture> Qa;
    std::shared_ptr<CS230::Texture> Art;
    std::shared_ptr<CS230::Texture> Producer;
    std::shared_ptr<CS230::Texture> Sound;
    std::shared_ptr<CS230::Texture> Holmes;
    std::shared_ptr<CS230::Texture> Rudy;
    std::shared_ptr<CS230::Texture> Dixon;
    std::shared_ptr<CS230::Texture> Digipen;
    std::shared_ptr<CS230::Texture> SFX;
    std::shared_ptr<CS230::Texture> Team;
    std::shared_ptr<CS230::Texture> ST;
    std::shared_ptr<CS230::Texture> backToMenu;
    std::shared_ptr<CS230::Sprite> menuSprite;
};
