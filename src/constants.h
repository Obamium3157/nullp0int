//
// Created by obamium3157 on 14.11.2025.
//

#ifndef NULLP0INT_CONSTANTS_H
#define NULLP0INT_CONSTANTS_H
#include <cmath>

constexpr unsigned SCREEN_WIDTH               = 1920;
constexpr unsigned SCREEN_HEIGHT              = 1080;
const sf::Vector2f PLAYER_INITIAL_POSITION    = { 100.f, 100.f };
constexpr float    PLAYER_RADIUS              = 17.f;
constexpr float    PLAYER_SPEED               = 250.f;
constexpr float    PLAYER_VELOCITY_MULTIPLIER = 4.f;
constexpr float    PLAYER_ROTATION_SPEED      = 120.f;
const sf::Color    PLAYER_COLOR               = sf::Color::Cyan;

constexpr float    FOV                             = static_cast<float>(M_PI) / 2.5;
constexpr float    HALF_FOV                        = FOV / 2;
constexpr float    RAY_ANGLE_OFFSET                = 0.0001f;
constexpr float    BIG_EPSILON                     = 1e-6f;
constexpr unsigned AMOUNT_OF_RAYS                  = 500; //SCREEN_WIDTH / 2;
constexpr unsigned HALF_AMOUNT_OF_RAYS             = AMOUNT_OF_RAYS / 2;
constexpr float    DELTA_ANGLE                     = FOV / AMOUNT_OF_RAYS;
constexpr unsigned MAX_DEPTH                       = 100;
constexpr float    MAX_LINEAR_ATTENUATION_DISTANCE = 16.f;

constexpr float TILE_SCALE                         = 64.f;

#endif //NULLP0INT_CONSTANTS_H