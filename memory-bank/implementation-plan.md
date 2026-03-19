# implementation-plan.md — 實作計畫
> Soul Knight (OOP 2025 期末專案)
> 版本: 1.2 | 最後更新: 2026-03-20（整合 Q&A：玩家初始位置、武器數值、能量系統、層數、Boss 分段）

---

## 開發原則

1. **一步一驗收**：每個 Step 完成後等待人工確認，絕不擴大 Scope
2. **模組化**：每個系統獨立成一對 `.hpp/.cpp` 檔，放在對應子目錄
3. **驗收方式**：在本地 CLion + MinGW 編譯執行後，回報結果（PASS 或貼 Error Log）
4. **進度更新**：每完成一個 Step，更新 `progress.md`

---

## M1 — 基礎渲染與玩家移動

**目標**：能在螢幕上看到一個測試房間，玩家可以用 WASD 移動，相機跟隨，不能穿牆。

---

### Step 1.1 — 素材複製腳本 + 目錄建立

**目的**：建立 `Resources/` 子目錄，並用 PowerShell 腳本將必要素材從 `1.07/Sprite/` 複製過來。

**要複製的素材清單**：
| 目標目錄 | 素材 | 說明 |
|---------|------|------|
| `Resources/Characters/` | `c01_0.png` ~ `c01_8.png` | 角色 1 所有幀 |
| `Resources/Tiles/` | `f101.png` | 第 1 層地板 Tile |
| `Resources/Tiles/` | `w001.png`、`w004.png` | 第 1 層牆壁 Tile（頂部+側邊） |
| `Resources/Enemies/` | `enemy22_0.png` ~ `enemy22_6.png` | M2 用敵人 |
| `Resources/Boss/` | `boss08_0.png` ~ `boss08_7.png` | M4 用 Boss |
| `Resources/Objects/` | `effect04_5.png` ~ `effect04_12.png` | 傳送陣動畫 |
| `Resources/UI/` | `ui_0.png` ~ `ui_20.png` | UI 元件 |
| `Resources/Weapons/` | `weapons_19.png` → `weapon_pistol.png` | 手槍 |
| `Resources/Weapons/` | `weapons_18.png` → `weapon_shotgun.png` | 散彈 |
| `Resources/Weapons/` | `weapons2_78.png` → `weapon_laser.png` | 雷射 |
| `Resources/Bullets/` | `28px-圆形子弹-敌方.png` → `enemy_bullet_pistol.png` | 哥布林子彈 |
| `Resources/Bullets/` | `突刺动图0.png` → `enemy_stab.png` | 突刺圖 |
| `Resources/Bullets/` | `42px-箭矢-敌方.png` → `enemy_arrow.png` | 箭矢 |

**驗收條件**：
- [ ] `Resources/` 下所有子目錄存在
- [ ] 所有上述素材檔案正確到位（含重命名）
- [ ] CMake 建置不報 missing file 錯誤

---

### Step 1.2 — Tile 類別與測試房間渲染

**目的**：在螢幕上看到一個 16 列 × 10 行 的測試房間（地板 + 牆壁外框）。

**新增檔案**：
- `include/World/Tile.hpp`
- `src/World/Tile.cpp`
- `include/World/Room.hpp`
- `src/World/Room.cpp`

**技術規格**：
- `Tile` 繼承 `Util::GameObject`，`SetDrawable` 設定素材
- Tile 大小：scale = `{3, 3}`（16×16px → 48×48px）
- 房間中心對齊世界座標原點 `(0, 0)`
- 外框牆壁（1 格厚）：使用 `w001.png`
- 底部牆壁（南側，有立體感）：額外使用 `w004.png`
- 內部地板：使用 `f101.png`
- 牆壁 Z-Index：頂/左/右牆固定 `0.5f`；底部牆動態 Y-Sorting
- 地板 Z-Index：固定 `0.0f`

**世界座標換算**：
```cpp
const float TILE_SIZE = 48.0f;
float worldX =  col * TILE_SIZE - (COLS * TILE_SIZE) / 2.0f + TILE_SIZE / 2.0f;
float worldY = -(row * TILE_SIZE - (ROWS * TILE_SIZE) / 2.0f + TILE_SIZE / 2.0f);
```

**驗收條件**：
- [ ] 編譯成功，無警告
- [ ] 畫面中央出現 16×10 房間（地板填滿，外圍一圈牆壁）
- [ ] 牆壁視覺上是上方有花紋（w001），南側牆壁有側邊感（w004）

---

### Step 1.3 — 玩家類別與 WASD 移動

**目的**：玩家可以在房間內移動，動畫正確播放，朝向（左/右）正確。

**新增檔案**：
- `include/Entity/Entity.hpp`
- `include/Entity/Player.hpp`
- `src/Entity/Player.cpp`

**技術規格**：
- `Entity` 繼承 `Util::GameObject`，定義 `Update(float dt)` 虛函式
- `Player` 繼承 `Entity`
- **初始位置**：`m_WorldPos = {-300.0f, -100.0f}`（16×10 房間左側地板，視覺上為可行走區域）
- 移動速度：`300.0f` pixel/second
- 斜向移動：需 `glm::normalize()` 歸一化速度向量（零向量不歸一化）
- 動畫：
  - Idle（靜止）：`c01_4.png` ~ `c01_7.png`，間隔 150ms，循環
  - Walk（移動中）：`c01_0.png` ~ `c01_3.png`，間隔 100ms，循環
- 朝向：只有左/右；向右 = `scale.x = +3`，向左 = `scale.x = -3`
- 上下移動時維持上一次的左/右朝向（`m_FacingLeft` 只在有水平輸入時更新）

**驗收條件**：
- [ ] WASD 移動方向正確（W=上，S=下，A=左，D=右）
- [ ] 斜向移動速度與單向移動速度相同（已 Normalize）
- [ ] 移動時播放 Walk 動畫，靜止時播放 Idle 動畫
- [ ] 左右朝向正確翻轉；上下移動不改變朝向

---

### Step 1.4 — 相機跟隨系統

**目的**：玩家移動時，所有場景物件一起偏移，使玩家永遠顯示在畫面中央。

**新增檔案**：
- `include/World/Camera.hpp`
- `src/World/Camera.cpp`

**技術規格**：
- Camera 儲存 `m_Position`（世界偏移量），等於玩家當前 `m_WorldPos`
- **每幀更新順序**：
  1. Camera 更新 `m_Position = player.GetWorldPos()`
  2. 所有 Entity / Tile 呼叫 `SyncRenderTransform(Camera::GetPosition())`
  3. `m_Transform.translation = m_WorldPos - cameraPos`
- **M1 策略**：Instant Follow（即時追蹤，不做 Lerp），玩家永遠置中
- **M1 策略**：不限制相機邊界（超出房間可以看到黑色背景，可接受）
- `Camera::GetPosition()` 提供全域存取（static 或透過 App 傳遞）

**驗收條件**：
- [ ] 玩家移動時，房間跟著移動（玩家始終在畫面中央）
- [ ] 相機無延遲、無抖動

---

### Step 1.5 — 玩家 ↔ 牆壁碰撞

**目的**：玩家無法穿越牆壁 Tile。

**新增/修改檔案**：
- `include/System/CollisionSystem.hpp`
- `src/System/CollisionSystem.cpp`
- 修改 `Player::Update()` 呼叫碰撞檢測

**技術規格**：
- 玩家碰撞盒（AABB）：
  - 寬 `24px`，高 `20px`
  - 中心點偏移：`HIT_OFFSET_Y = -10.0f`（中心向下偏移 10px，覆蓋腳部）
  - 實際碰撞中心 = `m_WorldPos + {0, HIT_OFFSET_Y}`
- 碰撞解決：**穿透深度（Penetration Depth）Push Back**
  - 計算 X / Y 各維度的 overlap
  - 只對穿透最淺的軸進行最小反向位移
  - **禁止** 暴力歸位（`worldPos = previousPos`）
- `CollisionSystem::IsBlocked(pos, size)` — 回傳 bool，供 EnemyAI 避障使用

**驗收條件**：
- [ ] 玩家碰到任何 WallTile 都無法穿越
- [ ] 玩家在兩牆夾角時不會閃爍或卡死（穿透深度分軸 Push Back 驗證）
- [ ] 玩家沿牆壁滑動行走正常

---

**M1 整體驗收條件**：
- [ ] 畫面中央有一個 16×10 房間（地板+牆壁）
- [ ] WASD 移動，動畫正確（Walk/Idle），朝向正確
- [ ] 相機跟隨玩家（即時，無 Lerp）
- [ ] 玩家無法穿越牆壁，不卡角

---

## M2 — 武器與戰鬥系統

**目標**：玩家可以按攻擊鍵向面向方向射擊，敵人有 AI 追蹤，雙方有血量判定。

---

### Step 2.1 — 敵人基礎類別

**新增檔案**：
- `include/Entity/Enemy.hpp`（Enemy 基類 + GoblinEnemy + 三個子類宣告）
- `src/Entity/Enemy.cpp`
- `src/Entity/PistolGoblin.cpp`
- `src/Entity/SpearGoblin.cpp`
- `src/Entity/ArcherGoblin.cpp`

**技術規格 — Enemy 基類**：
- `Enemy` 繼承 `Entity`，新增 `SetElite(bool)` 與純虛函式 `UpdateAI(float dt)`
- 測試敵人：`enemy22_N.png`，7 幀，interval 150ms，looping=true
- 血量：8，速度：150.0f pixel/second
- `EnemyManager`：持有並管理所有 `Enemy` 實體，提供 `Update()` 統一更新

**技術規格 — 三種 Goblin 子類**：

| 類別 | 移動 AI | 攻擊方式 | 攻擊力（普/精） | 冷卻 |
|------|---------|---------|---------|------|
| `PistolGoblin` | 保持中遠程距離，玩家靠近則後退 | 射 1 枚（普通）/ Burst 3 枚（精英） | 2 / 3 | 2.0s；精英 Burst 間隔 0.15s |
| `SpearGoblin` | 直線追蹤靠近玩家 | AABB 重疊時突刺（靜態碰撞盒展開） | 3 / 3（盒×1.5） | 1.0s |
| `ArcherGoblin` | 保持距離；瞄準期間停止 | 瞄準 1.0s 後射箭 | 4 / 5（箭×1.5） | 瞄準1.0s→射出→冷卻2.5s |

**敵人 AI 避障（TryMove 機制）**：
```cpp
void GoblinEnemy::TryMove(glm::vec2 wishDir, float speed, float dt) {
    glm::vec2 nextPos = m_WorldPos + wishDir * speed * dt;
    if (!CollisionSystem::IsBlocked(nextPos, m_HitboxSize)) {
        m_WorldPos = nextPos; return;
    }
    // 嘗試只走 X 軸
    glm::vec2 xOnly = m_WorldPos + glm::vec2(wishDir.x, 0) * speed * dt;
    if (!CollisionSystem::IsBlocked(xOnly, m_HitboxSize)) { m_WorldPos = xOnly; return; }
    // 嘗試只走 Y 軸
    glm::vec2 yOnly = m_WorldPos + glm::vec2(0, wishDir.y) * speed * dt;
    if (!CollisionSystem::IsBlocked(yOnly, m_HitboxSize)) { m_WorldPos = yOnly; return; }
    // 兩軸都碰牆，停止移動
}
```

**驗收條件**：
- [ ] 螢幕上各生成 1 隻哥布林（或 enemy22），動畫播放正確
- [ ] PistolGoblin：與玩家保持距離，會後退
- [ ] SpearGoblin：直線追蹤玩家
- [ ] ArcherGoblin：保持距離，瞄準時停止不動
- [ ] 敵人靠近牆壁時不卡住、不抖動

---

### Step 2.2 — 武器與子彈系統

**新增檔案**：
- `include/Weapon/Weapon.hpp`
- `include/Weapon/Bullet.hpp`
- `include/Weapon/BulletManager.hpp`
- `src/Weapon/Bullet.cpp`
- `src/Weapon/BulletManager.cpp`
- `src/Weapon/GunWeapon.cpp`（手槍實作，初始武器）

**技術規格**：
- `BulletManager`：對象池，上限 100 顆
  - `Spawn(worldPos, direction, speed, damage, isPlayer)`
  - Spawn 時呼叫 `SetVisible(true)`；Deactivate 時呼叫 `SetVisible(false)`（防幽靈子彈）
- `Bullet`：繼承 `Util::GameObject`，固定 **Z-Index = 199.0f**
- 發射原點：從玩家中心往面向方向偏移 **20px**（防止子彈在玩家身體內生成）
- 子彈 Hitbox：統一使用 **10×10px** 核心小判定盒（與圖片尺寸無關）
- 攻擊鍵：**`Util::Keycode::J`**（已定案）

**三種武器規格**：
| 武器 | 類別 | 速度 | 能量/次 | 攻擊力 | 射擊邏輯 |
|------|------|------|---------|--------|---------|
| 手槍 | `GunWeapon` | 700.0f | 0 | 10 | IsKeyDown(J)，每次 1 發 |
| 散彈 | `ShotgunWeapon` | 600.0f | 10 | 8 | IsKeyDown(J)，同時 3 發（扇形±15°） |
| 雷射 | `LaserWeapon` | 持續型 | 2/幀 | 5/幀 | IsKeyPressed(J)，每幀持續 |

**驗收條件**：
- [ ] 玩家按 J 鍵（手槍），子彈從玩家位置向面向方向飛出
- [ ] 子彈飛出畫面或碰牆後消失（`SetVisible(false)`，無幽靈殘影）
- [ ] 子彈 Z-Index = 199.0f，永遠在玩家/敵人之上

---

### Step 2.3 — 碰撞系統擴充

**修改檔案**：`CollisionSystem.cpp`、`BulletManager.cpp`

**新增碰撞對**：
| 碰撞對 | 結果 |
|--------|------|
| 玩家子彈 ↔ 敵人 AABB | 敵人 TakeDamage()，子彈 Deactivate |
| 敵人子彈 ↔ 玩家 AABB | 玩家 TakeDamage()，子彈 Deactivate |
| 任何子彈 ↔ 牆壁 AABB | 子彈 Deactivate |
| 敵人（近戰）↔ 玩家 AABB | 每次觸發扣血，0.5 秒 Cooldown |

**驗收條件**：
- [ ] 子彈打中敵人 → 敵人扣血（HP: 8 → 逐漸減少至 0）
- [ ] 敵人碰觸玩家 → 玩家每 0.5s 扣一次血
- [ ] 子彈碰牆消失，無殘影
- [ ] 玩家子彈不傷害玩家，敵人子彈不傷害敵人

---

### Step 2.4 — 玩家血量/能量 + HUD

**新增檔案**：`include/UI/HUD.hpp`、`src/UI/HUD.cpp`

**技術規格**：
- 血量條：UI 素材圖片（`ui_N.png`），HUD Z-Index = 200.0f
- 能量條：UI 素材圖片，最大值 200，與武器消耗聯動
- 敵人死亡：播放死亡動畫（`looping=false`），`State::ENDED` 後從 Renderer 移除
- 玩家血量 ≤ 0 → 切換到 GameOver 狀態（直接切換）

**驗收條件**：
- [ ] HUD 血量條正確顯示並隨受傷減少
- [ ] HUD 能量條正確顯示並隨射擊減少（手槍不消耗，散彈每發 -10）
- [ ] 敵人有死亡動畫播放，動畫結束後從畫面消失
- [ ] 玩家血量歸零後進入 GameOver（直接切換畫面）

---

**M2 整體驗收條件**：
- [ ] 有三種哥布林敵人追蹤玩家（各有不同行為）
- [ ] 玩家按 J 鍵發射子彈（面向方向，手槍為預設）
- [ ] 子彈可擊殺敵人（敵人有死亡動畫）
- [ ] 敵人碰觸玩家扣血（0.5s Cooldown）
- [ ] HUD 血量條/能量條顯示正確

---

## M3 — 房間系統與地城生成

**目標**：多個房間以走廊串連，門的邏輯正確，可在房間間移動，迷你地圖正確。

---

### Step 3.1 — 多種房間模板

**技術規格**：
- 設計 3–5 種不同大小的房間模板（以 Tile 陣列 / Template ID 定義）
- 大小範圍：最小 15×10，最大 30×20
- `Room::Generate(int width, int height, int templateId)` 根據模板生成 TileMap

**驗收條件**：
- [ ] 可隨機選取不同模板生成房間，大小和佈局各不相同
- [ ] 每種模板都能正常渲染，無 Tile 錯位

---

### Step 3.2 — 地城生成演算法

**技術規格**：
- `DungeonGenerator::Generate(seed, numRooms)` 生成 6–9 個普通房間 + 1 個 Boss 房間
- 結構：樹狀/網狀分支，允許玩家選擇不同路徑
- 相鄰房間以**走廊**銜接（走廊寬度 2~3 格，純地板 Tile 構成）
- 確保所有房間連通（起始房間可達所有房間）

**驗收條件**：
- [ ] 每次生成的地城結構不同（seed 不同時）
- [ ] 所有房間可達（從起始房間能走到所有房間）
- [ ] 走廊視覺正確（地板 Tile，無牆壁阻擋）

---

### Step 3.3 — 門的生成邏輯

**技術規格**：
- `DoorTile`：可切換貼圖（關閉 vs 開啟）
- 有走廊銜接的方向才生成門；無走廊維持牆壁
- 門初始狀態：**關閉（阻擋移動）**
- 房間內所有敵人死亡 → `Room::IsCleared()=true` → 所有門切換為開啟（移除碰撞）
- 玩家走入走廊 → 進入相鄰房間 → 觸發該房間的敵人 Spawn

**驗收條件**：
- [ ] 進房間後門關閉，玩家無法通過
- [ ] 打完所有敵人後門自動開啟
- [ ] 走入走廊後可抵達相鄰房間，觸發敵人生成

---

### Step 3.4 — 迷你地圖

**技術規格**：
- 位置：畫面右中上角（固定 HUD 位置）
- 樣式：簡單方塊圖示（每個房間用一個小矩形）
- 進入房間後才在迷你地圖上顯示（`m_Visited` 旗標）
- 當前房間用不同顏色標示

**驗收條件**：
- [ ] 迷你地圖隨玩家移動更新
- [ ] 未探索房間不顯示，已探索房間顯示
- [ ] 當前房間有明顯視覺區分（顏色不同）

---

## M4 — 完整第一大關流程

**目標**：完整的多層 Roguelike 流程（含 Boss）。

---

### Step 4.1 — LevelManager（分層結構）

- `LevelManager`：記錄當前層數（從 1 開始），管理層間切換
- **第 1~4 層**：普通層（隨機地城，6–9 房間）
- **第 5 層**：Boss 層（只有 Boss 房間，無其他敵人）
- 每層重新生成地城（新 seed，不保留前一層狀態）

**驗收條件**：
- [ ] 層數計數正確（1 → 4 普通，第 5 層為 Boss）
- [ ] 每層地城獨立生成（結構不同）
- [ ] 層切換時舊 Room 物件正確銷毀（無 Memory Leak）

---

### Step 4.2 — 傳送陣

- 素材：`effect04_5.png` ~ `effect04_12.png`（8 幀循環動畫）
- 出現時機：當前層所有 `Room::IsCleared()=true`
- 位置：當前房間中央（FloorTile 中心）
- 碰撞：玩家踩上傳送陣 → 進入下一層

**驗收條件**：
- [ ] 清完所有房間後傳送陣出現（動畫播放）
- [ ] 踩上傳送陣後進入下一層（地城重新生成）

---

### Step 4.3 — Boss

- 素材：`boss08_0.png` ~ `boss08_7.png`（8 幀動畫）
- Boss 房間：只有 Boss，無出口（打完 Boss 才開門）
- **Phase 1（HP > 50%）**：站在中央，發射扇形子彈（5 發，每 2.5s 一輪）
- **Phase 2（HP ≤ 50%）**：在 Phase 1 基礎上，增加橫向快速衝刺（衝刺後短暫停頓）
- Boss 死亡 → 觸發 Victory 畫面

**驗收條件**：
- [ ] Boss 有兩段攻擊模式（HP > 50% 和 HP ≤ 50% 行為不同）
- [ ] 玩家打敗 Boss 後觸發 Victory 畫面

---

### Step 4.4 — Roguelike 重開機制

- 玩家死亡 → 直接切換 GameOver 畫面
- 玩家確認 → 回到主選單
- 所有遊戲狀態完整重置（不保留任何進度）

**驗收條件**：
- [ ] 玩家死亡後進入 GameOver 畫面（顯示存活層數、擊殺數）
- [ ] 重開後所有狀態歸零

---

## M5 — 主選單、選角、UI 完善

**目標**：遊戲有完整的主選單、選角、升級流程。

### Step 5.1 — 主選單
- 按鈕：開始遊戲、退出、設定

### Step 5.2 — 角色選擇介面
- 3 個可選角色（c01、c02、c03）
- 各角色展示動畫 + 特性說明
  | 角色 | 技能 |
  |------|------|
  | c01 | 雙持武器 |
  | c02 | 翻滾 |
  | c03 | 法陣（陣內回血） |

### Step 5.3 — 角色升級室
- 選角後進入升級室，可花費「關卡資源」升級角色
- 可升級屬性：血量上限、速度、能量上限（待 M5 前確認具體設計）

### Step 5.4 — 過關/死亡畫面
- Victory：顯示通過層數、擊殺數
- GameOver：顯示存活層數、擊殺數
- 兩者皆有「返回主選單」按鈕

---

## 開發注意事項

1. **每個 Step 完成後必須等待驗收**，不跳躍實作
2. **攻擊設計**：按鍵攻擊（`Util::Keycode::J`），向面向方向發射，非自動攻擊
3. **能量系統**：手槍 0/散彈 10/雷射 2幀，最大值 200
4. **素材優先使用**：`c01`（角色1），`enemy22`（敵人），`f101`（地板），`w001`/`w004`（牆壁）
5. **Z-Index 公式**：動態 Y-Sorting = `1000 - worldY`，clamp `[2.0f, 198.0f]`，子彈固定 `199.0f`
6. **禁止擴大 Scope**：每次只做一個 Step
7. **files.cmake**：每新增 `.hpp`/`.cpp` 必須立即登記，否則 CMake 不會編譯
