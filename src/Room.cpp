#include "Room.hpp"
#include "Entity/Enemy.hpp"
#include "Util/Image.hpp"

// ── Tile 隨機主題輔助 ─────────────────────────────────────────────────────────

std::string Room::RandFloor() {
    static const char* FLOORS[] = {
        "f101","f101","f101","f101",   // 80%：基礎地板
        "f102","f103","f104","f105","f106"
    };
    return std::string(RESOURCE_DIR "/Tiles/") + FLOORS[m_Rng() % 9] + ".png";
}

void Room::ApplyWall(Util::GameObject* o, int s) {
    static const char* W[] = {"w001","w002"};
    o->SetDrawable(std::make_shared<Util::Image>(
        std::string(RESOURCE_DIR "/Tiles/") + W[s & 1] + ".png"));
}
void Room::ApplyFace(Util::GameObject* o, int s) {
    static const char* F[] = {"w004","w005"};
    o->SetDrawable(std::make_shared<Util::Image>(
        std::string(RESOURCE_DIR "/Tiles/") + F[s & 1] + ".png"));
}

// ── 建構 ─────────────────────────────────────────────────────────────────────

Room::Room(RoomTemplate tmpl, glm::ivec2 gridPos, glm::vec2 worldOffset)
    : m_Template(tmpl)
    , m_GridPos(gridPos)
    , m_WorldOffset(worldOffset)
{
    // 依 gridPos 確定性播種（相同 seed 相同房間外觀）
    m_Rng.seed(static_cast<unsigned>(gridPos.x * 1031u + gridPos.y * 113u + 7u));

    const auto sz = RoomSpec::GetSize(tmpl);
    m_Cols = sz.cols;
    m_Rows = sz.rows;
    m_TileMap.assign(m_Rows, std::vector<std::shared_ptr<Tile>>(m_Cols, nullptr));
    Build();
}

// ── 世界座標換算（靜態，不含偏移）────────────────────────────────────────────
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
// 佈局（以 17×17 SPAWN 為例）：
//   Row  0        : WallTile cap    (w001，結構頂蓋)
//   Row  1        : NorthFaceTile   (w004，+TILE_SIZE/4 往上，佔格子上半部)
//   Row  2~13     : FloorTile (+TILE_SIZE/4 往上) + 左右 WallTile（側牆）
//   Row  14       : WallTile        (Z=97.5f，南牆頂蓋)
//   Row  15       : SouthFaceTile   (w004，Y-Sort Z，+TILE_SIZE*0.75f)
//   Row  16       : WallTile base   (結構底基)
//
// 地板格子往上移 TILE_SIZE/4 (12px)，使其頂部對齊 NorthFaceTile 底部（worldY+12-12=worldY），
// 視覺上無縫連接；最靠近北牆面的地板格子上半部會被 NorthFaceTile (Z=0.3) 壓住，縮為半牌。
void Room::Build() {
    // 每欄預抽一個 set：北牆 cap+face 同欄共用，南牆 cap+face 同欄共用
    std::vector<int> northSet(m_Cols), southSet(m_Cols);
    for (int c = 0; c < m_Cols; c++) {
        northSet[c] = static_cast<int>(m_Rng() & 1u);
        southSet[c] = static_cast<int>(m_Rng() & 1u);
    }

    for (int row = 0; row < m_Rows; row++) {
        for (int col = 0; col < m_Cols; col++) {
            const glm::vec2 pos = TileToWorld(row, col);

            const bool isEdgeCol  = (col == 0 || col == m_Cols - 1);
            const bool isCapRow   = (row == 0);
            const bool isNFace    = (row == 1);
            const bool isSWallCap = (row == m_Rows - 2);
            const bool isSFace    = (row == m_Rows - 1);

            if (isCapRow && !isEdgeCol) {
                // 北牆頂蓋（非邊角）：與同欄的 NorthFaceTile 同 set
                auto t = std::make_shared<WallTile>(pos);
                ApplyWall(t.get(), northSet[col]);
                m_TileMap[row][col] = std::move(t);

            } else if (isNFace && !isEdgeCol) {
                // 北牆面：同欄 northSet
                auto t = std::make_shared<NorthFaceTile>(pos);
                ApplyFace(t.get(), northSet[col]);
                m_TileMap[row][col] = std::move(t);

            } else if (isSWallCap && !isEdgeCol) {
                // 南牆頂蓋（非邊角）：與同欄的 SouthFaceTile 同 set
                auto t = std::make_shared<WallTile>(pos);
                ApplyWall(t.get(), southSet[col]);
                t->SetZIndex(97.5f);
                m_TileMap[row][col] = std::move(t);

            } else if (isSFace && !isEdgeCol) {
                // 南牆面：同欄 southSet
                auto t = std::make_shared<SouthFaceTile>(pos);
                ApplyFace(t.get(), southSet[col]);
                m_TileMap[row][col] = t;
                m_SouthFaces.push_back(std::move(t));

            } else if ((isCapRow || isEdgeCol || isSWallCap) && !isSFace) {
                // 邊角 WallTile / 側牆：無配對的 face，每格獨立抽
                auto t = std::make_shared<WallTile>(pos);
                ApplyWall(t.get(), static_cast<int>(m_Rng() & 1u));
                if (isSWallCap) t->SetZIndex(97.5f);
                m_TileMap[row][col] = std::move(t);

            } else if (isSFace) {
                // 邊角南牆面（角落）：獨立抽，face 與上方 cap 欄位相同
                auto t = std::make_shared<SouthFaceTile>(pos);
                ApplyFace(t.get(), southSet[col]);
                m_TileMap[row][col] = t;
                m_SouthFaces.push_back(std::move(t));

            } else {
                // 地板
                auto t = std::make_shared<FloorTile>(
                    glm::vec2{pos.x, pos.y + TILE_SIZE * 0.5f});
                t->SetDrawable(std::make_shared<Util::Image>(RandFloor()));
                m_TileMap[row][col] = std::move(t);

                if (row == m_Rows - 3) {
                    auto b = std::make_shared<FloorTile>(
                        glm::vec2{pos.x, pos.y - TILE_SIZE * 0.5f});
                    b->SetDrawable(std::make_shared<Util::Image>(RandFloor()));
                    m_BottomFill.push_back(std::move(b));
                }
            }
        }
    }
}

// ── Renderer 整合 ─────────────────────────────────────────────────────────────
void Room::AddToRenderer(Util::Renderer& renderer) {
    for (int row = 0; row < m_Rows; ++row)
        for (int col = 0; col < m_Cols; ++col)
            renderer.AddChild(m_TileMap[row][col]);
    for (auto& t : m_BottomFill)
        renderer.AddChild(t);
    for (auto& t : m_SideFaces)
        renderer.AddChild(t);
}

// ── 每幀更新 ──────────────────────────────────────────────────────────────────
void Room::SyncTransforms(glm::vec2 cameraPos) {
    for (int row = 0; row < m_Rows; ++row)
        for (int col = 0; col < m_Cols; ++col)
            m_TileMap[row][col]->SyncTransform(cameraPos);

    for (auto& t : m_BottomFill)
        t->SyncTransform(cameraPos);

    for (auto& f : m_SouthFaces)
        f->UpdateZIndex();
    for (auto& f : m_SideFaces)
        f->SyncTransform(cameraPos);
}

// ── 開門（Step 3.3）──────────────────────────────────────────────────────────
// 在指定方向的邊牆中央 6 格替換為 DoorTile（初始關閉）
// 規則：
//   EAST/WEST — 側牆單列，6 個 DoorTile，Y-Sort Z（在 DoorTile 建構子處理）
//   NORTH     — row 0（WallCap，invisible）+ row 1（NorthFace，visible）各 6 格
//   SOUTH     — row m_Rows-2（SouthWallCap，invisible）+ row m_Rows-1（SouthFace，visible）各 6 格
void Room::OpenDoor(Direction dir) {
    switch (dir) {

    case Direction::EAST: {
        const int col = m_Cols - 1;
        const int rS  = (m_Rows - 6) / 2;
        {
            const int s = static_cast<int>(m_Rng() & 1u);  // cap + face 同 set
            auto cap = std::make_shared<WallTile>(TileToWorld(rS - 1, col));
            ApplyWall(cap.get(), s);
            cap->SetZIndex(97.5f);
            m_TileMap[rS - 1][col] = std::move(cap);
            auto sf = std::make_shared<SideWallFaceTile>(TileToWorld(rS, col));
            ApplyFace(sf.get(), s);
            m_SideFaces.push_back(std::move(sf));
        }
        for (int r = rS; r < rS + 6; r++) {
            auto door = std::make_shared<DoorTile>(TileToWorld(r, col));
            m_TileMap[r][col] = door;
            m_Doors.push_back(std::move(door));
        }
        {
            const glm::vec2 belowPos = TileToWorld(rS + 6, col);
            auto wall = std::make_shared<WallTile>(belowPos);
            ApplyWall(wall.get(), static_cast<int>(m_Rng() & 1u));
            wall->SetZIndex(glm::clamp(50.0f - belowPos.y / 64.0f, 2.0f, 98.0f));
            m_TileMap[rS + 6][col] = std::move(wall);
        }
        break;
    }

    case Direction::WEST: {
        const int col = 0;
        const int rS  = (m_Rows - 6) / 2;
        {
            const int s = static_cast<int>(m_Rng() & 1u);
            auto cap = std::make_shared<WallTile>(TileToWorld(rS - 1, col));
            ApplyWall(cap.get(), s);
            cap->SetZIndex(97.5f);
            m_TileMap[rS - 1][col] = std::move(cap);
            auto sf = std::make_shared<SideWallFaceTile>(TileToWorld(rS, col));
            ApplyFace(sf.get(), s);
            m_SideFaces.push_back(std::move(sf));
        }
        for (int r = rS; r < rS + 6; r++) {
            auto door = std::make_shared<DoorTile>(TileToWorld(r, col));
            m_TileMap[r][col] = door;
            m_Doors.push_back(std::move(door));
        }
        {
            const glm::vec2 belowPos = TileToWorld(rS + 6, col);
            auto wall = std::make_shared<WallTile>(belowPos);
            ApplyWall(wall.get(), static_cast<int>(m_Rng() & 1u));
            wall->SetZIndex(glm::clamp(50.0f - belowPos.y / 64.0f, 2.0f, 98.0f));
            m_TileMap[rS + 6][col] = std::move(wall);
        }
        break;
    }

    case Direction::NORTH: {
        // row 0（WallCap）：invisible DoorTile，僅提供碰撞開關
        // row 1（NorthFace）：visible DoorTile，顯示門圖案
        const int cS = (m_Cols - 6) / 2;
        for (int c = cS; c < cS + 6; c++) {
            auto wallDoor = std::make_shared<DoorTile>(TileToWorld(1, c));
            wallDoor->SetVisible(false);
            m_TileMap[0][c] = wallDoor;
            m_Doors.push_back(wallDoor);

            auto faceDoor = std::make_shared<DoorTile>(TileToWorld(0, c));
            m_TileMap[1][c] = faceDoor;
            m_Doors.push_back(faceDoor);
        }
        break;
    }

    case Direction::SOUTH: {
        // row m_Rows-2（SouthWallCap）：invisible DoorTile，僅提供碰撞開關
        // row m_Rows-1（SouthFace）：visible DoorTile，顯示門圖案
        const int cS = (m_Cols - 6) / 2;
        for (int c = cS; c < cS + 6; c++) {
            auto wallDoor = std::make_shared<DoorTile>(TileToWorld(m_Rows - 1, c));
            wallDoor->SetVisible(false);
            m_TileMap[m_Rows - 2][c] = wallDoor;
            m_Doors.push_back(wallDoor);

            auto faceDoor = std::make_shared<DoorTile>(TileToWorld(m_Rows - 2, c));
            m_TileMap[m_Rows - 1][c] = faceDoor;
            m_Doors.push_back(faceDoor);
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
    return true;
}

void Room::OpenAllDoors() {
    for (auto& door : m_Doors)
        door->Open();
    m_DoorsOpened   = true;
    m_CombatStarted = false;
}

void Room::LockDoors() {
    if (m_DoorsOpened) return;
    m_CombatStarted = true;
    for (auto& door : m_Doors)
        door->Close();
}

void Room::DebugCloseDoors() {
    m_DoorsOpened   = false;
    m_CombatStarted = false;
    for (auto& door : m_Doors)
        door->Close();
}

void Room::CheckAndOpenDoors() {
    if (m_DoorsOpened)   return;
    if (!m_CombatStarted) return;
    if (IsCleared()) OpenAllDoors();
}

// ── 碰撞查詢 ──────────────────────────────────────────────────────────────────
bool Room::IsWallAt(int row, int col) const {
    if (row < 0 || row >= m_Rows || col < 0 || col >= m_Cols) return true;
    return m_TileMap[row][col]->IsWall();
}
