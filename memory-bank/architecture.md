# architecture.md — 架構設計文件
> Soul Knight (OOP 2025 期末專案)
> 版本: 1.5 | 最後更新: 2026-03-21（更新 Player 射擊方向為 WASD m_LastMoveDir、新增地城 5×5 網格布局架構）

---

## 一、高層架構概覽

```
main.cpp（不修改）
└── App（狀態機：START → UPDATE → END）
    ├── App::Start()   ← 初始化所有系統
    ├── App::Update()  ← 每幀：輸入 → 邏輯 → 碰撞 → 渲染
    └── App::End()     ← 釋放資源

App 持有：
├── GameManager          ← 遊戲狀態機（MainMenu/Game/GameOver/Victory）
│   ├── SceneManager     ← 場景切換（直接切換，無淡出效果）
│   └── LevelManager     ← 層數管理（第 1~4 層普通，第 5 層 Boss）
│
├── World                ← 遊戲世界（當前層）
│   ├── Room[]           ← 6–9 個房間
│   ├── DungeonGenerator ← 地城生成（樹狀/網狀，走廊銜接）
│   └── Camera           ← 視角控制（即時跟隨玩家，M1 不做 Lerp）
│
├── Player               ← 玩家角色
│   ├── PlayerController ← WASD 輸入、J 鍵攻擊
│   ├── WeaponSystem     ← 武器持有、攻擊方向、能量消耗
│   └── HealthSystem     ← 血量 + 能量管理（m_MaxEnergy = 200）
│
├── EnemyManager         ← 敵人管理
│   └── Enemy[]          ← 各種敵人實體
│
├── BulletManager        ← 子彈管理（對象池，上限 100）
│   └── Bullet[]         ← 子彈實體
│
├── CollisionSystem      ← AABB 碰撞偵測
│
└── HUD                  ← UI 系統
    ├── HealthBar        ← 使用素材圖片
    ├── EnergyBar        ← 最大值 200
    ├── WeaponIcon
    └── Minimap          ← 右中上角，進入房間才顯示
```

---

## 二、核心類別設計

### 2.1 繼承樹

```
Util::GameObject（PTSD 基類）
├── Entity                      ← 所有可移動的有生命物件
│   ├── Player
│   └── Enemy
│       ├── GoblinEnemy             ← 哥布林小兵基類（共用素材、血量、速度）
│       │   ├── PistolGoblin            ← 手槍哥布林（保距 + 射彈）
│       │   ├── SpearGoblin             ← 長槍哥布林（追直 + 近戰突刺）
│       │   └── ArcherGoblin            ← 弓箭哥布林（保距 + 瞄準停止 + 射箭）
│       ├── BasicEnemy              ← 近戰追蹤型（直線追蹤）
│       ├── RangedEnemy             ← 遠程型（隨機移動 + 射子彈）
│       └── Boss                    ← boss08，Phase 1 扇形彈幕 + Phase 2 衝刺
├── Tile
│   ├── FloorTile               ← Z=0，固定不動
│   ├── WallTile                ← 動態 Y-Sorting（具立體感）
│   └── DoorTile                ← 可切換開/關貼圖，敵人全清後才開
├── Pickup
│   ├── WeaponPickup
│   ├── HealthPickup
│   └── EnergyPickup
└── Bullet                      ← 由 BulletManager 對象池管理

（獨立類別，不繼承 GameObject）
Weapon                          ← 武器資料 + 行為類別
├── GunWeapon                   ← 手槍（初始武器，能量消耗 0）
├── ShotgunWeapon               ← 散彈（能量消耗 10/發，3 顆扇形）
└── LaserWeapon                 ← 雷射（能量消耗 2/幀，持續型）
```

### 2.2 Entity 基類

```cpp
class Entity : public Util::GameObject {
public:
    virtual ~Entity() = default;  // ✅ 多型 delete 必要
    virtual void Update(float dt) = 0;
    virtual void TakeDamage(int damage);
    virtual bool IsDead() const { return m_HP <= 0; }

    // Y-Sorting：每幀根據「世界 Y 座標」動態更新 Z-Index
    // ⚠️ 必須用 m_WorldPos.y，不得用 m_Transform.translation.y！
    void UpdateZIndex() {
        SetZIndex(glm::clamp(1000.0f - m_WorldPos.y, 2.0f, 198.0f));
    }

    // 將世界座標轉換為渲染座標（每幀在 Update 末端呼叫）
    void SyncRenderTransform(glm::vec2 cameraPos) {
        m_Transform.translation = m_WorldPos - cameraPos;
    }

protected:
    // ⭐ 核心：世界座標（物理移動與碰撞的唯一真實座標）
    glm::vec2 m_WorldPos  = {0.0f, 0.0f};

    int       m_HP;
    int       m_MaxHP;
    float     m_Speed;
    glm::vec2 m_Velocity;

    // AABB 碰撞盒（相對於 m_WorldPos 中心點的偏移 + 大小）
    glm::vec2 m_HitboxOffset = {0, 0};   // 玩家設為向下偏移 {0, -10}
    glm::vec2 m_HitboxSize   = {24, 20}; // 玩家預設值
};
```

> [!CAUTION]
> **⚠️ 世界座標 vs 渲染座標 — 核心架構，絕對不可混淆**
>
> | 變數 | 用途 | 誰可以修改它 |
> |------|------|-------------|
> | `m_WorldPos` | 物理移動、碰撞判定、AI 目標計算、Y-Sort | 遊戲邏輯（移動、AI、碰撞） |
> | `m_Transform.translation` | **僅用於 PTSD Renderer 渲染** | 只有 `SyncRenderTransform()` |
>
> ```cpp
> // ✅ 正確的每幀更新流程
> void Entity::Update(float dt) {
>     m_WorldPos += m_Velocity * dt;      // 1. 物理移動
>     // 2. CollisionSystem 使用並修改 m_WorldPos
>     SyncRenderTransform(Camera::GetPosition());  // 3. 最後同步渲染
>     UpdateZIndex();
> }
>
> // ❌ 嚴禁：直接修改 translation 作為物理位移
> m_Transform.translation += velocity * dt;
> ```


### 2.3 Player

```cpp
class Player : public Entity {
public:
    void Update(float dt) override {
        HandleInput();      // WASD 移動（斜向需 Normalize）
        UpdateFacing();     // 左/右朝向（scale.x 正負）
        TryShoot();         // 按 J 鍵才發射，朝玩家面向方向
        ApplyPhysics(dt);
        UpdateWeaponSpriteTransform();
        SyncRenderTransform(Camera::GetPosition());
        UpdateZIndex();
    }

    // 已確認規格
    static constexpr float SPEED        = 300.0f;   // pixel/second
    static constexpr int   HIT_W        = 24;
    static constexpr int   HIT_H        = 20;
    static constexpr float HIT_OFFSET_Y = -10.0f;  // 碰撞盒中心向下偏移（Step 1.5 後微調）

    // 初始世界座標（M1 測試房間用）
    static constexpr float INITIAL_X = -300.0f;
    static constexpr float INITIAL_Y = -100.0f;

private:
    std::shared_ptr<Weapon>           m_CurrentWeapon;  // 武器資料 + 行為
    std::shared_ptr<Util::GameObject> m_WeaponSprite;   // 武器視覺物件（獨立 GameObject）

    int       m_Energy      = 200;            // ⭐ 初始滿能量
    int       m_MaxEnergy   = 200;            // ⭐ 最大能量（已確認為 200）
    bool      m_FacingLeft  = false;          // 控制 Sprite 翻轉（只在有水平輸入時更新）
    glm::vec2 m_LastMoveDir = {1.0f, 0.0f};  // ⭐ 射擊方向：記錄最後一次 WASD 輸入（Normalize），無輸入時沿用
};
```

> [!WARNING]
> **⚠️ `m_WeaponSprite` 必須隨玩家朝向同步鏡像翻轉，且基於 WorldPos 計算！**
>
> ```cpp
> void Player::UpdateWeaponSpriteTransform() {
>     float flip      = m_FacingLeft ? -1.0f : 1.0f;
>     float baseScale = 3.0f;
>     m_WeaponSprite->m_Transform.scale = {flip * baseScale, baseScale};
>     float handOffset = flip * 16.0f;
>     glm::vec2 weaponWorldPos = { m_WorldPos.x + handOffset, m_WorldPos.y };
>     m_WeaponSprite->m_Transform.translation = weaponWorldPos - Camera::GetPosition();
> }
> ```


### 2.4 Enemy

```cpp
class Enemy : public Entity {
public:
    void Update(float dt) override {
        UpdateAI(dt);
        SyncRenderTransform(Camera::GetPosition());
        UpdateZIndex();
    }
    void SetTarget(Player* player);
    void SetElite(bool elite) { m_IsElite = elite; }

    static constexpr int   BASE_HP    = 8;
    static constexpr float BASE_SPEED = 150.0f;

protected:
    Player* m_Target       = nullptr;
    bool    m_IsElite      = false;
    float   m_AttackCooldown = 0.0f;

    virtual void UpdateAI(float dt) = 0;
};

class GoblinEnemy : public Enemy {
protected:
    float m_AimTimer = 0.0f;  // ArcherGoblin 用

    // 所有哥布林移動均透過此函式（內含 CollisionSystem::IsBlocked 避障）
    void TryMove(glm::vec2 wishDir, float speed, float dt);
};

class PistolGoblin : public GoblinEnemy {
    void UpdateAI(float dt) override;
    int m_BurstCount = 0;
    // 普通：1 發，攻擊力 2，冷卻 2.0s
    // 精英：Burst 3 發（間隔 0.15s），攻擊力 3，全射完後冷卻 2.0s
};

class SpearGoblin : public GoblinEnemy {
    void UpdateAI(float dt) override;
    float m_StabRange = 60.0f;  // 精英形態: 90.0f
    // 普通：突刺 1 次，攻擊力 3，冷卻 1.0s
    // 精英：突刺碰撞盒寬高 ×1.5
};

class ArcherGoblin : public GoblinEnemy {
    void UpdateAI(float dt) override;
    static constexpr float AIM_DURATION = 1.0f;
    // 普通：瞄準 1.0s → 射箭，攻擊力 4，冷卻 2.5s
    // 精英：箭矢 Scale ×1.5，攻擊力 5
};
```

### 2.5 Weapon（非 GameObject）

```cpp
// Weapon 是「資料 + 行為」類別，不繼承 GameObject
class Weapon {
public:
    virtual ~Weapon() = default;
    virtual void Fire(glm::vec2 origin, glm::vec2 direction,
                      BulletManager& bulletMgr) = 0;
    virtual bool RequiresEnergy() const { return false; }
    virtual int  EnergyCostPerShot() const { return 0; }  // 手槍 = 0
    int         GetDamage()     const { return m_Damage; }
    float       GetFireRate()   const { return m_FireRate; }
    std::string GetSpritePath() const { return m_SpritePath; }

protected:
    int         m_Damage    = 10;
    float       m_FireRate  = 0.5f;
    std::string m_SpritePath;
};

// 手槍（初始武器）
class GunWeapon : public Weapon {
public:
    // 素材：Resources/Weapons/weapon_pistol.png
    // 能量消耗：0（免費）
    // 射擊：IsKeyDown(J)，每次 1 發，速度 700.0f px/s，攻擊力 10
    void Fire(glm::vec2 origin, glm::vec2 dir, BulletManager& mgr) override;
    bool RequiresEnergy() const override { return false; }
    int  EnergyCostPerShot() const override { return 0; }
};

// 散彈
class ShotgunWeapon : public Weapon {
public:
    // 素材：Resources/Weapons/weapon_shotgun.png
    // 能量消耗：10 / 發（整輪扣 10）
    // 射擊：IsKeyDown(J)，同時射 3 顆（正前方 ±15°），速度 600.0f px/s，攻擊力 8
    void Fire(glm::vec2 origin, glm::vec2 dir, BulletManager& mgr) override;
    bool RequiresEnergy() const override { return true; }
    int  EnergyCostPerShot() const override { return 10; }
};

// 雷射
class LaserWeapon : public Weapon {
public:
    // 素材：Resources/Weapons/weapon_laser.png
    // 能量消耗：2 / 幀（按住 J 每幀扣）
    // 射擊：IsKeyPressed(J)，每幀持續射出，攻擊力 5/幀（M2 可先用普通子彈代替）
    void Fire(glm::vec2 origin, glm::vec2 dir, BulletManager& mgr) override;
    bool RequiresEnergy() const override { return true; }
    int  EnergyCostPerShot() const override { return 2; }  // per frame
};
```

> [!WARNING]
> **⚠️ 子彈 Hitbox 與發射偏移強制規則（防止發射立刻碰牆消失）**
>
> - **子彈 AABB Hitbox：一律使用核心小判定盒 `10 × 10 px`**，不得直接拿圖片尺寸作為判定盒
> - **發射原點偏移：`Fire()` 的 `origin` 必須從玩家中心往 `direction` 偏移 `20px`**
>
> ```cpp
> void GunWeapon::Fire(glm::vec2 origin, glm::vec2 dir, BulletManager& mgr) {
>     glm::vec2 spawnPos = origin + dir * 20.0f;
>     mgr.Spawn(spawnPos, dir, 700.0f, m_Damage, /*isPlayer=*/true);
> }
> ```

> [!WARNING]
> **⚠️ BulletManager 對象池「幽靈渲染」Bug — 必須配合 SetVisible()！**
>
> ```cpp
> void BulletManager::Deactivate(Bullet* b) {
>     b->m_Active = false;
>     b->SetVisible(false);   // ← 必須！
> }
> void BulletManager::Spawn(glm::vec2 worldPos, glm::vec2 dir, float speed,
>                           int damage, bool isPlayer) {
>     Bullet* b = GetInactive();
>     if (!b) return;
>     b->m_WorldPos = worldPos;
>     b->m_Velocity = dir * speed;
>     b->m_Active   = true;
>     b->SetVisible(true);    // ← 必須！
> }
> ```

### 2.6 Room

```cpp
class Room {
public:
    enum class Direction { UP, DOWN, LEFT, RIGHT };

    void Generate(int width, int height, int templateId);
    void SetDoor(Direction dir, bool exists);
    void OpenDoor(Direction dir);
    void SpawnEnemies(EnemyManager& mgr);
    bool IsCleared() const;

    std::vector<std::vector<std::unique_ptr<Tile>>> m_TileMap;   // [row][col]
    std::map<Direction, Room*>                      m_Neighbors;
    std::vector<std::unique_ptr<Enemy>>             m_Enemies;
    bool m_Visited = false;
};
```

> [!CAUTION]
> **⚠️ `m_TileMap` 存取順序死命令：必須用 `[row][col]`，絕對不得用 `[x][y]`！**
>
> ```cpp
> // ✅ 正確：外層 row（高），內層 col（寬）
> for (int row = 0; row < height; ++row)
>     for (int col = 0; col < width; ++col)
>         m_TileMap[row][col] = ...;
>
> // ❌ 錯誤：vector subscript out of range 崩潰
> for (int x = 0; x < width; ++x)
>     for (int y = 0; y < height; ++y)
>         m_TileMap[x][y] = ...;
> ```

> [!IMPORTANT]
> **⚠️ 敵人生成位置強制規則：必須從 FloorTile 隨機挑選**
>
> ```cpp
> void Room::SpawnEnemies(EnemyManager& mgr) {
>     std::vector<glm::vec2> floorPositions;
>     for (int row = 0; row < m_Height; ++row)
>         for (int col = 0; col < m_Width; ++col)
>             if (dynamic_cast<FloorTile*>(m_TileMap[row][col].get()))
>                 floorPositions.push_back(TileToWorld(row, col));
>     std::shuffle(floorPositions.begin(), floorPositions.end(), rng);
>     for (int i = 0; i < enemyCount && i < (int)floorPositions.size(); ++i)
>         mgr.SpawnAt(floorPositions[i]);
> }
> ```

### 2.7 Boss（分段攻擊）

```cpp
class Boss : public Enemy {
public:
    enum class Phase { PHASE_1, PHASE_2 };

    void UpdateAI(float dt) override {
        // Phase 判斷：HP > 50% → PHASE_1，HP ≤ 50% → PHASE_2
        m_Phase = (m_HP > m_MaxHP / 2) ? Phase::PHASE_1 : Phase::PHASE_2;

        UpdatePhase1(dt);       // 扇形彈幕（兩 Phase 均執行）
        if (m_Phase == Phase::PHASE_2)
            UpdateDash(dt);     // 橫向衝刺（只在 Phase 2 執行）
    }

private:
    Phase m_Phase       = Phase::PHASE_1;
    float m_BulletTimer = 0.0f;   // 扇形彈幕冷卻（2.5s）
    float m_DashTimer   = 0.0f;   // 衝刺冷卻
    bool  m_IsDashing   = false;  // 衝刺中旗標

    // Phase 1：每 2.5s 發射扇形子彈 5 發
    void UpdatePhase1(float dt);
    // Phase 2：增加橫向快速衝刺
    void UpdateDash(float dt);
};
```

### 2.8 DungeonGenerator

```cpp
class DungeonGenerator {
public:
    // 生成 6–9 個普通房間 + 1 個 Boss 房間
    std::vector<Room*> Generate(int seed, int numRooms);

private:
    void ConnectRoomsWithCorridor(Room* a, Room* b, Direction dir);
    void PlaceBossRoom(std::vector<Room*>& rooms);
};
```

---

## 三、遊戲流程（狀態機）

```
[MainMenu]（開始遊戲、退出、設定）
    │ 選角色 → 進入選角室（可升級）→ 開始
    ▼
[Game] — 層數：第 1~4 層普通，第 5 層 Boss
    │ 進入房間瞬間 Spawn 全部敵人
    │   ├── 敵人全死 → 門開啟
    │   ├── 走廊 → 相鄰房間（觸發新 Spawn）
    │   ├── 所有房間清空 → 傳送陣出現（房間中央）
    │   └── 玩家踩傳送陣 → 進入下一層
    │
    ├── HP ≤ 0 → [GameOver]（直接切換，顯示存活層數/擊殺數）
    │               │ 重開 → [MainMenu]
    └── Boss 死亡 → [Victory]（直接切換，顯示層數/擊殺數）
                    │ 返回 → [MainMenu]
```

---

## 四、地圖座標系統（已確認）

```
TileMap 座標（row, col）：
  (0,0) 在左上角，row 向下增加，col 向右增加

世界座標換算（房間中心 = 世界原點 0,0）：
  TILE_SIZE = 48（16px × 3 倍 scale）
  worldX =  col * TILE_SIZE - (mapCols * TILE_SIZE) / 2 + TILE_SIZE / 2
  worldY = -(row * TILE_SIZE - (mapRows * TILE_SIZE) / 2 + TILE_SIZE / 2)  ← ⚠️ 負號不可省

M1 測試房間（17×17 格含牆，地板 15×15）：
  總寬 = 17 × 48 = 816px，總高 = 17 × 48 = 816px
  可見行：Row 1（NorthFace）~ Row 15（SouthFace），恰好填滿 720px 螢幕高度
  玩家初始位置 = (-300.0f, -100.0f)（左側地板區域）
```

---

## 五、模組檔案規劃

```
include/
├── Core/
│   ├── GameManager.hpp
│   ├── SceneManager.hpp
│   └── LevelManager.hpp
├── World/
│   ├── Tile.hpp
│   ├── Room.hpp
│   ├── DungeonGenerator.hpp
│   └── Camera.hpp
├── Entity/
│   ├── Entity.hpp
│   ├── Player.hpp
│   ├── Enemy.hpp         （Enemy + GoblinEnemy + PistolGoblin + SpearGoblin + ArcherGoblin）
│   └── Boss.hpp
├── Weapon/
│   ├── Weapon.hpp        （Weapon + GunWeapon + ShotgunWeapon + LaserWeapon）
│   ├── Bullet.hpp
│   └── BulletManager.hpp
├── Pickup/
│   ├── WeaponPickup.hpp
│   ├── HealthPickup.hpp
│   └── EnergyPickup.hpp
├── System/
│   ├── CollisionSystem.hpp
│   └── EnemyAI.hpp
└── UI/
    ├── HUD.hpp
    └── Minimap.hpp
```

---

## 六、碰撞系統（AABB）

| 碰撞對 | 結果 |
|--------|------|
| 玩家 ↔ 牆壁 | 阻擋移動（穿透深度 Push Back） |
| 敵人 ↔ 牆壁 | 阻擋移動（TryMove 避障） |
| 玩家子彈 ↔ 敵人 | 敵人扣血，子彈消失 |
| 敵人子彈 ↔ 玩家 | 玩家扣血，子彈消失 |
| 任何子彈 ↔ 牆壁 | 子彈消失 |
| 敵人（近戰）↔ 玩家 | 每次觸發扣血，0.5 秒 Cooldown |
| 玩家 ↔ 掉落物 | 撿取（HP/Energy/武器） |
| 玩家 ↔ 傳送陣 | 進入下一層 |
| 玩家 ↔ 門（關閉） | 阻擋移動 |

> [!IMPORTANT]
> **⚠️ AABB Push Back 必須使用「穿透深度」，禁止暴力歸位！**
>
> ```cpp
> void CollisionSystem::ResolveAABB(glm::vec2& worldPos, glm::vec2 hitboxSize,
>                                   glm::vec2 wallCenter, glm::vec2 wallSize) {
>     float overlapX = (hitboxSize.x + wallSize.x) / 2.0f
>                    - std::abs(worldPos.x - wallCenter.x);
>     float overlapY = (hitboxSize.y + wallSize.y) / 2.0f
>                    - std::abs(worldPos.y - wallCenter.y);
>     if (overlapX <= 0 || overlapY <= 0) return;
>     if (overlapX < overlapY)
>         worldPos.x += (worldPos.x < wallCenter.x) ? -overlapX : overlapX;
>     else
>         worldPos.y += (worldPos.y < wallCenter.y) ? -overlapY : overlapY;
> }
> ```

---

## 七、動態 Y-Sorting（2.5D 渲染排序）

> [!CAUTION]
> **⚠️ PTSD Z-Index 硬性限制（2026-03-25 確認）**
> `zIndex` 直接作為 3D Z 座標，nearClip=-100, farClip=100。
> **Z > 100 或 Z < -100 → 被 GPU 裁剪，完全不可見！**

| 物件類型 | Z-Index 策略 | 具體值 |
|---------|------------|--------|
| 地板 Tile | 固定 | `0.0f` |
| 地板裝飾（血跡等） | 固定 | `1.0f` |
| WallTile（上/左/右牆）| 固定 | `0.5f` |
| NorthFaceTile | 固定 | `0.6f` |
| 玩家、敵人、掉落物、SouthFaceTile | 動態 Y-Sorting | `clamp(50 - worldY/6, 2.0f, 98.0f)` |
| 武器精靈（m_WeaponSprite） | 動態 | 玩家 Z-Index + 0.1f（不超過 98.5f）|
| 子彈 | 固定 | `99.0f` |
| HUD / UI | 固定 | `99.5f` |

```cpp
void Entity::UpdateZIndex() {
    // ⚠️ 必須使用 m_WorldPos.y，不得用 m_Transform.translation.y
    // ⚠️ Z 必須在 (-100, 100) 內，否則被 PTSD GPU 裁剪不可見
    SetZIndex(glm::clamp(50.0f - m_WorldPos.y / 6.0f, 2.0f, 98.0f));
}
```

---

## 八、玩家攻擊設計（已定案）

- **攻擊鍵**：`Util::Keycode::J`（已定案，禁止使用其他按鍵）
- **射擊方向**：讀取 `m_LastMoveDir`（與 `m_FacingLeft` 獨立）

> [!IMPORTANT]
> **⚠️ `m_LastMoveDir` vs `m_FacingLeft` — 兩個獨立的方向狀態**
>
> | 變數 | 更新時機 | 用途 |
> |------|---------|------|
> | `m_FacingLeft` | 有水平輸入（A/D）時才更新 | 控制 Sprite 翻轉（左/右鏡像） |
> | `m_LastMoveDir` | 有任何 WASD 輸入時更新（Normalize 後存入） | 決定子彈飛行方向 |
>
> 兩者互不影響。例如：玩家按 W 向上走時，`m_FacingLeft` 保持上次值（不變），`m_LastMoveDir` 更新為 `{0,1}`。

```
1. 有 WASD 輸入時：m_LastMoveDir = normalize(input)（同時更新 m_FacingLeft 如有水平輸入）
2. 玩家按下 J 鍵：direction = m_LastMoveDir
3. 呼叫 m_CurrentWeapon->Fire(m_WorldPos, direction, bulletMgr)
4. 若 RequiresEnergy() = true，先檢查 m_Energy 是否足夠
   - 不足：不射擊（無視按鍵輸入）
   - 足夠：射擊並扣除 EnergyCostPerShot()
```

---

## 九、效能注意事項

1. **Bullet 對象池**：上限 100 顆，避免頻繁 new/delete
2. **Tile 數量**：30×20 房間 = 最多 600 個 GameObject。M1 先照普通做法；M3 後考慮合併地板 Texture
3. **Y-Sorting 效能**：每幀一次浮點計算，開銷忽略
4. **動畫幀率**：行走 100ms，Idle 150ms，死亡動畫 `looping=false`
5. **哥布林 AI 效能**：每幀一次狀態評估 + 一次距離計算，可忽略

---

## 十、哥布林攻擊設計詳解

### 10.1 移動 AI 狀態機

#### PistolGoblin 與 ArcherGoblin（保距型）
```
PATROL → CHASE → MAINTAIN_DISTANCE → ATTACK → MAINTAIN_DISTANCE
```

#### 弓箭哥布林額外：瞄準順序
```
MAINTAIN_DISTANCE → ATTACK_AIM（停止 1.0s）→ ATTACK_SHOOT → 冷卻 2.5s → MAINTAIN_DISTANCE
```

#### SpearGoblin（追蹤型）
```
PATROL → CHASE → ATTACK（突刺 + 冷卻 1.0s）→ CHASE
```

### 10.2 攻擊冷卻
| 哥布林 | 普通冷卻 | 精英冷卻 |
|------|---------|---------|
| PistolGoblin | 2.0s / 次 | Burst 間隔 0.15s，完成後 2.0s |
| SpearGoblin | 1.0s / 次 | 同左（碰撞盒大） |
| ArcherGoblin | 瞄準 1.0s → 冷卻 2.5s | 同左（箭矢更大） |

### 10.3 子彈/箭矢 Spawn 參數
| 敵人 | 素材 | Scale | 速度 |
|------|------|-------|------|
| PistolGoblin | `enemy_bullet_pistol.png` | {1,1} / 同 | 350.0f px/s |
| SpearGoblin | `enemy_stab.png` | {1,1} / {1.5,1.5} | 靜態（附著敵人前方） |
| ArcherGoblin | `enemy_arrow.png` | {1,1} / {1.5,1.5} | 500.0f px/s |

> **注意**：SpearGoblin 的「突刺」不是射出的 Bullet，而是在敵人前方展開的靜態碰撞盒，脫離有效範圍後自動消滅。
