#pragma once

#include "Util/GameObject.hpp"
#include "Util/Renderer.hpp"
#include "pch.hpp"

#include <memory>

// Portal — 傳送陣動畫（effect04_5~12，8 幀循環）
// 出現時機：當前普通層所有 BASIC 房間清空後
// 觸發條件：玩家踩上（距離 < TRIGGER_RADIUS）→ App 切換下一層
class Portal {
public:
    Portal();

    void AddToRenderer(Util::Renderer& renderer);
    void RemoveFromRenderer(Util::Renderer& renderer);
    void SyncRender(glm::vec2 cameraPos);

    void Show(glm::vec2 worldPos);
    void Hide();

    bool IsVisible()  const { return m_Visible; }
    bool IsTriggered(glm::vec2 playerPos) const;

    static constexpr float TRIGGER_RADIUS = 32.0f;

private:
    std::shared_ptr<Util::GameObject> m_Obj;
    glm::vec2 m_WorldPos = {0.0f, 0.0f};
    bool      m_Visible  = false;
};
