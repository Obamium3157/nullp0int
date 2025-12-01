//
// Created by obamium3157 on 27.11.2025.
//

#ifndef NULLP0INT_CONFIGURATION_H
#define NULLP0INT_CONFIGURATION_H

#include <SFML/System/Vector2.hpp>

#include "../math/mathUtils.h"

enum ResolutionOption
{
  EXTRA_LOW = 40,
  LOW = 100,
  MEDIUM = 200,
  HIGH = 600,
};

struct Configuration
{
  sf::Vector2f player_initial_position = sf::Vector2f{ 100.f, 100.f };
  float player_radius = 17.f;
  float player_speed = 250.f;
  float player_velocity_multiplier = 4.f;
  float player_rotation_speed = 120.f;
  float fov = radiansFromDegrees(90);
  float resolution_option = ResolutionOption::HIGH;
  unsigned render_distance = 1000u;
  float attenuation_distance = 16.f;
  bool enable_fish_eye = false;
  float tile_size = 64.f;
  float player_eye_height = tile_size * 2.f / 3.f;
};

#endif //NULLP0INT_CONFIGURATION_H