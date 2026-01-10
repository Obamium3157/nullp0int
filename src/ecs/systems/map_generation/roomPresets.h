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
  "####################",
  "#                  #",
  "#            NNNNN #",
  "#  NNN       N     #",
  "#     N      N 9   #",
  "#      N     N     #",
  "#            N     #",
  "#    &       N     #",
  "#   N22N     N     #",
  "#    NN            #",
  "#                  #",
  "#  &    &  NNN     #",
  "#    @     NNN     #",
  "#  &    &  NNN     #",
  "#                  #",
  "####################",
},
{
  "####################",
  "#                  #",
  "#            NNNNN #",
  "#  NNN       N     #",
  "#     N            #",
  "#      N     &     #",
  "#                  #",
  "#            N     #",
  "#            N     #",
  "#    @ @           #",
  "#                  #",
  "#    @ @   NNN     #",
  "#          N@N     #",
  "#          N N     #",
  "#                  #",
  "####################",
},
{
  "####################",
  "#                  #",
  "#            NNNNN #",
  "#  NNN       N     #",
  "#     N      N  &  #",
  "#      N  9  N     #",
  "#         9  N  &  #",
  "#         9  N     #",
  "#   N22N  9  N     #",
  "#    NN            #",
  "#                  #",
  "#          NNN     #",
  "#          NNN  &  #",
  "#          NNN     #",
  "#                  #",
  "####################",
},
  {
    "############",
    "#          #",
    "#    &     #",
    "#    &     #",
    "#    &     #",
    "#    &     #",
    "#    &     #",
    "#    &     #",
    "#          #",
    "#          #",
    "#          #",
    "############",
  },
  {
    "##############################",
    "#                      #     #",
    "#   @            #  &  #     #",
    "#      @    9    #           #",
    "#   @            #  &  #     #",
    "#                      #     #",
    "##############################",
  },

};

#endif //NULLP0INT_ROOMPRESETS_H