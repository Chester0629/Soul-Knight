#pragma once

#include "Tile.hpp"
#include "Util/Renderer.hpp"

#include <memory>
#include <vector>

class Enemy;  // 前向宣告（Room 持有非擁有式指標）

// 四方向（5×5 網格：row 向下遞增，col 向右遞增；PTSD Y 軸向上為正）
enum class Direction { NORTH = 0, EAST = 1, SOUTH = 2, WEST = 3 };

// ── 房間模板（含牆總格數）────────────────────────────────────────────────────
// 牆壁結構（北/南各 2 行，左/右各 1 列）：
//   Row 0          : WallTile cap  (w001，通常超出螢幕)
//   Row 1          : NorthFace     (w004，可見，固定 Z=0.6f)
//   Row 2~ROWS-3   : 地板 + 側牆
//   Row ROWS-2     : SouthWallCap  (w001，Z=97.5f)
//   Row ROWS-1     : SouthFace     (w004，Y-Sort Z)
enum class RoomTemplate {
    SPAWN  = 0,  // 17×17 — 初始房間（正方形）
    SMALL  = 1,  // 17×13 — 小型怪物房間
    MEDIUM = 2,  // 22×17 — 中型怪物房間
    LARGE  = 3,  // 30×20 — 大型怪物房間
    WIDE   = 4,  // 26×15 — 寬型房間
    COUNT  = 5
};

namespace RoomSpec {
    constexpr int H_CORR_W = 9;  // 水平走廊總高（WallCap+NorthFace+5地板+SouthWallCap+SouthFace）
    constexpr int V_CORR_W = 8;  // 垂直走廊總寬（6 地板 + 兩側牆）
    constexpr int CORR_W   = V_CORR_W; // 向後相容（舊程式碼仍可用）

    struct Size { int cols, rows; };
    // 各模板尺寸（含牆）
    constexpr Size TEMPLATE_SIZES[] = {
        {17, 17},   // SPAWN
        {17, 13},   // SMALL
        {21, 17},   // MEDIUM (22→21，確保奇數列，走廊長度整數對齊)
        {29, 19},   // LARGE  (30×20→29×19，同上)
        {25, 15},   // WIDE   (26→25，同上)
    };

    inline Size GetSize(RoomTemplate t) {
        return TEMPLATE_SIZES[static_cast<int>(t)];
    }
}

// ── Room ──────────────────────────────────────────────────────────────────────
class Room {
public:
    // 以模板建立房間；worldOffset 為房間中心的世界座標偏移（Step 3.2 多房間用）
    explicit Room(RoomTemplate tmpl     = RoomTemplate::SPAWN,
                  glm::ivec2  gridPos   = {0, 0},
                  glm::vec2   worldOffset = {0.0f, 0.0f});
    ~Room() = default;

    void AddToRenderer(Util::Renderer& renderer);
    void SyncTransforms(glm::vec2 cameraPos);

    // 開門：將指定方向邊牆中央 6 格替換為 DoorTile（Step 3.2 走廊銜接）
    // ⚠️ 必須在 AddToRenderer 之前呼叫，否則舊 WallTile 仍留在 Renderer
    void OpenDoor(Direction dir);

    // Step 3.3：門的生成邏輯
    void AssignEnemies(std::vector<Enemy*> enemies);  // 指派非擁有式指標
    bool HasEnemies()  const { return !m_Enemies.empty(); }
    bool IsCleared()   const;                         // 無敵人 or 全部死亡
    void CheckAndOpenDoors();                         // 若已開戰且 IsCleared() → 開門
    void OpenAllDoors();                              // 強制開啟所有門
    void LockDoors();                                 // 進房觸發：關門（開始戰鬥）
    bool AreDoorsOpen() const { return m_DoorsOpened; } // 查詢門目前是否開啟
    void DebugCloseDoors();                              // 強制關閉（Debug 用，繞過清場守衛）


    bool IsWallAt(int row, int col) const;
    int GetCols()  const { return m_Cols; }
    int GetRows()  const { return m_Rows; }
    glm::ivec2 GetGridPos()    const { return m_GridPos; }
    glm::vec2  GetWorldOffset() const { return m_WorldOffset; }
    RoomTemplate GetTemplate() const { return m_Template; }

    // 世界座標換算（含房間偏移）
    glm::vec2 TileToWorld(int row, int col) const;
    // 靜態版（不含偏移，向後相容）
    static glm::vec2 TileToWorld(int row, int col, int mapRows, int mapCols);

private:
    void Build();

    RoomTemplate m_Template;
    glm::ivec2   m_GridPos;      // 5×5 地城網格中的格子座標
    glm::vec2    m_WorldOffset;  // 房間中心的世界座標偏移

    int m_Cols;
    int m_Rows;

    std::vector<std::vector<std::shared_ptr<Tile>>> m_TileMap;
    std::vector<std::shared_ptr<SouthFaceTile>>     m_SouthFaces;
    std::vector<std::shared_ptr<Tile>>               m_BackfillTiles;

    // Step 3.3
    std::vector<std::shared_ptr<DoorTile>> m_Doors;
    std::vector<Enemy*>                    m_Enemies;         // 非擁有
    bool                                   m_DoorsOpened  = false; // 永久開啟（已清場）
    bool                                   m_CombatStarted = false; // 已進房觸發戰鬥

};
