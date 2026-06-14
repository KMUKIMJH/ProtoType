#pragma once
#include "../Engine/GameState.h"
#include "../Engine/Texture.h"
#include "../Engine/Sprite.h"
#include <memory>
#include <vector>

class HowToPlay : public CS230::GameState {
public:
    HowToPlay() = default;
    void Load() override;
    void Update([[maybe_unused]] double dt) override;
    void Unload() override;
    void Draw() override;
    std::string GetName() override { return "HowToPlay"; }

    static int previousState;

private:
    std::shared_ptr<CS230::Sprite> menuSprite;
    std::shared_ptr<CS230::Texture> titleTex;
    std::shared_ptr<CS230::Texture> backToMenu;
    std::vector<std::shared_ptr<CS230::Texture>> instructions;
};