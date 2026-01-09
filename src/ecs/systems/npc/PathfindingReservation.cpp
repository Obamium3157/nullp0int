//
// Created by obamium3157 on 09.01.2026.
//

#include "PathfindingReservation.h"

#include <algorithm>

namespace ecs::npc
{
  void resolveMoveReservations(
    const Grid& g,
    const std::unordered_set<int>& initiallyOccupied,
    std::vector<MoveReservation>& reservations
  )
  {
    for (auto& r : reservations)
    {
      r.finalTile = r.fromTile;
    }

    std::vector<std::size_t> candidates;
    candidates.reserve(reservations.size());
    for (std::size_t i = 0; i < reservations.size(); ++i)
    {
      const auto& r = reservations[i];
      if (!r.wantsMove) continue;
      if (r.intendedTile == r.fromTile) continue;
      if (!g.inBounds(r.intendedTile.x, r.intendedTile.y)) continue;
      if (g.blocked(r.intendedTile.x, r.intendedTile.y)) continue;

      const int intendedIdx = g.idx(r.intendedTile.x, r.intendedTile.y);
      const int fromIdx = g.idx(r.fromTile.x, r.fromTile.y);
      if (intendedIdx != fromIdx && initiallyOccupied.contains(intendedIdx)) continue;

      candidates.push_back(i);
    }

    auto destKey = [&](const std::size_t idx) -> int {
      const auto& r = reservations[idx];
      return g.idx(r.intendedTile.x, r.intendedTile.y);
    };

    std::ranges::sort(
      candidates, [&](const std::size_t a, const std::size_t b) {
      const int da = destKey(a);
      const int db = destKey(b);
      if (da != db) return da < db;
      return entityIndex(reservations[a].entity) < entityIndex(reservations[b].entity);
    });

    std::unordered_set<int> reservedDest;
    reservedDest.reserve(candidates.size());

    std::size_t i = 0;
    while (i < candidates.size())
    {
      const int dest = destKey(candidates[i]);

      std::size_t winner = candidates[i];
      std::size_t j = i + 1;
      while (j < candidates.size() && destKey(candidates[j]) == dest)
      {
        const auto ea = entityIndex(reservations[candidates[j]].entity);
        const auto eb = entityIndex(reservations[winner].entity);
        if (ea < eb) winner = candidates[j];
        ++j;
      }

      if (!reservedDest.contains(dest))
      {
        reservations[winner].finalTile = reservations[winner].intendedTile;
        reservedDest.insert(dest);
      }

      i = j;
    }
  }
}