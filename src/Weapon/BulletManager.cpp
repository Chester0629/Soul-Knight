#include "Weapon/BulletManager.hpp"

#include "System/CollisionSystem.hpp"

BulletManager::BulletManager() {
    m_Bullets.reserve(MAX_BULLETS);
    for (int i = 0; i < MAX_BULLETS; ++i)
        m_Bullets.push_back(std::make_unique<Bullet>());
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

void BulletManager::Update(float dt, glm::vec2 cameraPos) {
    const glm::vec2 hitbox{Bullet::HIT_SIZE, Bullet::HIT_SIZE};

    for (auto& b : m_Bullets) {
        if (!b->m_Active) continue;

        // 1. 移動
        b->m_WorldPos += b->m_Velocity * dt;

        // 2. 壽命倒數（SpearGoblin 突刺等限時子彈）
        if (b->m_Lifetime > 0.0f) {
            b->m_Lifetime -= dt;
            if (b->m_Lifetime <= 0.0f) {
                Deactivate(b.get());
                continue;
            }
        }

        // 3. 碰牆檢查
        if (CollisionSystem::IsBlocked(b->m_WorldPos, hitbox)) {
            Deactivate(b.get());
            continue;
        }

        // 4. 越界檢查（離世界原點超過 OUT_OF_RANGE 視為越界）
        if (glm::length(b->m_WorldPos) > OUT_OF_RANGE) {
            Deactivate(b.get());
            continue;
        }

        // 5. 同步渲染座標
        b->m_Transform.translation = b->m_WorldPos - cameraPos;
    }
}

void BulletManager::AddToRenderer(Util::Renderer& root) {
    for (auto& b : m_Bullets)
        root.AddChild(b);
}
