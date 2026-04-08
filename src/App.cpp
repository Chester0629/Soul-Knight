#include "App.hpp"

#include "Entity/Enemy.hpp"
#include "System/CollisionSystem.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Logger.hpp"
#include "Util/Time.hpp"
#include "World/Camera.hpp"

void App::Start() {
    LOG_TRACE("Start");

    m_Room.AddToRenderer(m_Root);
    m_Room.SyncTransforms({0.0f, 0.0f});

    CollisionSystem::SetRoom(&m_Room);

    // Step 2.2：子彈對象池加入渲染樹（100 顆 GameObject 一次性加入）
    m_BulletManager.AddToRenderer(m_Root);

    // 玩家（注入 BulletManager*）
    m_Player = std::make_shared<Player>(&m_BulletManager);
    m_Root.AddChild(m_Player);
    m_Player->AddWeaponSpriteToRenderer(m_Root);

    // 測試哥布林（注入 BulletManager*）
    {
        auto pistol = std::make_shared<PistolGoblin>(&m_BulletManager);
        pistol->SetWorldPos({200.0f, 100.0f});

        auto spear = std::make_shared<SpearGoblin>(&m_BulletManager);
        spear->SetWorldPos({-150.0f, 50.0f});

        auto archer = std::make_shared<ArcherGoblin>(&m_BulletManager);
        archer->SetWorldPos({0.0f, -150.0f});

        m_EnemyManager.AddEnemy(pistol);
        m_EnemyManager.AddEnemy(spear);
        m_EnemyManager.AddEnemy(archer);
        m_EnemyManager.AddToRenderer(m_Root);
        m_EnemyManager.SetTarget(m_Player.get());
    }

    m_CurrentState = State::UPDATE;
}

void App::Update() {
    const float dt = Util::Time::GetDeltaTimeMs() / 1000.0f;

    m_Player->Update(dt);
    Camera::Update(m_Player->GetWorldPos());
    m_Player->SyncRender(Camera::GetPosition());
    m_EnemyManager.Update(dt);
    m_BulletManager.Update(dt, Camera::GetPosition());
    m_Room.SyncTransforms(Camera::GetPosition());

    m_Root.Update();

    if (Util::Input::IsKeyUp(Util::Keycode::ESCAPE) ||
        Util::Input::IfExit()) {
        m_CurrentState = State::END;
    }
}

void App::End() {
    LOG_TRACE("End");
}
