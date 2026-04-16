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

    // 所有 BASIC 房間自動生成敵人
    {
        std::mt19937 rng(seed ^ 0xCAFEBABEu);

        for (int i = 0; i < m_World.GetRoomCount(); ++i) {
            if (m_World.GetRoomType(i) != RoomType::BASIC) continue;

            const glm::vec2 center = m_World.GetRoomOffset(i);
            const int cols = m_World.GetRoomCols(i);
            const int rows = m_World.GetRoomRows(i);

            // 安全生成區域：距牆壁 3 格
            const float hw = (cols * 0.5f - 3.0f) * TILE_SIZE;
            const float hh = (rows * 0.5f - 4.0f) * TILE_SIZE;
            std::uniform_real_distribution<float> dx(-hw, hw);
            std::uniform_real_distribution<float> dy(-hh, hh);

            // 敵人數量依房間大小
            const int area = cols * rows;
            int count;
            if      (area < 300) count = 3 + static_cast<int>(rng() % 3);   // 3-5
            else if (area < 450) count = 4 + static_cast<int>(rng() % 4);   // 4-7
            else                 count = 6 + static_cast<int>(rng() % 4);   // 6-9

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
                m_EnemyManager.AddEnemy(e);
            }
            m_World.AssignEnemiesToRoom(i, std::move(ptrs));
        }

        m_EnemyManager.AddToRenderer(m_Root);
        m_EnemyManager.SetTarget(m_Player.get());
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
