#pragma once

#include "pch.hpp" // IWYU pragma: export

#include "Entity/Player.hpp"
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
    Room           m_Room{RoomSpec::START_W, RoomSpec::START_H};
    // Step 1.3：玩家
    std::shared_ptr<Player> m_Player;
};
