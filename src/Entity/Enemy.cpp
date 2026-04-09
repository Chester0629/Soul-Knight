#include "Entity/Enemy.hpp"

#include "Entity/Player.hpp"
#include "System/CollisionSystem.hpp"
#include "Weapon/BulletManager.hpp"
#include "World/Camera.hpp"

#include <cmath>

// ─── Enemy::Update ────────────────────────────────────────────────────────────

void Enemy::Update(float dt) {
    if (m_AttackCooldown > 0.0f)
        m_AttackCooldown -= dt;
    if (m_ContactTimer > 0.0f)
        m_ContactTimer -= dt;

    UpdateAI(dt);

    // 接觸傷害：任何敵人的 AABB 與玩家重疊時扣血（0.5s CD）
    if (m_Target && !m_Target->IsDead() && m_ContactTimer <= 0.0f) {
        const glm::vec2 pCenter = m_Target->GetWorldPos() + glm::vec2{0.0f, Player::HIT_OFFSET_Y};
        const glm::vec2 pHalf   = {Player::HIT_W * 0.5f, Player::HIT_H * 0.5f};

        if (std::abs(m_WorldPos.x - pCenter.x) < m_HitboxHalf.x + pHalf.x &&
            std::abs(m_WorldPos.y - pCenter.y) < m_HitboxHalf.y + pHalf.y) {
            m_Target->TakeDamage(CONTACT_DAMAGE);
            m_ContactTimer = CONTACT_COOLDOWN;
        }
    }

    SyncRender(Camera::GetPosition());
}

// ─── GoblinEnemy ──────────────────────────────────────────────────────────────

GoblinEnemy::GoblinEnemy(BulletManager* bulletMgr)
    : m_BulletMgr(bulletMgr)
{
    m_HP    = Enemy::BASE_HP;
    m_MaxHP = Enemy::BASE_HP;
    m_Speed = Enemy::BASE_SPEED;

    m_Anim = std::make_shared<Util::Animation>(
        std::vector<std::string>{
            RESOURCE_DIR "/Enemies/enemy22_0.png",
            RESOURCE_DIR "/Enemies/enemy22_1.png",
            RESOURCE_DIR "/Enemies/enemy22_2.png",
            RESOURCE_DIR "/Enemies/enemy22_3.png",
            RESOURCE_DIR "/Enemies/enemy22_4.png",
            RESOURCE_DIR "/Enemies/enemy22_5.png",
            RESOURCE_DIR "/Enemies/enemy22_6.png",
        },
        true, 150, true, 0
    );

    // 死亡動畫：同樣 7 幀，非循環、快速播放一次
    m_DeathAnim = std::make_shared<Util::Animation>(
        std::vector<std::string>{
            RESOURCE_DIR "/Enemies/enemy22_0.png",
            RESOURCE_DIR "/Enemies/enemy22_1.png",
            RESOURCE_DIR "/Enemies/enemy22_2.png",
            RESOURCE_DIR "/Enemies/enemy22_3.png",
            RESOURCE_DIR "/Enemies/enemy22_4.png",
            RESOURCE_DIR "/Enemies/enemy22_5.png",
            RESOURCE_DIR "/Enemies/enemy22_6.png",
        },
        false, 60, false, 0   // play=false（手動觸發），60ms/幀，不循環
    );

    SetDrawable(m_Anim);
    m_Transform.scale = {3.0f, 3.0f};
    SetVisible(true);
    UpdateZIndex();
    SyncRenderTransform(Camera::GetPosition());
}

void GoblinEnemy::StartDying() {
    m_IsDying = true;
    SetDrawable(m_DeathAnim);
    m_DeathAnim->Play();
}

bool GoblinEnemy::IsDeathDone() const {
    return m_DeathAnim->GetState() == Util::Animation::State::ENDED;
}

void GoblinEnemy::TryMove(glm::vec2 wishDir, float speed, float dt) {
    const glm::vec2 hitSize{GOBLIN_HIT_W, GOBLIN_HIT_H};

    glm::vec2 nextPos = m_WorldPos + wishDir * speed * dt;
    if (!CollisionSystem::IsBlocked(nextPos, hitSize)) {
        m_WorldPos = nextPos;
        return;
    }
    glm::vec2 xOnly = m_WorldPos + glm::vec2(wishDir.x, 0.0f) * speed * dt;
    if (!CollisionSystem::IsBlocked(xOnly, hitSize)) {
        m_WorldPos = xOnly;
        return;
    }
    glm::vec2 yOnly = m_WorldPos + glm::vec2(0.0f, wishDir.y) * speed * dt;
    if (!CollisionSystem::IsBlocked(yOnly, hitSize)) {
        m_WorldPos = yOnly;
        return;
    }
}
