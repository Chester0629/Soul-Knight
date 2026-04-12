#pragma once

#include "Room.hpp"  // RoomTemplate, Direction

#include <array>
#include <vector>

// 房間類型（決定敵人配置，M3.3 以後細化）
enum class RoomType { SPAWN, BASIC, EXTRA, CHEST, PORTAL, BOSS };

// 一個房間節點（地城佈局資訊）
struct RoomNode {
    glm::ivec2              gridPos;       // 5×5 網格座標，(2,2) 為中心
    RoomType                type;
    RoomTemplate            tmpl;
    std::array<bool, 4>     connections;   // [N, E, S, W] 是否有走廊連接

    RoomNode() : gridPos{0, 0}, type(RoomType::SPAWN),
                 tmpl(RoomTemplate::SPAWN), connections{false,false,false,false} {}
};

// 一條走廊連接（fromIdx → toIdx，dir 為 from 到 to 的方向）
struct DungeonConnection {
    int       fromIdx;
    int       toIdx;
    Direction dir;
};

// 完整地城佈局（純資料，不含 Tile/Renderer）
struct DungeonLayout {
    std::vector<RoomNode>        rooms;
    std::vector<DungeonConnection> connections;
};

// DungeonGenerator — 根據 seed 與層數生成地城佈局
// 佈局規則（已確認）：
//   普通層（1~4）：Spawn → 2 Basic → Portal
//   Boss 層（5）：Spawn → 3 Basic → Boss
//   輔助房間（Chest/Extra）以 50% 機率生成於 Basic 相鄰空格
class DungeonGenerator {
public:
    static DungeonLayout Generate(unsigned seed, int floorIndex);

    // 方向偏移（grid row/col delta）
    static const glm::ivec2 DIR_OFFSET[4];  // [N, E, S, W]

    static Direction Opposite(Direction d) {
        return static_cast<Direction>((static_cast<int>(d) + 2) % 4);
    }

private:
    static bool InBounds(glm::ivec2 pos);
};
