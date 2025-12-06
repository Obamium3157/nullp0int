//
// Created by obamium3157 on 14.11.2025.
//

#ifndef NULLP0INT_CONSTANTS_H
#define NULLP0INT_CONSTANTS_H

constexpr unsigned SCREEN_WIDTH                       = 1920;
constexpr unsigned SCREEN_HEIGHT                      = 1080;
constexpr unsigned HALF_SCREEN_WIDTH                  = SCREEN_WIDTH / 2.f;
constexpr unsigned HALF_SCREEN_HEIGHT                 = SCREEN_HEIGHT / 2.f;
constexpr float    RAY_ANGLE_OFFSET                   = 0.0001f;
constexpr double   SMALL_EPSILON                      = 0.0001;
constexpr double   BIG_EPSILON                        = 1e-6;
constexpr double   EPSILON_DIST                       = 1e-9;
constexpr float    SAFE_REPEAT_BEFORE_NORMALIZATION   = 1024.f;
constexpr unsigned MAX_ITEMS_TO_RENDER                = 8192;

#endif //NULLP0INT_CONSTANTS_H