#include "World/Corridor.hpp"

Corridor::Corridor(glm::vec2 centerPos, int cols, int rows, bool isHorizontal)
    : m_Center(centerPos), m_Cols(cols), m_Rows(rows), m_IsHorizontal(isHorizontal)
{
    m_TileMap.assign(m_Rows, std::vector<std::shared_ptr<Tile>>(m_Cols, nullptr));
    Build();
}

// ── 座標換算（與 Room::TileToWorld 相同公式，加上中心偏移）─────────────────
glm::vec2 Corridor::TileToWorld(int row, int col) const {
    const float wx =  col * TILE_SIZE - (m_Cols * TILE_SIZE) / 2.0f + TILE_SIZE / 2.0f + m_Center.x;
    const float wy = -(row * TILE_SIZE - (m_Rows * TILE_SIZE) / 2.0f + TILE_SIZE / 2.0f) + m_Center.y;
    return {wx, wy};
}

// ── 碰撞查詢 ──────────────────────────────────────────────────────────────────
bool Corridor::IsWallAtGrid(int row, int col) const {
    if (row < 0 || row >= m_Rows || col < 0 || col >= m_Cols) return true;
    if (m_TileMap[row][col]) return m_TileMap[row][col]->IsWall();
    return false;
}

// ── 生成 TileMap ──────────────────────────────────────────────────────────────
// 水平走廊（H_CORR_W=9 rows）：
//   row 0           : FloorTile +TILE_SIZE/4（落在相鄰房間地板區）
//   row 1           : NorthFaceTile +TILE_SIZE/4（w004，佔格子上半部）
//   row 2..m_Rows-3 : FloorTile +TILE_SIZE/4（通道，對齊 NorthFaceTile 底部）
//   row m_Rows-2    : WallTile（Z=97.5f，SouthWallCap）
//   row m_Rows-1    : SouthFaceTile（Y-Sort Z）
//
// 垂直走廊（V_CORR_W=8 cols）：
//   col 0 / col 7   : WallTile（左右牆）
//   col 1..6        : FloorTile（通道）
void Corridor::Build() {
    for (int r = 0; r < m_Rows; r++) {
        for (int c = 0; c < m_Cols; c++) {
            const glm::vec2 pos = TileToWorld(r, c);

            if (m_IsHorizontal) {
                const bool isTopCap   = (r == 0);
                const bool isNFace    = (r == 1);
                const bool isSWallCap = (r == m_Rows - 2);
                const bool isSFace    = (r == m_Rows - 1);

                if (isTopCap) {
                    // 落在相鄰房間地板區，同步往上移 TILE_SIZE/4
                    m_TileMap[r][c] = std::make_shared<WallTile>(
                        glm::vec2{pos.x, pos.y});

                } else if (isNFace) {
                    // 水平走廊頂部牆面往下移 24px：
                    // NorthFaceTile 建構子會 +12px，所以傳入 pos.y - 24px，
                    // 最終視覺中心 = pos.y - 24 + 12 = pos.y - 12
                    m_TileMap[r][c] = std::make_shared<NorthFaceTile>(
                        glm::vec2{pos.x, pos.y});

                } else if (isSWallCap) {
                    auto tile = std::make_shared<WallTile>(pos);
                    tile->SetZIndex(97.5f);
                    m_TileMap[r][c] = std::move(tile);

                } else if (isSFace) {
                    auto tile = std::make_shared<SouthFaceTile>(pos);
                    m_TileMap[r][c] = tile;
                    m_SouthFaces.push_back(std::move(tile));

                } else {
                    // 通道地板往上移 TILE_SIZE/2
                    m_TileMap[r][c] = std::make_shared<FloorTile>(
                        glm::vec2{pos.x, pos.y + TILE_SIZE * 0.5f});
                    // 最後一排（緊接 SouthWallCap）補底
                    if (r == m_Rows - 3) {
                        m_BottomFill.push_back(std::make_shared<FloorTile>(glm::vec2{pos.x, pos.y - TILE_SIZE * 0.5f}));
                    }
                }

            } else {
                // ── 垂直走廊（V_CORR_W=8 cols）─────────────────────────────
                const bool isWall = (c == 0 || c == m_Cols - 1);
                if (isWall) {
                    auto tile = std::make_shared<WallTile>(pos);
                    // 最頂兩行的牆需蓋過相鄰房間南牆的 Z
                    if      (r == 0) tile->SetZIndex(98.0f);
                    else if (r == 1) tile->SetZIndex(99.0f);
                    m_TileMap[r][c] = std::move(tile);
                } else {
                    m_TileMap[r][c] = std::make_shared<FloorTile>(glm::vec2{pos.x, pos.y - TILE_SIZE * 0.5f});
                }
            }
        }
    }
}

// ── Renderer 整合 ─────────────────────────────────────────────────────────────
void Corridor::AddToRenderer(Util::Renderer& renderer) {
    for (int r = 0; r < m_Rows; r++)
        for (int c = 0; c < m_Cols; c++)
            renderer.AddChild(m_TileMap[r][c]);
    for (auto& t : m_BottomFill)
        renderer.AddChild(t);
}

// ── 每幀更新 ──────────────────────────────────────────────────────────────────
void Corridor::SyncTransforms(glm::vec2 cameraPos) {
    for (int r = 0; r < m_Rows; r++)
        for (int c = 0; c < m_Cols; c++)
            m_TileMap[r][c]->SyncTransform(cameraPos);
    for (auto& t : m_BottomFill)
        t->SyncTransform(cameraPos);
    for (auto& f : m_SouthFaces)
        f->UpdateZIndex();
}
