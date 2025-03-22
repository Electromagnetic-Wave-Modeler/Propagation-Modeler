#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <algorithm>

#define RESOLUTION_FACTOR 100
#define PRECISION 0.005
#define PRECISION_INCREMENT 0.05

const double SPEED_OF_LIGHT = 3e8;  // en m/s







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

    return 0;
}