#pragma once
#include "../Engine/Texture.h"
#include "../Engine/Matrix.h"
#include "../Game/PlayerHUD.h"
#include <memory>
#include <string>

namespace CS230 { class Player; }
class StageManager;

class StageUI
{
public:
    StageUI();
    ~StageUI() = default;

    void Update(StageManager* stageMgr, CS230::Player& player);
    void Draw(const CS230::Player& player, const Math::TransformationMatrix& camM);

    void Draw(StageManager* stageMgr, const CS230::Player& player, const Math::TransformationMatrix& camM);

    void SetPaused(bool p);
    bool IsPaused() const { return isPaused; }

    void TriggerGameOver();
    bool IsGameOver() const { return isGameOver; }
    void ResetGameOver() { isGameOver = false; }

    void TriggerEnding() { isEnding = true; }
    bool IsEnding() const { return isEnding; }

private:
    bool isPaused{ false };
    int pauseSelect{ 1 };
    const int maxPauseSelect{ 5 };

    std::shared_ptr<CS230::Texture> pauseBackOpt;
    std::shared_ptr<CS230::Texture> pauseSettingOpt;
    std::shared_ptr<CS230::Texture> pauseHowToPlayOpt;
    std::shared_ptr<CS230::Texture> pauseMenuOpt;
    std::shared_ptr<CS230::Texture> pauseExitOpt;

    void RenderPauseText();
    void HandlePauseInput(StageManager* stageMgr);
    void DrawPauseMenu();

    bool isGameOver{ false };
    int gameOverSelect{ 1 };
    const int maxGameOverSelect{ 3 };
    std::shared_ptr<CS230::Texture> gameOverTitleOpt;
    std::shared_ptr<CS230::Texture> gameOverRestartOpt;
    std::shared_ptr<CS230::Texture> gameOverMenuOpt;
    std::shared_ptr<CS230::Texture> gameOverExitOpt;

    void RenderGameOverText();
    void HandleGameOverInput(StageManager* stageMgr);
    void DrawGameOverMenu();

    bool isEnding{ false };
    std::shared_ptr<CS230::Texture> endingTex;

    void RenderEndingText();
    void HandleEndingInput();
    void DrawEndingScreen();

    std::vector<std::shared_ptr<CS230::Texture>> cachedStoryLines;
    int lastLoadedStoryId{ -1 };

    void RenderStoryText(int storyId);
    void HandleStoryInput(CS230::Player& player);
    void DrawStoryMenu();

    std::shared_ptr<CS230::Texture> bossHPLabelTex;
    void DrawBossHPBar(StageManager* stageMgr);

    PlayerHUD hud;
};