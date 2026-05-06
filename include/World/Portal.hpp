#pragma once

#include "Util/Animation.hpp"
#include "Util/GameObject.hpp"
#include "pch.hpp"

#include <memory>

class Portal : public Util::GameObject {
public:
    Portal();

    void SetWorldPos(glm::vec2 pos) { m_WorldPos = pos; }
    void Activate();
    bool IsActive() const { return m_Active; }

    // true when player center is within TRIGGER_RADIUS of portal center
    bool ContainsPlayer(glm::vec2 playerPos) const;
    void SyncTransform(glm::vec2 cameraPos);

    static constexpr float TRIGGER_RADIUS = 64.0f;

private:
    glm::vec2                        m_WorldPos = {0.0f, 0.0f};
    bool                             m_Active   = false;
    std::shared_ptr<Util::Animation> m_Anim;
};
