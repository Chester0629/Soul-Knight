#pragma once

#include "Entity/Entity.hpp"
#include "Util/Animation.hpp"

class Player; // 避免循環 include，在 .cpp 中 include Player.hpp

// ─── Enemy ────────────────────────────────────────────────────────────────────
class Enemy : public Entity {
public:
    void Update(float dt) override;
    void SetTarget(Player* player) { m_Target = player; }
    void SetElite(bool elite)      { m_IsElite = elite; }

    static constexpr int   BASE_HP    = 8;
    static constexpr float BASE_SPEED = 150.0f;

protected:
    Player* m_Target         = nullptr;
    bool    m_IsElite        = false;
    float   m_AttackCooldown = 0.0f;

    virtual void UpdateAI(float dt) = 0;
};

// ─── GoblinEnemy ──────────────────────────────────────────────────────────────
// 所有哥布林的中間基類
class GoblinEnemy : public Enemy {
public:
    GoblinEnemy();

protected:
    static constexpr float GOBLIN_HIT_W = 32.0f;
    static constexpr float GOBLIN_HIT_H = 28.0f;

    void TryMove(glm::vec2 wishDir, float speed, float dt);

private:
    std::shared_ptr<Util::Animation> m_Anim;
};

// ─── PistolGoblin ─────────────────────────────────────────────────────────────
class PistolGoblin : public GoblinEnemy {
public:
    PistolGoblin() = default;

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
    SpearGoblin() = default;

protected:
    void UpdateAI(float dt) override;

private:
    float m_StabRange = 60.0f;
    static constexpr float COOLDOWN = 1.0f;
};

// ─── ArcherGoblin ─────────────────────────────────────────────────────────────
class ArcherGoblin : public GoblinEnemy {
public:
    ArcherGoblin() = default;

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
