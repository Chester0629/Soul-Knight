# Soul Knight — OOP 2025 期末專案

> 使用 PTSD 引擎（C++17）復刻 Soul Knight 手遊的 Roguelike 地城探索遊戲

---

## 專案概覽

| 項目 | 說明 |
|------|------|
| 課程 | OOP 2025 期末專案 (OOP2025f_HW0) |
| 引擎 | [PTSD](https://github.com/ntut-open-source-club/practical-tools-for-simple-design)（SDL2 + OpenGL + glm） |
| 語言 | C++17 |
| 建置 | CMake + MinGW（CLion） |
| 平台 | Windows |
| 解析度 | 1280 × 720 @ 60 fps |

---

## 遊戲說明

- **類型**：俯視角 Roguelike 地城探索
- **進程**：通過 4 層普通層 + 1 層 Boss 層
- **玩家**：WASD 移動，`J` 鍵射擊，向面向方向發射子彈
- **特色**：隨機地城生成、三種哥布林小兵 AI、武器/能量系統、Y-Sorting 2.5D 渲染

---

## 建置方式

> **警告**：請使用 `Debug` 模式建置，`Release` 路徑尚未完整設定。

```bash
# 1. Clone（含 PTSD 子模組）
git clone https://github.com/Chester0629/Soul-Knight.git
cd Soul-Knight

# 2. CMake 設定（Debug 模式）
cmake -DCMAKE_BUILD_TYPE=Debug -B build

# 3. 編譯
cmake --build build
```

---

## 操作說明

| 操作 | 按鍵 |
|------|------|
| 移動 | `W` `A` `S` `D` |
| 射擊 | `J`（向面向方向發射） |
| 斜向移動 | 同時按兩個方向鍵（速度自動歸一化） |

---

## 武器系統

| 武器 | 能量消耗 | 說明 |
|------|---------|------|
| 手槍 | 0 / 發 | 初始武器，按一次 J 射一發 |
| 散彈 | 10 / 發 | 同時射出 3 顆扇形子彈 |
| 雷射 | 2 / 幀 | 按住 J 持續射出 |

> 能量最大值 200，能量球（EnergyPickup）為唯一回復來源

---

## 敵人 AI

| 敵人 | 行為 |
|------|------|
| PistolGoblin（手槍哥布林） | 與玩家保持中遠程距離，射圓形子彈 |
| SpearGoblin（長槍哥布林） | 直線追蹤玩家，近距離突刺 |
| ArcherGoblin（弓箭哥布林） | 保持距離，瞄準時停止，射出箭矢 |
| Boss | Phase 1：扇形彈幕；Phase 2（HP≤50%）：加入橫向衝刺 |

---

## 里程碑進度

| 里程碑 | 目標 | 狀態 |
|--------|------|------|
| **M1** 基礎系統 | 房間渲染、WASD 移動、相機跟隨、碰撞 | ⬜ 進行中 |
| **M2** 戰鬥核心 | 敵人 AI、武器子彈、血量 HUD | ⬜ 待做 |
| **M3** 地城系統 | 多房間地城生成、門邏輯、迷你地圖 | ⬜ 待做 |
| **M4** 完整關卡 | 多層結構、傳送陣、Boss、Roguelike 重開 | ⬜ 待做 |
| **M5** 主選單 | 主選單、角色選擇、升級室、結局畫面 | ⬜ 待做 |

詳細計畫請參閱 [`memory-bank/implementation-plan.md`](memory-bank/implementation-plan.md)

---

## 專案結構

```
Soul-Knight/
├── CMakeLists.txt          # 主建置檔
├── files.cmake             # 原始碼清單（新增檔案必須登記此處）
├── include/                # 標頭檔（.hpp）
│   ├── Core/               # GameManager, SceneManager, LevelManager
│   ├── World/              # Tile, Room, DungeonGenerator, Camera
│   ├── Entity/             # Entity, Player, Enemy, Boss
│   ├── Weapon/             # Weapon, Bullet, BulletManager
│   ├── Pickup/             # HealthPickup, EnergyPickup, WeaponPickup
│   ├── System/             # CollisionSystem, EnemyAI
│   └── UI/                 # HUD, Minimap
├── src/                    # 實作檔（.cpp）
├── Resources/              # 遊戲素材（PNG）
├── PTSD/                   # 引擎框架（不修改）
└── memory-bank/            # 開發記憶庫
    ├── PRD.md              # 產品需求文件
    ├── tech-stack.md       # 技術棧說明
    ├── architecture.md     # 架構設計
    ├── implementation-plan.md  # 實作計畫
    └── progress.md         # 開發進度
```

---

## 技術架構重點

- **座標系統**：PTSD 使用螢幕中心為原點，Y 軸向上為正（與螢幕座標相反）
- **世界座標與渲染座標分離**：`m_WorldPos` 用於物理/碰撞，`m_Transform.translation` 僅供渲染
- **Y-Sorting**：`Z = clamp(1000 - worldY, 2, 198)`，實現 2.5D 立體感
- **子彈對象池**：上限 100 顆，避免頻繁 new/delete
- **AABB 碰撞**：穿透深度 Push Back，防止牆角卡頓

---

## 開發規範

- `.hpp`：純宣告，`#pragma once`
- `.cpp`：純實作
- 所有註解使用**繁體中文**
- 新增 `.cpp`/`.hpp` 必須同步登記至 `files.cmake`
- 路徑字串統一使用正斜線 `/`（CMake + MinGW 相容）
