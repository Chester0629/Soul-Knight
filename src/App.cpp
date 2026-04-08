#include "App.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"

void App::Start() {
    LOG_TRACE("Start");

    // Step 1.2：測試房間加入渲染樹，以世界原點為中心
    m_Room.AddToRenderer(m_Root);
    m_Room.SyncTransforms({0.0f, 0.0f});

    m_CurrentState = State::UPDATE;
}

void App::Update() {
    // Step 1.2：房間靜態渲染（無玩家、無相機）
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
