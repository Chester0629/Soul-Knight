#include "World/World.hpp"
#include "Entity/Enemy.hpp"

#include <algorithm>
#include <cmath>

// ── 網格 → 世界座標 ────────────────────────────────────────────────────────────
// grid (2,2) = 世界原點 (0,0)；row 向下遞增 ↔ 世界 Y 向下遞減
glm::vec2 World::GridToWorld(glm::ivec2 gridPos) {
    return {
         (gridPos.y - 2) * GRID_SPACING_X,
        -(gridPos.x - 2) * GRID_SPACING_Y   // ⚠️ row 增大 = 世界 Y 減小
    };
}

// ── Generate ──────────────────────────────────────────────────────────────────
void World::Generate(unsigned seed, int floorIndex) {
    m_Rooms.clear();
    m_RoomTypes.clear();
    m_Corridors.clear();

    const DungeonLayout layout = DungeonGenerator::Generate(seed, floorIndex);

    // 1. 建立所有 Room（帶 worldOffset）
    for (const auto& node : layout.rooms) {
        glm::vec2 offset = GridToWorld(node.gridPos);
        auto room = std::make_unique<Room>(node.tmpl, node.gridPos, offset);
        if (node.type == RoomType::BASIC)
            room->SetIsEnemyRoom(true);
        m_Rooms.push_back(std::move(room));
        m_RoomTypes.push_back(node.type);
    }

    // 2. 開門 + 建立走廊
    for (const auto& conn : layout.connections) {
        Room& roomA = *m_Rooms[conn.fromIdx];
        Room& roomB = *m_Rooms[conn.toIdx];

        // 2a. 開門（必須在 AddToRenderer 前呼叫）
        roomA.OpenDoor(conn.dir);
        roomB.OpenDoor(DungeonGenerator::Opposite(conn.dir));

        // 2b. 計算走廊幾何
        glm::vec2 corrCenter;
        int corrCols, corrRows;
        bool isHorizontal;

        if (conn.dir == Direction::EAST || conn.dir == Direction::WEST) {
            // ── 水平走廊 ──────────────────────────────────────────────────
            isHorizontal = true;
            corrRows     = RoomSpec::H_CORR_W;  // 10（WallCap+NorthFace+6Floor+SouthWallCap+SouthFace）

            // westRoom = 在左（西）的房間；eastRoom = 在右（東）的房間
            Room& westRoom = (conn.dir == Direction::EAST) ? roomA : roomB;
            Room& eastRoom = (conn.dir == Direction::EAST) ? roomB : roomA;

            const float westEastEdge = westRoom.GetWorldOffset().x
                                     + westRoom.GetCols() * TILE_SIZE / 2.0f;
            const float eastWestEdge = eastRoom.GetWorldOffset().x
                                     - eastRoom.GetCols() * TILE_SIZE / 2.0f;

            corrCols   = std::max(1, static_cast<int>(std::round(
                             (eastWestEdge - westEastEdge) / TILE_SIZE)));
            // +TILE_SIZE/2：門洞中心 Y = worldOffset.y + 24（對任意奇數行數房間成立）
            corrCenter = {(westEastEdge + eastWestEdge) / 2.0f,
                           westRoom.GetWorldOffset().y };

        } else {
            // ── 垂直走廊 ──────────────────────────────────────────────────
            isHorizontal = false;
            corrCols     = RoomSpec::V_CORR_W;  // 8

            // northRoom = 在上方（高 Y）；southRoom = 在下方（低 Y）
            Room& northRoom = (conn.dir == Direction::SOUTH) ? roomA : roomB;
            Room& southRoom = (conn.dir == Direction::SOUTH) ? roomB : roomA;

            // northRoom 南邊界 Y（較小的正值或負值）
            const float northSouthEdge = northRoom.GetWorldOffset().y
                                       - northRoom.GetRows() * TILE_SIZE / 2.0f;
            // southRoom 北邊界 Y（較大的正值或負值）
            const float southNorthEdge = southRoom.GetWorldOffset().y
                                       + southRoom.GetRows() * TILE_SIZE / 2.0f;

            corrRows   = std::max(1, static_cast<int>(std::round(
                             (northSouthEdge - southNorthEdge) / TILE_SIZE)));
            // +2：讓走廊 row 0 對齊 northRoom SouthWallCap，row last 對齊 southRoom 北邊界
            corrRows  += 2;
            // corrCenter.y = midpoint + TILE_SIZE：
            //   可推導 row 0 Y = northRoom SouthWallCap Y（row m_Rows-2），
            //   使走廊側牆頂端與 northRoom SouthFaceTile 視覺底部無縫銜接
            corrCenter = {northRoom.GetWorldOffset().x - TILE_SIZE / 2.0f,
                          (northSouthEdge + southNorthEdge) / 2.0f + TILE_SIZE};
        }

        m_Corridors.push_back(
            std::make_unique<Corridor>(corrCenter, corrCols, corrRows, isHorizontal)
        );
    }

    // 所有門初始為開（讓玩家可自由進入）；LockDoors() 在進房時觸發
    for (auto& room : m_Rooms)
        room->OpenForEntry();
}

// ── Renderer 整合 ─────────────────────────────────────────────────────────────
void World::AddToRenderer(Util::Renderer& renderer) {
    for (auto& room : m_Rooms)
        room->AddToRenderer(renderer);
    for (auto& corr : m_Corridors)
        corr->AddToRenderer(renderer);
}

// ── 每幀更新 ──────────────────────────────────────────────────────────────────
void World::SyncTransforms(glm::vec2 cameraPos) {
    for (auto& room : m_Rooms)
        room->SyncTransforms(cameraPos);
    for (auto& corr : m_Corridors)
        corr->SyncTransforms(cameraPos);
}

// ── Step 3.3：敵人指派 + 門狀態更新 ─────────────────────────────────────────
void World::AssignEnemiesToRoom(int roomIdx, std::vector<Enemy*> enemies) {
    if (roomIdx >= 0 && roomIdx < static_cast<int>(m_Rooms.size()))
        m_Rooms[roomIdx]->AssignEnemies(std::move(enemies));
}

glm::vec2 World::GetRoomOffset(int roomIdx) const {
    if (roomIdx >= 0 && roomIdx < static_cast<int>(m_Rooms.size()))
        return m_Rooms[roomIdx]->GetWorldOffset();
    return {0.0f, 0.0f};
}

// 判斷 worldPos 落在哪個房間內（回傳索引，-1 表示走廊或越界）
// margin > 0 時向內縮（用於「確實進入房間」的觸發，避免在門口就誤觸）
static int GetRoomIndexAt(const std::vector<std::unique_ptr<Room>>& rooms,
                           glm::vec2 pos, float margin = 0.0f) {
    for (int i = 0; i < static_cast<int>(rooms.size()); ++i) {
        const auto& r   = *rooms[i];
        glm::vec2   off = r.GetWorldOffset();
        float       hx  = r.GetCols() * TILE_SIZE / 2.0f - margin;
        float       hy  = r.GetRows() * TILE_SIZE / 2.0f - margin;
        if (std::abs(pos.x - off.x) < hx && std::abs(pos.y - off.y) < hy)
            return i;
    }
    return -1;
}

void World::Update(glm::vec2 playerPos) {
    // 標準 AABB：用於迷你地圖 / SetVisited
    const int newRoomIdx = GetRoomIndexAt(m_Rooms, playerPos);
    if (newRoomIdx != m_CurrentRoomIdx) {
        m_CurrentRoomIdx = newRoomIdx;
        if (newRoomIdx >= 0)
            m_Rooms[newRoomIdx]->SetVisited();
    }

    // 縮小 AABB（向內 2 格）：確保玩家已穿過門口才觸發敵人生成與關門
    const int innerIdx = GetRoomIndexAt(m_Rooms, playerPos, TILE_SIZE * 2.0f);
    if (innerIdx >= 0 && innerIdx != m_InnerRoomIdx) {
        m_InnerRoomIdx = innerIdx;
        Room& room = *m_Rooms[innerIdx];
        if (room.IsEnemyRoom()) {
            if (!room.AreEnemiesSpawned() && m_OnEnterEnemyRoom)
                m_OnEnterEnemyRoom(innerIdx);
            if (!room.IsCleared())
                room.LockDoors();
        }
    } else if (innerIdx < 0) {
        m_InnerRoomIdx = -1;
    }

    // 每幀檢查：已開戰且敵人全清 → 開門
    for (auto& room : m_Rooms)
        room->CheckAndOpenDoors();
}

// ── 玩家出生位置（Spawn 房間中心）────────────────────────────────────────────
glm::vec2 World::GetSpawnPos() const {
    if (!m_Rooms.empty()) return m_Rooms[0]->GetWorldOffset();
    return {0.0f, 0.0f};
}

// ── 碰撞輔助：世界座標 → Tile 索引（含房間偏移）──────────────────────────────
static int worldToCol(float wx, float offsetX, int cols) {
    return static_cast<int>(std::floor(
        (wx - offsetX) / TILE_SIZE + (cols - 1) * 0.5f));
}
static int worldToRow(float wy, float offsetY, int rows) {
    return static_cast<int>(std::floor(
        -(wy - offsetY) / TILE_SIZE + (rows - 1) * 0.5f));
}

// ── AABB Push Back（穿透深度最小軸）─────────────────────────────────────────
static void resolveAABB(glm::vec2& center, glm::vec2 half,
                        glm::vec2 wallCenter, float wallHalf) {
    const float ox = half.x + wallHalf - std::abs(center.x - wallCenter.x);
    const float oy = half.y + wallHalf - std::abs(center.y - wallCenter.y);
    if (ox <= 0.0f || oy <= 0.0f) return;
    if (ox < oy)
        center.x += (center.x < wallCenter.x) ? -ox : ox;
    else
        center.y += (center.y < wallCenter.y) ? -oy : oy;
}

// ── 對單一 Tilemap 執行碰撞解決 ───────────────────────────────────────────────
void World::ResolveAgainstTilemap(
    glm::vec2& hitCenter, glm::vec2 hitHalf,
    glm::vec2 worldOffset, int cols, int rows,
    bool (*isWallFn)(int, int, const void*), const void* ctx)
{
    const float wallHalf = TILE_SIZE / 2.0f;

    // 求可能相交的 Tile 範圍（±2 格保護邊距）
    const int rMin = std::max(worldToRow(hitCenter.y + hitHalf.y, worldOffset.y, rows) - 2, 0);
    const int rMax = std::min(worldToRow(hitCenter.y - hitHalf.y, worldOffset.y, rows) + 2, rows - 1);
    const int cMin = std::max(worldToCol(hitCenter.x - hitHalf.x, worldOffset.x, cols) - 2, 0);
    const int cMax = std::min(worldToCol(hitCenter.x + hitHalf.x, worldOffset.x, cols) + 2, cols - 1);

    for (int r = rMin; r <= rMax; r++) {
        for (int c = cMin; c <= cMax; c++) {
            if (!isWallFn(r, c, ctx)) continue;
            // 牆壁中心（Room 公式）
            const float wx =  c * TILE_SIZE - cols * TILE_SIZE / 2.0f + TILE_SIZE / 2.0f + worldOffset.x;
            const float wy = -(r * TILE_SIZE - rows * TILE_SIZE / 2.0f + TILE_SIZE / 2.0f) + worldOffset.y;
            resolveAABB(hitCenter, hitHalf, {wx, wy}, wallHalf);
        }
    }
}

// ── 對單一 Tilemap 執行 IsBlocked ─────────────────────────────────────────────
bool World::IsBlockedByTilemap(
    glm::vec2 center, glm::vec2 halfSize,
    glm::vec2 worldOffset, int cols, int rows,
    bool (*isWallFn)(int, int, const void*), const void* ctx)
{
    const float wallHalf = TILE_SIZE / 2.0f;

    const int rMin = std::max(worldToRow(center.y + halfSize.y, worldOffset.y, rows) - 1, 0);
    const int rMax = std::min(worldToRow(center.y - halfSize.y, worldOffset.y, rows) + 1, rows - 1);
    const int cMin = std::max(worldToCol(center.x - halfSize.x, worldOffset.x, cols) - 1, 0);
    const int cMax = std::min(worldToCol(center.x + halfSize.x, worldOffset.x, cols) + 1, cols - 1);

    for (int r = rMin; r <= rMax; r++) {
        for (int c = cMin; c <= cMax; c++) {
            if (!isWallFn(r, c, ctx)) continue;
            const float wx =  c * TILE_SIZE - cols * TILE_SIZE / 2.0f + TILE_SIZE / 2.0f + worldOffset.x;
            const float wy = -(r * TILE_SIZE - rows * TILE_SIZE / 2.0f + TILE_SIZE / 2.0f) + worldOffset.y;
            const float ox = halfSize.x + wallHalf - std::abs(center.x - wx);
            const float oy = halfSize.y + wallHalf - std::abs(center.y - wy);
            if (ox > 0.0f && oy > 0.0f) return true;
        }
    }
    return false;
}

// ── Room / Corridor 的 isWallFn 包裝 ─────────────────────────────────────────
static bool roomWall(int r, int c, const void* ctx) {
    return reinterpret_cast<const Room*>(ctx)->IsWallAt(r, c);
}
static bool corrWall(int r, int c, const void* ctx) {
    return reinterpret_cast<const Corridor*>(ctx)->IsWallAtGrid(r, c);
}

// ── ResolveWall（玩家 / 敵人使用）────────────────────────────────────────────
void World::ResolveWall(glm::vec2& worldPos,
                        glm::vec2  hitOffset,
                        glm::vec2  hitSize) const
{
    glm::vec2 hitCenter = worldPos + hitOffset;
    const glm::vec2 hitHalf = hitSize * 0.5f;

    for (auto& room : m_Rooms)
        ResolveAgainstTilemap(hitCenter, hitHalf,
            room->GetWorldOffset(), room->GetCols(), room->GetRows(),
            roomWall, room.get());

    for (auto& corr : m_Corridors)
        ResolveAgainstTilemap(hitCenter, hitHalf,
            corr->GetWorldOffset(), corr->GetCols(), corr->GetRows(),
            corrWall, corr.get());

    worldPos = hitCenter - hitOffset;
}

// ── IsBlocked（敵人 TryMove 避障）────────────────────────────────────────────
bool World::IsBlocked(glm::vec2 center, glm::vec2 size) const {
    const glm::vec2 halfSize = size * 0.5f;

    for (auto& room : m_Rooms)
        if (IsBlockedByTilemap(center, halfSize,
                room->GetWorldOffset(), room->GetCols(), room->GetRows(),
                roomWall, room.get()))
            return true;

    for (auto& corr : m_Corridors)
        if (IsBlockedByTilemap(center, halfSize,
                corr->GetWorldOffset(), corr->GetCols(), corr->GetRows(),
                corrWall, corr.get()))
            return true;

    return false;
}


// ── DebugToggleDoors（F 鍵測試用）────────────────────────────────────────────
void World::DebugToggleDoors() {
    for (auto& room : m_Rooms) {
        if (room->AreDoorsOpen())
            room->DebugCloseDoors();
        else
            room->OpenAllDoors();
    }
}
