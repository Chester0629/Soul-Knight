#include "World/Corridor.hpp"
#include "Util/Image.hpp"

// ── Tile 主題輔助 ─────────────────────────────────────────────────────────────

std::string Corridor::RandFloor() {
    static const char* FLOORS[] = {
        "f101","f101","f101","f101",
        "f102","f103","f104","f105","f106"
    };
    return std::string(RESOURCE_DIR "/Tiles/") + FLOORS[m_Rng() % 9] + ".png";
}
void Corridor::ApplyWall(Util::GameObject* o, int s) {
    static const char* W[] = {"w001","w002"};
    o->SetDrawable(std::make_shared<Util::Image>(
        std::string(RESOURCE_DIR "/Tiles/") + W[s & 1] + ".png"));
}
void Corridor::ApplyFace(Util::GameObject* o, int s) {
    static const char* F[] = {"w004","w005"};
    o->SetDrawable(std::make_shared<Util::Image>(
        std::string(RESOURCE_DIR "/Tiles/") + F[s & 1] + ".png"));
}

// ── 建構 ─────────────────────────────────────────────────────────────────────

Corridor::Corridor(glm::vec2 centerPos, int cols, int rows, bool isHorizontal)
    : m_Center(centerPos), m_Cols(cols), m_Rows(rows), m_IsHorizontal(isHorizontal)
{
    // 以中心座標整數位元播種（確定性，避免浮點誤差）
    const unsigned seed = static_cast<unsigned>(static_cast<int>(centerPos.x)) * 1031u
                        ^ static_cast<unsigned>(static_cast<int>(centerPos.y)) * 113u;
    m_Rng.seed(seed);
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
    // 水平走廊：每欄預抽 set，確保 cap(r=0) + face(r=1) 同欄用同 set
    std::vector<int> northSet(m_Cols), southSet(m_Cols);
    for (int c = 0; c < m_Cols; c++) {
        northSet[c] = static_cast<int>(m_Rng() & 1u);
        southSet[c] = static_cast<int>(m_Rng() & 1u);
    }

    for (int r = 0; r < m_Rows; r++) {
        for (int c = 0; c < m_Cols; c++) {
            const glm::vec2 pos = TileToWorld(r, c);

            if (m_IsHorizontal) {
                const bool isTopCap   = (r == 0);
                const bool isNFace    = (r == 1);
                const bool isSWallCap = (r == m_Rows - 2);
                const bool isSFace    = (r == m_Rows - 1);

                if (isTopCap) {
                    auto t = std::make_shared<WallTile>(glm::vec2{pos.x, pos.y});
                    ApplyWall(t.get(), northSet[c]);
                    m_TileMap[r][c] = std::move(t);

                } else if (isNFace) {
                    auto t = std::make_shared<NorthFaceTile>(glm::vec2{pos.x, pos.y});
                    ApplyFace(t.get(), northSet[c]);  // 同欄同 set
                    m_TileMap[r][c] = std::move(t);

                } else if (isSWallCap) {
                    auto t = std::make_shared<WallTile>(pos);
                    ApplyWall(t.get(), southSet[c]);
                    t->SetZIndex(97.5f);
                    m_TileMap[r][c] = std::move(t);

                } else if (isSFace) {
                    auto t = std::make_shared<SouthFaceTile>(pos);
                    ApplyFace(t.get(), southSet[c]);  // 同欄同 set
                    m_TileMap[r][c] = t;
                    m_SouthFaces.push_back(std::move(t));

                } else {
                    auto t = std::make_shared<FloorTile>(
                        glm::vec2{pos.x, pos.y + TILE_SIZE * 0.5f});
                    t->SetDrawable(std::make_shared<Util::Image>(RandFloor()));
                    m_TileMap[r][c] = std::move(t);
                    if (r == m_Rows - 3) {
                        auto b = std::make_shared<FloorTile>(
                            glm::vec2{pos.x, pos.y - TILE_SIZE * 0.5f});
                        b->SetDrawable(std::make_shared<Util::Image>(RandFloor()));
                        m_BottomFill.push_back(std::move(b));
                    }
                }

            } else {
                // ── 垂直走廊：側牆無 face pair，每格獨立抽 ─────────────────
                const bool isWall = (c == 0 || c == m_Cols - 1);
                if (isWall) {
                    auto t = std::make_shared<WallTile>(pos);
                    ApplyWall(t.get(), static_cast<int>(m_Rng() & 1u));
                    if      (r == 0) t->SetZIndex(98.0f);
                    else if (r == 1) t->SetZIndex(99.0f);
                    m_TileMap[r][c] = std::move(t);
                } else {
                    auto t = std::make_shared<FloorTile>(
                        glm::vec2{pos.x, pos.y - TILE_SIZE * 0.5f});
                    t->SetDrawable(std::make_shared<Util::Image>(RandFloor()));
                    m_TileMap[r][c] = std::move(t);
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
