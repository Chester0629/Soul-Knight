#pragma once

#include "Entity/Entity.hpp"
#include "Util/Animation.hpp"

class Player;        // 避免循環 include，在 .cpp 中 include Player.hpp
class BulletManager; // 前向宣告

// ─── Enemy ────────────────────────────────────────────────────────────────────
class Enemy : public Entity {
public:
    void Update(float dt) override;
    void SetTarget(Player* player) { m_Target = player; }
    void SetElite(bool elite)      { m_IsElite = elite; }

    // 供 BulletManager 碰撞偵測使用
    glm::vec2 GetHitboxHalf() const { return m_HitboxHalf; }

    // 死亡動畫狀態
    bool IsDying()  const { return m_IsDying; }
    virtual bool IsDeathDone() const { return true; }   // 預設：立即完成
    virtual void StartDying()  { m_IsDying = true; SetVisible(false); }

    static constexpr int   BASE_HP    = 8;
    static constexpr float BASE_SPEED = 150.0f;

    // 接觸傷害（全體敵人）
    static constexpr float CONTACT_COOLDOWN = 0.5f;
    static constexpr int   CONTACT_DAMAGE   = 1;

protected:
    Player* m_Target         = nullptr;
    bool    m_IsElite        = false;
    bool    m_IsDying        = false;  // 正在播放死亡動畫
    float   m_AttackCooldown = 0.0f;
    float   m_ContactTimer   = 0.0f;   // 接觸傷害冷卻計時
    glm::vec2 m_HitboxHalf   = {16.0f, 14.0f};  // 預設 GoblinEnemy 的半碰撞盒

    virtual void UpdateAI(float dt) = 0;
};

// ─── GoblinEnemy ──────────────────────────────────────────────────────────────
// BulletManager* 在建構時注入（不擁有），供攻擊時發射子彈（Step 2.3 實裝）
class GoblinEnemy : public Enemy {
public:
    explicit GoblinEnemy(BulletManager* bulletMgr);

    bool IsDeathDone() const override;
    void StartDying()  override;

protected:
    BulletManager* m_BulletMgr = nullptr;  // 注入，不擁有

    static constexpr float GOBLIN_HIT_W = 32.0f;
    static constexpr float GOBLIN_HIT_H = 28.0f;

    void TryMove(glm::vec2 wishDir, float speed, float dt);

private:
    std::shared_ptr<Util::Animation> m_Anim;
    std::shared_ptr<Util::Animation> m_DeathAnim;
};

// ─── PistolGoblin ─────────────────────────────────────────────────────────────
class PistolGoblin : public GoblinEnemy {
public:
    explicit PistolGoblin(BulletManager* bulletMgr) : GoblinEnemy(bulletMgr) {}

protected:
    void UpdateAI(float dt) override;

private:
    static constexpr float PREFERRED_DIST = 200.0f;
    static constexpr float MIN_DIST       = 100.0f;
    static constexpr float ATTACK_RANGE   = 280.0f;
    static constexpr float COOLDOWN       = 2.0f;

    int   m_BurstRemaining = 0;
    float m_BurstTimer     = 0.0f;
};

// ─── SpearGoblin ──────────────────────────────────────────────────────────────
class SpearGoblin : public GoblinEnemy {
public:
    explicit SpearGoblin(BulletManager* bulletMgr) : GoblinEnemy(bulletMgr) {}

protected:
    void UpdateAI(float dt) override;

private:
    float m_StabRange = 60.0f;
    static constexpr float COOLDOWN = 1.0f;
};

// ─── ArcherGoblin ─────────────────────────────────────────────────────────────
class ArcherGoblin : public GoblinEnemy {
public:
    explicit ArcherGoblin(BulletManager* bulletMgr) : GoblinEnemy(bulletMgr) {}

protected:
    void UpdateAI(float dt) override;

private:
    enum class AiState { MOVE, AIM, COOLDOWN };
    AiState m_AiState    = AiState::MOVE;
    float   m_StateTimer = 0.0f;

    static constexpr float PREFERRED_DIST = 250.0f;
    static constexpr float MIN_DIST       = 150.0f;
    static constexpr float ATTACK_RANGE   = 360.0f;
    static constexpr float AIM_DURATION   = 1.0f;
    static constexpr float SHOOT_COOLDOWN = 2.5f;
};
