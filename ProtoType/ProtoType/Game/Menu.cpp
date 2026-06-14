#include "../Engine/Engine.h"
#include "States.h"
#include "Menu.h"
#include "Setting.h"

std::shared_ptr<CS230::Sprite> globalMenuSprite = nullptr;

Menu::Menu() : select(1) {}

void Menu::Load() {
    if (!globalMenuSprite) {
        globalMenuSprite = std::make_shared<CS230::Sprite>("Assets/image/Menu/Menu.spt", nullptr);
        globalMenuSprite->PlayAnimation(0);
    }
    menuSprite = globalMenuSprite;
    select = 1;

    game = Engine::GetFont(static_cast<int>(0)).PrintToTexture("Game Start", (select == 1) ? 0xFFFFFFFF : 0x81AC00FF);
    setting = Engine::GetFont(static_cast<int>(0)).PrintToTexture("Setting", (select == 2) ? 0xFFFFFFFF : 0x81AC00FF);
    howToPlay = Engine::GetFont(static_cast<int>(0)).PrintToTexture("How To Play", (select == 3) ? 0xFFFFFFFF : 0x81AC00FF);
    credits = Engine::GetFont(static_cast<int>(0)).PrintToTexture("Credits", (select == 4) ? 0xFFFFFFFF : 0x81AC00FF);
    exit = Engine::GetFont(static_cast<int>(0)).PrintToTexture("Exit", (select == 5) ? 0xFFFFFFFF : 0x81AC00FF);
}

void Menu::Draw() {
    Math::ivec2 win = Engine::GetWindow().GetSize();

    if (menuSprite) {
        Math::ivec2 fs = menuSprite->GetFrameSize();
        double sx = win.x / static_cast<double>(fs.x);
        double sy = win.y / static_cast<double>(fs.y);
        Engine::GetRenderer2D();
        menuSprite->Draw(Math::TranslationMatrix(Math::vec2{ win.x * 0.5, win.y * 0.5 }) * Math::ScaleMatrix(Math::vec2{ sx, sy }));
    }

    double yPos = win.y * 0.5;

    if (game) {
        game->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - game->GetSize().x) / 2, yPos }));
        yPos -= game->GetSize().y + 10.0;
    }
    if (setting) {
        setting->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - setting->GetSize().x) / 2, yPos }));
        yPos -= setting->GetSize().y + 10.0;
    }
    if (howToPlay) {
        howToPlay->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - howToPlay->GetSize().x) / 2, yPos }));
        yPos -= howToPlay->GetSize().y + 10.0;
    }
    if (credits) {
        credits->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - credits->GetSize().x) / 2, yPos }));
        yPos -= credits->GetSize().y + 10.0;
    }
    if (exit) {
        exit->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - exit->GetSize().x) / 2, yPos }));
    }
}

void Menu::Update([[maybe_unused]] double dt)
{
    if (menuSprite)
    {
        menuSprite->Update(dt);
    }

    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::S) && select < max_select)
    {
        Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
        update_select_text(select, 0x81AC00FF);
        ++select;
        update_select_text(select, 0xFFFFFFFF);
    }
    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::W) && select > 1)
    {
        Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
        update_select_text(select, 0x81AC00FF);
        --select;
        update_select_text(select, 0xFFFFFFFF);
    }

    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Enter))
    {
        Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
        if (select == 1)
        {
            update_select_text(select, 0xFFFFFFFF);
            Engine::GetGameStateManager().SetNextGameState(static_cast<int>(States::Stage));
        }
        else if (select == 2)
        {
            update_select_text(select, 0xFFFFFFFF);
            Setting::previousState = static_cast<int>(States::Menu);
            Engine::GetGameStateManager().SetNextGameState(static_cast<int>(States::Setting));
        }
        else if (select == 3)
        {
            update_select_text(select, 0xFFFFFFFF);
            Engine::GetGameStateManager().SetNextGameState(static_cast<int>(States::HowToPlay));
        }
        else if (select == 4)
        {
            update_select_text(select, 0xFFFFFFFF);
            Engine::GetGameStateManager().SetNextGameState(static_cast<int>(States::Credits));
        }
        else if (select == 5)
        {
            Engine::GetGameStateManager().ClearNextGameState();
        }
    }
}

void Menu::Unload()
{
    Engine::GetSFXManager().StopAll();
}

void Menu::update_select_text(int mselect, unsigned int color)
{
    if (mselect == 1) game = Engine::GetFont(static_cast<int>(0)).PrintToTexture("Game Start", color);
    else if (mselect == 2) setting = Engine::GetFont(static_cast<int>(0)).PrintToTexture("Setting", color);
    else if (mselect == 3) howToPlay = Engine::GetFont(static_cast<int>(0)).PrintToTexture("How To Play", color);
    else if (mselect == 4) credits = Engine::GetFont(static_cast<int>(0)).PrintToTexture("Credits", color);
    else if (mselect == 5) exit = Engine::GetFont(static_cast<int>(0)).PrintToTexture("Exit", color);
}