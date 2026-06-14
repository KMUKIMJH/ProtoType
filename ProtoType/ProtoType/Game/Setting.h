#pragma once
#include "../Engine/GameState.h"
#include "../Engine/Texture.h"
#include <memory>
#include <vector>

class Setting : public CS230::GameState {
public:
    Setting();
    void Load() override;
    void Update([[maybe_unused]] double dt) override;
    void Unload() override;
    void Draw() override;

    std::string GetName() override { return "Setting"; }

    static int previousState;

private:
    int select;
    const int max_select = 7;

    int masterVol;
    int bgmVol;
    int sfxVol;

    bool masterMute;
    bool bgmMute;
    bool sfxMute;

    int resIndex;
    int displayIndex;

    bool isBindingMode;
    int bindSelect;
    bool isWaitingForKey;

    std::shared_ptr<CS230::Texture> title;
    std::shared_ptr<CS230::Texture> masterOption;
    std::shared_ptr<CS230::Texture> bgmOption;
    std::shared_ptr<CS230::Texture> sfxOption;

    std::shared_ptr<CS230::Texture> resOption;
    std::shared_ptr<CS230::Texture> displayOption;

    std::shared_ptr<CS230::Texture> keyBindMenuOption;
    std::vector<std::shared_ptr<CS230::Texture>> actionOptions;

    std::shared_ptr<CS230::Texture> backOption;

    void ApplyVolume();
    void ApplyGraphics();
    void RenderText();
};