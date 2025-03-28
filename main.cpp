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
#include <utility>

#include "headers/emitter.hpp"
#include "headers/obstacle.hpp"
#include "headers/room.hpp"
#include "headers/display.hpp"

#include <SDL.h>


int main() {
    Room room(1920, 1080); // Résolution Full HD

    // Ajout d'un émetteur Wi-Fi
    room.addEmitter(Emitter(150, 150, -30, 2.4e9)); // -30 dBm, 2.4 GHz
    room.addEmitter(Emitter(700, 500, -28, 2.4e9)); // -28 dBm, 2.4 GHz

    // Ajout d'un mur vertical avec épaisseur
    room.addObstacle(Obstacle(100, 200, 100, 300, 10, 5)); // Mur vertical, épaisseur 10, 10 dB d'atténuation

    // Ajout d'un mur horizontal avec épaisseur
    room.addObstacle(Obstacle(50, 50, 250, 50, 5, 3)); // Mur horizontal, épaisseur 5, 8 dB d'atténuation

    // Ajout d'un mur rectangulaire
    room.addObstacle(Obstacle(400, 450, 600, 650, 10, 10));  // Mur rectangulaire, 10 dB d'atténuation

    // Ajout d'un mur circulaire
    room.addObstacle(Obstacle(200, 300, 15, 5));    // Mur circulaire, 5 dB d'atténuation
    room.addObstacle(Obstacle(600, 200, 30, 10));

    // Calcul de la puissance en chaque point
    room.computeSignalMap();

    // Marquer les obstacles sur la heatmap
    room.markObstaclesOnPowerMap();
    
    // Export en CSV
    //room.exportToCSV("heatmap.csv");

    std::cout << "Done!" << std::endl;

    displaying(&room.powerMap);

    return 0;
}