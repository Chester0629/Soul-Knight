#pragma once

#include "Util/GameObject.hpp"
#include "pch.hpp"

// Entity — 所有可移動有生命物件的基類
// 核心規則：
//   m_WorldPos              = 物理移動與碰撞的唯一真實座標
//   m_Transform.translation = 僅供 PTSD Renderer 渲染，由 SyncRenderTransform() 更新
//   ⚠️ 嚴禁直接修改 m_Transform.translation 作為物理位移！
class Entity : public Util::GameObject {
public:
    virtual ~Entity() = default;

    // 每幀更新（子類必須實作）
    virtual void Update(float dt) = 0;

    // 受傷（HP 減少）
    virtual void TakeDamage(int damage) { m_HP -= damage; }

    // 是否死亡
    virtual bool IsDead() const { return m_HP <= 0; }

    glm::vec2 GetWorldPos() const { return m_WorldPos; }
    void      SetWorldPos(glm::vec2 pos) { m_WorldPos = pos; }

    // 渲染同步（供 App 在 Camera 更新後呼叫）
    // 正確順序：UpdatePhysics → Camera::Update → SyncRender
    void SyncRender(glm::vec2 cameraPos) {
        m_Transform.translation = m_WorldPos - cameraPos;
        SetZIndex(glm::clamp(50.0f - m_WorldPos.y / 6.0f, 2.0f, 98.0f));
    }

protected:
    // ⚠️ 供子類在 Constructor 初始化時使用
    void SyncRenderTransform(glm::vec2 cameraPos) {
        m_Transform.translation = m_WorldPos - cameraPos;
    }
    void UpdateZIndex() {
        SetZIndex(glm::clamp(50.0f - m_WorldPos.y / 6.0f, 2.0f, 98.0f));
    }

    glm::vec2 m_WorldPos = {0.0f, 0.0f};
    int       m_HP       = 1;
    int       m_MaxHP    = 1;
    float     m_Speed    = 0.0f;
};
