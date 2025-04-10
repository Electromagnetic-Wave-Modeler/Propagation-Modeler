#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
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
    Room room(1220, 600); // Résolution Full HD

    // Ajout d'un émetteur Wi-Fi
    room.addEmitter(Emitter(150, 150, -30, 2.4e9)); // -30 dBm, 2.4 GHz
    room.addEmitter(Emitter(500, 500, -28, 2.4e9)); // -28 dBm, 2.4 GHz

    // Ajout d'un mur vertical avec épaisseur (utilisation de MurDroit)
    room.addObstacle(new MurDroit(100, 200, 100, 300, 10, 5)); // Mur vertical, épaisseur 10, 5 dB d'atténuation

    // Ajout d'un mur horizontal avec épaisseur (utilisation de MurDroit)

    // room.addObstacle(new MurDroit(50, 50, 250, 50, 15, 20)); // Mur horizontal, épaisseur 5, 3 dB d'atténuation

    room.addObstacle(new MurDroit(50, 50, 250, 50, 15, 20)); // Mur horizontal, épaisseur 5, 3 dB d'atténuation


    // Ajout d'un mur rectangulaire (utilisation de la classe Mur générique)
    room.addObstacle(new Mur(400, 450, 600, 650, 5, 10));  // Mur rectangulaire, 10 dB d'atténuation

    // Ajout de meubles circulaires (utilisation de obstacleCirculaire)
    room.addObstacle(new obstacleCirculaire(200, 300, 15, 5));    // Meuble rond, 5 dB d'atténuation
    room.addObstacle(new obstacleCirculaire(600, 200, 30, 10));   // Meuble rond, 10 dB d'atténuation

    // Calcul de la puissance en chaque point
    room.computeSignalMap();

    // Marquer les obstacles sur la heatmap
    room.markObstaclesOnPowerMap();
    
    // Export en CSV pour vérification des valeurs
    //room.exportToCSV("heatmap.csv");

    std::cout << "Carte de puissance calculee" << std::endl;

    displaying(&room);

    // Libération de la mémoire 
    for (auto obstacle : room.obstacles) {
        delete obstacle;
    }

    return 0;
}