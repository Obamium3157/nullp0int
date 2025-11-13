//
// Created by obamium3157 on 13.11.2025.
//

#include "Registry.h"
#include "Entity.h"

ecs::Entity ecs::Registry::createEntity()
{
  const Entity e = nextEntity++;
  m_entities.push_back(e);
  return e;
}

const std::vector<ecs::Entity> &ecs::Registry::entities() const
{
  return m_entities;
}

void ecs::Registry::destroyEntity(const Entity e)
{
  for (auto & [ti, remover] : m_componentRemovers)
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
}
