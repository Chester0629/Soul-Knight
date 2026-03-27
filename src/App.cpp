#include "App.hpp"

#include "System/CollisionSystem.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "World/Camera.hpp"

void App::Start() {
    LOG_TRACE("Start");

    // Step 1.2：測試用初始房間（17×17 含牆，地板 15×15）
    m_Room.AddToRenderer(m_Root);
    m_Room.SyncTransforms({0.0f, 0.0f});

    // Step 1.5：設定碰撞系統的目標房間
    CollisionSystem::SetRoom(&m_Room);

    // Step 1.3：玩家
    m_Player = std::make_shared<Player>();
    m_Root.AddChild(m_Player);

    m_CurrentState = State::UPDATE;
}

void App::Update() {
    const float dt = Util::Time::GetDeltaTimeMs() / 1000.0f;

    // 每幀更新順序（Step 1.3 + 1.4）：
    //   1. 玩家輸入 + 物理移動（更新 m_WorldPos）
    //   2. 相機跟隨玩家新位置（即時，無 Lerp）
    //   3. 玩家渲染同步（此時 camera 已是最新值，玩家完美置中）
    //   4. 房間渲染同步
    m_Player->Update(dt);
    Camera::Update(m_Player->GetWorldPos());
    m_Player->SyncRender(Camera::GetPosition());
    m_Room.SyncTransforms(Camera::GetPosition());

    // 渲染所有 GameObject（依 Z-index 排序）
    m_Root.Update();

    // ESC 或關閉視窗 → 結束
    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) ||
        Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

void App::End() {
    LOG_TRACE("End");
}
