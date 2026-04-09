#include "Entity/Enemy.hpp"
#include "Entity/Player.hpp"

// PistolGoblin AI：
//   dist < MIN_DIST       → 後退（遠離玩家）
//   dist ∈ [MIN, PREF]    → 靜止（保持位置）
//   dist > PREFERRED_DIST → 靠近玩家
//   dist < ATTACK_RANGE 且冷卻完成 → 攻擊（Step 2.3 前為空操作）
void PistolGoblin::UpdateAI(float dt) {
    if (!m_Target) return;

    const glm::vec2 toPlayer = m_Target->GetWorldPos() - m_WorldPos;
    const float     dist     = glm::length(toPlayer);

    // ── 移動 ──────────────────────────────────────────────────────────────────
    if (dist > 0.01f) {
        const glm::vec2 dir = glm::normalize(toPlayer);
        if (dist < MIN_DIST) {
            TryMove(-dir, m_Speed, dt);        // 後退
        } else if (dist > PREFERRED_DIST) {
            TryMove(dir, m_Speed * 0.8f, dt);  // 緩慢靠近（不衝向玩家）
        }
    }

    // 面向玩家
    if (dist > 0.01f)
        m_Transform.scale.x = (toPlayer.x < 0.0f) ? -3.0f : 3.0f;

    // ── 攻擊：射出子彈 ────────────────────────────────────────────────────────
    if (dist < ATTACK_RANGE && m_AttackCooldown <= 0.0f && m_BulletMgr) {
        const int       dmg = m_IsElite ? 3 : 2;
        const glm::vec2 dir = (dist > 0.01f) ? glm::normalize(toPlayer) : glm::vec2{1.0f, 0.0f};
        m_BulletMgr->Spawn(m_WorldPos, dir, 350.0f, dmg, false);
        m_AttackCooldown = COOLDOWN;
    }
}
