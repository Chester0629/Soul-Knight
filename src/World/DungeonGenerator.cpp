#include "World/DungeonGenerator.hpp"

#include <algorithm>
#include <random>
#include <unordered_set>

// ── 方向偏移定義 ──────────────────────────────────────────────────────────────
const glm::ivec2 DungeonGenerator::DIR_OFFSET[4] = {
    {-1,  0},  // NORTH（row 減少 = 世界 Y 增大）
    { 0, +1},  // EAST
    {+1,  0},  // SOUTH
    { 0, -1},  // WEST
};

// glm::ivec2 雜湊（供 unordered_set 使用）
struct IVec2Hash {
    size_t operator()(const glm::ivec2& v) const {
        return std::hash<int>()(v.x * 31 + v.y);
    }
};
struct IVec2Eq {
    bool operator()(const glm::ivec2& a, const glm::ivec2& b) const {
        return a.x == b.x && a.y == b.y;
    }
};

bool DungeonGenerator::InBounds(glm::ivec2 pos) {
    return pos.x >= 0 && pos.x <= 4 && pos.y >= 0 && pos.y <= 4;
}

// ── Generate ──────────────────────────────────────────────────────────────────
DungeonLayout DungeonGenerator::Generate(unsigned seed, int floorIndex) {
    std::mt19937 rng(seed);
    DungeonLayout layout;

    std::unordered_set<glm::ivec2, IVec2Hash, IVec2Eq> occupied;

    // ── 1. 初始房間（Spawn）─────────────────────────────────────────────────
    RoomNode spawn;
    spawn.gridPos = {2, 2};
    spawn.type    = RoomType::SPAWN;
    spawn.tmpl    = RoomTemplate::SPAWN;
    layout.rooms.push_back(spawn);
    occupied.insert({2, 2});

    // ── 2. 主線路徑 ──────────────────────────────────────────────────────────
    static const RoomTemplate MONSTER_TMPLS[] = {
        RoomTemplate::SMALL, RoomTemplate::MEDIUM,
        RoomTemplate::LARGE, RoomTemplate::WIDE
    };
    constexpr int TMPL_CNT = 4;

    const int numBasic = (floorIndex == 5) ? 3 : 2;

    int       prevIdx = 0;          // 上一個房間的索引
    glm::ivec2 cur    = {2, 2};
    Direction prevDir = Direction::NORTH;  // 初始無意義，hasPrev 控制
    bool hasPrev      = false;

    // 取可行方向：avoidReverse=true 時排除反向，若無路則自動 fallback 允許反向
    auto getValidDirs = [&](glm::ivec2 from) -> std::vector<Direction> {
        std::vector<Direction> valid;
        for (int d = 0; d < 4; d++) {
            Direction dir = static_cast<Direction>(d);
            if (hasPrev && dir == Opposite(prevDir)) continue;  // 避免回頭
            glm::ivec2 next = from + DIR_OFFSET[d];
            if (!InBounds(next) || occupied.count(next)) continue;
            valid.push_back(dir);
        }
        // fallback：允許所有未佔用且在範圍內的方向（包含回頭）
        if (valid.empty()) {
            for (int d = 0; d < 4; d++) {
                Direction dir = static_cast<Direction>(d);
                glm::ivec2 next = from + DIR_OFFSET[d];
                if (!InBounds(next) || occupied.count(next)) continue;
                valid.push_back(dir);
            }
        }
        return valid;
    };

    for (int i = 0; i < numBasic; i++) {
        auto valid = getValidDirs(cur);
        if (valid.empty()) break;

        Direction chosen = valid[rng() % valid.size()];
        glm::ivec2 next  = cur + DIR_OFFSET[static_cast<int>(chosen)];

        RoomNode basic;
        basic.gridPos = next;
        basic.type    = RoomType::BASIC;
        basic.tmpl    = MONSTER_TMPLS[rng() % TMPL_CNT];
        const int basicIdx = static_cast<int>(layout.rooms.size());
        layout.rooms.push_back(basic);
        occupied.insert(next);

        DungeonConnection conn;
        conn.fromIdx = prevIdx;
        conn.toIdx   = basicIdx;
        conn.dir     = chosen;
        layout.connections.push_back(conn);
        layout.rooms[prevIdx].connections[static_cast<int>(chosen)]          = true;
        layout.rooms[basicIdx].connections[static_cast<int>(Opposite(chosen))] = true;

        prevIdx = basicIdx;
        prevDir = chosen;
        hasPrev = true;
        cur     = next;
    }

    // ── 3. 終點房間（Portal / Boss）─────────────────────────────────────────
    {
        auto valid = getValidDirs(cur);
        if (!valid.empty()) {
            Direction chosen = valid[rng() % valid.size()];
            glm::ivec2 next  = cur + DIR_OFFSET[static_cast<int>(chosen)];

            RoomNode end;
            end.gridPos = next;
            end.type    = (floorIndex == 5) ? RoomType::BOSS : RoomType::PORTAL;
            end.tmpl    = RoomTemplate::SPAWN;  // Portal/Boss 房間使用與 Spawn 相同尺寸
            const int endIdx = static_cast<int>(layout.rooms.size());
            layout.rooms.push_back(end);
            occupied.insert(next);

            DungeonConnection conn;
            conn.fromIdx = prevIdx;
            conn.toIdx   = endIdx;
            conn.dir     = chosen;
            layout.connections.push_back(conn);
            layout.rooms[prevIdx].connections[static_cast<int>(chosen)]        = true;
            layout.rooms[endIdx].connections[static_cast<int>(Opposite(chosen))] = true;
        }
    }

    // ── 4. 輔助房間（Chest / Extra，50% 機率）──────────────────────────────
    const int mainCount = static_cast<int>(layout.rooms.size());
    for (int i = 0; i < mainCount; i++) {
        if (layout.rooms[i].type != RoomType::BASIC) continue;
        if ((rng() % 2) != 0) continue;  // 50% 機率

        const glm::ivec2 basePos = layout.rooms[i].gridPos;
        std::vector<std::pair<Direction, glm::ivec2>> freeAdj;
        for (int d = 0; d < 4; d++) {
            Direction dir = static_cast<Direction>(d);
            glm::ivec2 adj = basePos + DIR_OFFSET[d];
            if (!InBounds(adj) || occupied.count(adj)) continue;
            freeAdj.push_back({dir, adj});
        }
        if (freeAdj.empty()) continue;

        const auto& [dir, pos] = freeAdj[rng() % freeAdj.size()];
        const bool isChest     = (rng() % 2) == 0;

        RoomNode aux;
        aux.gridPos = pos;
        aux.type    = isChest ? RoomType::CHEST : RoomType::EXTRA;
        aux.tmpl    = MONSTER_TMPLS[rng() % TMPL_CNT];
        const int auxIdx = static_cast<int>(layout.rooms.size());
        layout.rooms.push_back(aux);
        occupied.insert(pos);

        DungeonConnection conn;
        conn.fromIdx = i;
        conn.toIdx   = auxIdx;
        conn.dir     = dir;
        layout.connections.push_back(conn);
        layout.rooms[i].connections[static_cast<int>(dir)]              = true;
        layout.rooms[auxIdx].connections[static_cast<int>(Opposite(dir))] = true;
    }

    return layout;
}
