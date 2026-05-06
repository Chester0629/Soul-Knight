#pragma once

#include "Util/GameObject.hpp"
#include "Util/Renderer.hpp"
#include "pch.hpp"

#include <array>
#include <memory>
#include <vector>

enum class MiniMapRoomType { NORMAL, SPAWN, PORTAL };

// MiniMap — 右側中央迷你地圖
//
// 房間揭露規則：
//   隱藏                       = 尚未揭露（未訪問且無走廊已進入）
//   深灰（minimap_room.png）   = 已揭露但未訪問 OR 玩家目前所在房間
//   淺灰（minimap_room_lit.png）= 玩家曾訪問的房間（非當前）
//
// 揭露觸發：玩家從房間走入走廊時，揭露走廊另一端的相鄰房間
// 走廊：任一端點已訪問即顯示
// 游標：以黃色方框環繞當前房間外圍（上/下/左/右 4 條細條）
// 圖示：ui_41（家/初始房）、ui_42（傳送門）僅在訪問後顯示
class MiniMap {
public:
    struct ConnInfo {
        int  fromIdx;
        int  toIdx;
        bool horizontal;  // true = E/W, false = N/S
    };

    void Init(int roomCount,
              const std::vector<glm::ivec2>&      gridPositions,
              const std::vector<MiniMapRoomType>& roomTypes,
              const std::vector<ConnInfo>&        connections);

    void AddToRenderer(Util::Renderer& root);
    void Update(int currentRoomIdx,
                const std::vector<bool>& visited,
                const std::vector<bool>& revealed);

    static constexpr float Z_MAP  = 99.45f;
    static constexpr float Z_ICON = 99.46f;
    static constexpr float Z_CUR  = 99.47f;

private:
    // ── 尺寸常數 ─────────────────────────────────────────────────────────────
    static constexpr float ROOM_W  = 19.0f;    // 房間方塊寬（ui_41 原始像素寬）
    static constexpr float ROOM_H  = 19.0f;    // 房間方塊高（正方形）
    static constexpr float CELL_W  = 32.0f;    // 水平格距（中心到中心）
    static constexpr float CELL_H  = 32.0f;    // 垂直格距（中心到中心）

    // 走廊填滿房間之間的空隙
    static constexpr float CORR_HW = CELL_W - ROOM_W;   // 12px 長
    static constexpr float CORR_HH = 5.0f;              // 5px 厚
    static constexpr float CORR_VW = 5.0f;              // 5px 厚
    static constexpr float CORR_VH = CELL_H - ROOM_H;  // 12px 長

    static constexpr float BORDER  = 2.0f;              // 游標邊框厚度

    // ── PNG 素材為 4×4，依目標尺寸推算 scale ─────────────────────────────────
    static constexpr float SX_ROOM  = ROOM_W  / 4.0f;
    static constexpr float SY_ROOM  = ROOM_H  / 4.0f;
    static constexpr float SX_CHW   = CORR_HW / 4.0f;
    static constexpr float SY_CHH   = CORR_HH / 4.0f;
    static constexpr float SX_CVW   = CORR_VW / 4.0f;
    static constexpr float SY_CVH   = CORR_VH / 4.0f;

    // 游標橫/直條 scale（minimap_cur.png 4×4）
    static constexpr float BAR_H_SX = (ROOM_W + 2.0f * BORDER) / 4.0f;
    static constexpr float BAR_H_SY = BORDER / 4.0f;
    static constexpr float BAR_V_SX = BORDER / 4.0f;
    static constexpr float BAR_V_SY = ROOM_H / 4.0f;

    // 圖示縮放（ui_41/42 原始寬 14px，以 scale 1.0 呈現原始尺寸）
    static constexpr float ICON_S   = 1.0f;

    // ── 地圖錨點：右側中央 ────────────────────────────────────────────────────
    static constexpr float MARGIN   = 12.0f;
    static constexpr float MAP_CX   = 640.0f - MARGIN - ROOM_W * 0.5f - 2.0f * CELL_W;
    static constexpr float MAP_CY   = 0.0f;

    glm::vec2 GridToScreen(glm::ivec2 gp) const;

    struct ConnRecord { int from; int to; bool horiz; };
    struct IconEntry  { int roomIdx; std::shared_ptr<Util::GameObject> obj; };

    // 每個房間兩個方塊，互斥顯示
    std::vector<std::shared_ptr<Util::GameObject>> m_RoomDarkObjs;  // 玩家所在 → 深灰
    std::vector<std::shared_ptr<Util::GameObject>> m_RoomLitObjs;   // 已訪問   → 淺灰
    std::vector<std::shared_ptr<Util::GameObject>> m_CorrObjs;
    std::vector<IconEntry>                          m_Icons;

    // 游標：4 條黃色細條組成方框
    // [0]=上, [1]=下, [2]=左, [3]=右
    std::array<std::shared_ptr<Util::GameObject>, 4> m_CursorBars;

    std::vector<glm::ivec2>  m_GridPos;
    std::vector<ConnRecord>  m_Conns;
};
