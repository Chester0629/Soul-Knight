#include "UI/HUD.hpp"

static std::shared_ptr<Util::GameObject> MakeObj(
    const std::string& path,
    float scaleX, float scaleY,
    float cx, float cy, float z)
{
    auto obj = std::make_shared<Util::GameObject>();
    obj->SetDrawable(std::make_shared<Util::Image>(path));
    obj->m_Transform.scale       = {scaleX, scaleY};
    obj->m_Transform.translation = {cx, cy};
    obj->SetZIndex(z);
    obj->SetVisible(true);
    return obj;
}

HUD::HUD() {
    // ─── ui_15 面板背景 ───────────────────────────────────────────────────────
    m_HpPanel = MakeObj(RESOURCE_DIR "/UI/ui_15.png",
                        HP_SCALE, HP_SCALE,
                        HP_CX, HP_CY, Z_INDEX);

    // ─── 三條填充（全在面板橫條區域內）──────────────────────────────────────
    // scale_y = TARGET_BAR_H / 原始高度
    const float hpScaleY = TARGET_BAR_H / HP_FILL_H_NAT;   // 10/16 = 0.625
    const float shScaleY = TARGET_BAR_H / SH_FILL_H_NAT;   // 10/16 = 0.625
    const float enScaleY = TARGET_BAR_H / EN_FILL_H_NAT;   // 10/32 = 0.3125

    // 初始 scale.x = 滿格（ratio=1），translation.x 後續由 Update 決定
    m_HpFill = MakeObj(RESOURCE_DIR "/UI/hp_fill.png",
                       FILL_BAR_W / HP_FILL_W_NAT, hpScaleY,
                       BAR_LEFT_X + FILL_X_OFFSET + FILL_BAR_W * 0.5f, Y_HP, Z_FILL);

    m_ShieldFill = MakeObj(RESOURCE_DIR "/UI/shield_fill.png",
                           FILL_BAR_W / SH_FILL_W_NAT, shScaleY,
                           BAR_LEFT_X + FILL_X_OFFSET + FILL_BAR_W * 0.5f, Y_SHIELD, Z_FILL);

    m_EnergyFill = MakeObj(RESOURCE_DIR "/UI/ui_27.png",
                           FILL_BAR_W / EN_FILL_W_NAT, enScaleY,
                           BAR_LEFT_X + FILL_X_OFFSET + FILL_BAR_W * 0.5f, Y_ENERGY, Z_FILL);
}

void HUD::AddToRenderer(Util::Renderer& root) {
    root.AddChild(m_HpPanel);
    root.AddChild(m_HpFill);
    root.AddChild(m_ShieldFill);
    root.AddChild(m_EnergyFill);
}

// 左對齊：填充從 (BAR_LEFT_X + FILL_X_OFFSET) 往右延伸 barW * ratio
void HUD::UpdateFill(Util::GameObject* fill,
                     float barW, float fillWNat, float ratio)
{
    const float fillW  = barW * ratio;
    const float scaleX = (ratio > 0.001f) ? fillW / fillWNat : 0.001f;
    fill->m_Transform.scale.x       = scaleX;
    fill->m_Transform.translation.x = BAR_LEFT_X + FILL_X_OFFSET + fillW * 0.5f;
    // fillW = barW * ratio，barW 已是 FILL_BAR_W（放大後寬度）
}

void HUD::Update(int hp,     int maxHP,
                 int shield, int maxShield,
                 int energy, int maxEnergy)
{
    const float hpRatio = (maxHP     > 0) ? glm::clamp((float)hp     / maxHP,     0.f, 1.f) : 0.f;
    const float shRatio = (maxShield > 0) ? glm::clamp((float)shield / maxShield, 0.f, 1.f) : 0.f;
    const float enRatio = (maxEnergy > 0) ? glm::clamp((float)energy / maxEnergy, 0.f, 1.f) : 0.f;

    UpdateFill(m_HpFill.get(),     FILL_BAR_W, HP_FILL_W_NAT, hpRatio);
    UpdateFill(m_ShieldFill.get(), FILL_BAR_W, SH_FILL_W_NAT, shRatio);
    UpdateFill(m_EnergyFill.get(), FILL_BAR_W, EN_FILL_W_NAT, enRatio);
}
