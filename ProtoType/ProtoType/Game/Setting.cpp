#include "../Engine/Engine.h"
#include "States.h"
#include "Setting.h"
#include "KeyBinding.h"
#include <sstream>
#include <cmath>

int Setting::previousState = 2;

Setting::Setting()
    : select(1), masterVol(10), bgmVol(10), sfxVol(10), masterMute(false), bgmMute(false), sfxMute(false),
    resIndex(0), displayIndex(0),
    isBindingMode(false), bindSelect(0), isWaitingForKey(false)
{}

void Setting::Load()
{
    select = 1;
    bindSelect = 0;
    isBindingMode = false;
    isWaitingForKey = false;

    masterVol = static_cast<int>(std::round(static_cast<double>(Engine::GetSFXManager().GetMasterVolume()) * 10.0 / SDL_MIX_MAXVOLUME));
    bgmVol = static_cast<int>(std::round(static_cast<double>(Engine::GetSFXManager().GetBGMVolume()) * 10.0 / SDL_MIX_MAXVOLUME));
    sfxVol = static_cast<int>(std::round(static_cast<double>(Engine::GetSFXManager().GetSFXVolume()) * 10.0 / SDL_MIX_MAXVOLUME));

    if (masterVol == 0) masterMute = true;
    if (bgmVol == 0) bgmMute = true;
    if (sfxVol == 0) sfxMute = true;

    title = Engine::GetFont(0).PrintToTexture("Settings", 0xFFFFFFFF);
    RenderText();
}

void Setting::Update([[maybe_unused]] double dt)
{
    if (isWaitingForKey) {
        for (int k = 1; k < static_cast<int>(CS230::Input::Keys::Count); ++k) {
            CS230::Input::Keys key = static_cast<CS230::Input::Keys>(k);
            if (Engine::GetInput().KeyJustPressed(key)) {
                if (key != CS230::Input::Keys::Escape) {
                    CS230::Action currentAction = static_cast<CS230::Action>(bindSelect);
                    CS230::KeyBinding::GetInstance().SetBinding(currentAction, key);
                }
                isWaitingForKey = false;
                RenderText();
                return;
            }
        }
        for (int m = 0; m < static_cast<int>(CS230::Input::MouseButtons::Count); ++m) {
            CS230::Input::MouseButtons btn = static_cast<CS230::Input::MouseButtons>(m);
            if (Engine::GetInput().MouseJustPressed(btn)) {
                CS230::Action currentAction = static_cast<CS230::Action>(bindSelect);
                CS230::KeyBinding::GetInstance().SetBinding(currentAction, btn);
                isWaitingForKey = false;
                RenderText();
                return;
            }
        }
        return;
    }

    if (isBindingMode) {
        const int max_bind_options = static_cast<int>(CS230::Action::Count) + 1;

        if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::S)) {
            if (bindSelect < max_bind_options - 1) {
                Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
                bindSelect++;
            }
            RenderText();
        }
        if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::W)) {
            if (bindSelect > 0) {
                Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
                bindSelect--;
            }
            RenderText();
        }
        if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Enter)) {
            Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
            if (bindSelect == max_bind_options - 1) {
                isBindingMode = false;
            }
            else {
                isWaitingForKey = true;
            }
            RenderText();
        }
        if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Escape)) {
            Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
            isBindingMode = false;
            RenderText();
        }
        return;
    }

    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::S)) {
        if (select < max_select) {
            Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
            select++;
        }
        RenderText();
    }
    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::W)) {
        if (select > 1) {
            Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
            select--;
        }
        RenderText();
    }

    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::A)) {
        Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
        if (select == 1 && masterVol > 0) { masterVol--; masterMute = false; ApplyVolume(); }
        else if (select == 2 && bgmVol > 0) { bgmVol--; bgmMute = false; ApplyVolume(); }
        else if (select == 3 && sfxVol > 0) { sfxVol--; sfxMute = false; ApplyVolume(); }
        else if (select == 4 && resIndex > 0) { resIndex--; ApplyGraphics(); }
        else if (select == 5 && displayIndex > 0) { displayIndex--; ApplyGraphics(); }
        RenderText();
    }

    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::D)) {
        Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
        if (select == 1 && masterVol < 10) { masterVol++; masterMute = false; ApplyVolume(); }
        else if (select == 2 && bgmVol < 10) { bgmVol++; bgmMute = false; ApplyVolume(); }
        else if (select == 3 && sfxVol < 10) { sfxVol++; sfxMute = false; ApplyVolume(); }
        else if (select == 4 && resIndex < 2) { resIndex++; ApplyGraphics(); }
        else if (select == 5 && displayIndex < 2) { displayIndex++; ApplyGraphics(); }
        RenderText();
    }

    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Enter)) {
        Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
        if (select == 1) { masterMute = !masterMute; ApplyVolume(); RenderText(); }
        else if (select == 2) { bgmMute = !bgmMute; ApplyVolume(); RenderText(); }
        else if (select == 3) { sfxMute = !sfxMute; ApplyVolume(); RenderText(); }
        else if (select == 6) { isBindingMode = true; bindSelect = 0; RenderText(); }
        else if (select == max_select) {
            Engine::GetGameStateManager().SetNextGameState(previousState);
        }
    }

    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Escape)) {
        Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
        Engine::GetGameStateManager().SetNextGameState(previousState);
    }
}

void Setting::Draw()
{
    Engine::GetWindow().Clear(0x222222FFu);
    Math::ivec2 win = Engine::GetWindow().GetSize();

    double startY = win.y * 0.85;
    if (title) {
        title->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - title->GetSize().x) / 2, startY }));
    }

    if (isBindingMode) {
        double itemY = startY - 90.0;
        const double bindSpacing = 42.0;

        for (size_t i = 0; i < actionOptions.size(); ++i) {
            if (actionOptions[i]) {
                actionOptions[i]->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - actionOptions[i]->GetSize().x) / 2, itemY }));
                itemY -= bindSpacing;
            }
        }
        return;
    }

    double itemY = startY - 110.0;
    const double spacing = 55.0;

    if (masterOption) { masterOption->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - masterOption->GetSize().x) / 2, itemY })); itemY -= spacing; }
    if (bgmOption) { bgmOption->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - bgmOption->GetSize().x) / 2, itemY })); itemY -= spacing; }
    if (sfxOption) { sfxOption->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - sfxOption->GetSize().x) / 2, itemY })); itemY -= spacing; }

    itemY -= 20.0;
    if (resOption) { resOption->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - resOption->GetSize().x) / 2, itemY })); itemY -= spacing; }
    if (displayOption) { displayOption->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - displayOption->GetSize().x) / 2, itemY })); itemY -= spacing; }

    itemY -= 20.0;
    if (keyBindMenuOption) { keyBindMenuOption->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - keyBindMenuOption->GetSize().x) / 2, itemY })); itemY -= spacing; }

    itemY -= 20.0;
    if (backOption) { backOption->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - backOption->GetSize().x) / 2, itemY })); }
}

void Setting::ApplyVolume()
{
    if (masterMute) Engine::GetSFXManager().SetMasterVolume(0);
    else Engine::GetSFXManager().SetMasterVolume(static_cast<int>(std::round(masterVol / 10.0 * SDL_MIX_MAXVOLUME)));

    if (bgmMute) Engine::GetSFXManager().SetBGMVolume(0);
    else Engine::GetSFXManager().SetBGMVolume(static_cast<int>(std::round(bgmVol / 10.0 * SDL_MIX_MAXVOLUME)));

    if (sfxMute) Engine::GetSFXManager().SetSFXVolume(0);
    else Engine::GetSFXManager().SetSFXVolume(static_cast<int>(std::round(sfxVol / 10.0 * SDL_MIX_MAXVOLUME)));
}

void Setting::ApplyGraphics()
{
    int targetWidth = 1920;
    int targetHeight = 1080;

    switch (resIndex) {
    case 0: targetWidth = 1920; targetHeight = 1080; break;
    case 1: targetWidth = 2560; targetHeight = 1440; break;
    case 2: targetWidth = 3840; targetHeight = 2160; break;
    }

    bool isFullscreen = false;
    bool isBorderless = false;

    switch (displayIndex) {
    case 0: isFullscreen = false; isBorderless = false; break;
    case 1: isFullscreen = true;  isBorderless = true;  break;
    case 2: isFullscreen = true;  isBorderless = false; break;
    }

    Engine::GetWindow().SetFullscreen(isFullscreen, isBorderless);

    if (!isFullscreen) {
        Engine::GetWindow().SetResolution(targetWidth, targetHeight);
    }
}

void Setting::RenderText()
{
    if (isBindingMode) {
        actionOptions.clear();

        const char* actionNames[] = 
        {
            "Move Left", "Move Right", "Drop Through (Down)",
            "Jump", "Roll", "Attack", "Parry",
            "Time Slow Mode", "Interact", "Restart"
        };

        for (int i = 0; i < static_cast<int>(CS230::Action::Count); ++i) 
        {
            std::stringstream ss;
            ss << actionNames[i] << " : [ "
                << ((isWaitingForKey && bindSelect == i) ? "Press Any Key..." : CS230::KeyBinding::GetInstance().GetBindingName(static_cast<CS230::Action>(i)))
                << " ]";

            unsigned int color = (bindSelect == i) ? 0xFFFFFFFF : 0xAAAAAAFF;
            actionOptions.push_back(Engine::GetFont(0).PrintToTextureScaled(ss.str(), color, 0.6));
        }

        unsigned int backColor = (bindSelect == static_cast<int>(CS230::Action::Count)) ? 0xFFFFFFFF : 0xAAAAAAFF;
        actionOptions.push_back(Engine::GetFont(0).PrintToTextureScaled("Back to Settings", backColor, 0.6));
        return;
    }

    std::stringstream ss1, ss2, ss3, ssRes, ssDisp;
    ss1 << "Master Vol: < " << (masterMute ? "Muted" : std::to_string(masterVol * 10)) << " >";
    ss2 << "BGM Vol: < " << (bgmMute ? "Muted" : std::to_string(bgmVol * 10)) << " >";
    ss3 << "SFX Vol: < " << (sfxMute ? "Muted" : std::to_string(sfxVol * 10)) << " >";

    const char* resStr[] = { "1920x1080 (FHD)", "2560x1440 (QHD)", "3840x2160 (4K)" };
    ssRes << "Resolution: < " << resStr[resIndex] << " >";

    const char* dispStr[] = { "Windowed", "Borderless", "Fullscreen" };
    ssDisp << "Display: < " << dispStr[displayIndex] << " >";

    unsigned int c1 = (select == 1) ? 0xFFFFFFFF : 0xAAAAAAFF;
    unsigned int c2 = (select == 2) ? 0xFFFFFFFF : 0xAAAAAAFF;
    unsigned int c3 = (select == 3) ? 0xFFFFFFFF : 0xAAAAAAFF;
    unsigned int c4 = (select == 4) ? 0xFFFFFFFF : 0xAAAAAAFF;
    unsigned int c5 = (select == 5) ? 0xFFFFFFFF : 0xAAAAAAFF;
    unsigned int c6 = (select == 6) ? 0xFFFFFFFF : 0xAAAAAAFF;
    unsigned int c7 = (select == 7) ? 0xFFFFFFFF : 0xAAAAAAFF;

    masterOption = Engine::GetFont(0).PrintToTextureScaled(ss1.str(), c1, 0.7);
    bgmOption = Engine::GetFont(0).PrintToTextureScaled(ss2.str(), c2, 0.7);
    sfxOption = Engine::GetFont(0).PrintToTextureScaled(ss3.str(), c3, 0.7);

    resOption = Engine::GetFont(0).PrintToTextureScaled(ssRes.str(), c4, 0.7);
    displayOption = Engine::GetFont(0).PrintToTextureScaled(ssDisp.str(), c5, 0.7);

    keyBindMenuOption = Engine::GetFont(0).PrintToTextureScaled("Key Bindings", c6, 0.7);

    backOption = Engine::GetFont(0).PrintToTextureScaled("Back", c7, 0.7);
}

void Setting::Unload()
{
    title = nullptr;
    masterOption = nullptr;
    bgmOption = nullptr;
    sfxOption = nullptr;
    resOption = nullptr;
    displayOption = nullptr;
    keyBindMenuOption = nullptr;
    actionOptions.clear();
    backOption = nullptr;
}