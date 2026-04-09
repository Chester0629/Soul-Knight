#include "Entity/Enemy.hpp"
#include "Entity/Player.hpp"

// SpearGoblin AI：
//   dist > m_StabRange → 直線追蹤玩家（TryMove）
//   dist ≤ m_StabRange 且冷卻完成 → 突刺攻擊（Step 2.3 前為空操作）
void SpearGoblin::UpdateAI(float dt) {
    if (!m_Target) return;

    const glm::vec2 toPlayer = m_Target->GetWorldPos() - m_WorldPos;
    const float     dist     = glm::length(toPlayer);

    // ── 追蹤 ──────────────────────────────────────────────────────────────────
    if (dist > m_StabRange && dist > 0.01f) {
        TryMove(glm::normalize(toPlayer), m_Speed, dt);
    }

    // 面向玩家
    if (dist > 0.01f)
        m_Transform.scale.x = (toPlayer.x < 0.0f) ? -3.0f : 3.0f;

    // ── 突刺攻擊：在刺擊範圍內直接造成傷害 ──────────────────────────────────
    if (dist <= m_StabRange && m_AttackCooldown <= 0.0f && m_Target) {
        const int dmg = m_IsElite ? 3 : 3;  // 普通/精英均為 3
        m_Target->TakeDamage(dmg);
        m_AttackCooldown = COOLDOWN;
    }
}
