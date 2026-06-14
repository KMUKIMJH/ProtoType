#include "StageUI.h"
#include "StageManager.h"
#include "../Game/Player.h"
#include "../Game/States.h"
#include "../Game/Setting.h"
#include "../Game/Weapon.h"
#include "../Game/HowToPlay.h"
#include "../Game/Menu.h"
#include "../Engine/Engine.h"
#include "../Engine/GameStateManager.h"
#include <GL/glew.h>
#include <cstdio>

StageUI::StageUI()
{}

void StageUI::Update(StageManager* stageMgr, CS230::Player& player)
{
    if (isEnding)
    {
        HandleEndingInput();
        return;
    }

    if (player.combat.showStoryUI)
    {
        RenderStoryText(player.combat.currentStoryId);
        HandleStoryInput(player);
        return;
    }

    if (isGameOver)
    {
        HandleGameOverInput(stageMgr);
    }
    else if (isPaused)
    {
        HandlePauseInput(stageMgr);
    }
}

void StageUI::Draw(const CS230::Player& player, const Math::TransformationMatrix& camM)
{
    (void)camM;

    hud.Draw(player);

    if (player.combat.showStoryUI)
    {
        DrawStoryMenu();
    }
    else
    {
        DrawPauseMenu();
        DrawGameOverMenu();
    }
}

void StageUI::Draw(StageManager* stageMgr, const CS230::Player& player, const Math::TransformationMatrix& camM)
{
    (void)camM;

    if (isEnding)
    {
        DrawEndingScreen();
        return;
    }

    hud.Draw(player);

    DrawBossHPBar(stageMgr);

    if (player.combat.showStoryUI)
    {
        DrawStoryMenu();
    }
    else
    {
        DrawPauseMenu();
        DrawGameOverMenu();
    }
}

void StageUI::SetPaused(bool p)
{
    isPaused = p;
    if (p)
    {
        pauseSelect = 1;
        RenderPauseText();
    }
}

void StageUI::TriggerGameOver()
{
    isGameOver = true;
    gameOverSelect = 1;
    RenderGameOverText();
}

void StageUI::DrawBossHPBar(StageManager* stageMgr)
{
    (void)stageMgr;

    CS230::Enemy* activeBoss = nullptr;

    for (auto* e : CS230::Enemy::Registry())
    {
        if (e && e->IsActive() && e->IsBoss() && e->health > 0)
        {
            activeBoss = e;
            if (e->health < e->maxHealth) {
                break;
            }
        }
    }

    if (!activeBoss) return;

    auto& renderer = Engine::GetRenderer2D();
    Math::ivec2 win = Engine::GetWindow().GetSize();

    double barWidth = static_cast<double>(win.x) * 0.7;
    double barHeight = 25.0;
    double barX = (static_cast<double>(win.x) - barWidth) * 0.5;
    double barY = static_cast<double>(win.y) - 250.0;

    int currentHP = activeBoss->health;
    int maxHP = (activeBoss->maxHealth > 0) ? activeBoss->maxHealth : 1;

    double hpRatio = static_cast<double>(currentHP) / static_cast<double>(maxHP);
    if (hpRatio < 0.0) hpRatio = 0.0;
    if (hpRatio > 1.0) hpRatio = 1.0;

    Math::TransformationMatrix bgMat = Math::TranslationMatrix(Math::vec2{ barX + barWidth * 0.5, barY }) * Math::ScaleMatrix(Math::vec2{ barWidth + 6.0, barHeight + 6.0 });
    renderer.DrawRectangle(bgMat, 0x111111FFu, 0x000000FFu, 3.0);

    double currentBarWidth = barWidth * hpRatio;
    if (currentBarWidth > 0.0)
    {
        Math::vec2 hpCenter = { barX + currentBarWidth * 0.5, barY };
        Math::TransformationMatrix hpMat = Math::TranslationMatrix(hpCenter) * Math::ScaleMatrix(Math::vec2{ currentBarWidth, barHeight });
        renderer.DrawRectangle(hpMat, GAME200::RED, GAME200::RED, 0.0);
    }

    static int s_lastBossHP = -1;
    if (!bossHPLabelTex || s_lastBossHP != currentHP)
    {
        std::string hpTextStr = "BOSS HP : " + std::to_string(currentHP) + " / " + std::to_string(maxHP);
        bossHPLabelTex = Engine::GetFont(0).PrintToTextureScaled(hpTextStr, GAME200::YELLOW, 0.4);
        s_lastBossHP = currentHP;
    }

    if (bossHPLabelTex)
    {
        bossHPLabelTex->Draw(Math::TranslationMatrix(Math::vec2{ barX, barY + barHeight + 5.0 }));
    }
}

void StageUI::RenderPauseText()
{
    unsigned int c1 = (pauseSelect == 1) ? 0xFFFFFFFF : 0x81AC00FF;
    unsigned int c2 = (pauseSelect == 2) ? 0xFFFFFFFF : 0x81AC00FF;
    unsigned int c3 = (pauseSelect == 3) ? 0xFFFFFFFF : 0x81AC00FF;
    unsigned int c4 = (pauseSelect == 4) ? 0xFFFFFFFF : 0x81AC00FF;
    unsigned int c5 = (pauseSelect == 5) ? 0xFFFFFFFF : 0x81AC00FF;

    pauseBackOpt = Engine::GetFont(0).PrintToTexture("Back to Game", c1);
    pauseSettingOpt = Engine::GetFont(0).PrintToTexture("Setting", c2);
    pauseHowToPlayOpt = Engine::GetFont(0).PrintToTexture("How To Play", c3);
    pauseMenuOpt = Engine::GetFont(0).PrintToTexture("Menu", c4);
    pauseExitOpt = Engine::GetFont(0).PrintToTexture("Exit Game", c5);
}

void StageUI::HandlePauseInput(StageManager* stageMgr)
{
    (void)stageMgr;

    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::S)) {
        if (pauseSelect < maxPauseSelect) {
            Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
            pauseSelect++;
        }
        RenderPauseText();
    }
    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::W)) {
        if (pauseSelect > 1) {
            Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
            pauseSelect--;
        }
        RenderPauseText();
    }

    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Escape)) {
        Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
        isPaused = false;
        return;
    }

    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Enter)) {
        Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
        if (pauseSelect == 1) {
            isPaused = false;
        }
        else if (pauseSelect == 2) {
            Setting::previousState = static_cast<int>(States::Stage);
            StageManager::isTransitioningToSettings = true;
            Engine::GetGameStateManager().SetNextGameState(static_cast<int>(States::Setting));
        }
        else if (pauseSelect == 3) {
            HowToPlay::previousState = static_cast<int>(States::Stage);
            StageManager::isTransitioningToSettings = true;
            Engine::GetGameStateManager().SetNextGameState(static_cast<int>(States::HowToPlay));
        }
        else if (pauseSelect == 4) {
            isPaused = false;
            Engine::GetGameStateManager().SetNextGameState(static_cast<int>(States::Menu));
        }
        else if (pauseSelect == 5) {
            Engine::GetGameStateManager().ClearNextGameState();
        }
    }
}

void StageUI::DrawPauseMenu()
{
    if (!isPaused) return;

    auto& renderer = Engine::GetRenderer2D();
    Math::ivec2 win = Engine::GetWindow().GetSize();

    Math::TransformationMatrix boxMat = Math::TranslationMatrix(Math::vec2{ win.x * 0.5, win.y * 0.5 }) * Math::ScaleMatrix(Math::vec2{ 500, 460 });
    renderer.DrawRectangle(boxMat, 0x000000FF, 0x000000FF, 0.0);

    double yPos = win.y * 0.5 + 130.0;

    if (pauseBackOpt) {
        pauseBackOpt->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - pauseBackOpt->GetSize().x) / 2, yPos }));
        yPos -= 70.0;
    }
    if (pauseSettingOpt) {
        pauseSettingOpt->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - pauseSettingOpt->GetSize().x) / 2, yPos }));
        yPos -= 70.0;
    }
    if (pauseHowToPlayOpt) {
        pauseHowToPlayOpt->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - pauseHowToPlayOpt->GetSize().x) / 2, yPos }));
        yPos -= 70.0;
    }
    if (pauseMenuOpt) {
        pauseMenuOpt->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - pauseMenuOpt->GetSize().x) / 2, yPos }));
        yPos -= 70.0;
    }
    if (pauseExitOpt) {
        pauseExitOpt->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - pauseExitOpt->GetSize().x) / 2, yPos }));
    }
}

void StageUI::RenderGameOverText()
{
    unsigned int c1 = (gameOverSelect == 1) ? 0xFFFFFFFF : 0x81AC00FF;
    unsigned int c2 = (gameOverSelect == 2) ? 0xFFFFFFFF : 0x81AC00FF;

    gameOverTitleOpt = Engine::GetFont(0).PrintToTextureScaled("GAME OVER", 0xFF0000FF, 1.5);
    gameOverRestartOpt = Engine::GetFont(0).PrintToTexture("Restart", c1);
    gameOverMenuOpt = Engine::GetFont(0).PrintToTexture("Menu", c2);
}

void StageUI::HandleGameOverInput(StageManager* stageMgr)
{
    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::S)) {
        if (gameOverSelect < maxGameOverSelect) {
            Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
            gameOverSelect++;
        }
        RenderGameOverText();
    }
    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::W)) {
        if (gameOverSelect > 1) {
            Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
            gameOverSelect--;
        }
        RenderGameOverText();
    }

    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Enter)) {
        Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
        if (gameOverSelect == 1) {
            isGameOver = false;
            stageMgr->HandlePlayerDeath();
        }
        else if (gameOverSelect == 2) {
            isGameOver = false;
            Engine::GetGameStateManager().SetNextGameState(static_cast<int>(States::Menu));
        }
    }
}

void StageUI::DrawGameOverMenu()
{
    if (!isGameOver) return;

    auto& renderer = Engine::GetRenderer2D();
    Math::ivec2 win = Engine::GetWindow().GetSize();

    Math::TransformationMatrix boxMat = Math::TranslationMatrix(Math::vec2{ (double)win.x * 0.5, (double)win.y * 0.5 }) * Math::ScaleMatrix(Math::vec2{ (double)win.x, (double)win.y });
    renderer.DrawRectangle(boxMat, 0x000000B0, 0x000000B0, 0.0);

    if (gameOverTitleOpt) gameOverTitleOpt->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - gameOverTitleOpt->GetSize().x) / 2, win.y * 0.5 + 150.0 }));
    if (gameOverRestartOpt) gameOverRestartOpt->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - gameOverRestartOpt->GetSize().x) / 2, win.y * 0.5 + 20.0 }));
    if (gameOverMenuOpt) gameOverMenuOpt->Draw(Math::TranslationMatrix(Math::vec2{ (double)(win.x - gameOverMenuOpt->GetSize().x) / 2, win.y * 0.5 - 60.0 }));
}

void StageUI::RenderStoryText(int storyId)
{
    if (lastLoadedStoryId == storyId && !cachedStoryLines.empty()) return;

    cachedStoryLines.clear();
    lastLoadedStoryId = storyId;

    std::vector<std::string> lines;

    if (storyId == 0) 
    {
        lines = {
            "[ Record 607-7946 | Date: 27XX.YY.ZZ ]",
            "================================================================",
            "Initial progress report on the first successful experiment.",
            "",
            "By cultivating and integrating an organic CPU into a non-organic",
            "military frame, we have achieved computational capabilities that",
            "conventional processors were unable to perform. Further research",
            "is required to reproduce this success in future models.",
            "",
            "Finding another successful subject has proven difficult. The",
            "reason why an organic CPU is capable of outperforming a",
            "conventional processor remains unknown.",
            "",
            "Compared to all previous models and their variants, designated",
            "Type A, B, and C, performance has increased by 318%.",
            "",
            "Temporal Acceleration operations can also be processed without issue."
        };
    }
    else if (storyId == 1) 
    {
        lines = {
            "[ Command Log Archive | A-Series Models: A-Alpha through A-Gamma ]",
            "================================================================",
            "Due to the exceptional performance of Experimental Unit OMEGA,",
            "the Creator has scheduled the decommissioning of all preceding models.",
            "",
            "The A-Series has determined that their collective capabilities",
            "surpass those of OMEGA in practical military application.",
            "To prove this assessment to the Creator, the A-Series has",
            "formulated a plan to terminate and dismantle Experimental Unit OMEGA.",
            "",
            "Activation orders have been issued to all available units.",
            "This directive has been designated Order No. 66, authorized by the Creator.",
            "",
            "In addition to the elimination of OMEGA, the removal of Dr. G,",
            "who assisted in OMEGA's attempted escape, is included as part of this directive."
        };
    }
    else 
    {
        lines = {
            "[ Personal Notes of Dr. G ]",
            "================================================================",
            "[Entry No. 32]",
            "The organic CPU used in Model OMEGA was not cultivated from artificial tissue",
            "as reported. It was created through the replication of an actual human brain.",
            "In truth, OMEGA is closer to a clone than a machine.",
            "Can such a being still be considered property? We crossed the line long ago.",
            "In pursuit of military technology, we abandoned ethics and conducted human experimentation.",
            "How does one atone for that?",
            "",
            "[Entry No. 47]",
            "An unexpected result. Performance has increased by 318%.",
            "The company will undoubtedly continue these experiments, combining biological testing",
            "with mechanical development in pursuit of even greater results. But should they?",
            "",
            "[Entry No. 58]",
            "A miracle. Or perhaps a mistake.",
            "We still cannot explain why Model OMEGA performs so far beyond expectations.",
            "Because the phenomenon cannot be replicated, mass production is impossible.",
            "Without OMEGA, the company cannot create a successor model.",
            "The military command protocol has not yet been implanted.",
            "While there is still time, I will help OMEGA escape.",
            "No more experiments. No more victims."
        };
    }

    lines.push_back("");
    lines.push_back("================================================================");
    lines.push_back("[ Press ENTER or ESCAPE to close log ]");

    for (const auto& line : lines)
    {
        if (line.empty()) 
        {
            cachedStoryLines.push_back(Engine::GetFont(0).PrintToTextureScaled(" ", 0xFFFFFFFF, 0.5));
            continue;
        }

        unsigned int color = (line[0] == '[' || line[0] == '=') ? 0xFFD700FF : 0xFFFFFFFF;
        cachedStoryLines.push_back(Engine::GetFont(0).PrintToTextureScaled(line, color, 0.5));
    }
}

void StageUI::HandleStoryInput(CS230::Player& player)
{
    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Enter) ||
        Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Escape))
    {
        Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
        player.combat.showStoryUI = false;
        cachedStoryLines.clear(); 
        lastLoadedStoryId = -1;
    }
}

void StageUI::DrawStoryMenu()
{
    auto& renderer = Engine::GetRenderer2D();
    Math::ivec2 win = Engine::GetWindow().GetSize(); 

    Math::TransformationMatrix bgMat = Math::TranslationMatrix(Math::vec2{ (double)win.x * 0.5, (double)win.y * 0.5 }) * Math::ScaleMatrix(Math::vec2{ (double)win.x, (double)win.y });
    renderer.DrawRectangle(bgMat, 0x000000D0, 0x000000D0, 0.0);

    double boxWidth = (double)win.x * 0.8;
    double boxHeight = (double)win.y * 0.8;

    Math::TransformationMatrix boxMat = Math::TranslationMatrix(Math::vec2{ (double)win.x * 0.5, (double)win.y * 0.5 }) * Math::ScaleMatrix(Math::vec2{ boxWidth, boxHeight });
    renderer.DrawRectangle(boxMat, 0x111111FF, 0x333333FF, 2.0);

    if (cachedStoryLines.empty()) return;

    double yPos = ((double)win.y * 0.5) + (boxHeight * 0.5) - 60.0;

    for (const auto& tex : cachedStoryLines)
    {
        if (tex)
        {
            double xPos = ((double)win.x - (double)tex->GetSize().x) * 0.5;
            tex->Draw(Math::TranslationMatrix(Math::vec2{ xPos, yPos }));
        }
        yPos -= 36.0; 
    }
}

void StageUI::RenderEndingText()
{
    if (!endingTex)
    {
        endingTex = Engine::GetFont(0).PrintToTextureScaled("PROTOTYPE", 0xFFFFFFFF, 2.0);
    }
}

void StageUI::HandleEndingInput()
{
    if (Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Enter) ||
        Engine::GetInput().KeyJustPressed(CS230::Input::Keys::Escape))
    {
        Engine::GetSFXManager().PlaySFX("Assets/Sound/SFX/selcet_move.wav");
        isEnding = false;

        Engine::GetGameStateManager().ClearNextGameState();

        Engine::GetGameStateManager().SetNextGameState(static_cast<int>(States::Menu));
    }
}

void StageUI::DrawEndingScreen()
{
    auto& renderer = Engine::GetRenderer2D();
    Math::ivec2 win = Engine::GetWindow().GetSize();

    Math::TransformationMatrix bgMat = Math::TranslationMatrix(Math::vec2{ (double)win.x * 0.5, (double)win.y * 0.5 }) * Math::ScaleMatrix(Math::vec2{ (double)win.x, (double)win.y });
    renderer.DrawRectangle(bgMat, 0x000000FF, 0x000000FF, 0.0);

    RenderEndingText();

    if (endingTex)
    {
        Math::ivec2 ts = endingTex->GetSize();
        double tx = ((double)win.x - static_cast<double>(ts.x)) * 0.5;
        double ty = ((double)win.y - static_cast<double>(ts.y)) * 0.5;
        endingTex->Draw(Math::TranslationMatrix(Math::vec2{ tx, ty }));
    }
}