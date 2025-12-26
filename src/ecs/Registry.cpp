//
// Created by obamium3157 on 13.11.2025.
//

#include "Registry.h"
#include "Entity.h"

#include <algorithm>
#include <iterator>
#include <limits>
#include <ranges>
#include <stdexcept>

ecs::Entity ecs::Registry::createEntity()
{
  EntityIndex idx;

  if (!m_freeIndices.empty())
  {
    idx = m_freeIndices.back();
    m_freeIndices.pop_back();
    m_alive[idx] = 1;
  }
  else
  {
    if (m_nextIndex == (std::numeric_limits<EntityIndex>::max)())
    {
      throw std::runtime_error("Registry: Entity index space exhausted");
    }

    idx = m_nextIndex++;
    m_generations.push_back(0);
    m_alive.push_back(1);
  }

  const EntityGeneration gen = m_generations[idx];
  const Entity e = makeEntity(idx, gen);

  m_entities.push_back(e);
  return e;
}

bool ecs::Registry::isAlive(const Entity e) const
{
  if (e == INVALID_ENTITY) return false;

  const EntityIndex idx = entityIndex(e);
  if (idx >= m_generations.size() || idx >= m_alive.size()) return false;

  return m_alive[idx] != 0 && m_generations[idx] == entityGeneration(e);
}

const std::vector<ecs::Entity>& ecs::Registry::entities() const
{
  return m_entities;
}

void ecs::Registry::destroyEntity(const Entity e)
{
  if (!isAlive(e)) return;

  for (auto &remover: m_componentRemovers | std::views::values)
  {
    if (remover) remover(e);
  }
  const auto it = std::ranges::find(
    m_entities, e);
  if (it != m_entities.end())
  {
    std::iter_swap(it, std::prev(m_entities.end()));
    m_entities.pop_back();
  }

  const EntityIndex idx = entityIndex(e);
  m_alive[idx] = 0;
  ++m_generations[idx];
  m_freeIndices.push_back(idx);
}
