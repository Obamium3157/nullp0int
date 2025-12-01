//
// Created by obamium3157 on 30.11.2025.
//

#ifndef NULLP0INT_ROOMPRESETS_H
#define NULLP0INT_ROOMPRESETS_H

#include <string>
#include <vector>

using RoomPreset = std::vector<std::string>;

const RoomPreset entrancePreset = {
  "PPPPPPPP",
  "P      P",
  "P  *   P",
  "P      P",
  "PPPPPPPP"
};

const RoomPreset exitPreset = {
  "NNNNNNNN",
  "N      N",
  "N  >   N",
  "N      N",
  "NNNNNNNN"
};

const std::vector<RoomPreset> normalPresets = {
  {
    "11111111",
    "1      1",
    "1     11",
    "1    111",
    "1     11",
    "11111111"
   },
   {
     "22222222",
     "2      2",
     "2     22",
     "2    222",
     "2     22",
     "22222222"
    }
};

#endif //NULLP0INT_ROOMPRESETS_H