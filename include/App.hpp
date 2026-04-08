#pragma once

#include "pch.hpp" // IWYU pragma: export

#include "Room.hpp"
#include "Util/Renderer.hpp"

class App {
public:
    enum class State {
        START,
        UPDATE,
        END,
    };

    State GetCurrentState() const { return m_CurrentState; }

    void Start();
    void Update();
    void End(); // NOLINT(readability-convert-member-functions-to-static)

private:
    State m_CurrentState = State::START;

    Util::Renderer m_Root;
    // Step 1.2：測試房間（17×17 含牆，地板 15×15）
    Room m_Room{RoomSpec::START_W, RoomSpec::START_H};
};
