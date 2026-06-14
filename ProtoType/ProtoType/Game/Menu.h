#pragma once

#include "../Engine/GameState.h"
#include "../Engine/Texture.h"
#include "../Engine/Sprite.h"
#include <memory>
#include <string>

class Menu : public CS230::GameState {
public:
    Menu();
    void Load() override;
    void Update([[maybe_unused]] double dt) override;
    void Unload() override;
    void Draw() override;
    std::string GetName() override { return "Menu"; }
private:
    int select;
    const int max_select = 5; 
    std::shared_ptr<CS230::Texture> game;
    std::shared_ptr<CS230::Texture> setting;
    std::shared_ptr<CS230::Texture> howToPlay;
    std::shared_ptr<CS230::Texture> credits;
    std::shared_ptr<CS230::Texture> exit;
    std::shared_ptr<CS230::Texture> background;
    std::shared_ptr<CS230::Sprite> menuSprite;

    void update_select_text(int mselect, unsigned int color);
};