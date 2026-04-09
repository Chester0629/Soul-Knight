#include "Room.hpp"

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
            const bool isBaseRow  = (row == m_Rows - 1);
            const bool isNFace    = (row == 1);
            const bool isSWallCap = (row == m_Rows - 2);  // row 14：南牆頂蓋（w001）
            const bool isSFace    = (row == m_Rows - 1);  // row 15：南牆面（w004，Y-Sort）

            if ((isCapRow || isEdgeCol) && !isSFace) {
                // 結構牆：頂蓋、底基、左右側牆
                m_TileMap[row][col] = std::make_shared<WallTile>(pos);

            } else if (isNFace) {
                // 北牆面：w004 頂部大部分透明，僅底部有橘色磚面像素。
                // 將渲染中心上移 TILE_SIZE，使可見磚面對齊 WallTile cap 底部。
                // 原 row 1 格子因此空出，補一塊 FloorTile 填補黑色間隙。
                const glm::vec2 facePos = {pos.x, pos.y + TILE_SIZE};
                m_TileMap[row][col] = std::make_shared<NorthFaceTile>(facePos);
                m_BackfillTiles.push_back(std::make_shared<FloorTile>(pos));

            } else if (isSWallCap) {
                // 南牆頂蓋：WallTile w001，需在玩家之前渲染（Z 與 SouthFaceTile 同層）
                auto tile = std::make_shared<WallTile>(pos);
                tile->SetZIndex(97.5f);  // ⚠️ 覆蓋 WallTile 預設的 0.5f，確保在玩家（max≈98）之前
                m_TileMap[row][col] = std::move(tile);

            } else if (isSFace) {
                // 南牆面：同 NorthFace，w004 底部 24px 可見，上移 TILE_SIZE/2 對齊 SouthWallCap 底部。
                const glm::vec2 facePos = {pos.x, pos.y + TILE_SIZE};
                auto tile = std::make_shared<SouthFaceTile>(facePos);
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
    // row m_Rows-1（南牆底基 row 16）完全超出螢幕，跳過不渲染
    // row 0（北牆頂蓋）保留渲染，由 tile 自然 peek 入螢幕頂部
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

// ── 碰撞查詢 ──────────────────────────────────────────────────────────────────
bool Room::IsWallAt(int row, int col) const {
    if (row < 0 || row >= m_Rows || col < 0 || col >= m_Cols) return true;
    return m_TileMap[row][col]->IsWall();
}
