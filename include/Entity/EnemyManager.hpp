#pragma once

#include "Entity/Enemy.hpp"
#include "Util/Renderer.hpp"

#include <memory>
#include <vector>

class Player;

// EnemyManager — 持有並統一管理所有敵人實體
// 使用方式：
//   App::Start()  → AddEnemy() + AddToRenderer() + SetTarget()
//   App::Update() → Update(dt)
class EnemyManager {
public:
    void AddEnemy(std::shared_ptr<Enemy> enemy);
    void AddToRenderer(Util::Renderer& renderer);
    void SetTarget(Player* player);
    void Update(float dt);

    const std::vector<std::shared_ptr<Enemy>>& GetEnemies() const { return m_Enemies; }
    bool AllDead() const;

private:
    std::vector<std::shared_ptr<Enemy>> m_Enemies;
};
