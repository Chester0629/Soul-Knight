#include "UI/MiniMap.hpp"
#include "Util/Image.hpp"

static const char* IMG_DARK = RESOURCE_DIR "/UI/minimap_room.png";
static const char* IMG_LIT  = RESOURCE_DIR "/UI/minimap_room_lit.png";
static const char* IMG_CUR  = RESOURCE_DIR "/UI/minimap_cur.png";
static const char* IMG_HOME = RESOURCE_DIR "/UI/ui_41.png";
static const char* IMG_PORT = RESOURCE_DIR "/UI/ui_42.png";

glm::vec2 MiniMap::GridToScreen(glm::ivec2 gp) const {
    return {
        MAP_CX + (gp.y - 2) * CELL_W,
        MAP_CY - (gp.x - 2) * CELL_H
    };
}

static std::shared_ptr<Util::GameObject> MakeBlock(
    const char* img, float sx, float sy, glm::vec2 pos, float z)
{
    auto obj = std::make_shared<Util::GameObject>();
    obj->SetDrawable(std::make_shared<Util::Image>(img));
    obj->m_Transform.scale       = {sx, sy};
    obj->m_Transform.translation = pos;
    obj->SetZIndex(z);
    obj->SetVisible(false);
    return obj;
}

void MiniMap::Init(int roomCount,
                   const std::vector<glm::ivec2>&      gridPositions,
                   const std::vector<MiniMapRoomType>& roomTypes,
                   const std::vector<ConnInfo>&        connections)
{
    m_GridPos = gridPositions;
    m_RoomDarkObjs.clear();
    m_RoomLitObjs.clear();
    m_CorrObjs.clear();
    m_Icons.clear();
    m_Conns.clear();

    // ── 房間方塊：深灰（當前）/ 淺灰（已訪問）各一組 ────────────────────────
    for (int i = 0; i < roomCount; ++i) {
        glm::vec2 pos = GridToScreen(gridPositions[i]);
        m_RoomDarkObjs.push_back(MakeBlock(IMG_DARK, SX_ROOM, SY_ROOM, pos, Z_MAP));
        m_RoomLitObjs .push_back(MakeBlock(IMG_LIT,  SX_ROOM, SY_ROOM, pos, Z_MAP));
    }

    // ── 走廊連接段（淺灰，任一端點已探索即顯示） ────────────────────────────
    for (const auto& c : connections) {
        m_Conns.push_back({c.fromIdx, c.toIdx, c.horizontal});
        glm::vec2 mid = (GridToScreen(gridPositions[c.fromIdx]) +
                         GridToScreen(gridPositions[c.toIdx])) * 0.5f;
        float sx = c.horizontal ? SX_CHW : SX_CVW;
        float sy = c.horizontal ? SY_CHH : SY_CVH;
        m_CorrObjs.push_back(MakeBlock(IMG_LIT, sx, sy, mid, Z_MAP));
    }

    // ── 特殊圖示（探索後顯示） ───────────────────────────────────────────────
    for (int i = 0; i < roomCount; ++i) {
        const char* img = nullptr;
        if      (roomTypes[i] == MiniMapRoomType::SPAWN)  img = IMG_HOME;
        else if (roomTypes[i] == MiniMapRoomType::PORTAL) img = IMG_PORT;
        if (!img) continue;

        auto icon = std::make_shared<Util::GameObject>();
        icon->SetDrawable(std::make_shared<Util::Image>(img));
        icon->m_Transform.scale       = {ICON_S, ICON_S};
        icon->m_Transform.translation = GridToScreen(gridPositions[i]);
        icon->SetZIndex(Z_ICON);
        icon->SetVisible(false);
        m_Icons.push_back({i, icon});
    }

    // ── 游標：4 條黃色細條圍成方框 ──────────────────────────────────────────
    // [0]=上, [1]=下, [2]=左, [3]=右
    for (auto& bar : m_CursorBars) bar = nullptr;

    m_CursorBars[0] = MakeBlock(IMG_CUR, BAR_H_SX, BAR_H_SY, {0.0f, 0.0f}, Z_CUR);
    m_CursorBars[1] = MakeBlock(IMG_CUR, BAR_H_SX, BAR_H_SY, {0.0f, 0.0f}, Z_CUR);
    m_CursorBars[2] = MakeBlock(IMG_CUR, BAR_V_SX, BAR_V_SY, {0.0f, 0.0f}, Z_CUR);
    m_CursorBars[3] = MakeBlock(IMG_CUR, BAR_V_SX, BAR_V_SY, {0.0f, 0.0f}, Z_CUR);
}

void MiniMap::AddToRenderer(Util::Renderer& root) {
    for (auto& c : m_CorrObjs)     root.AddChild(c);
    for (auto& r : m_RoomDarkObjs) root.AddChild(r);
    for (auto& r : m_RoomLitObjs)  root.AddChild(r);
    for (auto& ie : m_Icons)       root.AddChild(ie.obj);
    for (auto& b : m_CursorBars)   root.AddChild(b);
}

void MiniMap::Update(int currentRoomIdx,
                     const std::vector<bool>& visited,
                     const std::vector<bool>& revealed)
{
    const int n  = static_cast<int>(visited.size());
    const int nr = static_cast<int>(m_RoomDarkObjs.size());
    const int rn = static_cast<int>(revealed.size());

    // ── 房間方塊 ─────────────────────────────────────────────────────────────
    for (int i = 0; i < nr; ++i) {
        const bool vis   = i < n  && visited[i];
        const bool rev   = i < rn && revealed[i];
        const bool isCur = (i == currentRoomIdx);

        // 深灰：已揭露但未訪問 OR 當前房間
        m_RoomDarkObjs[i]->SetVisible((rev && !vis) || isCur);
        // 淺灰：已訪問且非當前
        m_RoomLitObjs[i]->SetVisible(vis && !isCur);
    }

    // ── 走廊：任一端點已訪問即顯示 ──────────────────────────────────────────
    for (int ci = 0; ci < static_cast<int>(m_CorrObjs.size()); ++ci) {
        const auto& cr = m_Conns[ci];
        const bool show = (cr.from < n && visited[cr.from]) ||
                          (cr.to   < n && visited[cr.to]);
        m_CorrObjs[ci]->SetVisible(show);
    }

    // ── 圖示：所屬房間已探索才顯示 ──────────────────────────────────────────
    for (auto& ie : m_Icons)
        ie.obj->SetVisible(ie.roomIdx < n && visited[ie.roomIdx]);

    // ── 游標：黃色方框環繞當前房間 ──────────────────────────────────────────
    if (currentRoomIdx >= 0 && currentRoomIdx < static_cast<int>(m_GridPos.size())) {
        const glm::vec2 c = GridToScreen(m_GridPos[currentRoomIdx]);
        const float hx = ROOM_W * 0.5f;
        const float hy = ROOM_H * 0.5f;
        const float bh = BORDER * 0.5f;

        m_CursorBars[0]->m_Transform.translation = {c.x,        c.y + hy + bh};  // 上
        m_CursorBars[1]->m_Transform.translation = {c.x,        c.y - hy - bh};  // 下
        m_CursorBars[2]->m_Transform.translation = {c.x - hx - bh, c.y};         // 左
        m_CursorBars[3]->m_Transform.translation = {c.x + hx + bh, c.y};         // 右

        for (auto& b : m_CursorBars) b->SetVisible(true);
    } else {
        for (auto& b : m_CursorBars) b->SetVisible(false);
    }
}
