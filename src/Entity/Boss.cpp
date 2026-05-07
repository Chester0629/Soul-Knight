#include "Entity/Boss.hpp"
#include "Entity/Player.hpp"
#include "System/CollisionSystem.hpp"
#include "Weapon/BulletManager.hpp"
#include "World/Camera.hpp"

#include <cmath>

Boss::Boss(BulletManager* bulletMgr)
    : m_BulletMgr(bulletMgr)
{
    m_HP           = BOSS_HP;
    m_MaxHP        = BOSS_HP;
    m_Speed        = BOSS_SPEED;
    m_HitboxHalf   = {BOSS_HIT_W * 0.5f, BOSS_HIT_H * 0.5f};
    m_DashCooldown = DASH_COOLDOWN;

    m_Anim = std::make_shared<Util::Animation>(
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
        true, 120, true, 0
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

    SetDrawable(m_Anim);
    m_Transform.scale = {3.0f, 3.0f};
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

void Boss::TryMove(glm::vec2 wishDir, float speed, float dt) {
    const glm::vec2 hitSize{BOSS_HIT_W, BOSS_HIT_H};
    glm::vec2 nextPos = m_WorldPos + wishDir * speed * dt;
    if (!CollisionSystem::IsBlocked(nextPos, hitSize)) { m_WorldPos = nextPos; return; }
    glm::vec2 xOnly = m_WorldPos + glm::vec2(wishDir.x, 0.0f) * speed * dt;
    if (!CollisionSystem::IsBlocked(xOnly, hitSize)) { m_WorldPos = xOnly; return; }
    glm::vec2 yOnly = m_WorldPos + glm::vec2(0.0f, wishDir.y) * speed * dt;
    if (!CollisionSystem::IsBlocked(yOnly, hitSize)) { m_WorldPos = yOnly; return; }
}

void Boss::UpdateAI(float dt) {
    if (!m_Target || m_IsDying) return;

    const glm::vec2 toPlayer = m_Target->GetWorldPos() - m_WorldPos;
    const float     dist     = glm::length(toPlayer);
    const bool      phase2   = (m_HP <= BOSS_HP / 2);

    // ── Phase 2：衝刺 ─────────────────────────────────────────────────────────
    if (phase2) {
        if (m_DashCooldown > 0.0f) m_DashCooldown -= dt;
        if (m_IsDashing) {
            m_DashTimer -= dt;
            TryMove(m_DashDir, DASH_SPEED, dt);
            if (m_DashTimer <= 0.0f) m_IsDashing = false;
        } else if (m_DashCooldown <= 0.0f && dist > 0.01f) {
            m_IsDashing    = true;
            m_DashTimer    = DASH_DURATION;
            m_DashDir      = glm::normalize(toPlayer);
            m_DashCooldown = DASH_COOLDOWN;
        }
    }

    // ── 緩慢靠近（非衝刺時）──────────────────────────────────────────────────
    if (!m_IsDashing && dist > 80.0f && dist > 0.01f)
        TryMove(glm::normalize(toPlayer), m_Speed, dt);

    // 面向玩家
    if (dist > 0.01f)
        m_Transform.scale.x = (toPlayer.x < 0.0f) ? -3.0f : 3.0f;

    // ── 扇形射擊（Phase 1 + 2）────────────────────────────────────────────────
    if (m_AttackCooldown <= 0.0f && dist > 0.01f && m_BulletMgr) {
        const glm::vec2 base = glm::normalize(toPlayer);
        const float step = (FAN_COUNT > 1)
                           ? (FAN_SPREAD_RAD * 2.0f / static_cast<float>(FAN_COUNT - 1))
                           : 0.0f;
        for (int i = 0; i < FAN_COUNT; ++i) {
            const float a  = -FAN_SPREAD_RAD + step * static_cast<float>(i);
            const float ca = std::cos(a), sa = std::sin(a);
            const glm::vec2 dir{base.x * ca - base.y * sa,
                                base.x * sa + base.y * ca};
            m_BulletMgr->Spawn(m_WorldPos, dir, 300.0f, 2, false);
        }
        m_AttackCooldown = FAN_COOLDOWN;
    }
}
