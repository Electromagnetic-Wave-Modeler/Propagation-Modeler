#ifndef MYSDL_HPP
#define MYSDL_HPP

#include <SDL.h>
#include <vector>
<<<<<<< HEAD
=======
#include "room.hpp"
#include <vector>

#include "../lib/SDL2_ttf/include/SDL_ttf.h"

>>>>>>> 6342e4a8d7b42d8fc7b342b387891e74bda67b81
std::vector<std::vector<double>> loadCSV(const std::string& filename);

SDL_Color dBmToColor(double power, double min_power, double max_power);

int displaying(Room* room);

int handlepowerMap(Room* room, SDL_Renderer* renderer);

SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color textColor);

#endif // MYSDL_HPP