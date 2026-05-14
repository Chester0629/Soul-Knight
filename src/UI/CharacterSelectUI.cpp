#include "UI/CharacterSelectUI.hpp"

#include "Util/Color.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"
#include "Util/Time.hpp"

static constexpr const char* FONT = PTSD_ASSETS_DIR "/fonts/Inter.ttf";

// ── Static character data ─────────────────────────────────────────────────────

const std::array<CharacterSelectUI::CharData, 3> CharacterSelectUI::kChars = {{
    {
        "Knight", "Dual Wield",
        "Use two guns simultaneously\nfor a short time (K key)",
        RESOURCE_DIR "/UI/ui_skill01.png",
        {RESOURCE_DIR "/Characters/c01_4.png",
         RESOURCE_DIR "/Characters/c01_5.png",
         RESOURCE_DIR "/Characters/c01_6.png",
         RESOURCE_DIR "/Characters/c01_7.png"},
        6, 7, 5, 6, 180, 200, 5, 2
    },
    {
        "Rogue", "Dodge Roll",
        "Roll in last move direction\nBrief invincibility (5s CD, K key)",
        RESOURCE_DIR "/UI/ui_skill02.png",
        {RESOURCE_DIR "/Characters/c02_4.png",
         RESOURCE_DIR "/Characters/c02_5.png",
         RESOURCE_DIR "/Characters/c02_6.png",
         RESOURCE_DIR "/Characters/c02_7.png"},
        5, 6, 3, 4, 180, 200, 10, 3
    },
    {
        "Wizard", "Magic Circle",
        "Deploy a healing zone\n+1 HP/s inside for 5s (15s CD, K key)",
        RESOURCE_DIR "/UI/ui_skill03.png",
        {RESOURCE_DIR "/Characters/c03_4.png",
         RESOURCE_DIR "/Characters/c03_5.png",
         RESOURCE_DIR "/Characters/c03_6.png",
         RESOURCE_DIR "/Characters/c03_7.png"},
        3, 4, 5, 6, 240, 260, 0, 5
    }
}};

// ── Build helpers ─────────────────────────────────────────────────────────────

static std::shared_ptr<Util::GameObject> MakeImg(
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

static std::shared_ptr<Util::GameObject> MakeTxt(
    const std::shared_ptr<Util::Text>& t,
    float cx, float cy, float z)
{
    auto obj = std::make_shared<Util::GameObject>();
    obj->SetDrawable(t);
    obj->m_Transform.scale       = {1.0f, 1.0f};
    obj->m_Transform.translation = {cx, cy};
    obj->SetZIndex(z);
    obj->SetVisible(true);
    return obj;
}

// ── Init ──────────────────────────────────────────────────────────────────────

void CharacterSelectUI::Init(
    Util::Renderer& root,
    int crystals,
    const std::array<bool,3>& upgraded)
{
    m_Root     = &root;
    m_FocusIdx = 0;
    m_Upgraded = upgraded;

    // ── Background ──────────────────────────────────────────────────────────
    m_Bg = MakeImg(RESOURCE_DIR "/UI/common_room_bg.png",
                   3.03f, 3.03f, 0.0f, 0.0f, 98.0f);
    root.AddChild(m_Bg);

    // ── Banner ──────────────────────────────────────────────────────────────
    m_BannerObj = MakeImg(RESOURCE_DIR "/UI/ui_1.png",
                          8.0f, 4.0f, 0.0f, 295.0f, 99.3f);
    root.AddChild(m_BannerObj);

    // ── Title text ──────────────────────────────────────────────────────────
    m_TitleText = std::make_shared<Util::Text>(
        FONT, 26, "Choose Hero", Util::Color{255, 230, 180, 255});
    m_TitleTextObj = MakeTxt(m_TitleText, -50.0f, 293.0f, 99.4f);
    root.AddChild(m_TitleTextObj);

    // ── Crystal display (top-right) ──────────────────────────────────────────
    m_CrystalIconObj = MakeImg(RESOURCE_DIR "/UI/ui_52.png",
                                3.0f, 3.0f, 530.0f, 293.0f, 99.4f);
    root.AddChild(m_CrystalIconObj);

    m_CrystalText = std::make_shared<Util::Text>(
        FONT, 22, std::to_string(crystals), Util::Color{100, 220, 255, 255});
    m_CrystalTextObj = MakeTxt(m_CrystalText, 570.0f, 293.0f, 99.4f);
    root.AddChild(m_CrystalTextObj);

    // ── Stat icons (left column, fixed) ─────────────────────────────────────
    const std::array<const char*, 4> iconPaths = {
        RESOURCE_DIR "/UI/ui_33.png",  // HP
        RESOURCE_DIR "/UI/ui_34.png",  // Shield
        RESOURCE_DIR "/UI/ui_35.png",  // Energy
        RESOURCE_DIR "/UI/ui_37.png"   // Crit
    };
    const float statIconX = -460.0f;
    const float statStartY =  130.0f;
    const float statStepY  = -70.0f;

    for (int i = 0; i < 4; ++i) {
        m_StatIconObjs[i] = MakeImg(iconPaths[i],
                                     3.5f, 3.5f,
                                     statIconX,
                                     statStartY + i * statStepY,
                                     99.5f);
        root.AddChild(m_StatIconObjs[i]);

        m_StatTexts[i] = std::make_shared<Util::Text>(
            FONT, 22, "0", Util::Color{255, 255, 255, 255});
        m_StatTextObjs[i] = MakeTxt(m_StatTexts[i],
                                     statIconX + 50.0f,
                                     statStartY + i * statStepY,
                                     99.5f);
        root.AddChild(m_StatTextObjs[i]);
    }

    // ── Portrait frame (center) ───────────────────────────────────────────────
    m_PortraitFrame = MakeImg(RESOURCE_DIR "/UI/ui_38.png",
                               6.0f, 6.0f, 0.0f, 60.0f, 99.4f);
    root.AddChild(m_PortraitFrame);

    // ── Character animation (center) — placeholder, RefreshDynamic sets it ──
    m_CharAnim = std::make_shared<Util::Animation>(
        std::vector<std::string>{
            kChars[0].idleFrames[0], kChars[0].idleFrames[1],
            kChars[0].idleFrames[2], kChars[0].idleFrames[3]},
        true, 150, true, 0);
    m_CharObj = std::make_shared<Util::GameObject>();
    m_CharObj->SetDrawable(m_CharAnim);
    m_CharObj->m_Transform.scale       = {6.0f, 6.0f};
    m_CharObj->m_Transform.translation = {0.0f, 65.0f};
    m_CharObj->SetZIndex(99.45f);
    m_CharObj->SetVisible(true);
    root.AddChild(m_CharObj);

    // ── Stars (5 below portrait) ──────────────────────────────────────────────
    const float starY       = -100.0f;
    const float starSpacing =   50.0f;
    const float starStart   = -2.0f * starSpacing; // 5 stars centred
    for (int i = 0; i < 5; ++i) {
        m_StarObjs[i] = MakeImg(RESOURCE_DIR "/UI/ui_47.png",
                                 4.0f, 4.0f,
                                 starStart + i * starSpacing,
                                 starY, 99.45f);
        root.AddChild(m_StarObjs[i]);
    }

    // ── Skill icon (right column) ─────────────────────────────────────────────
    m_SkillIconImg = std::make_shared<Util::Image>(kChars[0].skillIconPath);
    m_SkillIconObj = std::make_shared<Util::GameObject>();
    m_SkillIconObj->SetDrawable(m_SkillIconImg);
    m_SkillIconObj->m_Transform.scale       = {4.0f, 4.0f};
    m_SkillIconObj->m_Transform.translation = {330.0f, 120.0f};
    m_SkillIconObj->SetZIndex(99.5f);
    m_SkillIconObj->SetVisible(true);
    root.AddChild(m_SkillIconObj);

    // ── Skill name ────────────────────────────────────────────────────────────
    m_SkillNameText = std::make_shared<Util::Text>(
        FONT, 24, kChars[0].skillName, Util::Color{255, 230, 100, 255});
    m_SkillNameObj = MakeTxt(m_SkillNameText, 380.0f, 50.0f, 99.5f);
    root.AddChild(m_SkillNameObj);

    // ── Skill description ─────────────────────────────────────────────────────
    m_SkillDescText = std::make_shared<Util::Text>(
        FONT, 18, kChars[0].skillDesc, Util::Color{200, 200, 200, 255});
    m_SkillDescObj = MakeTxt(m_SkillDescText, 380.0f, -30.0f, 99.5f);
    root.AddChild(m_SkillDescObj);

    // ── Upgrade button ────────────────────────────────────────────────────────
    m_BtnUpgradeImg = std::make_shared<Util::Image>(RESOURCE_DIR "/UI/ui_28.png");
    m_BtnUpgradeObj = std::make_shared<Util::GameObject>();
    m_BtnUpgradeObj->SetDrawable(m_BtnUpgradeImg);
    m_BtnUpgradeObj->m_Transform.scale       = {4.5f, 4.5f};
    m_BtnUpgradeObj->m_Transform.translation = {BTN_UPG_X, BTN_UPG_Y};
    m_BtnUpgradeObj->SetZIndex(99.5f);
    m_BtnUpgradeObj->SetVisible(true);
    root.AddChild(m_BtnUpgradeObj);

    m_BtnUpgradeText = std::make_shared<Util::Text>(
        FONT, 22, "Upgrade", Util::Color{30, 15, 0, 255});
    m_BtnUpgradeTextObj = MakeTxt(m_BtnUpgradeText,
                                   BTN_UPG_X - 80.0f, BTN_UPG_Y, 99.51f);
    root.AddChild(m_BtnUpgradeTextObj);

    m_BtnCrystalIconObj = MakeImg(RESOURCE_DIR "/UI/ui_52.png",
                                   2.5f, 2.5f,
                                   BTN_UPG_X + 55.0f, BTN_UPG_Y, 99.51f);
    root.AddChild(m_BtnCrystalIconObj);

    m_BtnCostText = std::make_shared<Util::Text>(
        FONT, 22, "500", Util::Color{30, 15, 0, 255});
    m_BtnCostTextObj = MakeTxt(m_BtnCostText,
                                BTN_UPG_X + 90.0f, BTN_UPG_Y, 99.51f);
    root.AddChild(m_BtnCostTextObj);

    // ── Navigation arrows ──────────────────────────────────────────────────────
    m_ArrowLeft = MakeImg(RESOURCE_DIR "/UI/ui_24.png",
                          -7.0f, 7.0f, -600.0f, 0.0f, 99.5f); // neg x scale = flip
    m_ArrowRight = MakeImg(RESOURCE_DIR "/UI/ui_24.png",
                            7.0f, 7.0f,  600.0f, 0.0f, 99.5f);
    root.AddChild(m_ArrowLeft);
    root.AddChild(m_ArrowRight);

    // ── Character name ────────────────────────────────────────────────────────
    m_CharNameText = std::make_shared<Util::Text>(
        FONT, 26, kChars[0].name, Util::Color{255, 255, 255, 255});
    m_CharNameObj = MakeTxt(m_CharNameText, 0.0f, -295.0f, 99.5f);
    root.AddChild(m_CharNameObj);

    // ── Initial refresh ───────────────────────────────────────────────────────
    RefreshDynamic();
    SetUpgradeButtonState(crystals);
}

// ── RefreshDynamic ────────────────────────────────────────────────────────────

void CharacterSelectUI::RefreshDynamic() {
    const CharData& cd = kChars[m_FocusIdx];
    const bool upg = m_Upgraded[m_FocusIdx];

    // Stats
    const int hp     = upg ? cd.upgradeHP     : cd.baseHP;
    const int shield = upg ? cd.upgradeShield : cd.baseShield;
    const int energy = upg ? cd.upgradeEnergy : cd.baseEnergy;
    const int crit   = cd.crit;

    m_StatTexts[0]->SetText(std::to_string(hp));
    m_StatTexts[1]->SetText(std::to_string(shield));
    m_StatTexts[2]->SetText(std::to_string(energy));
    m_StatTexts[3]->SetText(std::to_string(crit) + "%");

    // Character animation
    m_CharAnim = std::make_shared<Util::Animation>(
        std::vector<std::string>{
            cd.idleFrames[0], cd.idleFrames[1],
            cd.idleFrames[2], cd.idleFrames[3]},
        true, 150, true, 0);
    m_CharObj->SetDrawable(m_CharAnim);

    // Skill icon
    m_SkillIconImg = std::make_shared<Util::Image>(cd.skillIconPath);
    m_SkillIconObj->SetDrawable(m_SkillIconImg);

    // Skill texts
    m_SkillNameText->SetText(cd.skillName);
    m_SkillDescText->SetText(cd.skillDesc);

    // Difficulty stars
    for (int i = 0; i < 5; ++i) {
        const bool filled = (i < cd.stars);
        const std::string starPath = filled
            ? RESOURCE_DIR "/UI/ui_47.png"
            : RESOURCE_DIR "/UI/star_empty.png";
        m_StarObjs[i]->SetDrawable(std::make_shared<Util::Image>(starPath));
    }

    // Character name
    m_CharNameText->SetText(cd.name);
}

// ── SetUpgradeButtonState ─────────────────────────────────────────────────────

void CharacterSelectUI::SetUpgradeButtonState(int crystals) {
    const bool alreadyUpgraded = m_Upgraded[m_FocusIdx];
    const bool canAfford       = (crystals >= 500);

    if (alreadyUpgraded) {
        m_BtnUpgradeObj->SetDrawable(
            std::make_shared<Util::Image>(RESOURCE_DIR "/UI/ui_btn1.png"));
        m_BtnUpgradeObj->m_Transform.scale = {16.0f, 10.0f}; // stretch to match
        m_BtnUpgradeText->SetText("Upgraded");
        m_BtnUpgradeText->SetColor(Util::Color{120, 120, 120, 200});
        m_BtnCostTextObj->SetVisible(false);
        m_BtnCrystalIconObj->SetVisible(false);
    } else if (!canAfford) {
        m_BtnUpgradeObj->SetDrawable(
            std::make_shared<Util::Image>(RESOURCE_DIR "/UI/ui_btn1.png"));
        m_BtnUpgradeObj->m_Transform.scale = {16.0f, 10.0f};
        m_BtnUpgradeText->SetText("Upgrade");
        m_BtnUpgradeText->SetColor(Util::Color{100, 100, 100, 180});
        m_BtnCostTextObj->SetVisible(true);
        m_BtnCrystalIconObj->SetVisible(true);
    } else {
        m_BtnUpgradeObj->SetDrawable(
            std::make_shared<Util::Image>(RESOURCE_DIR "/UI/ui_28.png"));
        m_BtnUpgradeObj->m_Transform.scale = {4.5f, 4.5f};
        m_BtnUpgradeText->SetText("Upgrade");
        m_BtnUpgradeText->SetColor(Util::Color{30, 15, 0, 255});
        m_BtnCostTextObj->SetVisible(true);
        m_BtnCrystalIconObj->SetVisible(true);
    }
}

// ── HitTest / CursorToWorld ───────────────────────────────────────────────────

bool CharacterSelectUI::HitTest(glm::vec2 wp, glm::vec2 center, glm::vec2 halfSize) {
    return std::abs(wp.x - center.x) <= halfSize.x &&
           std::abs(wp.y - center.y) <= halfSize.y;
}

glm::vec2 CharacterSelectUI::CursorToWorld() {
    const glm::vec2 cur = Util::Input::GetCursorPosition();
    return {cur.x - 640.0f, 360.0f - cur.y};
}

// ── Update ────────────────────────────────────────────────────────────────────

CharacterSelectUI::Result CharacterSelectUI::Update(int& crystals) {
    using K = Util::Keycode;
    using I = Util::Input;

    // ESC → back
    if (I::IsKeyDown(K::ESCAPE)) return Result::Back;

    // A/LEFT → previous character
    if (I::IsKeyDown(K::A) || I::IsKeyDown(K::LEFT)) {
        m_FocusIdx = (m_FocusIdx + 2) % 3; // wrap left
        RefreshDynamic();
        SetUpgradeButtonState(crystals);
    }

    // D/RIGHT → next character
    if (I::IsKeyDown(K::D) || I::IsKeyDown(K::RIGHT)) {
        m_FocusIdx = (m_FocusIdx + 1) % 3;
        RefreshDynamic();
        SetUpgradeButtonState(crystals);
    }

    // Upgrade: U key or mouse click on button
    const glm::vec2 wp = CursorToWorld();
    const bool clickUpgrade =
        I::IsKeyDown(K::U) ||
        (I::IsKeyDown(K::MOUSE_LB) &&
         HitTest(wp, {BTN_UPG_X, BTN_UPG_Y}, {BTN_UPG_HW, BTN_UPG_HH}));

    if (clickUpgrade && !m_Upgraded[m_FocusIdx] && crystals >= 500) {
        crystals -= 500;
        m_Upgraded[m_FocusIdx] = true;
        m_CrystalText->SetText(std::to_string(crystals));
        RefreshDynamic();
        SetUpgradeButtonState(crystals);
    }

    // ENTER/J → confirm selection
    if (I::IsKeyDown(K::RETURN) || I::IsKeyDown(K::J)) {
        return Result::Confirmed;
    }

    return Result::None;
}

// ── Cleanup ───────────────────────────────────────────────────────────────────

void CharacterSelectUI::Cleanup(Util::Renderer& root) {
    auto rm = [&](std::shared_ptr<Util::GameObject>& obj) {
        if (obj) { root.RemoveChild(obj); obj.reset(); }
    };

    rm(m_Bg);
    rm(m_BannerObj);
    rm(m_CrystalIconObj);
    rm(m_CrystalTextObj);
    rm(m_TitleTextObj);

    for (auto& obj : m_StatIconObjs) rm(obj);
    for (auto& obj : m_StatTextObjs) rm(obj);
    for (auto& obj : m_StarObjs)     rm(obj);

    rm(m_PortraitFrame);
    rm(m_CharObj);
    rm(m_SkillIconObj);
    rm(m_SkillNameObj);
    rm(m_SkillDescObj);
    rm(m_BtnUpgradeObj);
    rm(m_BtnUpgradeTextObj);
    rm(m_BtnCrystalIconObj);
    rm(m_BtnCostTextObj);
    rm(m_ArrowLeft);
    rm(m_ArrowRight);
    rm(m_CharNameObj);

    m_CrystalText.reset();
    m_TitleText.reset();
    for (auto& t : m_StatTexts) t.reset();
    m_SkillNameText.reset();
    m_SkillDescText.reset();
    m_SkillIconImg.reset();
    m_CharAnim.reset();
    m_BtnUpgradeText.reset();
    m_BtnCostText.reset();
    m_CharNameText.reset();

    m_Root = nullptr;
}
