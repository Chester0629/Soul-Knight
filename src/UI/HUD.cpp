#include "UI/HUD.hpp"

static constexpr const char* FONT = PTSD_ASSETS_DIR "/fonts/Inter.ttf";
static constexpr int FONT_SIZE = 15;

// ── 輔助 ─────────────────────────────────────────────────────────────────────

std::shared_ptr<Util::GameObject> HUD::MakeImage(
    const std::string& path, float sx, float sy,
    float cx, float cy, float z)
{
    auto obj = std::make_shared<Util::GameObject>();
    obj->SetDrawable(std::make_shared<Util::Image>(path));
    obj->m_Transform.scale       = {sx, sy};
    obj->m_Transform.translation = {cx, cy};
    obj->SetZIndex(z);
    obj->SetVisible(true);
    return obj;
}

std::shared_ptr<Util::GameObject> HUD::MakeTextObj(
    const std::shared_ptr<Util::Text>& text,
    float cx, float cy, float z)
{
    auto obj = std::make_shared<Util::GameObject>();
    obj->SetDrawable(text);
    obj->m_Transform.scale       = {1.0f, 1.0f};
    obj->m_Transform.translation = {cx, cy};
    obj->SetZIndex(z);
    obj->SetVisible(true);
    return obj;
}

// 更新填充條寬度，左對齊於 BAR_LEFT
void HUD::UpdateFill(Util::GameObject* fill, float ratio, float fillWNat)
{
    const float fillW  = BAR_W * ratio;
    const float scaleX = (ratio > 0.001f) ? fillW / fillWNat : 0.001f;
    fill->m_Transform.scale.x       = scaleX;
    fill->m_Transform.translation.x = BAR_LEFT + fillW * 0.5f;
}

// ── 建構 ─────────────────────────────────────────────────────────────────────

HUD::HUD() {
    const Util::Color WHITE{255, 255, 255, 255};
    const float textX = BAR_LEFT + BAR_W * 0.5f;  // 文字疊加於 bar 中央

    // 面板背景（等比縮放）
    m_Panel = MakeImage(
        RESOURCE_DIR "/UI/ui_15.png",
        SCALE, SCALE,
        PANEL_CX, PANEL_CY, Z_PANEL);

    // ── HP 填充條 ─────────────────────────────────────────────────────────
    m_HpFill = MakeImage(
        RESOURCE_DIR "/UI/ui_h.png",
        0.001f, BAR_FILL_H / UH_H,   // scaleY = (9×SCALE)/48
        BAR_LEFT, HP_Y, Z_FILL);

    m_HpText    = std::make_shared<Util::Text>(FONT, FONT_SIZE, "?/?", WHITE);
    m_HpTextObj = MakeTextObj(m_HpText, textX, HP_Y, Z_TEXT);

    // ── 護盾填充條 ────────────────────────────────────────────────────────
    m_ShieldFill = MakeImage(
        RESOURCE_DIR "/UI/ui_s.png",
        0.001f, BAR_FILL_H / US_H,   // scaleY = (9×SCALE)/47
        BAR_LEFT, SH_Y, Z_FILL);

    m_ShieldText    = std::make_shared<Util::Text>(FONT, FONT_SIZE, "?/?", WHITE);
    m_ShieldTextObj = MakeTextObj(m_ShieldText, textX, SH_Y, Z_TEXT);

    // ── 能量填充條 ────────────────────────────────────────────────────────
    m_EnergyFill = MakeImage(
        RESOURCE_DIR "/UI/ui_e.png",
        0.001f, BAR_FILL_H / UE_H,   // scaleY = (9×SCALE)/48
        BAR_LEFT, EN_Y, Z_FILL);

    m_EnergyText    = std::make_shared<Util::Text>(FONT, FONT_SIZE, "?/?", WHITE);
    m_EnergyTextObj = MakeTextObj(m_EnergyText, textX, EN_Y, Z_TEXT);
}

// ── AddToRenderer ─────────────────────────────────────────────────────────────

void HUD::AddToRenderer(Util::Renderer& root) {
    root.AddChild(m_Panel);
    root.AddChild(m_HpFill);
    root.AddChild(m_ShieldFill);
    root.AddChild(m_EnergyFill);
    root.AddChild(m_HpTextObj);
    root.AddChild(m_ShieldTextObj);
    root.AddChild(m_EnergyTextObj);
}

// ── Update ────────────────────────────────────────────────────────────────────

void HUD::Update(int hp,     int maxHP,
                 int shield, int maxShield,
                 int energy, int maxEnergy)
{
    auto clampRatio = [](int val, int maxVal) -> float {
        if (maxVal <= 0) return 0.0f;
        return glm::clamp(static_cast<float>(val) / static_cast<float>(maxVal),
                          0.0f, 1.0f);
    };

    static constexpr float TEXT_CX = BAR_LEFT + BAR_W * 0.5f;

    // HP
    UpdateFill(m_HpFill.get(), clampRatio(hp, maxHP), UH_W);
    m_HpText->SetText(std::to_string(hp) + "/" + std::to_string(maxHP));
    m_HpTextObj->m_Transform.translation.x = TEXT_CX;

    // 護盾
    UpdateFill(m_ShieldFill.get(), clampRatio(shield, maxShield), US_W);
    m_ShieldText->SetText(std::to_string(shield) + "/" + std::to_string(maxShield));
    m_ShieldTextObj->m_Transform.translation.x = TEXT_CX;

    // 能量
    UpdateFill(m_EnergyFill.get(), clampRatio(energy, maxEnergy), UE_W);
    m_EnergyText->SetText(std::to_string(energy) + "/" + std::to_string(maxEnergy));
    m_EnergyTextObj->m_Transform.translation.x = TEXT_CX;
}
