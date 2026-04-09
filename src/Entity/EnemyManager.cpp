#include "Entity/EnemyManager.hpp"
#include "Entity/Player.hpp"

void EnemyManager::AddEnemy(std::shared_ptr<Enemy> enemy) {
    m_Enemies.push_back(std::move(enemy));
}

void EnemyManager::AddToRenderer(Util::Renderer& renderer) {
    for (auto& e : m_Enemies)
        renderer.AddChild(e);
}

void EnemyManager::SetTarget(Player* player) {
    for (auto& e : m_Enemies)
        e->SetTarget(player);
}

void EnemyManager::Update(float dt) {
    for (auto& e : m_Enemies) {
        if (e->IsDead()) {
            if (!e->IsDying()) {
                e->StartDying();        // 首次死亡：啟動死亡動畫
            } else if (e->IsDeathDone()) {
                e->SetVisible(false);   // 動畫播完：隱藏
            }
            continue;                   // 死亡中不執行 AI
        }
        e->Update(dt);
    }
}

bool EnemyManager::AllDead() const {
    for (const auto& e : m_Enemies)
        if (!e->IsDead()) return false;
    return true;
}
