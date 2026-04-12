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
void Corridor::Build() {
    for (int r = 0; r < m_Rows; r++) {
        for (int c = 0; c < m_Cols; c++) {
            const glm::vec2 pos = TileToWorld(r, c);
            // 水平走廊：上下兩行為牆；垂直走廊：左右兩列為牆
            const bool isWall = m_IsHorizontal
                ? (r == 0 || r == m_Rows - 1)
                : (c == 0 || c == m_Cols - 1);

            if (isWall)
                m_TileMap[r][c] = std::make_shared<WallTile>(pos);
            else
                m_TileMap[r][c] = std::make_shared<FloorTile>(pos);
        }
    }
}

// ── Renderer 整合 ─────────────────────────────────────────────────────────────
void Corridor::AddToRenderer(Util::Renderer& renderer) {
    for (int r = 0; r < m_Rows; r++)
        for (int c = 0; c < m_Cols; c++)
            renderer.AddChild(m_TileMap[r][c]);
}

// ── 每幀更新 ──────────────────────────────────────────────────────────────────
void Corridor::SyncTransforms(glm::vec2 cameraPos) {
    for (int r = 0; r < m_Rows; r++)
        for (int c = 0; c < m_Cols; c++)
            m_TileMap[r][c]->SyncTransform(cameraPos);
}
