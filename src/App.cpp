#include "App.hpp"

#include "Entity/Boss.hpp"
#include "Entity/Enemy.hpp"
#include "System/CollisionSystem.hpp"
#include "Util/Color.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "World/Camera.hpp"

#include <cstdlib>
#include <ctime>
#include <random>
#include <string>

static constexpr const char* FONT = PTSD_ASSETS_DIR "/fonts/Inter.ttf";

// ── Overlay 輔助 ──────────────────────────────────────────────────────────────

static std::shared_ptr<Util::GameObject> MakeTextObj(
    const std::shared_ptr<Util::Text>& text, float cx, float cy, float z)
{
    auto obj = std::make_shared<Util::GameObject>();
    obj->SetDrawable(text);
    obj->m_Transform.scale       = {1.0f, 1.0f};
    obj->m_Transform.translation = {cx, cy};
    obj->SetZIndex(z);
    obj->SetVisible(false);
    return obj;
}

// ── App::Start ────────────────────────────────────────────────────────────────

void App::Start() {
    LOG_TRACE("Start");
    if (m_Initialized) {
        Cleanup();
        HideOverlay();
    }
    m_MainMenuUI.Init(m_Root);
    m_CurrentState = State::MAIN_MENU;
}

// ── App::UpdateMainMenu ───────────────────────────────────────────────────────

void App::UpdateMainMenu() {
    m_Root.Update();
    const auto action = m_MainMenuUI.Update();
    if (action == MainMenuUI::Action::StartGame) {
        m_MainMenuUI.Cleanup(m_Root);
        const std::array<bool,3> upgraded = {m_C01Upgraded, m_C02Upgraded, m_C03Upgraded};
        m_CharSelectUI.Init(m_Root, m_Crystals, upgraded);
        m_CurrentState = State::CHARACTER_SELECT;
    } else if (action == MainMenuUI::Action::Quit) {
        m_CurrentState = State::END;
    }
}

// ── App::UpdateCharacterSelect ────────────────────────────────────────────────

void App::UpdateCharacterSelect() {
    m_Root.Update();
    const auto result = m_CharSelectUI.Update(m_Crystals);
    if (result == CharacterSelectUI::Result::Back) {
        m_CharSelectUI.Cleanup(m_Root);
        m_MainMenuUI.Init(m_Root);
        m_CurrentState = State::MAIN_MENU;
    } else if (result == CharacterSelectUI::Result::Confirmed) {
        // Persist upgrade states
        m_C01Upgraded = m_CharSelectUI.GetUpgraded(0);
        m_C02Upgraded = m_CharSelectUI.GetUpgraded(1);
        m_C03Upgraded = m_CharSelectUI.GetUpgraded(2);
        m_CharSelectUI.Cleanup(m_Root);
        // For now jump straight to gameplay (UPGRADE room not yet implemented)
        m_CurrentState = State::UPGRADE;
    }
}

// ── App::UpdateUpgrade ────────────────────────────────────────────────────────

void App::UpdateUpgrade() {
    // Placeholder: upgrade room not yet implemented — go straight to game
    if (m_Initialized) Cleanup();
    m_LevelManager.Reset();
    m_Seed          = static_cast<unsigned>(std::time(nullptr));
    m_PortalActive  = false;
    m_VictoryLocked = false;
    m_Portal.Hide();
    InitFloor();
}

// ── InitFloor ─────────────────────────────────────────────────────────────────

void App::InitFloor() {
    m_World.Generate(m_Seed, m_LevelManager.GetFloor());
    m_World.AddToRenderer(m_Root);
    CollisionSystem::SetWorld(&m_World);

    if (!m_Initialized) {
        m_BulletManager.AddToRenderer(m_Root);
    }

    m_Player = std::make_shared<Player>(&m_BulletManager);
    m_Player->SetWorldPos(m_World.GetSpawnPos());
    m_Root.AddChild(m_Player);
    m_Player->AddWeaponSpriteToRenderer(m_Root);

    m_EnemyManager.AddToRenderer(m_Root);
    m_EnemyManager.SetTarget(m_Player.get());

    m_World.SetOnEnterEnemyRoom([this](int idx) {
        SpawnEnemiesInRoom(idx);
    });

    m_Portal.AddToRenderer(m_Root);

    if (!m_Initialized) {
        m_HUD.AddToRenderer(m_Root);

        // Overlay — 建立一次，常駐 Renderer，SetVisible 切換顯示
        m_OverlayTitle = std::make_shared<Util::Text>(FONT, 42, "GAME OVER",
                             Util::Color{255, 80, 80, 255});
        m_OverlayStats = std::make_shared<Util::Text>(FONT, 22, "",
                             Util::Color{255, 255, 255, 255});
        m_OverlayHint  = std::make_shared<Util::Text>(FONT, 16, "Press R to restart  |  ESC to quit",
                             Util::Color{180, 180, 180, 255});

        m_OverlayTitleObj = MakeTextObj(m_OverlayTitle,  0.0f,  120.0f, 99.9f);
        m_OverlayStatsObj = MakeTextObj(m_OverlayStats,  0.0f,   20.0f, 99.9f);
        m_OverlayHintObj  = MakeTextObj(m_OverlayHint,   0.0f, -100.0f, 99.9f);

        m_Root.AddChild(m_OverlayTitleObj);
        m_Root.AddChild(m_OverlayStatsObj);
        m_Root.AddChild(m_OverlayHintObj);
    }

    {
        std::vector<glm::ivec2> gridPos;
        gridPos.reserve(m_World.GetRoomCount());
        for (int i = 0; i < m_World.GetRoomCount(); ++i)
            gridPos.push_back(m_World.GetRoomGridPos(i));
        m_MiniMap.Init(m_World.GetRoomCount(), gridPos);
        m_MiniMap.AddToRenderer(m_Root);
    }

    m_World.Update(m_World.GetSpawnPos());

    m_Initialized   = true;
    m_CurrentState  = State::UPDATE;
}

// ── Cleanup ───────────────────────────────────────────────────────────────────

void App::Cleanup() {
    m_World.RemoveFromRenderer(m_Root);
    m_EnemyManager.Clear(m_Root);
    m_Portal.RemoveFromRenderer(m_Root);
    m_MiniMap.RemoveFromRenderer(m_Root);
    if (m_Player) {
        m_Root.RemoveChild(m_Player);
        m_Player->RemoveWeaponSpriteFromRenderer(m_Root);
    }
    m_BulletManager.Reset();
}

// ── LoadNextFloor ─────────────────────────────────────────────────────────────

void App::LoadNextFloor() {
    Cleanup();
    m_Seed        ^= static_cast<unsigned>(m_LevelManager.GetFloor() * 0x9E3779B9u);
    m_PortalActive  = false;
    m_VictoryLocked = false;
    m_Portal.Hide();
    InitFloor();
}

// ── App::Update ───────────────────────────────────────────────────────────────

void App::Update() {
    const float dt = Util::Time::GetDeltaTimeMs() / 1000.0f;

    m_Player->Update(dt);
    Camera::Update(m_Player->GetWorldPos());
    m_World.Update(m_Player->GetWorldPos());
    m_Player->SyncRender(Camera::GetPosition());
    m_EnemyManager.Update(dt);
    m_BulletManager.Update(dt, Camera::GetPosition(), m_Player.get(), &m_EnemyManager);
    m_World.SyncTransforms(Camera::GetPosition());
    m_Portal.SyncRender(Camera::GetPosition());

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

    // 累計擊殺數
    m_LevelManager.AddKills(m_EnemyManager.TakeKills());

    m_Root.Update();

    // ── 死亡檢查 ──────────────────────────────────────────────────────────────
    if (m_Player->IsDead()) {
        ShowGameOver();
        m_CurrentState = State::GAME_OVER;
        return;
    }

    // ── Boss 層勝利 ───────────────────────────────────────────────────────────
    if (!m_VictoryLocked && m_LevelManager.IsBossFloor()) {
        if (m_World.IsBossCleared()) {
            m_VictoryLocked = true;
            ShowVictory();
            m_CurrentState = State::VICTORY;
            return;
        }
    }

    // ── 傳送陣出現（普通層，所有 BASIC 房間清空）────────────────────────────
    if (!m_PortalActive && !m_LevelManager.IsBossFloor()) {
        if (m_World.HasPortalRoom() && m_World.AllCombatRoomsCleared()) {
            m_PortalActive = true;
            m_Portal.Show(m_World.GetPortalRoomPos());
        }
    }

    // ── 踩上傳送陣 → 進入下一層 ──────────────────────────────────────────────
    if (m_PortalActive && m_Portal.IsTriggered(m_Player->GetWorldPos())) {
        m_LevelManager.NextFloor();
        LoadNextFloor();
        return;
    }

    // Debug：F 鍵切換門
    if (Util::Input::IsKeyDown(Util::Keycode::F))
        m_World.DebugToggleDoors();

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) ||
        Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

// ── App::UpdateOverlay ────────────────────────────────────────────────────────

void App::UpdateOverlay() {
    m_Root.Update();

    if (Util::Input::IsKeyDown(Util::Keycode::R)) {
        HideOverlay();
        m_CurrentState = State::START;  // re-enters main menu
        return;
    }
    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) ||
        Util::Input::IfExit()) {
        HideOverlay();
        m_CurrentState = State::END;
    }
}

// ── App::End ──────────────────────────────────────────────────────────────────

void App::End() {
    LOG_TRACE("End");
}

// ── Overlay 輔助 ──────────────────────────────────────────────────────────────

void App::ShowGameOver() {
    m_OverlayTitle->SetColor(Util::Color{255, 80, 80, 255});
    m_OverlayTitle->SetText("GAME OVER");
    m_OverlayStats->SetText(
        "Floor: " + std::to_string(m_LevelManager.GetFloor()) +
        "   Kills: " + std::to_string(m_LevelManager.GetKills()));
    m_OverlayTitleObj->SetVisible(true);
    m_OverlayStatsObj->SetVisible(true);
    m_OverlayHintObj->SetVisible(true);
}

void App::ShowVictory() {
    m_OverlayTitle->SetColor(Util::Color{255, 220, 60, 255});
    m_OverlayTitle->SetText("VICTORY!");
    m_OverlayStats->SetText(
        "All 5 Floors Cleared!   Kills: " +
        std::to_string(m_LevelManager.GetKills()));
    m_OverlayTitleObj->SetVisible(true);
    m_OverlayStatsObj->SetVisible(true);
    m_OverlayHintObj->SetVisible(true);
}

void App::HideOverlay() {
    m_OverlayTitleObj->SetVisible(false);
    m_OverlayStatsObj->SetVisible(false);
    m_OverlayHintObj->SetVisible(false);
}

// ── SpawnEnemiesInRoom ────────────────────────────────────────────────────────

void App::SpawnEnemiesInRoom(int roomIdx) {
    if (m_World.AreEnemiesSpawned(roomIdx)) return;
    m_World.MarkRoomEnemiesSpawned(roomIdx);

    // Boss 層：BOSS 房間生成 Boss，其餘 BASIC 房間生成普通敵人
    if (m_World.GetRoomType(roomIdx) == RoomType::BOSS) {
        auto boss = std::make_shared<Boss>(&m_BulletManager);
        boss->SetWorldPos(m_World.GetRoomOffset(roomIdx));
        Enemy* rawPtr = boss.get();
        m_EnemyManager.AddEnemyLive(boss);
        m_World.AssignEnemiesToRoom(roomIdx, {rawPtr});
        return;
    }

    // 普通敵人生成（沿用 M3 邏輯）
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
