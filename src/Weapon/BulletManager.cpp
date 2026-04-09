#include "Weapon/BulletManager.hpp"

#include "Entity/EnemyManager.hpp"
#include "Entity/Player.hpp"
#include "System/CollisionSystem.hpp"

#include <cmath>

BulletManager::BulletManager() {
    m_Bullets.reserve(MAX_BULLETS);
    for (int i = 0; i < MAX_BULLETS; ++i)
        m_Bullets.push_back(std::make_shared<Bullet>());
}

Bullet* BulletManager::GetInactive() {
    for (auto& b : m_Bullets)
        if (!b->m_Active) return b.get();
    return nullptr;  // 池滿時靜默丟棄
}

void BulletManager::Spawn(glm::vec2 worldPos, glm::vec2 dir, float speed,
                           int damage, bool isPlayer, float lifetime) {
    Bullet* b = GetInactive();
    if (!b) return;

    b->m_WorldPos = worldPos;
    b->m_Velocity = (glm::length(dir) > 0.0f) ? glm::normalize(dir) * speed
                                                : glm::vec2{0.0f, 0.0f};
    b->m_Lifetime = lifetime;
    b->m_Damage   = damage;
    b->m_IsPlayer = isPlayer;
    b->m_Active   = true;
    b->SetVisible(true);  // ⚠️ 必須，防止幽靈渲染
    b->m_Transform.scale = {2.0f, 2.0f};
}

void BulletManager::Deactivate(Bullet* b) {
    b->m_Active   = false;
    b->m_Lifetime = -1.0f;
    b->SetVisible(false);  // ⚠️ 必須，防止幽靈渲染
}

void BulletManager::Update(float dt, glm::vec2 cameraPos,
                            Player* player, EnemyManager* enemyMgr) {
    const glm::vec2 hitbox{Bullet::HIT_SIZE, Bullet::HIT_SIZE};
    const float     bHalf = Bullet::HIT_SIZE * 0.5f;

    for (auto& b : m_Bullets) {
        if (!b->m_Active) continue;

        // 1. 移動
        b->m_WorldPos += b->m_Velocity * dt;

        // 2. 壽命倒數
        if (b->m_Lifetime > 0.0f) {
            b->m_Lifetime -= dt;
            if (b->m_Lifetime <= 0.0f) { Deactivate(b.get()); continue; }
        }

        // 3. 碰牆檢查
        if (CollisionSystem::IsBlocked(b->m_WorldPos, hitbox)) { Deactivate(b.get()); continue; }

        // 4. 越界檢查
        if (glm::length(b->m_WorldPos) > OUT_OF_RANGE) { Deactivate(b.get()); continue; }

        // 5. 實體碰撞
        if (b->m_IsPlayer && enemyMgr) {
            // 玩家子彈 ↔ 敵人 AABB
            for (auto& e : enemyMgr->GetEnemies()) {
                if (e->IsDead()) continue;
                const glm::vec2 eCenter = e->GetWorldPos();
                const glm::vec2 eHalf   = e->GetHitboxHalf();
                if (std::abs(b->m_WorldPos.x - eCenter.x) < bHalf + eHalf.x &&
                    std::abs(b->m_WorldPos.y - eCenter.y) < bHalf + eHalf.y) {
                    e->TakeDamage(b->m_Damage);
                    Deactivate(b.get());
                    break;
                }
            }
        } else if (!b->m_IsPlayer && player && !player->IsDead()) {
            // 敵人子彈 ↔ 玩家 AABB
            const glm::vec2 pCenter = player->GetWorldPos() + glm::vec2{0.0f, Player::HIT_OFFSET_Y};
            const glm::vec2 pHalf   = {Player::HIT_W * 0.5f, Player::HIT_H * 0.5f};
            if (std::abs(b->m_WorldPos.x - pCenter.x) < bHalf + pHalf.x &&
                std::abs(b->m_WorldPos.y - pCenter.y) < bHalf + pHalf.y) {
                player->TakeDamage(b->m_Damage);
                Deactivate(b.get());
            }
        }

        // 6. 同步渲染座標（仍存活才更新）
        if (b->m_Active)
            b->m_Transform.translation = b->m_WorldPos - cameraPos;
    }
}

void BulletManager::AddToRenderer(Util::Renderer& root) {
    for (auto& b : m_Bullets)
        root.AddChild(b);
}
