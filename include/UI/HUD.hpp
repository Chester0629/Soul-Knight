#pragma once

#include "Util/Color.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Util/Renderer.hpp"
#include "Util/Text.hpp"
#include "pch.hpp"

#include <memory>
#include <string>

// HUD — 左上角能力面板
//
// 背景：ui_15.png（79×39）
//   - col 6-12：圓形 icon（紅/灰/藍），3 排對應 HP / 護盾 / 能量
//   - col 15-76：bar 空槽（62px natural），縮放至 BAR_W=340px
//   - row 8 / 18 / 28：三排 bar 中心（natural coords）
//
// Fill bar 素材：
//   ui_h.png (12×48) 紅色 HP
//   ui_s.png (12×47) 灰色護盾
//   ui_e.png  (6×48) 藍色能量
class HUD {
public:
    HUD();

    void AddToRenderer(Util::Renderer& root);
    void Update(int hp, int maxHP, int shield, int maxShield,
                int energy, int maxEnergy);

    static constexpr float Z_PANEL = 99.50f;
    static constexpr float Z_FILL  = 99.51f;
    static constexpr float Z_TEXT  = 99.52f;

private:
    // ── ui_15.png 自然尺寸 ───────────────────────────────────────────────
    static constexpr float UI15_W_NAT   = 79.0f;
    static constexpr float UI15_H_NAT   = 39.0f;
    static constexpr float UI15_BAR_COL = 16.0f;   // bar 從第 15 列開始
    static constexpr float BAR_NAT_W    = 58.0f;   // bar 自然寬度
    static constexpr float BAR_NAT_H    =  8.5f;   // bar 自然高度（每排 slot）

    // ── 使用者指定 bar 渲染寬度，推導唯一縮放比例（等比） ───────────────
    static constexpr float BAR_W        = 174.0f;
    static constexpr float SCALE        = BAR_W / BAR_NAT_W;  // ≈5.690

    // ── ui_15 等比縮放後的面板尺寸 ──────────────────────────────────────
    static constexpr float PANEL_W      = UI15_W_NAT * SCALE;  // ≈449
    static constexpr float PANEL_H      = UI15_H_NAT * SCALE;  // ≈222

    // ── 螢幕錨點 ─────────────────────────────────────────────────────────
    static constexpr float LEFT = -640.0f + 6.0f;   // -634
    static constexpr float TOP  =  360.0f - 6.0f;   //  354

    static constexpr float PANEL_CX = LEFT + PANEL_W * 0.5f;
    static constexpr float PANEL_CY = TOP  - PANEL_H * 0.5f;

    // ── Bar 左邊緣 & 三排 Y 中心（依 SCALE 推導） ───────────────────────
    static constexpr float BAR_LEFT = LEFT + UI15_BAR_COL * SCALE;

    static constexpr float HP_Y = TOP -  8.0f * SCALE;   // row 8
    static constexpr float SH_Y = TOP - 18.0f * SCALE;   // row 18
    static constexpr float EN_Y = TOP - 28.0f * SCALE;   // row 28

    // ── Fill bar 渲染高度 = bar 自然高度 × SCALE ─────────────────────────
    static constexpr float BAR_FILL_H = BAR_NAT_H * SCALE;   // ≈51

    // ── Fill bar 素材自然尺寸 ─────────────────────────────────────────────
    static constexpr float UH_W = 12.0f, UH_H = 47.0f;   // ui_h.png
    static constexpr float US_W = 12.0f, US_H = 47.0f;   // ui_s.png
    static constexpr float UE_W = 12.0f, UE_H = 47.0f;   // ui_e.png

    // ── GameObjects ──────────────────────────────────────────────────────
    std::shared_ptr<Util::GameObject> m_Panel;

    std::shared_ptr<Util::GameObject> m_HpFill;
    std::shared_ptr<Util::GameObject> m_ShieldFill;
    std::shared_ptr<Util::GameObject> m_EnergyFill;

    std::shared_ptr<Util::Text>       m_HpText;
    std::shared_ptr<Util::GameObject> m_HpTextObj;
    std::shared_ptr<Util::Text>       m_ShieldText;
    std::shared_ptr<Util::GameObject> m_ShieldTextObj;
    std::shared_ptr<Util::Text>       m_EnergyText;
    std::shared_ptr<Util::GameObject> m_EnergyTextObj;

    // ── 輔助 ─────────────────────────────────────────────────────────────
    static std::shared_ptr<Util::GameObject> MakeImage(
        const std::string& path, float sx, float sy,
        float cx, float cy, float z);

    static std::shared_ptr<Util::GameObject> MakeTextObj(
        const std::shared_ptr<Util::Text>& text,
        float cx, float cy, float z);

    static void UpdateFill(Util::GameObject* fill, float ratio,
                           float fillWNat);
};
