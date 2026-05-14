#include "World/Portal.hpp"
#include "Util/Animation.hpp"

#include <cmath>
#include <string>
#include <vector>

Portal::Portal() {
    std::vector<std::string> frames;
    frames.reserve(8);
    for (int i = 5; i <= 12; ++i)
        frames.push_back(std::string(RESOURCE_DIR) + "/Objects/effect04_" + std::to_string(i) + ".png");

    auto anim = std::make_shared<Util::Animation>(frames, true, 100, true, 0);

    m_Obj = std::make_shared<Util::GameObject>();
    m_Obj->SetDrawable(anim);
    m_Obj->m_Transform.scale = {3.0f, 3.0f};
    m_Obj->SetZIndex(1.5f);
    m_Obj->SetVisible(false);
}

void Portal::AddToRenderer(Util::Renderer& renderer) {
    renderer.AddChild(m_Obj);
}

void Portal::RemoveFromRenderer(Util::Renderer& renderer) {
    renderer.RemoveChild(m_Obj);
}

void Portal::SyncRender(glm::vec2 cameraPos) {
    if (!m_Visible) return;
    m_Obj->m_Transform.translation = m_WorldPos - cameraPos;
}

void Portal::Show(glm::vec2 worldPos) {
    m_WorldPos = worldPos;
    m_Visible  = true;
    m_Obj->SetVisible(true);
}

void Portal::Hide() {
    m_Visible = false;
    m_Obj->SetVisible(false);
}

bool Portal::IsTriggered(glm::vec2 playerPos) const {
    if (!m_Visible) return false;
    const float dx = playerPos.x - m_WorldPos.x;
    const float dy = playerPos.y - m_WorldPos.y;
    return (dx * dx + dy * dy) < (TRIGGER_RADIUS * TRIGGER_RADIUS);
}
