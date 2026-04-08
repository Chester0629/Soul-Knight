#include "Entity/Enemy.hpp"
#include "Entity/Player.hpp"

// ArcherGoblin AI 狀態機：
//   MOVE     → 保持 PREFERRED_DIST；進入 ATTACK_RANGE 且冷卻完成 → 切換 AIM
//   AIM      → 停止移動，等待 AIM_DURATION 秒後射箭 → 切換 COOLDOWN
//   COOLDOWN → 可移動保距；等待 SHOOT_COOLDOWN 秒後 → 切換 MOVE
void ArcherGoblin::UpdateAI(float dt) {
    if (!m_Target) return;

    const glm::vec2 toPlayer = m_Target->GetWorldPos() - m_WorldPos;
    const float     dist     = glm::length(toPlayer);

    // 面向玩家
    if (dist > 0.01f)
        m_Transform.scale.x = (toPlayer.x < 0.0f) ? -3.0f : 3.0f;

    switch (m_AiState) {
    // ── MOVE：保持距離，等待攻擊機會 ─────────────────────────────────────────
    case AiState::MOVE: {
        if (dist > 0.01f) {
            const glm::vec2 dir = glm::normalize(toPlayer);
            if (dist < MIN_DIST)
                TryMove(-dir, m_Speed, dt);
            else if (dist > PREFERRED_DIST)
                TryMove(dir, m_Speed, dt);
        }
        // 進入射程且冷卻完成 → 開始瞄準
        if (dist < ATTACK_RANGE && m_AttackCooldown <= 0.0f) {
            m_AiState    = AiState::AIM;
            m_StateTimer = 0.0f;
        }
        break;
    }
    // ── AIM：停止移動，瞄準 AIM_DURATION 秒 ──────────────────────────────────
    case AiState::AIM: {
        m_StateTimer += dt;
        if (m_StateTimer >= AIM_DURATION) {
            // TODO (Step 2.3): bulletMgr->Spawn(arrow, m_WorldPos, dir, 500.0f, dmg, false)
            m_AiState        = AiState::COOLDOWN;
            m_StateTimer     = 0.0f;
            m_AttackCooldown = SHOOT_COOLDOWN;
        }
        break;
    }
    // ── COOLDOWN：冷卻期間仍可保距移動 ───────────────────────────────────────
    case AiState::COOLDOWN: {
        if (dist > 0.01f) {
            const glm::vec2 dir = glm::normalize(toPlayer);
            if (dist < MIN_DIST)
                TryMove(-dir, m_Speed, dt);
            else if (dist > PREFERRED_DIST)
                TryMove(dir, m_Speed, dt);
        }
        if (m_AttackCooldown <= 0.0f)
            m_AiState = AiState::MOVE;
        break;
    }
    }
}
