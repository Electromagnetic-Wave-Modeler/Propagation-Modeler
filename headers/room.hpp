#ifndef ROOM_HPP
#define ROOM_HPP

#include <vector>
#include "emitter.hpp"
#include "obstacle.hpp"


// Simulation avec grille
class Room {
    public:
        int width, height;
        std::vector<Emitter> emitters;
        std::vector<Obstacle> obstacles;
        std::vector<std::vector<double>> powerMap;
    
        Room(int width, int height) : width(width), height(height) {
            powerMap.resize(height, std::vector<double>(width, -90.0)); // -90 dB par défaut
        }
    
        void addEmitter(Emitter e) {                    
            emitters.push_back(e);
        }
    
        void addObstacle(Obstacle o) {
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
                            if (obstacle.isBlocking(x, y, emitter.getX(), emitter.getY())) {
                                power -= obstacle.attenuation;
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
                // Obtenir la limite de l'obstacle (en tenant compte de l'épaisseur)
                double min_x, min_y, max_x, max_y;
                obstacle.getExpandedBounds(min_x, min_y, max_x, max_y);
            
                // Convertir en plage d'index de grille
                int start_x = static_cast<int>(std::floor(min_x));
                int end_x = static_cast<int>(std::ceil(max_x));
                int start_y = static_cast<int>(std::floor(min_y));
                int end_y = static_cast<int>(std::ceil(max_y));
            
                // Assurez que l'index est dans les limites
                start_x = std::max(0, start_x);
                end_x = std::min(width - 1, end_x);
                start_y = std::max(0, start_y);
                end_y = std::min(height - 1, end_y);
            
                // Traverser la zone de grille couverte d'obstacles
                for (int y = start_y; y <= end_y; y++) {
                    for (int x = start_x; x <= end_x; x++) {
                        if (obstacle.isPointInside(x, y)) {
                            powerMap[y][x] = -555; // Marquer comme valeur spéciale
                        }
                    }
                }

                // Marquez les bords de la pièce comme -555
                // Le bord gauche (x=0) et le bord droit (x = largeur-1)
                for (int y = 0; y < height; y++) {
                    powerMap[y][0] = -555;               // le bord gauche
                    powerMap[y][1] = -555;               // le bord gauche +1
                    powerMap[y][width - 1] = -555;       // le bord droit
                    powerMap[y][width - 2] = -555;       // le bord droit -1
                }
            
                // Le bord supérieur (y=0) et le bord inférieur (y=height-1)
                for (int x = 0; x < width; x++) {
                    powerMap[0][x] = -555;               // Le bord supérieur
                    powerMap[1][x] = -555;               // Le bord supérieur + 1
                    powerMap[height - 1][x] = -555;      // Le bord inférieur
                    powerMap[height - 2][x] = -555;      // Le bord inférieur - 1
                }
            }
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