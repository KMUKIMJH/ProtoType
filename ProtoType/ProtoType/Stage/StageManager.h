#pragma once
#include "../Engine/GameState.h"
#include "../Engine/Texture.h"
#include "StageUI.h"
#include "../Game/Player.h"
#include "../Game/Enemy.h"
#include "../Game/Platform.h"
#include "../Game/ProtoPortal.h"
#include "../Game/Backdoor.h"
#include "../Game/Generator.h"
#include "../Game/HPStation.h"
#include "RunProgressManager.h"
#include <vector>
#include <memory>
#include <string>
#include <functional>
#include <utility>

namespace CS230
{
    class MapLoader;
}

enum class EnemyType
{
    Melee,
    Ranged,
    Shield
};

enum class RespawnMode
{
    None,
    EntryPortal,
    ExitPortal,
    MidBossRoom
};

struct GameEntity
{
    CS230::GameObject* go{ nullptr };
    CS230::Enemy* logic{ nullptr };
    std::function<void(const CS230::Player&, double, bool)> updateFunc;
};

struct StageHandlers
{
    std::function<void(class StageManager&)> Setup;
    std::function<void(class StageManager&, double)> Update;
    std::function<void(class StageManager&, const Math::TransformationMatrix&)> Draw;
    std::function<void(class StageManager&)> Unload;
};

class StageManager : public CS230::GameState
{
    friend class CS230::MapLoader;

public:
    StageManager();
    void InitStage(int width, int height);

    Platform* AddPlatform(Math::irect rect);

    void AddText(std::string text, Math::vec2 position, unsigned int color, double scale);
    ProtoPortal* AddPortal(Math::ivec2 bottomLeft);
    CS230::GameObject* AddEnemy(Math::vec2 pos, EnemyType type, bool dropsStoryItem = false);
    void SetPortals(ProtoPortal* left, ProtoPortal* right);
    void Reset();

    void Load() override;
    void Update(double dt) override;
    void Draw() override;
    void Unload() override;

    std::string GetName() override
    {
        return "StageManager";
    }

    void HandlePlayerDeath();
    void NotifyEnemyKilled();

    void SetBackdoorUIOffset(int bdIndex, Math::vec2 offset);

    std::string currentBGM{ "" };
    int worldWidthCur{ 0 };
    int worldHeightCur{ 0 };

    std::vector<CS230::GameObject*> staticObjects;
    std::vector<GameEntity> enemies;
    std::vector<CS230::GameObject*> items;
    std::vector<CS230::Backdoor*> backdoors;

    CS230::Generator* generator{ nullptr };
    CS230::Player* GetPlayerPtr()
    {
        return &player;
    }

    StageHandlers handlers;

    static bool isTransitioningToSettings;

private:
    void BuildStage(int index);
    void DrawSolids(const Math::TransformationMatrix& camM) const;
    void ApplyPortalTargetsForSlot(int slot);
    void ClearEnemyInstances();
    bool AreEnemiesCleared() const;
    Math::vec2 PortalSpawnBottomLeft(const ProtoPortal* portal, const Math::vec2& fallback) const;
    Math::vec2 MidBossRespawnPoint(double groundTop, const Math::ivec2& offset) const;

    std::shared_ptr<CS230::Texture> timer_texture;
    std::shared_ptr<CS230::Texture> backgroundTex;

    CS230::Player player;
    StageUI ui;
    CS230::RunProgressManager runProgress;

    int stageIndex{ 0 };
    int currentSlotLoaded{ 0 };
    int pendingSlot{ -1 };
    RespawnMode pendingRespawn{ RespawnMode::None };

    double slowMotionTimerSec{ 0.0 };
    double freezeTimerSec{ 0.0 };
    bool hasPlayerSpawnOverride{ false };
    Math::vec2 playerSpawnOverride{ 0.0, 0.0 };

    int ignoreFloorTimer{ 0 };
    CS230::GameObject* ignoreFloorTarget{ nullptr };

    bool isBackdoorMenuOpen{ false };
    int backdoorMenuSelection{ 0 };
    CS230::Backdoor* activeBackdoor{ nullptr };

    std::vector<std::pair<std::shared_ptr<CS230::Texture>, Math::vec2>> texts;

    ProtoPortal* leftPortal{ nullptr };
    ProtoPortal* rightPortal{ nullptr };
};