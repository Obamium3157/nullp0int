//
// Created by obamium3157 on 09.01.2026.
//

#include "PathfindingDistanceField.h"

#include <queue>

namespace ecs::npc
{
  void DistanceFieldCache::rebuildIfNeeded(const TilemapComponent &map, const Grid &g, const sf::Vector2i playerTile)
  {
    const bool needRebuild =
      !m_valid ||
      m_map != &map ||
      m_w != g.w ||
      m_h != g.h ||
      m_playerTile != playerTile;

    if (!needRebuild) return;

    m_map = &map;
    m_w = g.w;
    m_h = g.h;
    m_playerTile = playerTile;
    m_dist.assign(static_cast<std::size_t>(g.w) * static_cast<std::size_t>(g.h), -1);
    m_valid = true;

    if (g.blocked(playerTile.x, playerTile.y)) return;

    static const sf::Vector2i kDirs8[8] = {
      { 1,  0}, {-1,  0}, { 0,  1}, { 0, -1},
      { 1,  1}, { 1, -1}, {-1,  1}, {-1, -1}
    };

    std::queue<sf::Vector2i> q;
    m_dist[g.idx(playerTile.x, playerTile.y)] = 0;
    q.push(playerTile);

    while (!q.empty())
    {
      const sf::Vector2i cur = q.front();
      q.pop();

      const int curD = m_dist[g.idx(cur.x, cur.y)];
      for (const auto d : kDirs8)
      {
        const int nx = cur.x + d.x;
        const int ny = cur.y + d.y;
        if (!g.inBounds(nx, ny)) continue;
        if (g.blocked(nx, ny)) continue;

        if (d.x != 0 && d.y != 0)
        {
          if (g.blocked(cur.x + d.x, cur.y) || g.blocked(cur.x, cur.y + d.y)) continue;
        }

        const int nIndex = g.idx(nx, ny);
        if (m_dist[nIndex] != -1) continue;

        m_dist[nIndex] = curD + 1;
        q.emplace(nx, ny);
      }
    }
  }

  int DistanceFieldCache::getDist(const Grid& g, const sf::Vector2i tile) const
  {
    if (!m_valid) return -1;
    if (!g.inBounds(tile.x, tile.y)) return -1;
    return m_dist[g.idx(tile.x, tile.y)];
  }
}