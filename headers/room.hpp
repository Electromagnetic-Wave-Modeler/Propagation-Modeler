#ifndef ROOM_HPP
#define ROOM_HPP

#include <vector>
#include <fstream>
#include <iostream>
#include "emitter.hpp"
#include "obstacle.hpp"

/**
 * Classe représentant une salle de simulation de propagation de signaux
 * Gère une grille 2D avec des émetteurs et des obstacles
 */
class Room {
    private :
    /**
     * Marque les bords de la salle comme zones obstacles
     * Ajoute une bordure de sécurité de 2 unités
     */
    void markRoomBoundaries() {
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


    public:
        int width, height;
        std::vector<Emitter> emitters;
        std::vector<Obstacle*> obstacles;
        std::vector<std::vector<double>> powerMap;
    
        Room(int width, int height) : width(width), height(height) {
            powerMap.resize(height, std::vector<double>(width, -90.0)); // -90 dB par défaut
        }
    
        void addEmitter(Emitter e) {                    
            emitters.push_back(e);
        }
    
        void addObstacle(Obstacle* o) {
            
            obstacles.push_back(o);
        }
    
        // Optimisation: calcul parallèle des signaux ou pré-calcul des obstacles
        void computeSignalMap() {
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

        // Marquer les obstacles sur la heatmap
        void markObstaclesOnPowerMap() {
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

        void exportToCSV(const std::string& filename) {
            std::ofstream file(filename);
            if (!file) {
                std::cerr << "Error opening file!" << std::endl;
                return;
            }
    
            for (const auto& row : powerMap) {
                for (size_t i = 0; i < row.size(); i++) {
                    file << row[i];
                    if (i < row.size() - 1) file << ",";
                }
                file << "\n";
            }
            file.close();
            std::cout << "Exported to " << filename << std::endl;
        }

    };

#endif // ROOM_HPP