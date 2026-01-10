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
constexpr double   BIGGER_EPSILON                     = 1e-12;
constexpr double   EPSILON_DIST                       = 1e-9;
constexpr float    SAFE_REPEAT_BEFORE_NORMALIZATION   = 1024.f;
constexpr unsigned MAX_ITEMS_TO_RENDER                = 8192;
constexpr char     FLOOR_MARKER                       = ' ';
constexpr char     SPAWN_MARKER                       = '*';
constexpr char     END_MARKER                         = '>';
constexpr char     MELEE_ENEMY_SPAWN_MARKER           = '&';
constexpr char     RANGE_ENEMY_SPAWN_MARKER           = '9';
constexpr char     SUPPORT_ENEMY_SPAWN_MARKER         = '@';
constexpr float    MOUSE_DEG_PER_PIXEL                = 0.12f;

constexpr float    PARRY_RANGE_TILES                  = 2.5f;
constexpr float    PARRY_FOV_DEGREES                  = 10.0f;
constexpr float    PARRY_COOLDOWN_SECONDS             = 1.0f;
constexpr float    PARRY_INVULNERABILITY_SECONDS      = 1.5f;
constexpr float    PARRY_PROJECTILE_SPEED_MULTIPLIER  = 1.2f;
constexpr float    PARRY_PROJECTILE_DAMAGE            = 100.f;
constexpr float    PARRY_HEAL_ON_ENEMY_PROJECTILE     = 30.f;
constexpr float    PARRY_ANIM_FRAME_TIME_SECONDS      = 0.09f;
constexpr float    PARRY_CROSSHAIR_FLASH_SECONDS      = 0.15f;

constexpr float    HITMARKER_DURATION_SECONDS         = 0.12f;


#endif //NULLP0INT_CONSTANTS_H