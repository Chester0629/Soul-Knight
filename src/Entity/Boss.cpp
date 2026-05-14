#include "Entity/Boss.hpp"

#include "Entity/Player.hpp"
#include "System/CollisionSystem.hpp"
#include "Weapon/BulletManager.hpp"
#include "World/Camera.hpp"

#include <cmath>
#include <vector>
#include <string>

static constexpr float PI = 3.14159265f;

Boss::Boss(BulletManager* bulletMgr)
    : m_BulletMgr(bulletMgr)
{
    m_HP         = BOSS_HP;
    m_MaxHP      = BOSS_HP;
    m_Speed      = BOSS_SPEED;
    m_HitboxHalf = {BOSS_HIT_W * 0.5f, BOSS_HIT_H * 0.5f};

    m_IdleAnim = std::make_shared<Util::Animation>(
        std::vector<std::string>{
            RESOURCE_DIR "/Boss/boss08_0.png",
            RESOURCE_DIR "/Boss/boss08_1.png",
            RESOURCE_DIR "/Boss/boss08_2.png",
            RESOURCE_DIR "/Boss/boss08_3.png",
            RESOURCE_DIR "/Boss/boss08_4.png",
            RESOURCE_DIR "/Boss/boss08_5.png",
            RESOURCE_DIR "/Boss/boss08_6.png",
            RESOURCE_DIR "/Boss/boss08_7.png",
        },
        true, 150, true, 0
    );

    m_DeathAnim = std::make_shared<Util::Animation>(
        std::vector<std::string>{
            RESOURCE_DIR "/Boss/boss08_0.png",
            RESOURCE_DIR "/Boss/boss08_1.png",
            RESOURCE_DIR "/Boss/boss08_2.png",
            RESOURCE_DIR "/Boss/boss08_3.png",
            RESOURCE_DIR "/Boss/boss08_4.png",
            RESOURCE_DIR "/Boss/boss08_5.png",
            RESOURCE_DIR "/Boss/boss08_6.png",
            RESOURCE_DIR "/Boss/boss08_7.png",
        },
        false, 80, false, 0
    );

    SetDrawable(m_IdleAnim);
    m_Transform.scale = {4.0f, 4.0f};
    SetVisible(true);
    UpdateZIndex();
    SyncRenderTransform(Camera::GetPosition());
}

void Boss::StartDying() {
    m_IsDying = true;
    SetDrawable(m_DeathAnim);
    m_DeathAnim->Play();
}

bool Boss::IsDeathDone() const {
    return m_DeathAnim->GetState() == Util::Animation::State::ENDED;
}

void Boss::FireFanShot() {
    if (!m_Target || !m_BulletMgr) return;

    const glm::vec2 toPlayer = m_Target->GetWorldPos() - m_WorldPos;
    const float baseAngle = std::atan2(toPlayer.y, toPlayer.x);
    const float spread    = 40.0f * (PI / 180.0f);  // ±40 度

    const int bulletCount = (m_Phase == Phase::TWO) ? 7 : 5;
    const float step = (bulletCount > 1) ? (2.0f * spread) / static_cast<float>(bulletCount - 1) : 0.0f;

    for (int i = 0; i < bulletCount; ++i) {
        const float angle = baseAngle - spread + static_cast<float>(i) * step;
        const glm::vec2 dir = {std::cos(angle), std::sin(angle)};
        m_BulletMgr->Spawn(m_WorldPos, dir, 270.0f, 2, false);
    }
}

void Boss::UpdateAI(float dt) {
    if (!m_Target || m_Target->IsDead()) return;

    // Phase 切換
    if (m_Phase == Phase::ONE && m_HP <= BOSS_HP / 2)
        m_Phase = Phase::TWO;

    m_FanTimer  -= dt;
    m_DashTimer -= dt;

    const glm::vec2 toPlayer    = m_Target->GetWorldPos() - m_WorldPos;
    const float     dist        = glm::length(toPlayer);
    const glm::vec2 dirToPlayer = (dist > 0.01f)
                                  ? toPlayer / dist
                                  : glm::vec2{1.0f, 0.0f};

    // 面向更新
    m_Transform.scale.x = (toPlayer.x < 0.0f) ? -4.0f : 4.0f;

    switch (m_AiState) {

    case AiState::APPROACH:
        // 緩速接近
        if (dist > 100.0f) {
            const glm::vec2 next = m_WorldPos + dirToPlayer * m_Speed * dt;
            if (!CollisionSystem::IsBlocked(next, {BOSS_HIT_W, BOSS_HIT_H}))
                m_WorldPos = next;
        }
        // 扇形射擊
        if (m_FanTimer <= 0.0f) {
            FireFanShot();
            m_FanTimer = FAN_COOLDOWN;
        }
        // Phase 2：衝刺觸發
        if (m_Phase == Phase::TWO && m_DashTimer <= 0.0f) {
            m_DashDir    = dirToPlayer;
            m_AiState    = AiState::DASHING;
            m_StateTimer = DASH_DURATION;
        }
        break;

    case AiState::DASHING:
        {
            const glm::vec2 next = m_WorldPos + m_DashDir * DASH_SPEED * dt;
            if (!CollisionSystem::IsBlocked(next, {BOSS_HIT_W, BOSS_HIT_H}))
                m_WorldPos = next;
            else {
                // 撞牆立刻結束衝刺
                m_AiState    = AiState::RECOVERING;
                m_StateTimer = RECOVER_DURATION;
                break;
            }
            m_StateTimer -= dt;
            if (m_StateTimer <= 0.0f) {
                m_AiState    = AiState::RECOVERING;
                m_StateTimer = RECOVER_DURATION;
                m_DashTimer  = DASH_COOLDOWN;
            }
        }
        break;

    case AiState::RECOVERING:
        m_StateTimer -= dt;
        if (m_StateTimer <= 0.0f)
            m_AiState = AiState::APPROACH;
        break;
    }
}
