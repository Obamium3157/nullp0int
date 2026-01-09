//
// Created by obamium3157 on 09.01.2026.
//

#include "Movement.h"

#include <cmath>

namespace ecs::npc
{
  static const sf::Vector2i kDirs8[8] = {
    { 1,  0}, {-1,  0}, { 0,  1}, { 0, -1},
    { 1,  1}, { 1, -1}, {-1,  1}, {-1, -1}
  };

  sf::Vector2i pickNextTileToward(
    const Grid& g,
    const std::vector<int>& distField,
    const std::unordered_set<int>& occupied,
    const sf::Vector2i curTile,
    const sf::Vector2i playerTile
  )
  {
    if (!g.inBounds(curTile.x, curTile.y)) return curTile;

    const int curIndex = g.idx(curTile.x, curTile.y);
    const int curDist = distField[curIndex];
    if (curDist < 0) return curTile;

    sf::Vector2i best = curTile;
    int bestDist = curDist;

    for (const auto d : kDirs8)
    {
      const int nx = curTile.x + d.x;
      const int ny = curTile.y + d.y;

      if (!g.inBounds(nx, ny)) continue;
      if (g.blocked(nx, ny)) continue;

      if (d.x != 0 && d.y != 0)
      {
        if (g.blocked(curTile.x + d.x, curTile.y) || g.blocked(curTile.x, curTile.y + d.y)) continue;
      }

      const int nIndex = g.idx(nx, ny);
      const int nd = distField[nIndex];
      if (nd < 0) continue;
      if (nd >= bestDist) continue;

      if (!(nx == playerTile.x && ny == playerTile.y))
      {
        if (occupied.contains(nIndex) && nIndex != curIndex) continue;
      }

      bestDist = nd;
      best = {nx, ny};
    }

    return best;
  }

  sf::Vector2i pickNextTileAway(
    const Grid& g,
    const std::vector<int>& distField,
    const std::unordered_set<int>& occupied,
    const sf::Vector2i curTile
  )
  {
    if (!g.inBounds(curTile.x, curTile.y)) return curTile;
    const int curIndex = g.idx(curTile.x, curTile.y);
    const int curDist = distField[curIndex];
    if (curDist < 0) return curTile;

    sf::Vector2i best = curTile;
    int bestDist = curDist;

    for (const auto d : kDirs8)
    {
      const int nx = curTile.x + d.x;
      const int ny = curTile.y + d.y;

      if (!g.inBounds(nx, ny)) continue;
      if (g.blocked(nx, ny)) continue;

      if (d.x != 0 && d.y != 0)
      {
        if (g.blocked(curTile.x + d.x, curTile.y) || g.blocked(curTile.x, curTile.y + d.y)) continue;
      }

      const int nIndex = g.idx(nx, ny);
      const int nd = distField[nIndex];
      if (nd < 0) continue;
      if (nd <= bestDist) continue;

      if (occupied.contains(nIndex) && nIndex != curIndex) continue;

      bestDist = nd;
      best = {nx, ny};
    }

    return best;
  }

  sf::Vector2f perpendicularStrafeDir(const sf::Vector2f toPlayerDir, const bool clockwise)
  {
    return clockwise
      ? sf::Vector2f{-toPlayerDir.y, toPlayerDir.x}
      : sf::Vector2f{toPlayerDir.y, -toPlayerDir.x};
  }

  sf::Vector2i pickOrbitTile(
    const Grid& g,
    const std::unordered_set<int>& occupied,
    const sf::Vector2i curTile,
    const sf::Vector2f curWorld,
    const sf::Vector2f playerWorld,
    const float desiredRangeTiles,
    const float toleranceTiles,
    const sf::Vector2f toPlayerDir,
    const bool clockwise,
    const OrbitTuning& tuning
  )
  {
    if (!g.inBounds(curTile.x, curTile.y)) return curTile;

    const int curIndex = g.idx(curTile.x, curTile.y);
    const float curDistTiles = (g.tileSize > 0.f)
      ? (std::hypot(curWorld.x - playerWorld.x, curWorld.y - playerWorld.y) / g.tileSize)
      : 0.f;

    sf::Vector2i best = curTile;
    float bestScore = std::numeric_limits<float>::infinity();

    const sf::Vector2f tangent = perpendicularStrafeDir(toPlayerDir, clockwise);

    for (const auto d : kDirs8)
    {
      const int nx = curTile.x + d.x;
      const int ny = curTile.y + d.y;

      if (!g.inBounds(nx, ny)) continue;
      if (g.blocked(nx, ny)) continue;

      if (d.x != 0 && d.y != 0)
      {
        if (g.blocked(curTile.x + d.x, curTile.y) || g.blocked(curTile.x, curTile.y + d.y)) continue;
      }

      const int nIndex = g.idx(nx, ny);
      if (occupied.contains(nIndex) && nIndex != curIndex) continue;

      const sf::Vector2f candidateCenter = g.tileCenterWorld(nx, ny);

      const float candidateDistTiles = (g.tileSize > 0.f)
        ? (std::hypot(candidateCenter.x - playerWorld.x, candidateCenter.y - playerWorld.y) / g.tileSize)
        : 0.f;

      const float rangeErr = std::abs(candidateDistTiles - desiredRangeTiles);
      if (rangeErr > toleranceTiles * tuning.toleranceExpand) continue;

      const sf::Vector2f stepDir = normalizedOrZero(candidateCenter - g.tileCenterWorld(curTile.x, curTile.y));
      if (stepDir.x == 0.f && stepDir.y == 0.f) continue;

      const float dotTang = (stepDir.x * tangent.x + stepDir.y * tangent.y);
      const float tangentialScore = 1.f - clamp01((dotTang + 1.f) * 0.5f);

      const float wrongSide = (dotTang < 0.f) ? tuning.wrongSidePenalty : 0.f;
      const float driftPenalty = std::abs(candidateDistTiles - curDistTiles) * tuning.driftWeight;

      const float score =
        rangeErr * tuning.rangeWeight +
        tangentialScore * tuning.tangentialWeight +
        wrongSide +
        driftPenalty;

      if (score < bestScore)
      {
        bestScore = score;
        best = {nx, ny};
      }
    }

    return best;
  }
}