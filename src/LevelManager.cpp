#include "LevelManager.hpp"

void LevelManager::Reset() {
    m_Floor = 1;
    m_Kills = 0;
}

void LevelManager::NextFloor() {
    ++m_Floor;
}

void LevelManager::AddKills(int n) {
    m_Kills += n;
}
