# progress.md — 開發進度追蹤
> Soul Knight (OOP 2025 期末專案)
> 版本: 1.4 | 最後更新: 2026-03-21

---

## 當前狀態

- **目前里程碑**: M2 進行中
- **下一步**: **M2 Step 2.3 驗收 → Step 2.4 HUD**

---

## Memory Bank 建立狀態

- [x] PRD.md — v1.3（武器系統、能量值、Boss 分段、層數、評分標準均已確認）
- [x] tech-stack.md — v1.1（已整合 files.cmake 格式、App skeleton 狀態）
- [x] architecture.md — v1.4（新增武器類別設計、Boss 分段、能量系統）
- [x] implementation-plan.md — v1.2（各 Step 含完整驗收條件與已確認數值）
- [x] progress.md — 本文件

---

## M1 — 基礎渲染與玩家移動

| Step | 狀態 | 說明 |
|------|------|------|
| 1.1 素材複製腳本 + 目錄建立 | ✅ 完成 | PowerShell 腳本，複製 c01, f101, w001, w004, enemy22, boss08, effect04, ui, weapons；審計 OK |
| 1.2 Tile 類別與房間渲染 | ✅ 完成 | 17×17 測試房間；NorthFaceTile 使用 w004+Z=0.3f+上移偏移方案，SouthFaceTile Y-Sort 公式修正，畫面正常驗收通過 |
| 1.3 玩家類別與 WASD 移動 | ✅ 完成 | 300.0f 速度，Idle/Walk 動畫，左右朝向，初始位置 (-300, -100)，驗收通過 |
| 1.4 相機跟隨系統 | ✅ 完成 | 即時追蹤，4 步驟更新順序（玩家永遠完美置中），驗收通過 |
| 1.5 玩家↔牆壁碰撞 | ✅ 完成 | CollisionSystem，穿透深度 Push Back；HIT_W=44，SouthFaceTile/SouthWallCap Z 修正 |

---

## M2 — 武器與戰鬥系統

| Step | 狀態 | 說明 |
|------|------|------|
| 2.1 敵人基礎類別 | ✅ 完成 | Enemy 基類 + GoblinEnemy + 3 種哥布林（Pistol/Spear/Archer），HP=8，Speed=150；驗收通過 |
| 2.2 武器與子彈系統 | ✅ 完成 | BulletManager 對象池（上限100），手槍為初始武器，J 鍵攻擊；驗收通過 |
| 2.3 碰撞系統擴充 | 🔄 實作完成 | 子彈↔敵人/牆壁，接觸傷害（0.5s CD），3種哥布林攻擊實裝；等待驗收 |
| 2.4 玩家血量/能量 + HUD | ⬜ 待做 | 素材圖片血量條+能量條（最大200），敵人死亡動畫，GameOver 觸發 |

---

## M3 — 房間系統與地城生成

| Step | 狀態 | 說明 |
|------|------|------|
| 3.1 多種房間模板 | ⬜ 待做 | 3–5種模板，最小15×10，最大30×20 |
| 3.2 地城生成演算法 | ⬜ 待做 | 樹狀/網狀，6–9房間，走廊銜接 |
| 3.3 門的生成邏輯 | ⬜ 待做 | 敵人全清才開門，走廊觸發 Spawn |
| 3.4 迷你地圖 | ⬜ 待做 | 右中上角，已探索才顯示 |

---

## M4 — 完整第一大關流程

| Step | 狀態 | 說明 |
|------|------|------|
| 4.1 LevelManager（分層） | ⬜ 待做 | 4 層普通 + 1 層 Boss（共 5 層） |
| 4.2 傳送陣 | ⬜ 待做 | effect04_5-12.png，所有房間清空後出現 |
| 4.3 Boss | ⬜ 待做 | boss08，Phase 1 扇形彈幕 + Phase 2 衝刺 |
| 4.4 Roguelike 重開機制 | ⬜ 待做 | 死亡→GameOver→主選單，完整重置 |

---

## M5 — 主選單、選角、UI 完善

| Step | 狀態 | 說明 |
|------|------|------|
| 5.1 主選單 | ⬜ 待做 | 開始遊戲、退出、設定 |
| 5.2 角色選擇介面 | ⬜ 待做 | c01雙持、c02翻滾、c03法陣 |
| 5.3 角色升級室 | ⬜ 待做 | 選角後進入，依關卡數給資源 |
| 5.4 過關/死亡畫面 | ⬜ 待做 | 顯示層數、擊殺數統計 |

---

## 開發日誌

### 2026-03-27
- ✅ Step 1.5 驗收通過：玩家無法穿牆，沿牆滑行正常，南牆正確遮擋玩家
- 🔧 同日 Bug 修正：HIT_W 24→44（視覺對齊）；SouthFaceTile clamp 50→98；SouthWallCap Z=97.5f
- ➡️ M1 全部完成，下一步：M2 Step 2.1 敵人基礎類別
- 🔄 Step 2.1 實作完成，等待驗收
  - 新增 `include/Entity/Enemy.hpp`：Enemy + GoblinEnemy + PistolGoblin + SpearGoblin + ArcherGoblin
  - 新增 `include/Entity/EnemyManager.hpp`
  - 新增 `src/Entity/Enemy.cpp`：Enemy::Update + GoblinEnemy 建構子 + TryMove 避障
  - 新增 `src/Entity/PistolGoblin.cpp`：保距 AI（MIN_DIST=100, PREF=200）+ 攻擊 stub
  - 新增 `src/Entity/SpearGoblin.cpp`：追蹤 AI + 突刺 stub（stab range=60）
  - 新增 `src/Entity/ArcherGoblin.cpp`：保距 + AIM/COOLDOWN 狀態機 + 射箭 stub
  - 新增 `src/Entity/EnemyManager.cpp`
  - 修改 `Entity/Entity.hpp`：加入 SetWorldPos() public 方法
  - 修改 `App.hpp`：加入 EnemyManager 成員
  - 修改 `App.cpp`：生成 3 隻測試哥布林（PistolGoblin/SpearGoblin/ArcherGoblin）
  - 攻擊相關為 TODO stub，Step 2.3 實作子彈整合
  - 更新順序：Camera 更新後才呼叫 EnemyManager::Update，確保 SyncRender 使用正確相機

### 2026-03-26
- 🔄 Step 1.5 實作完成，等待驗收
  - 新增 `include/System/CollisionSystem.hpp` + `src/System/CollisionSystem.cpp`
  - `ResolveWall(worldPos, hitOffset, hitSize)` — 穿透深度最小軸 Push Back
  - `IsBlocked(center, size)` — 供 M2 敵人 TryMove() 避障使用
  - `App::Start()` 呼叫 `CollisionSystem::SetRoom(&m_Room)`
  - `Player::Update()` 在 HandleInput 後呼叫 `ResolveWall`
  - `files.cmake` 登記新檔案

### 2026-03-25（續）
- ✅ Step 1.2 驗收通過：畫面正常，17×17 房間渲染正確
- ✅ Step 1.3 驗收通過：WASD 移動、Walk/Idle 動畫、左右朝向均正確
- ✅ Step 1.4 驗收通過：相機即時跟隨，玩家完美置中，無延遲抖動

### 2026-03-25
- 🔍 診斷 Step 1.2 錯誤渲染（map_AI.png）：`NorthFaceTile` 誤用 `w004.png`，導致頂部出現黑色橫帶
- ✅ 修正 `src/Tile.cpp`：NorthFaceTile 改用 `w001.png`（與 WallTile 一致；只有 SouthFaceTile 用 w004）
- ✅ 澄清並記錄 Q1~Q9 答案，確認：17×17 房間、走廊 8 格、m_WorldPos.y 為 Y-Sort 唯一標準
- ✅ 修正 Memory Bank 所有已知錯誤（6 處房間尺寸錯誤、1 處 Y-Sort Bug、2 處 files.cmake 格式錯誤、1 處走廊寬度錯誤）
- ✅ 發現 PTSD 根本性 Z-Index Bug：nearClip=-100, farClip=100，Z>100 被 GPU 裁剪不可見
- ✅ 修正 `SouthFaceTile::UpdateZIndex()` 公式：`1000-worldY clamp[2,198]` → `50-worldY/6 clamp[2,98]`
- ✅ 更新 Memory Bank (tech-stack.md §8, architecture.md §7) Z-Index 全表
- ⬜ 等待重新編譯並驗收 Step 1.2

### 2026-03-21
- ✅ 完成第三輪澄清問答（15 題，涵蓋環境確認、戰鬥細節、地城布局、UI 素材）
- ✅ 射擊方向定案：`m_LastMoveDir`（WASD 輸入方向，Normalize 存入，與 `m_FacingLeft` 獨立）
- ✅ 掉落物數值定案：HealthPickup +3 HP、EnergyPickup +80 Energy
- ✅ 地城布局定案：5×5 網格，初始→2基礎→傳送門（Boss層多一格基礎房間）
- ✅ 走廊規格定案：WallTile 兩側，固定寬度 4 格
- ✅ 每房間敵人數定案：15~25 隻哥布林，分 2~3 波，隨層數增加
- ✅ HUD 素材定案：`UI_15.png`（血條/護盾條/能量條組合）
- ✅ GameOver/Victory 背景定案：`Settlement screen.png`
- ✅ WeaponPickup 地板展示：與 HUD 相同圖示（原尺寸）
- ✅ 精英哥布林機率：依層數線性增加（M3 實作時確認具體值）
- ✅ CMakeLists.txt 確認：需在 Step 1.1 加入 `RESOURCE_DIR` compile_definition
- ✅ App.cpp skeleton 確認：已有佔位程式碼（整合覆蓋方式）
- ✅ files.cmake 確認：三段式格式（INCLUDE_FILES / SRC_FILES / TEST_FILES）
- ✅ 開發策略確認：全力推進 100 分，按里程碑順序

### 2026-03-20
- ✅ 完成第二輪澄清問答（13 題，涵蓋 M1~M4 全範圍）
- ✅ 攻擊鍵正式定案：`Util::Keycode::J`（從「待確認」改為「已定案」）
- ✅ 能量上限確認：`m_MaxEnergy = 200`（非 100）
- ✅ 武器系統完整定案：手槍/散彈/雷射，含素材路徑與能量消耗
- ✅ 玩家初始位置確認：(-300.0f, -100.0f)（M1 測試房間）
- ✅ 層數結構確認：4 層普通 + 1 層 Boss
- ✅ Boss 攻擊分段確認：Phase 1 扇形彈幕 / Phase 2 衝刺（HP≤50%）
- ✅ 評分標準確認：M1+M2=60分、M3+M4=80分、M5=100分
- ✅ 相機策略確認：M1 不做 Lerp，即時追蹤
- ✅ files.cmake 現有格式確認（INCLUDE_FILES / SRC_FILES / TEST_FILES）
- ✅ App skeleton 狀態確認（Start/Update/Draw 已有框架，邏輯待填充）

### 2026-03-19
- ✅ Memory Bank 全面更新（v1.2 PRD / v1.3 architecture / v1.2 implementation-plan / v1.2 progress）
- ✅ 制定三種哥布林小兵攻擊規格：PistolGoblin、SpearGoblin、ArcherGoblin
- ✅ 更新 Enemy 繼承樹（新增 GoblinEnemy 中間層 + 3 子類）
- ✅ 建立 AI 狀態機設計（保距型 / 追蹤型、弓箭哥布林瞄準機制）
- ✅ 將攻擊素材路徑對映整理到 Resources/Bullets/

### 2026-03-14
- ✅ 完成 64 題澄清問題的回答
- ✅ 完成所有 Memory Bank 文件 v1.1 更新
- ✅ 架構已整合所有澄清答案（攻擊設計修正：非自動攻擊，改為按鍵攻擊）
- ✅ 各 Step 補充完整驗收條件與技術規格

---

## 已知決策與問題追蹤

| 問題 | 決策 | 狀態 |
|------|------|------|
| 攻擊鍵 | `Util::Keycode::J` | ✅ 已定案 |
| 射擊方向 | `m_LastMoveDir`（WASD 方向，與 `m_FacingLeft` 獨立） | ✅ 已定案 |
| 掉落物數值 | HealthPickup +3 HP / EnergyPickup +80 Energy | ✅ 已定案 |
| HUD 素材 | `UI_15.png`（三條組合）+數字疊加 | ✅ 已定案 |
| GameOver/Victory 背景 | `Settlement screen.png` | ✅ 已定案 |
| 地城布局 | 5×5 網格，初始→2基礎→傳送門（Boss層3基礎） | ✅ 已定案 |
| 走廊寬度 | 固定 6 格地板 + 兩側各 1 格牆 = 8 格總寬 | ✅ 已定案 |
| 每房間敵人數 | 15~25 隻，2~3 波，隨層數增加 | ✅ 已定案 |
| 精英哥布林機率 | 依層數線性增加（具體值 M3 確認） | ⬜ M3 細化 |
| 能量上限 | `m_MaxEnergy = 200` | ✅ 已定案 |
| 武器種類 | 手槍(0能量)/散彈(10/發)/雷射(2/幀) | ✅ 已定案 |
| 初始武器 | 手槍（進入遊戲時預設持有） | ✅ 已定案 |
| 玩家初始位置 | (-300.0f, -100.0f)（M1 測試房間） | ✅ 已定案 |
| 層數結構 | 4 層普通 + 1 層 Boss | ✅ 已定案 |
| Boss 攻擊模式 | Phase1 扇形彈幕 / Phase2 衝刺 | ✅ 已定案（M4 前可細化） |
| 相機 Lerp | M1 不做，即時追蹤 | ✅ 已定案 |
| 玩家碰撞盒偏移 | HIT_OFFSET_Y = -10.0f | ✅ 先用此值，Step 1.5 後微調 |
| Z-Index Y-Sorting 公式 | `1000 - worldY`，clamp [2, 198] | ✅ 確認 |
| 碰撞採用 AABB | 最簡方案，穿透深度 Push Back | ✅ 確認 |
| NorthFaceTile 素材 | 使用 w001.png（非 w004），與 WallTile 視覺一致；只有 SouthFaceTile 才用 w004 | ✅ 已確認（2026-03-25 修正）|
| PTSD Z-Index 限制 | nearClip=-100, farClip=100，Z 必須在 (-100,100)。舊設計 [2,198]/199/200 全錯，已修正為 clamp[2,98]/99/99.5 | ✅ 根本 Bug（2026-03-25 發現修正）|
| 子彈使用 Object Pool | 上限 100，確認實作 | ⬜ 待實作 |
| Player/GoblinEnemy 持有 BulletManager* | 建構時注入，不透過 Update 參數傳遞 | ✅ 已定案 |
| SpearGoblin 突刺 | 速度 0 靜態子彈，lifetime=0.3f，isPlayer=false | ✅ 已定案 |
| Bullet lifetime 機制 | Spawn() 加 lifetime 參數（預設-1=不限時），BulletManager::Update() 倒數 Deactivate | ✅ 已定案 |
| 武器觸發方式 | 基類 IsKeyPressed(J)+FireRate；GunWeapon 額外 IsKeyDown(J) 可突破射速 | ✅ 已定案 |
| 武器替換行為 | 撿起新武器時舊武器掉落地板，地板同時只有 1 把 | ✅ 已定案 |
| 敵人所有權 | EnemyManager 擁有（unique_ptr），Room 持 raw pointer 參照 | ✅ 已定案 |
| 走廊實作 | 細長 Room（重用 Room 架構），無敵人，IsCleared()=true | ✅ 已定案 |
| 房間切換觸發 | World 每幀比對玩家 WorldPos 與所有 Room AABB | ✅ 已定案 |
| 迷你地圖渲染 | 每個 Room 對應一個 GameObject，SetVisible 控制顯示 | ✅ 已定案 |
| Boss 衝刺方向 | 朝玩家當前位置（不限橫向） | ✅ 已定案 |
| 狀態重置 | 各系統實作 Reset()，GameManager 統一呼叫 | ✅ 已定案 |
| 角色差異設計 | Player 基類 + CharacterC01/C02/C03 子類，覆寫 ActivateSkill() | ✅ 已定案 |
| 升級資源 | 跨局保留貨幣，選角前在升級室消費；每局死亡不歸零 | ✅ 已定案 |
| 地城生成演算法 | 樹狀/網狀，6–9房間，走廊銜接 | ⬜ 待 M3 實作 |
| Tile 效能優化 | M1 先用一般 GameObject；卡頓時 M3 後合併大 Texture | ⬜ 待觀察 |
| 音效 | 需要，PTSD 支援，M4/M5 實作 | ⬜ 待做 |
| Boss 攻擊具體數值 | Phase1 扇形 5 發每 2.5s，Phase2 衝刺觸發 HP≤50% | ⬜ 待 M4 細化 |
