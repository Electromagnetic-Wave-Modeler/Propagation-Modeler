#ifndef MYSDL_HPP
#define MYSDL_HPP

#include <SDL.h>
#include <vector>
#include "room.hpp"
#include <vector>

std::vector<std::vector<double>> loadCSV(const std::string& filename);

SDL_Color dBmToColor(double power, double min_power, double max_power);

int displaying(Room* room);

#endif // MYSDL_HPP