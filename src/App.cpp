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
#include <random>

void App::Start() {
    LOG_TRACE("Start");

    m_Seed = static_cast<unsigned>(std::time(nullptr));
    m_World.Generate(m_Seed, 1);
    m_World.AddToRenderer(m_Root);

    CollisionSystem::SetWorld(&m_World);

    m_BulletManager.AddToRenderer(m_Root);

    m_Player = std::make_shared<Player>(&m_BulletManager);
    m_Player->SetWorldPos(m_World.GetSpawnPos());
    m_Root.AddChild(m_Player);
    m_Player->AddWeaponSpriteToRenderer(m_Root);

    // EnemyManager 先初始化渲染/目標，敵人由懶生成填入
    m_EnemyManager.AddToRenderer(m_Root);
    m_EnemyManager.SetTarget(m_Player.get());

    // Step 3.5：接近觸發懶生成
    m_World.SetOnApproachEnemyRoom([this](int idx) {
        SpawnEnemiesInRoom(idx);
    });

    m_HUD.AddToRenderer(m_Root);

    {
        std::vector<glm::ivec2> gridPos;
        gridPos.reserve(m_World.GetRoomCount());
        for (int i = 0; i < m_World.GetRoomCount(); ++i)
            gridPos.push_back(m_World.GetRoomGridPos(i));
        m_MiniMap.Init(m_World.GetRoomCount(), gridPos);
        m_MiniMap.AddToRenderer(m_Root);
    }

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

void App::SpawnEnemiesInRoom(int roomIdx) {
    if (m_World.AreEnemiesSpawned(roomIdx)) return;
    m_World.MarkRoomEnemiesSpawned(roomIdx);

    const glm::vec2 center = m_World.GetRoomOffset(roomIdx);
    const int cols = m_World.GetRoomCols(roomIdx);
    const int rows = m_World.GetRoomRows(roomIdx);

    const float hw = (cols * 0.5f - 3.0f) * TILE_SIZE;
    const float hh = (rows * 0.5f - 4.0f) * TILE_SIZE;

    std::mt19937 rng(m_Seed ^ static_cast<unsigned>(roomIdx * 0x9E3779B9u));
    std::uniform_real_distribution<float> dx(-hw, hw);
    std::uniform_real_distribution<float> dy(-hh, hh);

    const int area = cols * rows;
    int count;
    if      (area < 300) count = 3 + static_cast<int>(rng() % 3);
    else if (area < 450) count = 4 + static_cast<int>(rng() % 4);
    else                 count = 6 + static_cast<int>(rng() % 4);

    std::vector<Enemy*> ptrs;
    ptrs.reserve(static_cast<size_t>(count));
    for (int k = 0; k < count; ++k) {
        std::shared_ptr<Enemy> e;
        switch (rng() % 3) {
            case 0:  e = std::make_shared<PistolGoblin>(&m_BulletManager); break;
            case 1:  e = std::make_shared<SpearGoblin> (&m_BulletManager); break;
            default: e = std::make_shared<ArcherGoblin>(&m_BulletManager); break;
        }
        e->SetWorldPos({center.x + dx(rng), center.y + dy(rng)});
        ptrs.push_back(e.get());
        m_EnemyManager.AddEnemyLive(e);
    }
    m_World.AssignEnemiesToRoom(roomIdx, std::move(ptrs));
    m_World.OpenRoomForEntry(roomIdx);
}
