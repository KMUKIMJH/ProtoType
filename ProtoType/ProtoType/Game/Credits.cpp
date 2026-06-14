#include "../Engine/Engine.h"
#include "States.h"
#include "Credits.h"

extern std::shared_ptr<CS230::Sprite> globalMenuSprite;

Credits::Credits()
{
}

void Credits::Load()
{
    if (!globalMenuSprite)
    {
        globalMenuSprite = std::make_shared<CS230::Sprite>("Assets/image/Menu/Menu.spt", nullptr);
        globalMenuSprite->PlayAnimation(0);
    }
    menuSprite = globalMenuSprite;

    Credit = Engine::GetFont(0).PrintToTexture("CREDITS", 0xFFFFFFFF);
    Team = Engine::GetFont(0).PrintToTexture("RAGTAG", 0xFFFFFFFF);
    Producer = Engine::GetFont(0).PrintToTexture("Producer: Yeonseo Kim", 0xFFFFFFFF);
    Art = Engine::GetFont(0).PrintToTexture("Art Lead: Sehoon Kim", 0xFFFFFFFF);
    Qa = Engine::GetFont(0).PrintToTexture("QA Lead: Junhwan Kim", 0xFFFFFFFF);
    Tech = Engine::GetFont(0).PrintToTexture("Tech Lead: Minchan Cho", 0xFFFFFFFF);
    SFX = Engine::GetFont(0).PrintToTexture("SFX: Minchan Cho", 0xFFFFFFFF);
    ST = Engine::GetFont(0).PrintToTexture("Special Thanks", 0xFFFFFFFF);
    Sound = Engine::GetFont(0).PrintToTexture("Sound: Donghyeok Hahm", 0xFFFFFFFF);
    Rudy = Engine::GetFont(0).PrintToTexture("Professer: Rudy Castan", 0xFFFFFFFF);
    Dixon = Engine::GetFont(0).PrintToTexture("Professer: Bryce Dixon", 0xFFFFFFFF);
    Holmes = Engine::GetFont(0).PrintToTexture("Professer: Jonathan Holmes", 0xFFFFFFFF);
    Digipen = Engine::GetFont(0).PrintToTexture("Copyright (C) 2026 DigiPen Institute of Technology", 0xFFFFFFFF);
    backToMenu = Engine::GetFont(0).PrintToTexture("Back to Menu", 0xFFFFFFFF);
}

void Credits::Update([[maybe_unused]] double dt)
{
    if (menuSprite)
    {
        menuSprite->Update(dt);
    }

    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Enter) || Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Escape)) {
        Engine::GetGameStateManager().SetNextGameState(static_cast<int>(States::Menu));
    }
}

void Credits::Draw()
{
    Engine::GetWindow().Clear(0x000000FFu);

    if (menuSprite)
    {
        Math::ivec2 fs = menuSprite->GetFrameSize();
        Math::ivec2 win = Engine::GetWindow().GetSize();
        double sx = win.x / static_cast<double>(fs.x);
        double sy = win.y / static_cast<double>(fs.y);
        Engine::GetRenderer2D();
        menuSprite->Draw(Math::TranslationMatrix(Math::vec2{ win.x * 0.5, win.y * 0.5 }) * Math::ScaleMatrix(Math::vec2{ sx, sy }));
    }

    Math::ivec2 win = Engine::GetWindow().GetSize();

    double startY = win.y * 0.70;

    if (Credit) 
    {
        Credit->Draw(Math::TranslationMatrix(Math::vec2{(double)(win.x - Credit->GetSize().x) / 2, startY}));
    }
    if (Team)
    {
        Team->Draw(Math::TranslationMatrix(Math::vec2{(double)(win.x - Team->GetSize().x) / 2, startY - 100.0}));
    }

    double leftX = win.x * 0.05;
    double rightX = win.x * 0.7;
    double colY = startY - 220.0;

    if (Producer) Producer->Draw(Math::TranslationMatrix(Math::vec2{leftX, colY}));
    if (Art) Art->Draw(Math::TranslationMatrix(Math::vec2{leftX, colY - 80.0}));
    if (Qa) Qa->Draw(Math::TranslationMatrix(Math::vec2{leftX, colY - 160.0}));
    if (Tech) Tech->Draw(Math::TranslationMatrix(Math::vec2{leftX, colY - 240.0}));
    if (SFX) SFX->Draw(Math::TranslationMatrix(Math::vec2{leftX, colY - 320.0}));

    if (ST) ST->Draw(Math::TranslationMatrix(Math::vec2{rightX-100, colY}));
    if (Sound) Sound->Draw(Math::TranslationMatrix(Math::vec2{rightX - 200.0, colY - 80.0}));
    if (Rudy) Rudy->Draw(Math::TranslationMatrix(Math::vec2{ rightX - 200.0, colY - 160.0 }));
    if (Dixon) Dixon->Draw(Math::TranslationMatrix(Math::vec2{ rightX - 200.0, colY - 240.0 }));
    if (Holmes) Holmes->Draw(Math::TranslationMatrix(Math::vec2{ rightX - 200.0, colY - 320.0 }));
    if (Digipen) Digipen->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - Digipen->GetSize().x) / 2, colY - 400.0 }));

    if (backToMenu) 
    {
        backToMenu->Draw(Math::TranslationMatrix(Math::vec2{(double)(win.x - backToMenu->GetSize().x) / 2, 50.0}));
    }
}

void Credits::Unload()
{
}
