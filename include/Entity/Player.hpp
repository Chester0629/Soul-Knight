#pragma once

#include "Entity/Entity.hpp"
#include "Util/Animation.hpp"

// Player — 玩家角色
// 規格：
//   初始位置  (-300, -100)，速度 300 px/s
//   Walk 動畫：c01_0~3，100ms，循環
//   Idle 動畫：c01_4~7，150ms，循環
//   朝向：scale.x 正/負 3，只在有水平輸入時切換
//   m_LastMoveDir：記錄最後一次 WASD 方向，初始 {1,0}
class Player : public Entity {
public:
    Player();
    void Update(float dt) override;

    static constexpr float SPEED        = 300.0f;
    static constexpr float INITIAL_X    = -300.0f;
    static constexpr float INITIAL_Y    = -100.0f;
    static constexpr int   HIT_W        = 44;
    static constexpr int   HIT_H        = 20;
    static constexpr float HIT_OFFSET_Y = -10.0f;

    glm::vec2 GetLastMoveDir() const { return m_LastMoveDir; }
    bool      IsFacingLeft()   const { return m_FacingLeft; }

private:
    void HandleInput(float dt);

    std::shared_ptr<Util::Animation> m_WalkAnim;
    std::shared_ptr<Util::Animation> m_IdleAnim;
    bool m_IsWalkingAnim = false;

    bool      m_FacingLeft  = false;
    glm::vec2 m_LastMoveDir = {1.0f, 0.0f};
};
