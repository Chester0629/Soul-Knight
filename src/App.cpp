#include "App.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "World/Camera.hpp"

void App::Start() {
    LOG_TRACE("Start");

    m_Room.AddToRenderer(m_Root);
    m_Room.SyncTransforms({0.0f, 0.0f});

    m_Player = std::make_shared<Player>();
    m_Root.AddChild(m_Player);

    m_CurrentState = State::UPDATE;
}

void App::Update() {
    const float dt = Util::Time::GetDeltaTimeMs() / 1000.0f;

    // Step 1.4 更新順序：
    //   1. 玩家輸入 + 物理移動（更新 m_WorldPos）
    //   2. 相機跟隨（即時，無 Lerp）
    //   3. 玩家渲染同步（此時相機已更新，玩家永遠置中）
    //   4. 房間渲染同步
    m_Player->Update(dt);
    Camera::Update(m_Player->GetWorldPos());
    m_Player->SyncRender(Camera::GetPosition());
    m_Room.SyncTransforms(Camera::GetPosition());

    m_Root.Update();

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) ||
        Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

void App::End() {
    LOG_TRACE("End");
}
