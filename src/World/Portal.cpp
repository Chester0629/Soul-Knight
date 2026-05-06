#include "World/Portal.hpp"

Portal::Portal() {
    m_Anim = std::make_shared<Util::Animation>(
        std::vector<std::string>{
            RESOURCE_DIR "/Objects/effect04_5.png",
            RESOURCE_DIR "/Objects/effect04_6.png",
            RESOURCE_DIR "/Objects/effect04_7.png",
            RESOURCE_DIR "/Objects/effect04_8.png",
            RESOURCE_DIR "/Objects/effect04_9.png",
            RESOURCE_DIR "/Objects/effect04_10.png",
            RESOURCE_DIR "/Objects/effect04_11.png",
            RESOURCE_DIR "/Objects/effect04_12.png",
        },
        true, 120, true, 0
    );
    SetDrawable(m_Anim);
    m_Transform.scale = {3.0f, 3.0f};
    SetZIndex(5.0f);
    SetVisible(false);
}

void Portal::Activate() {
    if (m_Active) return;
    m_Active = true;
    SetVisible(true);
}

bool Portal::ContainsPlayer(glm::vec2 playerPos) const {
    if (!m_Active) return false;
    const glm::vec2 d = playerPos - m_WorldPos;
    return d.x * d.x + d.y * d.y < TRIGGER_RADIUS * TRIGGER_RADIUS;
}

void Portal::SyncTransform(glm::vec2 cameraPos) {
    m_Transform.translation = m_WorldPos - cameraPos;
}
