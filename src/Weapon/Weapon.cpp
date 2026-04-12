#include "Weapon/Weapon.hpp"

#include "Weapon/BulletManager.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

#include <cmath>

// ─── Weapon（基類）────────────────────────────────────────────────────────────

bool Weapon::TryFire(float dt) {
    m_FireCooldown -= dt;
    if (!Util::Input::IsKeyPressed(Util::Keycode::J)) return false;
    if (m_FireCooldown > 0.0f) return false;
    m_FireCooldown = m_FireRate;
    return true;
}

// ─── GunWeapon ────────────────────────────────────────────────────────────────

GunWeapon::GunWeapon() {
    m_Damage     = 10;
    m_FireRate   = 0.25f;  // 4 發/秒（按住時）
    m_SpritePath = RESOURCE_DIR "/Weapons/weapon_pistol.png";
}

bool GunWeapon::TryFire(float dt) {
    // 快速點擊：IsKeyDown(J) 重置冷卻，允許突破射速上限
    if (Util::Input::IsKeyDown(Util::Keycode::J))
        m_FireCooldown = 0.0f;
    return Weapon::TryFire(dt);
}

void GunWeapon::Fire(glm::vec2 origin, glm::vec2 dir, BulletManager& mgr) {
    // 發射原點從玩家中心往方向偏移 20px，防止子彈在玩家體內生成
    glm::vec2 spawnPos = origin + dir * 20.0f;
    mgr.Spawn(spawnPos, dir, BULLET_SPEED, m_Damage, /*isPlayer=*/true);
}

// ─── ShotgunWeapon ────────────────────────────────────────────────────────────

ShotgunWeapon::ShotgunWeapon() {
    m_Damage     = 8;
    m_FireRate   = 0.6f;   // 散彈射速較慢
    m_SpritePath = RESOURCE_DIR "/Weapons/weapon_shotgun.png";
}

void ShotgunWeapon::Fire(glm::vec2 origin, glm::vec2 dir, BulletManager& mgr) {
    glm::vec2 spawnPos = origin + dir * 20.0f;

    // 以 dir 為中心，±SPREAD_ANGLE 發射 3 顆子彈
    const float angles[] = {-SPREAD_ANGLE, 0.0f, SPREAD_ANGLE};
    for (float angleDeg : angles) {
        float rad    = glm::radians(angleDeg);
        float cosA   = std::cos(rad);
        float sinA   = std::sin(rad);
        // 2D 旋轉
        glm::vec2 rotDir = {
            dir.x * cosA - dir.y * sinA,
            dir.x * sinA + dir.y * cosA
        };
        mgr.Spawn(spawnPos, rotDir, BULLET_SPEED, m_Damage, /*isPlayer=*/true);
    }
}

// ─── LaserWeapon ─────────────────────────────────────────────────────────────

LaserWeapon::LaserWeapon() {
    m_Damage     = 5;
    m_FireRate   = 0.0f;   // 每幀發射（按住 J 持續）
    m_SpritePath = RESOURCE_DIR "/Weapons/weapon_laser.png";
}

void LaserWeapon::Fire(glm::vec2 origin, glm::vec2 dir, BulletManager& mgr) {
    // M2：用普通子彈代替光束，M4/M5 再換持續型特效
    glm::vec2 spawnPos = origin + dir * 20.0f;
    mgr.Spawn(spawnPos, dir, BULLET_SPEED, m_Damage, /*isPlayer=*/true);
}
