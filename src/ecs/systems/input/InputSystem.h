//
// Created by obamium3157 on 14.11.2025.
//

#ifndef NULLP0INT_INPUTSYSTEM_H
#define NULLP0INT_INPUTSYSTEM_H
#include "../../Registry.h"


namespace ecs
{
  class InputSystem
  {
  public:
    static void update(Registry& registry);
  };
}


#endif //NULLP0INT_INPUTSYSTEM_H