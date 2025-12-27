//
// Created by obamium3157 on 14.11.2025.
//

#ifndef NULLP0INT_INPUTSYSTEM_H
#define NULLP0INT_INPUTSYSTEM_H
#include "../../Registry.h"
#include "../../../configuration/Configuration.h"


namespace ecs
{
  class InputSystem
  {
  public:
    static void update(Registry& registry, const Configuration& config, float deltaTime, float mouseDeltaX);
  };
}


#endif //NULLP0INT_INPUTSYSTEM_H