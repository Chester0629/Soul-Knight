#include "Weapon/Bullet.hpp"

#include "Util/Image.hpp"

Bullet::Bullet() {
    // 預設使用圓形子彈圖（玩家/敵人共用，Step 2.3 後可按需切換圖片）
    SetDrawable(std::make_shared<Util::Image>(
        RESOURCE_DIR "/Bullets/bullet_0.png"
    ));
    SetZIndex(Z_INDEX);
    SetVisible(false);  // 初始不可見；Spawn 時才 SetVisible(true)
    m_Transform.scale = {2.0f, 2.0f};
}
