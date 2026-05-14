#pragma once

#include "Util/GameObject.hpp"
#include "Util/Renderer.hpp"
#include "Util/Text.hpp"
#include <memory>

// MainMenuUI — 主選單
// Phase 1 (Splash): 只顯示背景圖 biaoti_01.png，等任意輸入
// Phase 2 (Active): 靜態顯示標題 Logo + 新遊戲/多人遊戲水平按鈕
class MainMenuUI {
public:
    enum class Action { None, StartGame, Quit };

    void Init(Util::Renderer& root);
    Action Update();
    void Cleanup(Util::Renderer& root);

private:
    void ShowPhase2();
    static bool HitTest(glm::vec2 wp, glm::vec2 center, glm::vec2 halfSize);

    bool m_SplashActive = true;
    bool m_Phase2Shown  = false;
    Util::Renderer* m_Root = nullptr;

    // Phase 1
    std::shared_ptr<Util::GameObject> m_Bg;

    // Phase 2 — title Logo
    std::shared_ptr<Util::GameObject> m_Swoosh;
    std::shared_ptr<Util::GameObject> m_TitleA;
    std::shared_ptr<Util::GameObject> m_TitleB;
    std::shared_ptr<Util::GameObject> m_Underline;

    // Phase 2 — buttons
    std::shared_ptr<Util::GameObject> m_BtnNewImg;
    std::shared_ptr<Util::Text>       m_BtnNewText;
    std::shared_ptr<Util::GameObject> m_BtnNewTextObj;
    std::shared_ptr<Util::GameObject> m_BtnMultiImg;
    std::shared_ptr<Util::Text>       m_BtnMultiText;
    std::shared_ptr<Util::GameObject> m_BtnMultiTextObj;

    // Button AABB constants
    static constexpr float BTN_NEW_X   = -190.0f;
    static constexpr float BTN_MULTI_X =  190.0f;
    static constexpr float BTN_Y       = -245.0f;
    static constexpr float BTN_HW      =  130.0f;
    static constexpr float BTN_HH      =   65.0f;
};
