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
            e->SetVisible(false);  // 死亡後立即隱藏（Step 2.4 補死亡動畫）
            continue;
        }
        e->Update(dt);
    }
}

bool EnemyManager::AllDead() const {
    for (const auto& e : m_Enemies)
        if (!e->IsDead()) return false;
    return true;
}
