#pragma once

#include "pch.hpp" // IWYU pragma: export

#include "Entity/EnemyManager.hpp"
#include "Entity/Player.hpp"
#include "UI/HUD.hpp"
#include "UI/MiniMap.hpp"
#include "Util/Renderer.hpp"
#include "Weapon/BulletManager.hpp"
#include "World/World.hpp"

class App {
public:
    enum class State {
        START,
        UPDATE,
        END,
    };

    State GetCurrentState() const { return m_CurrentState; }

    void Start();
    void Update();
    void End(); // NOLINT(readability-convert-member-functions-to-static)

private:
    State m_CurrentState = State::START;

    Util::Renderer m_Root;
    World          m_World;          // Step 3.2+：多房間地城世界

    // BulletManager 必須在 Player/EnemyManager 前宣告，確保生命週期
    BulletManager            m_BulletManager;
    std::shared_ptr<Player>  m_Player;
    EnemyManager             m_EnemyManager;
    HUD                      m_HUD;
    MiniMap                  m_MiniMap;

    unsigned m_Seed = 0;

    void SpawnEnemiesInRoom(int roomIdx);
};
