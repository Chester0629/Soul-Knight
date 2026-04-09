#include "Entity/Player.hpp"

#include "System/CollisionSystem.hpp"
#include <algorithm>
#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "World/Camera.hpp"

Player::Player(BulletManager* bulletMgr)
    : m_BulletMgr(bulletMgr)
{
    m_WorldPos = {INITIAL_X, INITIAL_Y};
    m_Speed    = SPEED;
    m_HP = m_MaxHP = 10;

    m_WalkAnim = std::make_shared<Util::Animation>(
        std::vector<std::string>{
            RESOURCE_DIR "/Characters/c01_0.png",
            RESOURCE_DIR "/Characters/c01_1.png",
            RESOURCE_DIR "/Characters/c01_2.png",
            RESOURCE_DIR "/Characters/c01_3.png",
        },
        true, 100, true, 0
    );

    m_IdleAnim = std::make_shared<Util::Animation>(
        std::vector<std::string>{
            RESOURCE_DIR "/Characters/c01_4.png",
            RESOURCE_DIR "/Characters/c01_5.png",
            RESOURCE_DIR "/Characters/c01_6.png",
            RESOURCE_DIR "/Characters/c01_7.png",
        },
        true, 150, true, 0
    );

    SetDrawable(m_IdleAnim);
    m_IsWalkingAnim = false;
    m_Transform.scale = {3.0f, 3.0f};
    SetVisible(true);

    // 初始武器：手槍
    m_CurrentWeapon = std::make_shared<GunWeapon>();

    // 武器精靈（手持顯示）
    m_WeaponSprite = std::make_shared<Util::GameObject>();
    m_WeaponSprite->SetDrawable(
        std::make_shared<Util::Image>(m_CurrentWeapon->GetSpritePath())
    );
    m_WeaponSprite->SetZIndex(GetZIndex() + 0.1f);
    m_WeaponSprite->m_Transform.scale = {3.0f, 3.0f};
    m_WeaponSprite->SetVisible(true);

    UpdateZIndex();
    SyncRenderTransform(Camera::GetPosition());
}

void Player::AddWeaponSpriteToRenderer(Util::Renderer& root) {
    root.AddChild(m_WeaponSprite);
}

void Player::HandleInput(float dt) {
    glm::vec2 dir = {0.0f, 0.0f};

    if (Util::Input::IsKeyPressed(Util::Keycode::W)) dir.y += 1.0f;
    if (Util::Input::IsKeyPressed(Util::Keycode::S)) dir.y -= 1.0f;
    if (Util::Input::IsKeyPressed(Util::Keycode::A)) dir.x -= 1.0f;
    if (Util::Input::IsKeyPressed(Util::Keycode::D)) dir.x += 1.0f;

    const bool isMoving = (dir.x != 0.0f || dir.y != 0.0f);

    if (isMoving) {
        const glm::vec2 normDir = glm::normalize(dir);
        m_LastMoveDir = normDir;

        if (dir.x != 0.0f)
            m_FacingLeft = (dir.x < 0.0f);

        m_WorldPos += normDir * m_Speed * dt;
    }

    if (isMoving && !m_IsWalkingAnim) {
        SetDrawable(m_WalkAnim);
        m_IsWalkingAnim = true;
    } else if (!isMoving && m_IsWalkingAnim) {
        SetDrawable(m_IdleAnim);
        m_IsWalkingAnim = false;
    }
}

void Player::TryShoot(float dt) {
    if (!m_CurrentWeapon || !m_BulletMgr) return;
    if (!m_CurrentWeapon->TryFire(dt)) return;

    if (m_CurrentWeapon->RequiresEnergy()) {
        if (m_Energy < m_CurrentWeapon->EnergyCostPerShot()) return;
        m_Energy -= m_CurrentWeapon->EnergyCostPerShot();
    }

    m_CurrentWeapon->Fire(m_WorldPos, m_LastMoveDir, *m_BulletMgr);
}

void Player::UpdateWeaponSpriteTransform() {
    if (!m_WeaponSprite) return;
    const float flip      = m_FacingLeft ? -1.0f : 1.0f;
    const float baseScale = 3.0f;
    m_WeaponSprite->m_Transform.scale = {flip * baseScale, baseScale};

    const float     handOffset    = flip * 16.0f;
    const glm::vec2 weaponWorldPos = {m_WorldPos.x + handOffset, m_WorldPos.y};
    m_WeaponSprite->m_Transform.translation = weaponWorldPos - Camera::GetPosition();

    // 使用與 SyncRender 相同公式直接算當前 Z，不依賴上一幀的 GetZIndex()
    const float playerZ = glm::clamp(50.0f - m_WorldPos.y / 6.0f, 2.0f, 98.0f);
    m_WeaponSprite->SetZIndex(playerZ + 0.1f);
}

void Player::Update(float dt) {
    HandleInput(dt);
    TryShoot(dt);

    // 護盾自動回復（每 SHIELD_REGEN_INTERVAL 秒 +1，上限 m_MaxShield）
    if (m_Shield < m_MaxShield) {
        m_ShieldRegenTimer += dt;
        if (m_ShieldRegenTimer >= SHIELD_REGEN_INTERVAL) {
            m_ShieldRegenTimer -= SHIELD_REGEN_INTERVAL;
            m_Shield = std::min(m_Shield + 1, m_MaxShield);
        }
    } else {
        m_ShieldRegenTimer = 0.0f;
    }

    CollisionSystem::ResolveWall(
        m_WorldPos,
        {0.0f, HIT_OFFSET_Y},
        {static_cast<float>(HIT_W), static_cast<float>(HIT_H)}
    );

    m_Transform.scale = {m_FacingLeft ? -3.0f : 3.0f, 3.0f};
    UpdateWeaponSpriteTransform();
}
