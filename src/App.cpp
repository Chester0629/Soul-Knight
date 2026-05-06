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

    m_BaseSeed = static_cast<unsigned>(std::time(nullptr));
    m_LevelManager.Reset();
    m_Player = std::make_shared<Player>(&m_BulletManager);
    LoadFloor();

    m_CurrentState = State::UPDATE;
}

void App::LoadFloor() {
    m_Seed = m_BaseSeed ^ (static_cast<unsigned>(m_LevelManager.GetFloor()) * 0x9E3779B9u);

    // 重置渲染樹（釋放舊 children shared_ptr；m_Root 位址穩定，EnemyManager 指標仍有效）
    m_Root = Util::Renderer();
    m_EnemyManager.ClearEnemies();
    m_BulletManager.DeactivateAll();

    m_World.Generate(m_Seed, m_LevelManager.GetFloor());
    m_World.AddToRenderer(m_Root);

    CollisionSystem::SetWorld(&m_World);

    m_BulletManager.AddToRenderer(m_Root);

    m_Player->SetWorldPos(m_World.GetSpawnPos());
    m_Root.AddChild(m_Player);
    m_Player->AddWeaponSpriteToRenderer(m_Root);

    m_EnemyManager.AddToRenderer(m_Root);
    m_EnemyManager.SetTarget(m_Player.get());

    m_World.SetOnEnterEnemyRoom([this](int idx) {
        SpawnEnemiesInRoom(idx);
    });

    m_World.SetOnPortalEntered([this]() {
        m_LevelManager.NextFloor();
        if (m_LevelManager.IsComplete())
            m_CurrentState = State::END;
        else
            LoadFloor();
    });

    m_HUD.AddToRenderer(m_Root);

    {
        const int n = m_World.GetRoomCount();
        std::vector<glm::ivec2>        gridPos;
        std::vector<MiniMapRoomType>   mmTypes;
        std::vector<MiniMap::ConnInfo> mmConns;
        gridPos.reserve(n);
        mmTypes.reserve(n);

        for (int i = 0; i < n; ++i) {
            gridPos.push_back(m_World.GetRoomGridPos(i));
            switch (m_World.GetRoomType(i)) {
                case RoomType::SPAWN:  mmTypes.push_back(MiniMapRoomType::SPAWN);  break;
                case RoomType::PORTAL: mmTypes.push_back(MiniMapRoomType::PORTAL); break;
                default:               mmTypes.push_back(MiniMapRoomType::NORMAL); break;
            }
        }
        for (const auto& c : m_World.GetConnections())
            mmConns.push_back({c.fromIdx, c.toIdx, c.isHorizontal});

        m_MiniMap.Init(n, gridPos, mmTypes, mmConns);
        m_MiniMap.AddToRenderer(m_Root);
    }

    m_World.Update(m_World.GetSpawnPos());
}

void App::Update() {
    const float dt = Util::Time::GetDeltaTimeMs() / 1000.0f;

    m_Player->Update(dt);
    Camera::Update(m_Player->GetWorldPos());
    m_World.Update(m_Player->GetWorldPos(),
                   Util::Input::IsKeyDown(Util::Keycode::E));  // E 鍵互動傳送門
    m_Player->SyncRender(Camera::GetPosition());
    m_EnemyManager.Update(dt);
    m_BulletManager.Update(dt, Camera::GetPosition(), m_Player.get(), &m_EnemyManager);
    m_World.SyncTransforms(Camera::GetPosition());

    m_HUD.Update(m_Player->GetHP(),     m_Player->GetMaxHP(),
                 m_Player->GetShield(), m_Player->GetMaxShield(),
                 m_Player->GetEnergy(), m_Player->GetMaxEnergy());

    {
        const int n = m_World.GetRoomCount();
        std::vector<bool> visited, revealed;
        visited.reserve(n);
        revealed.reserve(n);
        for (int i = 0; i < n; ++i) {
            visited.push_back(m_World.IsRoomVisited(i));
            revealed.push_back(m_World.IsRoomRevealed(i));
        }
        m_MiniMap.Update(m_World.GetCurrentRoomIdx(), visited, revealed);
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

    // Debug：N 鍵切換下一層
    if (Util::Input::IsKeyDown(Util::Keycode::N)) {
        m_LevelManager.NextFloor();
        if (m_LevelManager.IsComplete()) {
            m_CurrentState = State::END;
        } else {
            LoadFloor();
        }
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
}
