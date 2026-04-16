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

    // Step 3.3：Spawn 房間無敵人，門初始開啟。
    // Room[1]（第一個 Basic 房間）放測試哥布林，進房後門關閉，殺完才開。
    {
        const glm::vec2 room1Center = m_World.GetRoomOffset(1);

        auto pistol = std::make_shared<PistolGoblin>(&m_BulletManager);
        pistol->SetWorldPos(room1Center + glm::vec2{ 150.0f,  80.0f});

        auto spear  = std::make_shared<SpearGoblin>(&m_BulletManager);
        spear->SetWorldPos(room1Center + glm::vec2{-120.0f,  40.0f});

        auto archer = std::make_shared<ArcherGoblin>(&m_BulletManager);
        archer->SetWorldPos(room1Center + glm::vec2{  0.0f, -120.0f});

        m_EnemyManager.AddEnemy(pistol);
        m_EnemyManager.AddEnemy(spear);
        m_EnemyManager.AddEnemy(archer);
        m_EnemyManager.AddToRenderer(m_Root);
        m_EnemyManager.SetTarget(m_Player.get());

        m_World.AssignEnemiesToRoom(1, {pistol.get(), spear.get(), archer.get()});
    }

    // HUD + MiniMap 最後加入（確保 Z 在最上層）
    m_HUD.AddToRenderer(m_Root);

    // Step 3.4：迷你地圖
    {
        std::vector<glm::ivec2> gridPos;
        gridPos.reserve(m_World.GetRoomCount());
        for (int i = 0; i < m_World.GetRoomCount(); ++i)
            gridPos.push_back(m_World.GetRoomGridPos(i));
        m_MiniMap.Init(m_World.GetRoomCount(), gridPos);
        m_MiniMap.AddToRenderer(m_Root);
    }

    // Step 3.3：初始門狀態（Spawn 無敵人 → 立刻開啟，Room[1] 有敵人 → 保持關閉）
    m_World.Update(m_World.GetSpawnPos());

    m_CurrentState = State::UPDATE;
}

void App::Update() {
    const float dt = Util::Time::GetDeltaTimeMs() / 1000.0f;

    m_Player->Update(dt);
    Camera::Update(m_Player->GetWorldPos());
    m_World.Update(m_Player->GetWorldPos());   // 必須在 SyncRender 前，確保 ZOffset 正確
    m_Player->SyncRender(Camera::GetPosition());
    m_EnemyManager.Update(dt);
    m_BulletManager.Update(dt, Camera::GetPosition(), m_Player.get(), &m_EnemyManager);
    m_World.SyncTransforms(Camera::GetPosition());

    m_HUD.Update(m_Player->GetHP(),     m_Player->GetMaxHP(),
                 m_Player->GetShield(), m_Player->GetMaxShield(),
                 m_Player->GetEnergy(), m_Player->GetMaxEnergy());

    {
        std::vector<bool> visited;
        visited.reserve(m_World.GetRoomCount());
        for (int i = 0; i < m_World.GetRoomCount(); ++i)
            visited.push_back(m_World.IsRoomVisited(i));
        m_MiniMap.Update(m_World.GetCurrentRoomIdx(), visited);
    }

    m_Root.Update();

    if (m_Player->IsDead()) {
        m_CurrentState = State::END;
        return;
    }

    // Debug：F 鍵切換所有房間的門（Step 3.3 驗收用）
    if (Util::Input::IsKeyDown(Util::Keycode::F)) {
        m_World.DebugToggleDoors();
    }

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) ||
        Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

void App::End() {
    LOG_TRACE("End");
}
