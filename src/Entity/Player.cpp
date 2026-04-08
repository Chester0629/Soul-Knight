#include "Entity/Player.hpp"

#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

Player::Player() {
    m_WorldPos = {INITIAL_X, INITIAL_Y};
    m_Speed    = SPEED;
    m_HP = m_MaxHP = 10;

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

    SetDrawable(m_IdleAnim);
    m_IsWalkingAnim = false;
    m_Transform.scale = {3.0f, 3.0f};
    SetVisible(true);
    UpdateZIndex();
    // Step 1.3：尚無相機，使用世界原點作為渲染偏移
    SyncRenderTransform({0.0f, 0.0f});
}

void Player::HandleInput(float dt) {
    glm::vec2 dir = {0.0f, 0.0f};

    if (Util::Input::IsKeyPressed(Util::Keycode::W)) dir.y += 1.0f;
    if (Util::Input::IsKeyPressed(Util::Keycode::S)) dir.y -= 1.0f;
    if (Util::Input::IsKeyPressed(Util::Keycode::A)) dir.x -= 1.0f;
    if (Util::Input::IsKeyPressed(Util::Keycode::D)) dir.x += 1.0f;

    const bool isMoving = (dir.x != 0.0f || dir.y != 0.0f);

    if (isMoving) {
        const glm::vec2 normDir = glm::normalize(dir);
        m_LastMoveDir = normDir;

        if (dir.x != 0.0f)
            m_FacingLeft = (dir.x < 0.0f);

        m_WorldPos += normDir * m_Speed * dt;
    }

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

    // 朝向翻轉（scale.x 正/負）
    m_Transform.scale = {m_FacingLeft ? -3.0f : 3.0f, 3.0f};

    // Step 1.3：渲染座標 = 世界座標（無相機偏移）
    SyncRenderTransform({0.0f, 0.0f});
    UpdateZIndex();
}
