#pragma once

#include "pch.hpp"

#include <string>

class BulletManager;  // 前向宣告，避免循環 include

// ─── Weapon ───────────────────────────────────────────────────────────────────
// 武器資料 + 行為類別（不繼承 GameObject）
// 觸發邏輯：基類使用 IsKeyPressed(J) + m_FireCooldown
// 發射邏輯：子類覆寫 Fire() 實作不同彈道
class Weapon {
public:
    virtual ~Weapon() = default;

    // 每幀呼叫，回傳 true 代表本幀應發射
    // 基類：IsKeyPressed(J) + FireRate 冷卻
    virtual bool TryFire(float dt);

    // 執行實際發射（TryFire 回傳 true 後由 Player 呼叫）
    virtual void Fire(glm::vec2 origin, glm::vec2 direction, BulletManager& mgr) = 0;

    virtual bool RequiresEnergy()    const { return false; }
    virtual int  EnergyCostPerShot() const { return 0; }

    int         GetDamage()     const { return m_Damage; }
    float       GetFireRate()   const { return m_FireRate; }
    std::string GetSpritePath() const { return m_SpritePath; }

protected:
    int         m_Damage       = 10;
    float       m_FireRate     = 0.3f;   // 射速冷卻（秒）
    float       m_FireCooldown = 0.0f;   // 每幀倒數
    std::string m_SpritePath;
};

// ─── GunWeapon ────────────────────────────────────────────────────────────────
// 手槍（初始武器）
// 能量消耗：0（免費），攻擊力 10，子彈速度 700 px/s
// 觸發：IsKeyPressed(J) + FireRate（基類）
//        + IsKeyDown(J) 重置冷卻（快速點擊可突破射速上限）
class GunWeapon : public Weapon {
public:
    GunWeapon();
    bool TryFire(float dt) override;
    void Fire(glm::vec2 origin, glm::vec2 dir, BulletManager& mgr) override;
    bool RequiresEnergy()    const override { return false; }
    int  EnergyCostPerShot() const override { return 0; }

private:
    static constexpr float BULLET_SPEED = 700.0f;
};

// ─── ShotgunWeapon ────────────────────────────────────────────────────────────
// 散彈
// 能量消耗：10 / 輪（整輪 3 顆一起扣）；能量不足時完全不射擊
// 觸發：IsKeyPressed(J) + FireRate（基類）
// 射擊：同時 3 顆（正前方 ±15°），速度 600 px/s，攻擊力 8
class ShotgunWeapon : public Weapon {
public:
    ShotgunWeapon();
    void Fire(glm::vec2 origin, glm::vec2 dir, BulletManager& mgr) override;
    bool RequiresEnergy()    const override { return true; }
    int  EnergyCostPerShot() const override { return 10; }

private:
    static constexpr float BULLET_SPEED  = 600.0f;
    static constexpr float SPREAD_ANGLE  = 15.0f;   // ±15° 扇形
};

// ─── LaserWeapon ─────────────────────────────────────────────────────────────
// 雷射
// 能量消耗：2 / 幀；能量不足時完全不射擊
// 觸發：IsKeyPressed(J)，m_FireRate=0（每幀發射）
// M2：用普通子彈代替光束，M4/M5 再換持續型特效
class LaserWeapon : public Weapon {
public:
    LaserWeapon();
    void Fire(glm::vec2 origin, glm::vec2 dir, BulletManager& mgr) override;
    bool RequiresEnergy()    const override { return true; }
    int  EnergyCostPerShot() const override { return 2; }

private:
    static constexpr float BULLET_SPEED = 700.0f;
};
