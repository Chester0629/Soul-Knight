#pragma once

#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Util/Renderer.hpp"
#include "pch.hpp"

#include <memory>

// HUD — 三條橫條全部放在 ui_15 面板內
//
// 版面（Y 向上為正）：
//   ui_15 面板中心 = (HP_CX, HP_CY)
//   護盾填充 Y = HP_CY + 10
//   血量填充 Y = HP_CY
//   能量填充 Y = HP_CY - 10
//
// 所有填充左邊緣對齊 BAR_LEFT_X（面板橫條區起點），寬度 = BAR_W
class HUD {
public:
    HUD();

    void AddToRenderer(Util::Renderer& root);
    void Update(int hp, int maxHP, int shield, int maxShield,
                int energy, int maxEnergy);

    static constexpr float Z_INDEX = 99.5f;
    static constexpr float Z_FILL  = 99.51f;

private:
    // ─── ui_15.png（79×39）scale {3,3} ──────────────────────────────────────
    static constexpr float HP_SCALE  = 4.0f;
    static constexpr float HP_W_NAT  = 79.0f;
    static constexpr float HP_H_NAT  = 39.0f;
    static constexpr float HP_W      = HP_W_NAT * HP_SCALE;   // 237
    static constexpr float HP_H      = HP_H_NAT * HP_SCALE;   // 117

    // 左上角距螢幕邊界 6px
    static constexpr float LEFT_EDGE = -640.0f + 6.0f;        // -634
    static constexpr float TOP_EDGE  =  360.0f - 6.0f;        //  354

    // 面板中心
    static constexpr float HP_CX = LEFT_EDGE + HP_W * 0.5f;   // -515.5
    static constexpr float HP_CY = TOP_EDGE  - HP_H * 0.5f;   //  295.5

    // 圓形 icon 佔左側 ~49%（≈ 39px 方形）
    static constexpr float ICON_FRAC  = 0.494f;
    static constexpr float BAR_LEFT_X = LEFT_EDGE + HP_W * ICON_FRAC; // ≈-517
    static constexpr float BAR_W      = HP_W * (1.0f - ICON_FRAC);    // ≈120

    // 三條 bar 的 Y 偏移（在面板內）
    static constexpr float Y_HP     = HP_CY + 45.0f;
    static constexpr float Y_SHIELD = HP_CY + 5.0f;
    static constexpr float Y_ENERGY = HP_CY - 35.0f;

    // 每條 bar 渲染高度目標 ≈ 10px，各圖原始高度不同
    // hp_fill.png (64×16)、shield_fill.png (64×16)、ui_27.png (58×32)
    static constexpr float HP_FILL_W_NAT = 64.0f;
    static constexpr float HP_FILL_H_NAT = 16.0f;
    static constexpr float SH_FILL_W_NAT = 64.0f;
    static constexpr float SH_FILL_H_NAT = 16.0f;
    static constexpr float EN_FILL_W_NAT = 58.0f;
    static constexpr float EN_FILL_H_NAT = 32.0f;

    static constexpr float TARGET_BAR_H  = 25.0f;        // 目標渲染高度（10 × 2.5）
    static constexpr float FILL_BAR_W   = BAR_W * 1.45f; // 填充寬度統一（BAR_W × 3.5）
    static constexpr float FILL_X_OFFSET = -90.0f;      // 填充往左偏移 100px

    // ─── GameObjects ─────────────────────────────────────────────────────────
    std::shared_ptr<Util::GameObject> m_HpPanel;

    std::shared_ptr<Util::GameObject> m_HpFill;
    std::shared_ptr<Util::GameObject> m_ShieldFill;
    std::shared_ptr<Util::GameObject> m_EnergyFill;

    // 左對齊填充：從 BAR_LEFT_X 開始，依 ratio 向右延伸
    static void UpdateFill(Util::GameObject* fill,
                           float barW, float fillWNat, float ratio);
};
