//
// Created by obamium3157 on 13.11.2025.
//

#ifndef NULLP0INT_COMPONENTARRAY_H
#define NULLP0INT_COMPONENTARRAY_H

#include <unordered_map>
#include "Entity.h"

namespace ecs
{
  template<typename T>
  class ComponentArray
  {
    std::unordered_map<Entity, T> data;
  public:
    void add(Entity e, const T& comp)
    {
      data[e] = comp;
    }
    void remove(Entity e)
    {
      data.erase(e);
    }
    T* get(Entity e)
    {
      auto it = data.find(e);
      return it == data.end() ? nullptr : &it->second;
    }
    [[nodiscard]] bool has(Entity e) const
    {
      return data.contains(e);
    }
  };
}

#endif //NULLP0INT_COMPONENTARRAY_H