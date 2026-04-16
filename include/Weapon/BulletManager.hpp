#pragma once

#include "Weapon/Bullet.hpp"
#include "Util/Renderer.hpp"
#include "pch.hpp"

#include <memory>
#include <vector>

class Player;        // 前向宣告
class EnemyManager;  // 前向宣告

// BulletManager — 子彈對象池（上限 100 顆）
// 使用方式：
//   1. App::Start()  呼叫 AddToRenderer() 將所有子彈 GameObject 加入渲染樹
//   2. App::Update() 呼叫 Update(dt, cameraPos, player, enemyMgr) 每幀更新
//   3. Weapon::Fire() 呼叫 Spawn() 生成子彈
class BulletManager {
public:
    BulletManager();

    // 生成子彈（從非活躍子彈池取出一顆）
    // lifetime=-1.0f → 不限時；>0 → 秒數倒數後自動 Deactivate
    void Spawn(glm::vec2 worldPos, glm::vec2 dir, float speed,
               int damage, bool isPlayer, float lifetime = -1.0f);

    // 每幀更新：移動、碰牆/越界/壽命/實體碰撞、同步渲染
    void Update(float dt, glm::vec2 cameraPos, Player* player, EnemyManager* enemyMgr);

    // 停用子彈（碰牆、碰到目標、壽命結束時呼叫）
    void Deactivate(Bullet* b);

    // 將全部 100 顆 Bullet GameObject 加入渲染樹（App::Start 呼叫一次）
    void AddToRenderer(Util::Renderer& root);

    const std::vector<std::shared_ptr<Bullet>>& GetBullets() const { return m_Bullets; }

    static constexpr int   MAX_BULLETS  = 100;
    static constexpr float OUT_OF_RANGE = 6000.0f;  // 5×5 網格最遠房間距原點 ≈4320px，6000 含子彈飛行餘量

private:
    std::vector<std::shared_ptr<Bullet>> m_Bullets;

    // 從池中取得一顆非活躍子彈；池滿時回傳 nullptr（靜默丟棄）
    Bullet* GetInactive();
};
