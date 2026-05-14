#pragma once

#include "Entity/Enemy.hpp"
#include "Util/Animation.hpp"

#include <memory>

class BulletManager;

// Boss — 第 5 層 Boss（boss08_0~7，8 幀動畫）
// Phase 1（HP > 50）：緩速接近 + 每 2.5s 扇形 5 顆子彈
// Phase 2（HP ≤ 50）：同上 + 每 4s 衝刺
class Boss : public Enemy {
public:
    explicit Boss(BulletManager* bulletMgr);

    bool IsDeathDone() const override;
    void StartDying()  override;

    static constexpr int   BOSS_HP          = 100;
    static constexpr float BOSS_SPEED       = 60.0f;
    static constexpr float BOSS_HIT_W       = 64.0f;
    static constexpr float BOSS_HIT_H       = 52.0f;
    static constexpr float FAN_COOLDOWN     = 2.5f;
    static constexpr float DASH_COOLDOWN    = 4.0f;
    static constexpr float DASH_SPEED       = 500.0f;
    static constexpr float DASH_DURATION    = 0.35f;
    static constexpr float RECOVER_DURATION = 0.6f;

protected:
    void UpdateAI(float dt) override;

private:
    enum class Phase   { ONE, TWO };
    enum class AiState { APPROACH, DASHING, RECOVERING };

    Phase     m_Phase      = Phase::ONE;
    AiState   m_AiState    = AiState::APPROACH;
    float     m_StateTimer = 0.0f;
    float     m_FanTimer   = FAN_COOLDOWN;
    float     m_DashTimer  = DASH_COOLDOWN;
    glm::vec2 m_DashDir    = {0.0f, 0.0f};

    BulletManager* m_BulletMgr = nullptr;

    std::shared_ptr<Util::Animation> m_IdleAnim;
    std::shared_ptr<Util::Animation> m_DeathAnim;

    void FireFanShot();
};
