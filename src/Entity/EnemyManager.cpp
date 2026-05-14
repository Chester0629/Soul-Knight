#include "Entity/EnemyManager.hpp"
#include "Entity/Player.hpp"

void EnemyManager::AddEnemy(std::shared_ptr<Enemy> enemy) {
    m_Enemies.push_back(std::move(enemy));
}

void EnemyManager::AddToRenderer(Util::Renderer& renderer) {
    m_Renderer = &renderer;
    for (auto& e : m_Enemies)
        renderer.AddChild(e);
}

void EnemyManager::SetTarget(Player* player) {
    m_Target = player;
    for (auto& e : m_Enemies)
        e->SetTarget(player);
}

void EnemyManager::AddEnemyLive(std::shared_ptr<Enemy> enemy) {
    if (m_Target)   enemy->SetTarget(m_Target);
    if (m_Renderer) m_Renderer->AddChild(enemy);
    m_Enemies.push_back(std::move(enemy));
}

void EnemyManager::Update(float dt) {
    for (auto& e : m_Enemies) {
        if (e->IsDead()) {
            if (!e->IsDying()) {
                e->StartDying();
                ++m_PendingKills;       // 首次死亡計入擊殺數
            } else if (e->IsDeathDone()) {
                e->SetVisible(false);
            }
            continue;
        }
        e->Update(dt);
    }
}

void EnemyManager::Clear(Util::Renderer& renderer) {
    for (auto& e : m_Enemies)
        renderer.RemoveChild(e);
    m_Enemies.clear();
    m_PendingKills = 0;
}

int EnemyManager::TakeKills() {
    const int k = m_PendingKills;
    m_PendingKills = 0;
    return k;
}

bool EnemyManager::AllDead() const {
    for (const auto& e : m_Enemies)
        if (!e->IsDead()) return false;
    return true;
}
