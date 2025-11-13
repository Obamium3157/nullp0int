//
// Created by obamium3157 on 13.11.2025.
//

#ifndef NULLP0INT_REGISTRY_H
#define NULLP0INT_REGISTRY_H

#include <functional>
#include <memory>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>


#include "ComponentArray.h"
#include "Entity.h"

namespace ecs
{
  class Registry
  {
  public:
    Entity createEntity();

    const std::vector<Entity>& entities() const;

    void destroyEntity(Entity e);

    template<typename T>
    void addComponent(Entity e, const T& comp)
    {
      const auto ti = std::type_index(typeid(T));
      if (!m_components.contains(ti))
      {
        auto arr = std::make_shared<ComponentArray<T>>();
        m_components[ti] = arr;
        m_componentRemovers[ti] = [arr](Entity ent)
        {
          if (arr) arr->remove(ent);
        };
      }
      auto arr = std::static_pointer_cast<ComponentArray<T>>(m_components[ti]);
      arr->add(e, comp);
    }


    template<typename T>
    T* getComponent(Entity e)
    {
      const auto ti = std::type_index(typeid(T));
      const auto it = m_components.find(ti);
      if (it == m_components.end())
      {
        return nullptr;
      }
      auto arr = std::static_pointer_cast<ComponentArray<T>>(it->second);
      return arr->get(e);
    }

    template<typename T>
    void removeComponent(Entity e)
    {
      const auto ti = std::type_index(typeid(T));
      const auto it = m_components.find(ti);
      if (it == m_components.end()) return;
      if (auto arr = std::static_pointer_cast<ComponentArray<T>>(it->second)) arr->remove(e);
    }

    template<typename T>
    bool hasComponent(Entity e) const
    {
      const auto ti = std::type_index(typeid(T));
      const auto it = m_components.find(ti);
      if (it == m_components.end()) return false;
      auto arr = std::static_pointer_cast<ComponentArray<T>>(it->second);
      return arr->has(e);
    }

  private:
    Entity nextEntity{0};
    std::vector<Entity> m_entities;
    std::unordered_map<std::type_index, std::shared_ptr<void>> m_components;
    std::unordered_map<std::type_index, std::function<void(Entity)>> m_componentRemovers;
  };
}

#endif //NULLP0INT_REGISTRY_H