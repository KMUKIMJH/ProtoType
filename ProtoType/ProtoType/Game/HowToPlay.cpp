#include "../Engine/Engine.h"
#include "States.h"
#include "HowToPlay.h"

extern std::shared_ptr<CS230::Sprite> globalMenuSprite;

int HowToPlay::previousState = static_cast<int>(States::Menu);

void HowToPlay::Load() {
    if (!globalMenuSprite) {
        globalMenuSprite = std::make_shared<CS230::Sprite>("Assets/image/Menu/Menu.spt", nullptr);
        globalMenuSprite->PlayAnimation(0);
    }
    menuSprite = globalMenuSprite;

    titleTex = Engine::GetFont(0).PrintToTextureScaled("HOW TO PLAY", 0xFF0000FF, 1.5);

    instructions.push_back(Engine::GetFont(0).PrintToTexture("Move: A / D", 0xFFFFFFFF));
    instructions.push_back(Engine::GetFont(0).PrintToTexture("Jump: Space (Double Jump: Space x2)", 0xFFFFFFFF));
    instructions.push_back(Engine::GetFont(0).PrintToTexture("Attack: Left Mouse Click", 0xFFFFFFFF));
    instructions.push_back(Engine::GetFont(0).PrintToTexture("Parry: Right Mouse Click", 0xFFFFFFFF));
    instructions.push_back(Engine::GetFont(0).PrintToTexture("Roll: Shift", 0xFFFFFFFF));
    instructions.push_back(Engine::GetFont(0).PrintToTexture("Slow Motion Active: Q", 0xFFFFFFFF));
    instructions.push_back(Engine::GetFont(0).PrintToTexture("Interact / Portal: E", 0xFFFFFFFF));

    backToMenu = Engine::GetFont(0).PrintToTexture("Back (Enter / Esc)", 0xFFFFFFFF);
}

void HowToPlay::Update([[maybe_unused]] double dt) {
    if (menuSprite) {
        menuSprite->Update(dt);
    }

    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Enter) || Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Escape)) {
        Engine::GetGameStateManager().SetNextGameState(previousState);
    }
}

void HowToPlay::Draw() {
    Engine::GetWindow().Clear(0x000000FFu);
    Math::ivec2 win = Engine::GetWindow().GetSize();

    if (menuSprite) {
        Math::ivec2 fs = menuSprite->GetFrameSize();
        double sx = win.x / static_cast<double>(fs.x);
        double sy = win.y / static_cast<double>(fs.y);
        Engine::GetRenderer2D();
        menuSprite->Draw(Math::TranslationMatrix(Math::vec2{ win.x * 0.5, win.y * 0.5 }) * Math::ScaleMatrix(Math::vec2{ sx, sy }));
    }

    double currentY = win.y * 0.6;

    if (titleTex) {
        titleTex->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - titleTex->GetSize().x) / 2, currentY }));
        currentY -= 80.0;
    }

    for (const auto& tex : instructions) {
        if (tex) {
            tex->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - tex->GetSize().x) / 2, currentY }));
            currentY -= 50.0;
        }
    }

    if (backToMenu) {
        backToMenu->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - backToMenu->GetSize().x) / 2, 50.0 }));
    }
}

void HowToPlay::Unload() {
    titleTex = nullptr;
    backToMenu = nullptr;
    instructions.clear();
}