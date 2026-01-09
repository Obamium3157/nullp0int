cmake_minimum_required(VERSION 3.17)
project(nullp0int)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SFML_DIR "C:/SFML/lib/cmake/SFML")
find_package(SFML 2.6.2 COMPONENTS graphics window system REQUIRED PATHS "${SFML_DIR}" NO_DEFAULT_PATH)

add_executable(nullp0int src/main.cpp
        src/ecs/Entity.h
        src/ecs/Components.h
        src/ecs/ComponentArray.h
        src/ecs/Registry.h
        src/game/Game.h
        src/ecs/Registry.cpp
        src/game/entities/player/PlayerFactory.h
        src/game/entities/player/PlayerFactory.cpp
        src/game/Game.cpp
        src/ecs/systems/physics/PhysicsSystem.cpp
        src/ecs/systems/render/RenderSystem.h
        src/ecs/systems/render/RenderSystem.cpp
        src/ecs/systems/input/InputSystem.cpp
        src/ecs/systems/map/MapLoaderSystem.h
        src/ecs/systems/map/MapLoaderSystem.cpp
        src/constants.h
        src/math/mathUtils.h
        src/math/mathUtils.cpp
        src/ecs/systems/collision/CollisionSystem.h
        src/ecs/systems/collision/CollisionSystem.cpp
        src/ecs/systems/render/RayCasting.h
        src/ecs/systems/render/RayCasting.cpp
        src/ecs/systems/render/Animation.h
        src/ecs/systems/render/TextureManager.h
        src/ecs/systems/render/TextureManager.cpp
        src/ecs/systems/render/AnimationSystem.h
        src/ecs/systems/render/AnimationSystem.cpp
        src/configuration/Configuration.h
        src/ecs/systems/map_generation/MapGenerationSystem.h
        src/ecs/systems/map_generation/MapGenerationSystem.cpp
        src/ecs/systems/map_generation/roomPresets.h
        src/game/entities/enemy/EnemyFactory.h
        src/game/entities/enemy/EnemyFactory.cpp
        src/ecs/systems/npc/PathfindingSystem.h
        src/ecs/systems/npc/PathfindingSystem.cpp
        src/ecs/systems/weapon/WeaponSystem.h
        src/ecs/systems/weapon/WeaponSystem.cpp
        src/game/weapons/Weapon.h
        src/ecs/systems/projectile/ProjectileSystem.cpp
        src/game/GameUI.cpp
        src/game/GameUI.h
        src/game/GameMenuUXUI.cpp
        src/ecs/systems/npc/PathfindingTypes.h
        src/ecs/systems/npc/PathfindingPerception.h
        src/ecs/systems/npc/PathfindingPerception.cpp
        src/ecs/systems/npc/PathfindingDistanceField.h
        src/ecs/systems/npc/PathfindingDistanceField.cpp
        src/ecs/systems/npc/PathfindingAnimation.h
        src/ecs/systems/npc/PathfindingAnimation.cpp
        src/ecs/systems/npc/Combat.cpp
        src/ecs/systems/npc/Movement.h
        src/ecs/systems/npc/Movement.cpp
        src/ecs/systems/npc/PathfindingReservation.cpp
)

target_link_libraries(nullp0int PRIVATE sfml-graphics sfml-window sfml-system)

add_custom_command(TARGET nullp0int POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "C:/SFML/bin"
        $<TARGET_FILE_DIR:nullp0int>
)
