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

    // Émetteurs Wi-Fi (un dans chaque pièce)
    room.addEmitter(Emitter(300, 200, -30, 2.4e9));  // Émetteur dans le salon


    // Murs extérieurs de l'appartement (bordures)
    // Les bordures sont déjà les limites de la heatmap, pas besoin d'ajouter de murs

    // Murs intérieurs pour diviser l'appartement en 3 pièces

    // Mur vertical pour séparer salon et chambre
    room.addObstacle(new MurDroit(600, 0, 600, 200, 15, 10));

    room.addObstacle(new MurDroit(600, 250, 600, 350, 15, 10));

    room.addObstacle(new MurDroit(900, 250, 900, 350, 5, 50));


    // Mur horizontal partiel pour séparer le salon/chambre de la cuisine/salle de bain
    room.addObstacle(new MurDroit(0, 350, 400, 350, 15, 10));

    room.addObstacle(new MurDroit(500, 350, 1220, 350, 15, 10));


    // Salon
    // room.addObstacle(new obstacleCirculaire(150, 150, 20, 5));    



    // Cuisine
    room.addObstacle(new obstacleCirculaire(200, 500, 25, 6));   // Table à manger
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