#include <vector>
#include <fstream>
#include <iostream>
#include "emitter.hpp"
#include "../headers/obstacle.hpp"

#include "../headers/room.hpp"

Room::Room(int width, int height) : width(width), height(height) {
    powerMap.resize(height, std::vector<double>(width, -90.0)); // -90 dB par défaut (bruit de fond)
}

void Room::computeSignalMap() {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double totalPower = -100.0; // En dB
            
            for (const auto& emitter : emitters) {
                double power = emitter.computePower(x, y);

                for (const auto& obstacle : obstacles) {
                    if (obstacle->isBlocking(x, y, emitter.getX(), emitter.getY())) {
                        power -= obstacle->getAttenuation();
                    }
                }
                totalPower = std::max(totalPower, power);
            }
            powerMap[y][x] = totalPower;
        }
    }
}

void Room::markObstaclesOnPowerMap() {
    for (const auto& obstacle : obstacles) {
        double min_x, min_y, max_x, max_y;
        obstacle->getExpandedBounds(min_x, min_y, max_x, max_y); // Obtenir la zone d'influence

        // Conversion en indices de grille
        int start_x = std::max(0, static_cast<int>(std::floor(min_x)));
        int end_x = std::min(width-1, static_cast<int>(std::ceil(max_x)));
        int start_y = std::max(0, static_cast<int>(std::floor(min_y)));
        int end_y = std::min(height-1, static_cast<int>(std::ceil(max_y)));

        // Parcours de la zone potentiellement couverte
        for (int y = start_y; y <= end_y; y++) {
            for (int x = start_x; x <= end_x; x++) {
                if (obstacle->isPointInside(x, y)) { // Vérification précise
                    powerMap[y][x] = -555; // Marquage spécial
                }
            }
        }
    }

    // Marquage des bords de la salle comme obstacles
    markRoomBoundaries();
}

/**
 * Exporte la carte de puissance au format CSV
 * @param filename Nom du fichier de sortie
 */
void Room::exportToCSV(const std::string& filename) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Erreur d'ouverture du fichier !" << std::endl;
        return;
    }

    // Écriture ligne par ligne
    for (const auto& row : powerMap) {
        for (size_t i = 0; i < row.size(); i++) {
            file << row[i];
            if (i < row.size() - 1) file << ",";
        }
        file << "\n";
    }
    file.close();
    std::cout << "Carte exportée vers " << filename << std::endl;
}

// bool Room::deleteEmitter(double x, double y) {
//     for (auto it = emitters.begin(); it != emitters.end(); ++it) {
//         if (it->getX() == x && it->getY() == y) {
//             emitters.erase(it);
//             return true; // Émetteur supprimé
//         }
//     }
//     return false; // Émetteur non trouvé
// }

// bool Room::deleteObstacle(double x1, double y1, double x2, double y2) {
//     for (auto it = obstacles.begin(); it != obstacles.end(); ++it) {
//         // Vérifier si l'obstacle est un Mur en utilisant le cast dynamique
//         Mur* mur = dynamic_cast<Mur*>(*it);
        
//         // Si c'est un mur, vérifier ses coordonnées
//         if (mur != nullptr && 
//             std::abs(mur->x1 - x1) < 0.001 && 
//             std::abs(mur->y1 - y1) < 0.001 && 
//             std::abs(mur->x2 - x2) < 0.001 && 
//             std::abs(mur->y2 - y2) < 0.001) {
            
//             obstacles.erase(it);
//             return true; // Obstacle supprimé
//         }
//     }
//     return false; // Obstacle non trouvé
// }

void Room::markRoomBoundaries() {
    // Bords verticaux
    for (int y = 0; y < height; y++) {
        powerMap[y][0] = -555;        // Bord gauche
        powerMap[y][1] = -555;        // Zone de sécurité
        powerMap[y][width-1] = -555;  // Bord droit
        powerMap[y][width-2] = -555;  // Zone de sécurité
    }

    // Bords horizontaux
    for (int x = 0; x < width; x++) {
        powerMap[0][x] = -555;        // Bord supérieur
        powerMap[1][x] = -555;        // Zone de sécurité
        powerMap[height-1][x] = -555; // Bord inférieur
        powerMap[height-2][x] = -555;  // Zone de sécurité
    }
}
