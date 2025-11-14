//
// Created by obamium3157 on 14.11.2025.
//

#ifndef NULLP0INT_MAPLOADERSYSTEM_H
#define NULLP0INT_MAPLOADERSYSTEM_H
#include <string>

namespace ecs
{
  class Registry;

  class MapLoaderSystem
  {
  public:
    static void load(Registry& registry, const std::string& filename);
  };
}



#endif //NULLP0INT_MAPLOADERSYSTEM_H