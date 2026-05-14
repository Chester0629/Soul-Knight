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
    // 懶生成：生成後立即加入渲染樹（需先呼叫 SetRenderer / SetTarget）
    void AddEnemyLive(std::shared_ptr<Enemy> enemy);
    void AddToRenderer(Util::Renderer& renderer);
    void SetTarget(Player* player);
    void Update(float dt);

    // Step 4.1：清除所有敵人（從 renderer 移除並清空向量）
    void Clear(Util::Renderer& renderer);
    // Step 4.1：取出並重置本幀新增擊殺數（供 LevelManager 累計）
    int  TakeKills();

    const std::vector<std::shared_ptr<Enemy>>& GetEnemies() const { return m_Enemies; }
    bool AllDead() const;

private:
    std::vector<std::shared_ptr<Enemy>> m_Enemies;
    Util::Renderer*                     m_Renderer    = nullptr;
    Player*                             m_Target      = nullptr;
    int                                 m_PendingKills = 0;
};
