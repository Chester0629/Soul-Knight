#pragma once

#include "Entity/Entity.hpp"
#include <algorithm>
#include "Weapon/BulletManager.hpp"
#include "Weapon/Weapon.hpp"
#include "Util/Animation.hpp"
#include "Util/GameObject.hpp"

// Player — 玩家角色
// 規格：
//   初始位置  (-300, -100)，速度 300 px/s
//   Walk 動畫：c01_0~3，100ms，循環
//   Idle 動畫：c01_4~7，150ms，循環
//   朝向：scale.x 正/負 3，只在有水平輸入時切換
//   m_LastMoveDir：記錄最後一次 WASD 方向（Normalize），初始 {1,0}
//   m_CurrentWeapon：初始為 GunWeapon（手槍）
//   m_BulletMgr：建構時注入（不擁有）
class Player : public Entity {
public:
    explicit Player(BulletManager* bulletMgr);
    void Update(float dt) override;
    void TakeDamage(int damage) override {
        // 先扣護盾，護盾扣完再扣血
        const int shieldAbsorb = std::min(damage, m_Shield);
        m_Shield -= shieldAbsorb;
        m_HP     -= (damage - shieldAbsorb);
        m_ShieldRegenTimer = 0.0f;  // 受傷重置護盾回復計時
    }

    static constexpr float SPEED        = 300.0f;
    static constexpr float INITIAL_X    = -300.0f;
    static constexpr float INITIAL_Y    = -100.0f;
    static constexpr int   HIT_W        = 44;
    static constexpr int   HIT_H        = 20;
    static constexpr float HIT_OFFSET_Y = -10.0f;

    glm::vec2 GetLastMoveDir() const { return m_LastMoveDir; }
    bool      IsFacingLeft()   const { return m_FacingLeft; }
    int       GetHP()          const { return m_HP; }
    int       GetMaxHP()       const { return m_MaxHP; }
    int       GetShield()      const { return m_Shield; }
    int       GetMaxShield()   const { return m_MaxShield; }
    int       GetEnergy()      const { return m_Energy; }
    int       GetMaxEnergy()   const { return m_MaxEnergy; }

    void AddWeaponSpriteToRenderer(Util::Renderer& root);

private:
    void HandleInput(float dt);
    void TryShoot(float dt);
    void UpdateWeaponSpriteTransform();

    BulletManager* m_BulletMgr = nullptr;

    std::shared_ptr<Weapon>           m_CurrentWeapon;
    std::shared_ptr<Util::GameObject> m_WeaponSprite;

    std::shared_ptr<Util::Animation>  m_WalkAnim;
    std::shared_ptr<Util::Animation>  m_IdleAnim;
    bool m_IsWalkingAnim = false;

    bool      m_FacingLeft  = false;
    glm::vec2 m_LastMoveDir = {1.0f, 0.0f};

    int   m_Shield         = 4;
    int   m_MaxShield      = 10;
    float m_ShieldRegenTimer = 0.0f;
    static constexpr float SHIELD_REGEN_INTERVAL = 3.0f;  // 每 3 秒回復 1 點
    int m_Energy    = 200;
    int m_MaxEnergy = 200;
};
