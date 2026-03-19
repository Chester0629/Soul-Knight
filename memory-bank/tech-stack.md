# tech-stack.md — 技術棧文件
> Soul Knight (OOP 2025 期末專案)
> 版本: 1.1 | 最後更新: 2026-03-14（已整合澄清問題答案）

---

## 一、核心框架

### 引擎：PTSD (Pretty Terrible SDL)
- 基底：**SDL2 + OpenGL + glm**
- 路徑：`d:\Soul Knight\project\PTSD\`
- 狀態：已存在，可正常使用

### 語言 & 標準
- **C++17**
- **CMake** 建置系統（`add_subdirectory` 組織子目錄）
- IDE：**CLion**，編譯器：**MinGW**
- 格式：`.hpp` 純宣告，`.cpp` 純實作，全部使用 `#pragma once`，繁體中文註解

### 記憶體管理規範（C++17）

> [!IMPORTANT]
> **物件生命週期管理強制規則（防止 Memory Leak）**
>
> 1. **優先使用 Smart Pointer**：能用 `std::unique_ptr` / `std::shared_ptr` 時，不得使用 Raw Pointer。
> 2. **例外情形 — Raw Pointer 允許條件**：PTSD 引擎 API 接受 `Util::GameObject*` 等場合，或 `Room::m_TileMap` 等明確有 Owner 的容器。
> 3. **Raw Pointer 強制要求**：凡使用 Raw Pointer 管理的集合（如 `vector<Tile*>`、`vector<Enemy*>`），**必須在其 Owner 的 Destructor 中執行 `delete`**，並在 `clear()` 集合前逐一釋放。
> 4. **層切換風險**：`DungeonGenerator` 每次生成新層時，舊 `Room` 物件必須先呼叫 Destructor（或手動 `delete`），否則每次進入下一層就會 **Leak 整個 TileMap + Enemy 陣列**。

```cpp
// ✅ 正確範例：Raw Pointer Owner 的 Destructor
Room::~Room() {
    for (auto& row : m_TileMap)
        for (auto* tile : row)
            delete tile;
    for (auto* e : m_Enemies) delete e;
}

// ✅ 正確範例：改用 unique_ptr（推薦）
std::vector<std::vector<std::unique_ptr<Tile>>> m_TileMap;
// → 離開 scope 自動釋放，無需手動 delete
```


---

## 二、PTSD 主迴圈架構（已確認）

```
main.cpp
└── Core::Context（SDL 視窗 + OpenGL 環境）
    └── App（狀態機：START → UPDATE → END）
        ├── App::Start()   — 只執行一次，負責初始化（載入素材、生成場景）
        ├── App::Update()  — 每幀呼叫（輸入/邏輯/碰撞/渲染）
        └── App::End()     — 離開前的資源釋放
```

**主迴圈運行模式（main.cpp 原始 skeleton）**：
- `while (!context->GetExit())` 驅動整個遊戲
- `context->Update()` 每幀呼叫一次（交換緩衝區、更新 SDL 事件）
- 我們不修改 `main.cpp`，所有遊戲邏輯從 `App::Start()` / `App::Update()` 開始擴充

---

## 三、PTSD 核心 API 速查

> [!WARNING]
> **⚠️ PTSD API 使用規則：遇到不確定的 API，必須先讀標頭檔！**
>
> 下方速查表請勿手払般查請求。遇到以下情況時，**一定要先開 `d:\Soul Knight\project\PTSD\include\` 下對應的 `.hpp` 檔確認函式簽名，絕對不可自行捨造**：
> - 想得到視窗大小（寬 / 高）
> - 對應某個按鍵的第 N 個 Keycode
> - 利用引擎起動計時器或其他实用工具
> - 任何速查表上沒列出的功能

### 視窗設定
```cpp
// config.hpp（或 pch.hpp 中）
#define WINDOW_WIDTH  1280
#define WINDOW_HEIGHT 720
#define FPS           60
```

### GameObject（基礎物件）
```cpp
// PTSD/include/Util/GameObject.hpp
class Util::GameObject {
public:
    void SetDrawable(std::shared_ptr<Util::IDrawable> drawable);
    void SetZIndex(float zIndex);
    void SetVisible(bool visible);
    Util::Transform m_Transform;  // 位置、旋轉、縮放
};
```

### Transform（座標系統）
```cpp
// ⚠️ PTSD 使用螢幕中心為原點，Y 軸向上為正（與一般螢幕坐標相反）
struct Util::Transform {
    glm::vec2 translation = {0, 0};  // 世界座標（螢幕中心為 0,0）
    float     rotation    = 0;        // 弧度
    glm::vec2 scale       = {1, 1};  // 縮放倍數（Tile 用 {3,3} 放大至 48px）
};
```

### Image（靜態圖片）
```cpp
auto image = std::make_shared<Util::Image>("Resources/path/to/image.png");
gameObject->SetDrawable(image);
```

### Animation（動畫）
```cpp
// ⚠️ 真實簽名：(paths, play, interval, looping, cooldown)
auto anim = std::make_shared<Util::Animation>(
    std::vector<std::string>{"frame0.png", "frame1.png", ...},
    true,            // play：是否立刻開始播放
    150,             // interval：幀間隔（ms），型別 std::size_t
    true,            // looping：true=循環，false=播完停在最後一幀（死亡動畫）
    100              // cooldown：動畫重播冷卻（ms），預設值即可
);
gameObject->SetDrawable(anim);

// 判斷死亡動畫是否播完：
if (anim->GetState() == Util::Animation::State::ENDED) { /* 移除物件 */ }
```

### Sprite Flipping（左右翻轉）
```cpp
// 向右：m_Transform.scale.x = +3（等於 +1，視目前縮放比）
// 向左：m_Transform.scale.x = -3（負數 = 水平翻轉）
gameObject->m_Transform.scale.x = facing_left ? -3.0f : 3.0f;
```

### Renderer（全域渲染器）
```cpp
// ✅ AddChild 只需呼叫一次，Renderer 每幀自動讀取 Transform 重繪
Util::Renderer renderer;
renderer.AddChild(gameObjectPtr);
renderer.Update();  // 每幀呼叫，依 z-index 排序後渲染
```

### Input（輸入）
```cpp
Util::Input::IsKeyPressed(Util::Keycode::W);     // 持續按住（適合移動）
Util::Input::IsKeyDown(Util::Keycode::SPACE);    // 單次觸發（按下瞬間，適合技能/確認鍵）
Util::Input::IsKeyUp(Util::Keycode::ESCAPE);     // 放開瞬間
Util::Input::GetCursorPosition();                // 滑鼠位置（螢幕座標）
```

### Time（Delta Time）
```cpp
// ✅ 正確用法（GetDeltaTime() 已棄用）
float dtMs = Util::Time::GetDeltaTimeMs();  // 毫秒（例如 16.67ms @60fps）
float dt   = dtMs / 1000.0f;               // 換算成秒，再乘以速度

// 移動範例：
// ✅ 正確：移動修改「世界座標」，不得直接修改 translation
m_WorldPos += m_Velocity * dt;  // velocity 單位：pixel/second
// ❌ 錯誤：m_Transform.translation += m_Velocity * dt;  // ← 會污染座標，嚴禁！
```

### Renderer（渲染根節點）
```cpp
// 慣例：App 中宣告 Util::Renderer m_Root;
// AddChild 只需呼叫一次；Update() 每幀必須呼叫
m_Root.AddChild(someGameObjectPtr);  // shared_ptr<GameObject>
m_Root.Update();                     // 依 z-index 排序後渲染所有子物件
m_Root.RemoveChild(somePtr);         // 移除（死亡動畫結束後使用）
```

---

## 四、座標系統說明

> [!CAUTION]
> **⚠️ AI 實作警告：Y 軸方向陷阱**
> PTSD 世界座標的 Y 軸**向上為正**，但 TileMap 的 row 索引**向下增加**。
> 兩者是**鏡像關係**，必須套用以下轉換：
> ```
> worldY = -(row * TILE_SIZE - (mapRows * TILE_SIZE) / 2.0f + TILE_SIZE / 2.0f)
> ```
> **在所有涉及 Tile 擺放與 Entity 移動的邏輯中，嚴格遵守 `worldY = -tileY` 的鏡像轉換。**
> 若遺漏負號，M1.2 渲染出的房間將上下顛倒，Entity 移動方向也會相反。

```
PTSD 世界座標（螢幕中心為原點，Y 向上為正）：
        +Y (up)
         |
-X -----[0,0]----- +X
         |
        -Y (down)

世界座標 ↔ 螢幕座標 換算：
  screenX = worldX + WINDOW_WIDTH  / 2   →  (0,0) 對應螢幕中心
  screenY = -worldY + WINDOW_HEIGHT / 2   →  Y 軸方向相反

TileMap 世界座標換算（房間中心對齊世界原點）：
  TILE_SIZE = 48（16px 原圖 × 3 倍 scale）
  worldX =  col * TILE_SIZE - (mapCols * TILE_SIZE) / 2.0f + TILE_SIZE / 2.0f
  worldY = -(row * TILE_SIZE - (mapRows * TILE_SIZE) / 2.0f + TILE_SIZE / 2.0f)  ← ⚠️ 負號不可省略！
```

---

## 五、專案目錄結構

```
d:\Soul Knight\project\
├── CMakeLists.txt              ← 主建置檔（已存在，使用 add_subdirectory）
├── include/                    ← 遊戲自訂 Header（純宣告）
│   ├── App.hpp                 ← 已存在（skeleton）
│   ├── Core/                   ← GameManager, SceneManager, LevelManager
│   ├── World/                  ← Tile, Room, DungeonGenerator, Camera
│   ├── Entity/                 ← Entity, Player, Enemy, Boss
│   ├── Weapon/                 ← Weapon, Bullet, BulletManager
│   ├── Pickup/                 ← WeaponPickup, HealthPickup
│   ├── System/                 ← CollisionSystem, EnemyAI
│   └── UI/                     ← HUD, Minimap
├── src/                        ← 遊戲實作 Source（純實作）
│   ├── App.cpp                 ← 已存在（skeleton）
│   ├── main.cpp                ← 已存在（不修改）
│   ├── Core/
│   ├── World/
│   ├── Entity/
│   ├── Weapon/
│   ├── Pickup/
│   ├── System/
│   └── UI/
├── Resources/                  ← 遊戲素材（Step 1.1 腳本複製）
│   ├── Characters/             ← c01_N.png ~ c03_N.png
│   ├── Tiles/                  ← f101.png, w001.png 等
│   ├── Enemies/                ← enemy22_N.png 等
│   ├── Bullets/                ← bullet_N.png
│   ├── Boss/                   ← boss08_N.png
│   ├── Objects/                ← effect04_N.png（傳送陣）
│   └── UI/                     ← ui_N.png（血量横條等）
├── memory-bank/                ← 記憶庫（本目錄）
├── PTSD/                       ← 引擎框架（不修改）
└── 1.07/                       ← 素材來源（Soul Knight 1.07 解包）
    ├── Sprite/                 ← 2896 個 PNG
    └── Texture2D/              ← 144 個 PNG
```

---

## 六、素材資源速查（本專案使用）

### 角色 (M1 Hardcode: c01)
| 素材命名 | 說明 |
|---------|------|
| `c01_0.png` ~ `c01_3.png` | 行走動畫幀（方向待確認） |
| `c01_4.png` ~ `c01_7.png` | 靜止循環動畫（Idle） |
| `c01_8.png` | 其他幀 |

### 敵人 (M2 基準: enemy22)
| 素材命名 | 說明 |
|---------|------|
| `enemy22_0.png` ~ `enemy22_6.png` | 7 幀動畫 |

### Boss (M4: boss08)
| 素材命名 | 說明 |
|---------|------|
| `boss08_0.png` ~ `boss08_7.png` | 8 幀動畫 |

### 地形
| 素材命名 | 說明 |
|---------|------|
| `f101.png` | 第 1 層地板 Tile（16×16px） |
| `w001.png` | 第 1 層牆壁 Tile（頂部/通用） |
| `w004.png` | 第 1 層牆壁 Tile（側邊） |

### 特效
| 素材命名 | 說明 |
|---------|------|
| `effect04_5.png` ~ `effect04_12.png` | 傳送陣動畫（8 幀） |

### UI
| 素材命名 | 說明 |
|---------|------|
| `ui_0.png` ~ `ui_117.png` | 血量條、能量條等 UI 元件 |

---

## 七、建置指令

```bash
# 在 project/ 目錄下（CLion 通常自動執行以下步驟）
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

> [!IMPORTANT]
> **⚠️ CMakeLists.txt 關鍵設定說明（防止 AI 亂猜建置語法）**
>
> **PTSD 引擎單元名稱為 `PTSD`（大寫）**
> ```cmake
> # 引擎造傾（已存在於項目）—— AI 不得自行修改
> FetchContent_Declare(ptsd
>     URL        https://github.com/ntut-open-source-club/practical-tools-for-simple-design/archive/refs/tags/v0.2.zip
>     SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/PTSD
> )
> FetchContent_MakeAvailable(ptsd)
>
> # 自訂檔案列表：新增 .cpp / .hpp 必須登記在 files.cmake
> include(files.cmake)
> # files.cmake 格式：
> # set(SRC_FILES World/Room.cpp World/Camera.cpp Entity/Player.cpp ...)
> # set(INCLUDE_FILES World/Room.hpp World/Camera.hpp Entity/Player.hpp ...)
>
> # 連結引擎（目標名必須是 PTSD，大寫）
> target_link_libraries(${PROJECT_NAME}
>     SDL2::SDL2main
>     PTSD              ←  大寫 PTSD，不是 ptsd 小寫！
> )
>
> # 資源路徑註入（Debug 編譯必須加此 flag）
> target_compile_definitions(${PROJECT_NAME} PRIVATE
>     RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Resources"
> )
> # 在 C++ 中使用：
> // auto img = std::make_shared<Util::Image>(RESOURCE_DIR "/Tiles/f101.png");
> ```
>
> **新增一個 .cpp 檔案的完整流程：**
> 1. 在 `include/` 建立 `.hpp`
> 2. 在 `src/` 建立 `.cpp`
> 3. **在 `files.cmake` 登記這兩個檔名**（這步很常遭忘記！）
> 4. 重新 CMake Configure 就可編譯

---

## 八、Z-Index 分層規則（已確認）

| 物件類型 | Z-Index 策略 | 具體值 |
|---------|------------|--------|
| 地板 Tile | 固定 | `0.0f` |
| 地板裝飾（血跡等） | 固定 | `1.0f` |
| 掉落物、Pickup | 動態 Y-Sorting | `1000 - worldY` |
| 敵人 | 動態 Y-Sorting | `1000 - worldY` |
| 玩家 | 動態 Y-Sorting | `1000 - worldY` |
| 牆壁（具立體感，南側） | 動態 Y-Sorting | `1000 - worldY` |
| 子彈 | 固定 | `199.0f` |
| HUD / UI | 固定 | `200.0f` |

> [!IMPORTANT]
> **⚠️ 子彈必須固定在 `199.0f`，不得使用 `50.0f`。**
> Y-Sorting 動態範圍是 `[2.0f, 198.0f]`。若子彈設為 50.0f，玩家/敵人在畫面下方時（Z 接近 198）會把子彈「蓋住」，
> 視覺上子彈穿入身體背後——這是嚴重的視覺錯誤。`199.0f` 確保子彈永遠渲染在所有 Y-Sort 實體之上。

**Y-Sorting 公式（已確認）**：
```cpp
void Entity::UpdateZIndex() {
    // Y 越小（畫面越下方）→ Z 越大（越前面）
    SetZIndex(glm::clamp(1000.0f - m_Transform.translation.y, 2.0f, 198.0f));
}
```

---

## 九、效能注意事項

1. **Bullet 對象池**：上限 100 顆，避免頻繁 new/delete
2. **Tile 數量優化**：M1 先照一般 GameObject 做法；若卡頓，M3 後考慮合併地磚到一張大 Texture
3. **動畫幀率**：角色行走 100ms，靜止（Idle）循環 100~150ms
4. **Y-Sorting 效能**：每幀一次浮點計算，開銷可忽略

---

## 十、資源路徑格式規範（CMake / MinGW 相容）

> [!IMPORTANT]
> **程式碼內所有 `Resources/` 路徑，統一使用「正斜線 `/`」，禁止使用反斜線 `\`。**

```cpp
// ✅ 正確：CMake + MinGW 環境下可靠
auto img = std::make_shared<Util::Image>("Resources/Bullets/enemy_arrow.png");

// ❌ 錯誤：反斜線在某些 MinGW 建置環境下會導致路徑解析失敗
auto img = std::make_shared<Util::Image>("Resources\\Bullets\\enemy_arrow.png");
```

- **PowerShell 腳本**（copy_assets.ps1）：複製操作使用 `\` 沒問題，這是 Windows 原生行為
- **C++ 原始碼**（`.cpp` / `.hpp`）：所有字串路徑一律 `/`
- **CMakeLists.txt**：`target_compile_definitions` 中的路徑也使用 `/`
