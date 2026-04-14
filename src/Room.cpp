#include "Room.hpp"
#include "Entity/Enemy.hpp"

Room::Room(RoomTemplate tmpl, glm::ivec2 gridPos, glm::vec2 worldOffset)
    : m_Template(tmpl)
    , m_GridPos(gridPos)
    , m_WorldOffset(worldOffset)
{
    const auto sz = RoomSpec::GetSize(tmpl);
    m_Cols = sz.cols;
    m_Rows = sz.rows;
    m_TileMap.assign(m_Rows, std::vector<std::shared_ptr<Tile>>(m_Cols, nullptr));
    Build();
}

// ── 世界座標換算（靜態，不含偏移）────────────────────────────────────────────
// ⚠️ worldY 有負號：TileMap row 向下遞增，PTSD Y 軸向上為正
glm::vec2 Room::TileToWorld(int row, int col, int mapRows, int mapCols) {
    const float worldX =  col * TILE_SIZE - (mapCols * TILE_SIZE) / 2.0f + TILE_SIZE / 2.0f;
    const float worldY = -(row * TILE_SIZE - (mapRows * TILE_SIZE) / 2.0f + TILE_SIZE / 2.0f);
    return {worldX, worldY};
}

// ── 世界座標換算（含房間偏移）────────────────────────────────────────────────
glm::vec2 Room::TileToWorld(int row, int col) const {
    return TileToWorld(row, col, m_Rows, m_Cols) + m_WorldOffset;
}

// ── 生成 TileMap ──────────────────────────────────────────────────────────────
// 佈局（以 17×17 為例）：
//   Row  0        : WallTile cap    (w001，超出螢幕，結構牆)
//   Row  1        : NorthFaceTile   (w004，可見，北牆內側面)
//   Row  2 ~ 13  : FloorTile        (地板) + 左右 WallTile（側牆）
//   Row  14       : SouthWallCap    (w001，可見，南牆頂蓋)
//   Row  15       : SouthFaceTile   (w004，可見，Y-Sort Z)
//   Row  16       : WallTile base   (w001，超出螢幕，結構牆)
void Room::Build() {
    for (int row = 0; row < m_Rows; row++) {
        for (int col = 0; col < m_Cols; col++) {
            const glm::vec2 pos = TileToWorld(row, col);  // 含 m_WorldOffset

            const bool isEdgeCol  = (col == 0 || col == m_Cols - 1);
            const bool isCapRow   = (row == 0);

            const bool isNFace    = (row == 1);
            const bool isSWallCap = (row == m_Rows - 2);  // row 14：南牆頂蓋（w001）
            const bool isSFace    = (row == m_Rows - 1);  // row 15：南牆面（w004，Y-Sort）

            if ((isCapRow || isEdgeCol) && !isSFace) {
                // 結構牆：頂蓋、底基、左右側牆
                m_TileMap[row][col] = std::make_shared<WallTile>(pos);

            } else if (isNFace) {
                // 北牆面：w004 現為 16×8，偏移由 NorthFaceTile 建構子內部處理（+3/4 TILE_SIZE）。
                // 原 row 1 格子補 FloorTile 填補間隙。
                m_TileMap[row][col] = std::make_shared<NorthFaceTile>(pos);
                m_BackfillTiles.push_back(std::make_shared<FloorTile>(pos));

            } else if (isSWallCap) {
                // 南牆頂蓋：WallTile w001，需在玩家之前渲染（Z 與 SouthFaceTile 同層）
                auto tile = std::make_shared<WallTile>(pos);
                tile->SetZIndex(97.5f);  // ⚠️ 覆蓋 WallTile 預設的 0.5f，確保在玩家（max≈98）之前
                m_TileMap[row][col] = std::move(tile);

            } else if (isSFace) {
                // 南牆面：偏移由 SouthFaceTile 建構子內部處理（+3/4 TILE_SIZE）。
                auto tile = std::make_shared<SouthFaceTile>(pos);
                m_TileMap[row][col] = tile;
                m_SouthFaces.push_back(std::move(tile));

            } else {
                // 內部地板
                const glm::vec2 facePos = {pos.x, pos.y};
                m_TileMap[row][col] = std::make_shared<FloorTile>(facePos);
            }
        }
    }
}

// ── Renderer 整合 ─────────────────────────────────────────────────────────────
void Room::AddToRenderer(Util::Renderer& renderer) {
    for (int row = 0; row < m_Rows; ++row)
        for (int col = 0; col < m_Cols; ++col)
            renderer.AddChild(m_TileMap[row][col]);

    for (auto& t : m_BackfillTiles)
        renderer.AddChild(t);
}

// ── 每幀更新 ──────────────────────────────────────────────────────────────────
void Room::SyncTransforms(glm::vec2 cameraPos) {
    for (int row = 0; row < m_Rows; ++row)
        for (int col = 0; col < m_Cols; ++col)
            m_TileMap[row][col]->SyncTransform(cameraPos);

    for (auto& t : m_BackfillTiles)
        t->SyncTransform(cameraPos);

    for (auto& f : m_SouthFaces)
        f->UpdateZIndex();
}

// ── 開門（Step 3.3）──────────────────────────────────────────────────────────
// 在指定方向的邊牆中央 6 格替換為 DoorTile（初始關閉）
// 結束後立刻呼叫 CheckAndOpenDoors()：若無敵人，門在 Generate 階段即開啟
void Room::OpenDoor(Direction dir) {
    switch (dir) {
    case Direction::EAST: {
        // 東牆：col = m_Cols-1，row 範圍取中央 6 格
        int rS = (m_Rows - 6) / 2;
        // rS-1 升格為 SouthWallCap（Z=97.5f），讓門洞上方有正確的牆頂蓋
        {
            auto cap = std::make_shared<WallTile>(TileToWorld(rS - 1, m_Cols - 1));
            cap->SetZIndex(97.5f);
            m_TileMap[rS - 1][m_Cols - 1] = std::move(cap);
        }
        // 門洞：用 DoorTile（初始關閉）
        for (int r = rS; r < rS + 6; r++) {
            auto door = std::make_shared<DoorTile>(TileToWorld(r, m_Cols - 1));
            m_TileMap[r][m_Cols - 1] = door;
            m_Doors.push_back(door);
        }
        // 門洞底部加 SouthFaceTile（rS+5，最底 row）— 底部塗層 Y-Sort Z 高，正確遮擋玩家
        {
            auto sf = std::make_shared<SouthFaceTile>(TileToWorld(rS + 5, m_Cols - 1));
            m_SouthFaces.push_back(sf);
            m_BackfillTiles.push_back(sf);
        }
        break;
    }
    case Direction::WEST: {
        // 西牆：col = 0，row 範圍取中央 6 格
        int rS = (m_Rows - 6) / 2;
        // rS-1 升格為 SouthWallCap（Z=97.5f）
        {
            auto cap = std::make_shared<WallTile>(TileToWorld(rS - 1, 0));
            cap->SetZIndex(97.5f);
            m_TileMap[rS - 1][0] = std::move(cap);
        }
        // 門洞：用 DoorTile（初始關閉）
        for (int r = rS; r < rS + 6; r++) {
            auto door = std::make_shared<DoorTile>(TileToWorld(r, 0));
            m_TileMap[r][0] = door;
            m_Doors.push_back(door);
        }
        // 門洞底部加 SouthFaceTile（rS+5）— 底部塗層 Y-Sort Z 高，正確遮擋玩家
        {
            auto sf = std::make_shared<SouthFaceTile>(TileToWorld(rS + 5, 0));
            m_SouthFaces.push_back(sf);
            m_BackfillTiles.push_back(sf);
        }
        break;
    }
    case Direction::NORTH: {
        // 北牆：row 0（cap）+ row 1（NorthFace），col 取中央 6 格
        int cS = (m_Cols - 6) / 2;
        for (int c = cS; c < cS + 6; c++) {
            auto door0 = std::make_shared<DoorTile>(TileToWorld(0, c));
            auto door1 = std::make_shared<DoorTile>(TileToWorld(1, c));
            m_TileMap[0][c] = door0;
            m_TileMap[1][c] = door1;
            m_Doors.push_back(door0);
            m_Doors.push_back(door1);
        }
        break;
    }
    case Direction::SOUTH: {
        // 南牆：row m_Rows-2（SouthWallCap）+ row m_Rows-1（SouthFace），col 取中央 6 格
        int cS = (m_Cols - 6) / 2;
        for (int c = cS; c < cS + 6; c++) {
            auto door0 = std::make_shared<DoorTile>(TileToWorld(m_Rows - 2, c));
            auto door1 = std::make_shared<DoorTile>(TileToWorld(m_Rows - 1, c));
            m_TileMap[m_Rows - 2][c] = door0;
            m_TileMap[m_Rows - 1][c] = door1;
            m_Doors.push_back(door0);
            m_Doors.push_back(door1);
        }
        break;
    }
    }
}

// ── Step 3.3：門與敵人狀態 ────────────────────────────────────────────────────
void Room::AssignEnemies(std::vector<Enemy*> enemies) {
    m_Enemies = std::move(enemies);
}

bool Room::IsCleared() const {
    for (auto* e : m_Enemies)
        if (!e->IsDying() && !e->IsDead()) return false;
    return true;  // 無敵人 or 全部死亡/瀕死
}

void Room::OpenAllDoors() {
    for (auto& door : m_Doors)
        door->Open();
    m_DoorsOpened   = true;
    m_CombatStarted = false;
}

// 進房觸發：關閉所有門，鎖定房間進行戰鬥
void Room::LockDoors() {
    if (m_DoorsOpened) return;  // 已清場，不需鎖
    m_CombatStarted = true;
    for (auto& door : m_Doors)
        door->Close();
}

// Debug：強制關閉所有門（繞過 m_DoorsOpened 守衛，測試用）
void Room::DebugCloseDoors() {
    m_DoorsOpened   = false;
    m_CombatStarted = false;
    for (auto& door : m_Doors)
        door->Close();
}

void Room::CheckAndOpenDoors() {
    if (m_DoorsOpened)   return;  // 已永久開啟
    if (!m_CombatStarted) return; // 尚未進房，不需檢查
    if (IsCleared()) OpenAllDoors();
}

// ── 碰撞查詢 ──────────────────────────────────────────────────────────────────
bool Room::IsWallAt(int row, int col) const {
    if (row < 0 || row >= m_Rows || col < 0 || col >= m_Cols) return true;
    return m_TileMap[row][col]->IsWall();
}
