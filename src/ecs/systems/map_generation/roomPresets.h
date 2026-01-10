//
// Created by obamium3157 on 30.11.2025.
//

#ifndef NULLP0INT_ROOMPRESETS_H
#define NULLP0INT_ROOMPRESETS_H

#include <string>
#include <vector>

using RoomPreset = std::vector<std::string>;

const RoomPreset entrancePreset = {
  "pppppppp",
  "p      p",
  "p  *   p",
  "p      p",
  "pppppppp"
};

const RoomPreset exitPreset = {
  "eeeeeeee",
  "e      e",
  "e  >   e",
  "e      e",
  "eeeeeeee"
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
     "22     2",
     "222    2",
     "22     2",
     "22222222"
    },
  {
    "####################",
    "#                  #",
    "#            NNNNN #",
    "#  NNN       N     #",
    "#     N      N     #",
    "#      N     N     #",
    "#            N     #",
    "#            N     #",
    "#   N22N     N     #",
    "#    NN            #",
    "#                  #",
    "#          NNN     #",
    "#          NNN     #",
    "#          NNN     #",
    "#                  #",
    "####################",
  }
};

#endif //NULLP0INT_ROOMPRESETS_H