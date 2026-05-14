#pragma once

// LevelManager — 管理樓層進度（1~5）與擊殺數
// Floor 1~4：普通層（PORTAL 房間終點）
// Floor 5  ：Boss 層（BOSS 房間終點，打敗 Boss 觸發 Victory）
class LevelManager {
public:
    void Reset();
    void NextFloor();
    void AddKills(int n);

    int  GetFloor() const    { return m_Floor; }
    int  GetKills() const    { return m_Kills; }
    bool IsBossFloor() const { return m_Floor == 5; }
    bool IsComplete() const  { return m_Floor > 5; }

private:
    int m_Floor = 1;
    int m_Kills = 0;
};
