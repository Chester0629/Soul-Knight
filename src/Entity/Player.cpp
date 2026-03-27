#include "Entity/Player.hpp"

#include "System/CollisionSystem.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "World/Camera.hpp"

Player::Player() {
    m_WorldPos = {INITIAL_X, INITIAL_Y};
    m_Speed    = SPEED;
    m_HP = m_MaxHP = 10;  // M2 前暫用此值，後期由角色設計決定

    // Walk 動畫：c01_0~3，100ms，循環
    m_WalkAnim = std::make_shared<Util::Animation>(
        std::vector<std::string>{
            RESOURCE_DIR "/Characters/c01_0.png",
            RESOURCE_DIR "/Characters/c01_1.png",
            RESOURCE_DIR "/Characters/c01_2.png",
            RESOURCE_DIR "/Characters/c01_3.png",
        },
        true, 100, true, 0
    );

    // Idle 動畫：c01_4~7，150ms，循環
    m_IdleAnim = std::make_shared<Util::Animation>(
        std::vector<std::string>{
            RESOURCE_DIR "/Characters/c01_4.png",
            RESOURCE_DIR "/Characters/c01_5.png",
            RESOURCE_DIR "/Characters/c01_6.png",
            RESOURCE_DIR "/Characters/c01_7.png",
        },
        true, 150, true, 0
    );

    // 初始播放 Idle 動畫
    SetDrawable(m_IdleAnim);
    m_IsWalkingAnim = false;

    m_Transform.scale = {3.0f, 3.0f};  // 16px → 48px
    SetVisible(true);

    // 初始同步渲染位置
    UpdateZIndex();
    SyncRenderTransform(Camera::GetPosition());
}

void Player::HandleInput(float dt) {
    glm::vec2 dir = {0.0f, 0.0f};

    if (Util::Input::IsKeyPressed(Util::Keycode::W)) dir.y += 1.0f;
    if (Util::Input::IsKeyPressed(Util::Keycode::S)) dir.y -= 1.0f;
    if (Util::Input::IsKeyPressed(Util::Keycode::A)) dir.x -= 1.0f;
    if (Util::Input::IsKeyPressed(Util::Keycode::D)) dir.x += 1.0f;

    const bool isMoving = (dir.x != 0.0f || dir.y != 0.0f);

    if (isMoving) {
        // 斜向歸一化：確保對角線速度與單向相同
        const glm::vec2 normDir = glm::normalize(dir);
        m_LastMoveDir = normDir;

        // 只在有水平輸入時才更新朝向
        if (dir.x != 0.0f) {
            m_FacingLeft = (dir.x < 0.0f);
        }

        m_WorldPos += normDir * m_Speed * dt;
    }

    // 切換動畫（只在狀態改變時呼叫 SetDrawable，避免每幀重設）
    if (isMoving && !m_IsWalkingAnim) {
        SetDrawable(m_WalkAnim);
        m_IsWalkingAnim = true;
    } else if (!isMoving && m_IsWalkingAnim) {
        SetDrawable(m_IdleAnim);
        m_IsWalkingAnim = false;
    }
}

void Player::Update(float dt) {
    HandleInput(dt);

    // Step 1.5：AABB 碰撞解決（穿透深度 Push Back，禁止暴力歸位）
    // 碰撞中心下偏 HIT_OFFSET_Y，確保碰撞盒覆蓋腳部
    CollisionSystem::ResolveWall(
        m_WorldPos,
        {0.0f, HIT_OFFSET_Y},
        {static_cast<float>(HIT_W), static_cast<float>(HIT_H)}
    );

    // 更新朝向翻轉（scale.x 正/負，y 保持 +3）
    m_Transform.scale = {m_FacingLeft ? -3.0f : 3.0f, 3.0f};

    // ⚠️ SyncRender 由 App 在 Camera::Update 之後呼叫，確保玩家永遠置中
}
