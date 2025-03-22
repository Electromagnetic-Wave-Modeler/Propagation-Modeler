#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
// #include <SDL.h>
#include <algorithm>
#include <limits>
#include <cmath>
#include <cstdlib>

#include "headers/emitter.hpp"
#include "headers/obstacle.hpp"
#include "headers/room.hpp"
#include "headers/display.hpp"

#include <SDL.h>


int main() {
    Room room(920, 680); // Résolution Full HD

    // Ajout d'un émetteur Wi-Fi
    room.addEmitter(Emitter(150, 150, -30, 2.4e9)); // 30 dBm, 2.4 GHz

    // Ajout d'un mur vertical avec épaisseur
    room.addObstacle(Obstacle(200, 0, 200, 300, 10, 10)); // Mur vertical, épaisseur 10, 10 dB d'atténuation

    // Ajout d'un mur horizontal avec épaisseur
    room.addObstacle(Obstacle(50, 150, 250, 150, 5, 8)); // Mur horizontal, épaisseur 5, 8 dB d'atténuation

    // Calcul de la puissance en chaque point
    room.computeSignalMap();

    // Export en CSV
    room.exportToCSV("heatmap.csv");

    std::cout << "Done!" << std::endl;

    working();

    return 0;
}