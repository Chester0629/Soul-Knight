#pragma once

#include "Entity/Enemy.hpp"
#include "Util/Animation.hpp"

class BulletManager;

class Boss : public Enemy {
public:
    explicit Boss(BulletManager* bulletMgr);

    bool IsDeathDone() const override;
    void StartDying()  override;

    static constexpr int   BOSS_HP    = 50;
    static constexpr float BOSS_HIT_W = 48.0f;
    static constexpr float BOSS_HIT_H = 42.0f;

protected:
    void UpdateAI(float dt) override;

private:
    BulletManager* m_BulletMgr = nullptr;

    bool      m_IsDashing    = false;
    float     m_DashTimer    = 0.0f;
    float     m_DashCooldown = 0.0f;
    glm::vec2 m_DashDir      = {0.0f, 0.0f};

    std::shared_ptr<Util::Animation> m_Anim;
    std::shared_ptr<Util::Animation> m_DeathAnim;

    static constexpr float FAN_COOLDOWN   = 2.5f;
    static constexpr float FAN_SPREAD_RAD = 30.0f * 3.14159265f / 180.0f;  // 30°
    static constexpr int   FAN_COUNT      = 5;
    static constexpr float DASH_COOLDOWN  = 3.0f;
    static constexpr float DASH_SPEED     = 600.0f;
    static constexpr float DASH_DURATION  = 0.3f;
    static constexpr float BOSS_SPEED     = 80.0f;

    void TryMove(glm::vec2 wishDir, float speed, float dt);
};
