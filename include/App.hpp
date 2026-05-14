#pragma once

#include "pch.hpp" // IWYU pragma: export

#include "Entity/EnemyManager.hpp"
#include "Entity/Player.hpp"
#include "LevelManager.hpp"
#include "UI/CharacterSelectUI.hpp"
#include "UI/HUD.hpp"
#include "UI/MainMenuUI.hpp"
#include "UI/MiniMap.hpp"
#include "Util/GameObject.hpp"
#include "Util/Renderer.hpp"
#include "Util/Text.hpp"
#include "Weapon/BulletManager.hpp"
#include "World/Portal.hpp"
#include "World/World.hpp"

class App {
public:
    enum class State {
        START,
        MAIN_MENU,
        CHARACTER_SELECT,
        UPGRADE,
        UPDATE,
        GAME_OVER,
        VICTORY,
        END,
    };

    State GetCurrentState() const { return m_CurrentState; }

    void Start();
    void UpdateMainMenu();
    void UpdateCharacterSelect();
    void UpdateUpgrade();
    void Update();
    void UpdateOverlay();  // GAME_OVER / VICTORY 時呼叫（渲染 + 等待輸入）
    void End();

private:
    State m_CurrentState = State::START;
    bool  m_Initialized  = false;

    // ── 跨局持久狀態 ─────────────────────────────────────────────────────────
    int  m_Crystals     = 0;
    bool m_C01Upgraded  = false;
    bool m_C02Upgraded  = false;
    bool m_C03Upgraded  = false;

    Util::Renderer m_Root;
    World          m_World;

    BulletManager            m_BulletManager;
    std::shared_ptr<Player>  m_Player;
    EnemyManager             m_EnemyManager;
    MainMenuUI               m_MainMenuUI;
    CharacterSelectUI        m_CharSelectUI;
    HUD                      m_HUD;
    MiniMap                  m_MiniMap;
    Portal                   m_Portal;
    LevelManager             m_LevelManager;

    unsigned m_Seed          = 0;
    bool     m_PortalActive  = false;   // 本層傳送陣是否已出現
    bool     m_VictoryLocked = false;   // 防止 Boss 勝利條件觸發多次

    // ── Overlay（GameOver / Victory）────────────────────────────────────────
    std::shared_ptr<Util::Text>       m_OverlayTitle;
    std::shared_ptr<Util::Text>       m_OverlayStats;
    std::shared_ptr<Util::Text>       m_OverlayHint;
    std::shared_ptr<Util::GameObject> m_OverlayTitleObj;
    std::shared_ptr<Util::GameObject> m_OverlayStatsObj;
    std::shared_ptr<Util::GameObject> m_OverlayHintObj;

    // ── 內部流程 ─────────────────────────────────────────────────────────────
    void InitFloor();                        // 生成並設置當前樓層場景
    void Cleanup();                          // 從 Renderer 移除上一層所有物件
    void LoadNextFloor();                    // 切換至下一層（保留 LevelManager 狀態）

    void ShowGameOver();                     // 顯示死亡 Overlay
    void ShowVictory();                      // 顯示勝利 Overlay
    void HideOverlay();                      // 隱藏 Overlay

    void SpawnEnemiesInRoom(int roomIdx);    // 懶生成：進房觸發
};
