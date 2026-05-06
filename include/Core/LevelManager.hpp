#pragma once

// LevelManager — 追蹤當前層數（1~5），管理層間切換
//
// 層數規則：
//   第 1~4 層：普通層（隨機地城，BasicRoom + Portal）
//   第 5 層  ：Boss 層（BasicRoom × 3 + Boss Room）
class LevelManager {
public:
    static constexpr int TOTAL_FLOORS = 5;
    static constexpr int BOSS_FLOOR   = 5;

    void Reset()     { m_Floor = 1; }
    void NextFloor() { ++m_Floor; }

    int  GetFloor()    const { return m_Floor; }
    bool IsBossFloor() const { return m_Floor == BOSS_FLOOR; }
    bool IsComplete()  const { return m_Floor > TOTAL_FLOORS; }

private:
    int m_Floor = 1;
};
