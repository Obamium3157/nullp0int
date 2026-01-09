//
// Created by obamium3157 on 09.01.2026.
//

#ifndef NULLP0INT_PATHFINDINGDISTANCEFIELD_H
#define NULLP0INT_PATHFINDINGDISTANCEFIELD_H
#include "PathfindingTypes.h"

namespace ecs::npc
{
  class DistanceFieldCache
  {
  public:
    void rebuildIfNeeded(const TilemapComponent& map, const Grid& g, sf::Vector2i playerTile);

    [[nodiscard]] int getDist(const Grid& g, sf::Vector2i tile) const;

    [[nodiscard]] const std::vector<int>& field() { return m_dist; }

  private:
    const TilemapComponent* m_map = nullptr;
    int m_w = 0;
    int m_h = 0;
    sf::Vector2i m_playerTile{ -999, -999 };
    std::vector<int> m_dist;
    bool m_valid = false;
  };
}

#endif //NULLP0INT_PATHFINDINGDISTANCEFIELD_H