#include "App.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"

void App::Start() {
    LOG_TRACE("Start");

    m_Room.AddToRenderer(m_Root);
    m_Room.SyncTransforms({0.0f, 0.0f});

    // Step 1.3：玩家
    m_Player = std::make_shared<Player>();
    m_Root.AddChild(m_Player);

    m_CurrentState = State::UPDATE;
}

void App::Update() {
    const float dt = Util::Time::GetDeltaTimeMs() / 1000.0f;

    // Step 1.3：玩家更新（含 WASD 移動、動畫、朝向）
    // 尚無相機，房間固定在原點
    m_Player->Update(dt);
    m_Room.SyncTransforms({0.0f, 0.0f});
    m_Root.Update();

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) ||
        Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

void App::End() {
    LOG_TRACE("End");
}
