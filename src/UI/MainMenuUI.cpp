#include "UI/MainMenuUI.hpp"

#include "Util/Color.hpp"
#include "Util/Image.hpp"
#include "Util/Input.hpp"
#include "Util/Keycode.hpp"

static constexpr const char* FONT = PTSD_ASSETS_DIR "/fonts/Inter.ttf";

// ── 建立輔助 ──────────────────────────────────────────────────────────────────

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
    const std::shared_ptr<Util::Text>& t, float cx, float cy, float z)
{
    auto obj = std::make_shared<Util::GameObject>();
    obj->SetDrawable(t);
    obj->m_Transform.scale       = {1.0f, 1.0f};
    obj->m_Transform.translation = {cx, cy};
    obj->SetZIndex(z);
    obj->SetVisible(true);
    return obj;
}

// ── HitTest ───────────────────────────────────────────────────────────────────

bool MainMenuUI::HitTest(glm::vec2 wp, glm::vec2 center, glm::vec2 halfSize) {
    return std::abs(wp.x - center.x) <= halfSize.x &&
           std::abs(wp.y - center.y) <= halfSize.y;
}

// ── Init ──────────────────────────────────────────────────────────────────────

void MainMenuUI::Init(Util::Renderer& root) {
    m_Root         = &root;
    m_SplashActive = true;
    m_Phase2Shown  = false;

    // Phase 1: only background
    m_Bg = MakeImg(RESOURCE_DIR "/UI/biaoti_01.png",
                   1.25f, 1.25f, 0.0f, 0.0f, 99.0f);
    root.AddChild(m_Bg);
}

// ── ShowPhase2 ────────────────────────────────────────────────────────────────

void MainMenuUI::ShowPhase2() {
    if (m_Phase2Shown || !m_Root) return;
    m_Phase2Shown = true;

    Util::Renderer& root = *m_Root;

    // Title logo elements (static, no animation)
    m_Swoosh = MakeImg(RESOURCE_DIR "/UI/biaoti_02.png",
                       0.8f, 0.8f, -250.0f, 100.0f, 99.5f);
    m_TitleA = MakeImg(RESOURCE_DIR "/UI/biaoti02.png",
                       2.0f, 2.0f, -100.0f, 140.0f, 99.5f);
    m_TitleB = MakeImg(RESOURCE_DIR "/UI/biaoti03.png",
                       1.2f, 1.2f,   80.0f,  80.0f, 99.5f);
    m_Underline = MakeImg(RESOURCE_DIR "/UI/biaoti05.png",
                          1.0f, 1.0f, 50.0f, 20.0f, 99.5f);

    // New Game button (ui_118) — focusable
    m_BtnNewImg = MakeImg(RESOURCE_DIR "/UI/ui_118.png",
                          0.55f, 0.55f, BTN_NEW_X, BTN_Y, 99.5f);
    m_BtnNewText = std::make_shared<Util::Text>(
        FONT, 28, "New Game", Util::Color{30, 15, 0, 255});
    m_BtnNewTextObj = MakeTxt(m_BtnNewText, BTN_NEW_X, BTN_Y, 99.51f);

    // Multiplayer button — disabled grey (ui_btn1 stretched to match ui_118 size)
    m_BtnMultiImg = MakeImg(RESOURCE_DIR "/UI/ui_btn1.png",
                            16.0f, 10.0f, BTN_MULTI_X, BTN_Y, 99.5f);
    m_BtnMultiText = std::make_shared<Util::Text>(
        FONT, 22, "Multiplayer", Util::Color{100, 100, 100, 150});
    m_BtnMultiTextObj = MakeTxt(m_BtnMultiText, BTN_MULTI_X, BTN_Y, 99.51f);

    root.AddChild(m_Swoosh);
    root.AddChild(m_TitleA);
    root.AddChild(m_TitleB);
    root.AddChild(m_Underline);
    root.AddChild(m_BtnNewImg);
    root.AddChild(m_BtnNewTextObj);
    root.AddChild(m_BtnMultiImg);
    root.AddChild(m_BtnMultiTextObj);
}

// ── Update ────────────────────────────────────────────────────────────────────

MainMenuUI::Action MainMenuUI::Update() {
    using K = Util::Keycode;
    using I = Util::Input;

    if (m_SplashActive) {
        // Any key or left-click advances to Phase 2
        const bool anyInput =
            I::IsKeyDown(K::MOUSE_LB) || I::IsKeyDown(K::RETURN) ||
            I::IsKeyDown(K::SPACE)    || I::IsKeyDown(K::J)       ||
            I::IsKeyDown(K::A)        || I::IsKeyDown(K::D)       ||
            I::IsKeyDown(K::W)        || I::IsKeyDown(K::S)       ||
            I::IsKeyDown(K::LEFT)     || I::IsKeyDown(K::RIGHT)   ||
            I::IsKeyDown(K::UP)       || I::IsKeyDown(K::DOWN)    ||
            I::IsKeyDown(K::ESCAPE);
        if (anyInput) {
            m_SplashActive = false;
            ShowPhase2();
        }
        return Action::None;
    }

    // Phase 2 — handle ESC first
    if (I::IsKeyDown(K::ESCAPE)) return Action::Quit;

    // Convert cursor to world coords (screen origin top-left → world origin center)
    const glm::vec2 cur = I::GetCursorPosition();
    const float wx = cur.x - 640.0f;
    const float wy = 360.0f - cur.y;

    // New Game: ENTER / J / left-click inside AABB
    const bool clickNew =
        I::IsKeyDown(K::RETURN) || I::IsKeyDown(K::J) ||
        (I::IsKeyDown(K::MOUSE_LB) &&
         HitTest({wx, wy}, {BTN_NEW_X, BTN_Y}, {BTN_HW, BTN_HH}));

    if (clickNew) return Action::StartGame;

    // Multiplayer: click on button — no effect (disabled)
    // (Do nothing)

    return Action::None;
}

// ── Cleanup ───────────────────────────────────────────────────────────────────

void MainMenuUI::Cleanup(Util::Renderer& root) {
    auto rm = [&](std::shared_ptr<Util::GameObject>& obj) {
        if (obj) { root.RemoveChild(obj); obj.reset(); }
    };

    rm(m_Bg);
    rm(m_Swoosh);
    rm(m_TitleA);
    rm(m_TitleB);
    rm(m_Underline);
    rm(m_BtnNewImg);
    rm(m_BtnNewTextObj);
    rm(m_BtnMultiImg);
    rm(m_BtnMultiTextObj);

    m_BtnNewText.reset();
    m_BtnMultiText.reset();

    m_SplashActive = true;
    m_Phase2Shown  = false;
    m_Root         = nullptr;
}
