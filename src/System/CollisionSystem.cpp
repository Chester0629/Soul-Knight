#include "System/CollisionSystem.hpp"

const World* CollisionSystem::s_World = nullptr;

void CollisionSystem::SetWorld(const World* world) {
    s_World = world;
}

void CollisionSystem::ResolveWall(glm::vec2& worldPos,
                                   glm::vec2  hitOffset,
                                   glm::vec2  hitSize) {
    if (!s_World) return;
    s_World->ResolveWall(worldPos, hitOffset, hitSize);
}

bool CollisionSystem::IsBlocked(glm::vec2 center, glm::vec2 size) {
    if (!s_World) return false;
    return s_World->IsBlocked(center, size);
}
