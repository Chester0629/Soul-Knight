#pragma once

#include "Util/Animation.hpp"
#include "Util/GameObject.hpp"
#include "Util/Image.hpp"
#include "Util/Renderer.hpp"
#include "Util/Text.hpp"
#include <array>
#include <memory>

// CharacterSelectUI — 角色選擇介面
// 三欄布局：左側屬性面板 / 中央角色動畫 + 星級 / 右側技能說明
// A/D 切換角色，U 鍵升級，ENTER/J 確認，ESC 返回
class CharacterSelectUI {
public:
    enum class Result { None, Confirmed, Back };

    // upgraded[3]: 三個角色各自的升級狀態（跨局保留）
    void Init(Util::Renderer& root, int crystals, const std::array<bool,3>& upgraded);
    // Update: 傳入目前晶石數，可能因升級而減少。返回 Result。
    Result Update(int& crystals);
    int GetSelectedChar() const { return m_FocusIdx; }
    bool GetUpgraded(int idx) const { return m_Upgraded[idx]; }
    void Cleanup(Util::Renderer& root);

private:
    // ── 角色靜態資料 ──────────────────────────────────────────────────────────
    struct CharData {
        const char* name;
        const char* skillName;
        const char* skillDesc;
        const char* skillIconPath;
        std::array<const char*, 4> idleFrames;
        int baseHP, upgradeHP;
        int baseShield, upgradeShield;
        int baseEnergy, upgradeEnergy;
        int crit;   // 不隨升級變動
        int stars;  // 難度 1~5
    };
    static const std::array<CharData, 3> kChars;

    // ── 狀態 ─────────────────────────────────────────────────────────────────
    int  m_FocusIdx  = 0;
    std::array<bool,3> m_Upgraded = {false,false,false};

    // ── 輔助 ─────────────────────────────────────────────────────────────────
    void RefreshDynamic();  // 焦點/升級狀態改變後重繪動態元件
    void SetUpgradeButtonState(int crystals);
    static bool HitTest(glm::vec2 wp, glm::vec2 center, glm::vec2 halfSize);
    static glm::vec2 CursorToWorld();

    Util::Renderer* m_Root = nullptr;

    // ── 靜態元件（Init 後不再重建） ───────────────────────────────────────────
    std::shared_ptr<Util::GameObject> m_Bg;
    std::shared_ptr<Util::GameObject> m_BannerObj;

    // 晶石顯示
    std::shared_ptr<Util::GameObject> m_CrystalIconObj;
    std::shared_ptr<Util::Text>       m_CrystalText;
    std::shared_ptr<Util::GameObject> m_CrystalTextObj;

    // 標題文字
    std::shared_ptr<Util::Text>       m_TitleText;
    std::shared_ptr<Util::GameObject> m_TitleTextObj;

    // 屬性圖示（固定，4 個）
    std::array<std::shared_ptr<Util::GameObject>, 4> m_StatIconObjs;
    // 屬性數值文字（動態更新）
    std::array<std::shared_ptr<Util::Text>,       4> m_StatTexts;
    std::array<std::shared_ptr<Util::GameObject>, 4> m_StatTextObjs;

    // 中央肖像框
    std::shared_ptr<Util::GameObject> m_PortraitFrame;

    // 中央角色動畫（RefreshDynamic 時 swap drawable）
    std::shared_ptr<Util::Animation>  m_CharAnim;
    std::shared_ptr<Util::GameObject> m_CharObj;

    // 難度星星（5 個：實/空）
    std::array<std::shared_ptr<Util::GameObject>, 5> m_StarObjs;

    // 右側技能圖示
    std::shared_ptr<Util::Image>      m_SkillIconImg;
    std::shared_ptr<Util::GameObject> m_SkillIconObj;

    // 右側技能文字
    std::shared_ptr<Util::Text>       m_SkillNameText;
    std::shared_ptr<Util::GameObject> m_SkillNameObj;
    std::shared_ptr<Util::Text>       m_SkillDescText;
    std::shared_ptr<Util::GameObject> m_SkillDescObj;

    // 升級按鈕
    std::shared_ptr<Util::Image>      m_BtnUpgradeImg;
    std::shared_ptr<Util::GameObject> m_BtnUpgradeObj;
    std::shared_ptr<Util::Text>       m_BtnUpgradeText;
    std::shared_ptr<Util::GameObject> m_BtnUpgradeTextObj;
    std::shared_ptr<Util::GameObject> m_BtnCrystalIconObj;
    std::shared_ptr<Util::Text>       m_BtnCostText;
    std::shared_ptr<Util::GameObject> m_BtnCostTextObj;

    // 導覽箭頭 + 角色名稱
    std::shared_ptr<Util::GameObject> m_ArrowLeft;
    std::shared_ptr<Util::GameObject> m_ArrowRight;
    std::shared_ptr<Util::Text>       m_CharNameText;
    std::shared_ptr<Util::GameObject> m_CharNameObj;

    // Upgrade button AABB
    static constexpr float BTN_UPG_X  =   0.0f;
    static constexpr float BTN_UPG_Y  = -240.0f;
    static constexpr float BTN_UPG_HW =  160.0f;
    static constexpr float BTN_UPG_HH =   65.0f;
};
