#include "App.hpp"

#include "Entity/Enemy.hpp"
#include "System/CollisionSystem.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "World/Camera.hpp"

#include <cstdlib>
#include <ctime>

void App::Start() {
    LOG_TRACE("Start");

    // Step 3.2：生成多房間地城（隨機 seed）
    const unsigned seed = static_cast<unsigned>(std::time(nullptr));
    m_World.Generate(seed, 1);
    m_World.AddToRenderer(m_Root);

    // CollisionSystem 改為 World 版本
    CollisionSystem::SetWorld(&m_World);

    // Step 2.2：子彈對象池加入渲染樹
    m_BulletManager.AddToRenderer(m_Root);

    // 玩家：出生於 Spawn 房間中心
    m_Player = std::make_shared<Player>(&m_BulletManager);
    m_Player->SetWorldPos(m_World.GetSpawnPos());
    m_Root.AddChild(m_Player);
    m_Player->AddWeaponSpriteToRenderer(m_Root);

    // 測試哥布林（相對於 Spawn 房間中心偏移）
    {
        const glm::vec2 spawnCenter = m_World.GetSpawnPos();

        auto pistol = std::make_shared<PistolGoblin>(&m_BulletManager);
        pistol->SetWorldPos(spawnCenter + glm::vec2{200.0f,  100.0f});

        auto spear = std::make_shared<SpearGoblin>(&m_BulletManager);
        spear->SetWorldPos(spawnCenter + glm::vec2{-150.0f,  50.0f});

        auto archer = std::make_shared<ArcherGoblin>(&m_BulletManager);
        archer->SetWorldPos(spawnCenter + glm::vec2{0.0f, -150.0f});

        m_EnemyManager.AddEnemy(pistol);
        m_EnemyManager.AddEnemy(spear);
        m_EnemyManager.AddEnemy(archer);
        m_EnemyManager.AddToRenderer(m_Root);
        m_EnemyManager.SetTarget(m_Player.get());
    }

    // HUD 最後加入（確保 Z 在最上層）
    m_HUD.AddToRenderer(m_Root);

    m_CurrentState = State::UPDATE;
}

void App::Update() {
    const float dt = Util::Time::GetDeltaTimeMs() / 1000.0f;

    m_Player->Update(dt);
    Camera::Update(m_Player->GetWorldPos());
    m_Player->SyncRender(Camera::GetPosition());
    m_EnemyManager.Update(dt);
    m_BulletManager.Update(dt, Camera::GetPosition(), m_Player.get(), &m_EnemyManager);
    m_World.SyncTransforms(Camera::GetPosition());

    m_HUD.Update(m_Player->GetHP(),     m_Player->GetMaxHP(),
                 m_Player->GetShield(), m_Player->GetMaxShield(),
                 m_Player->GetEnergy(), m_Player->GetMaxEnergy());

    m_Root.Update();

    if (m_Player->IsDead()) {
        m_CurrentState = State::END;
        return;
    }

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) ||
        Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

void App::End() {
    LOG_TRACE("End");
}
