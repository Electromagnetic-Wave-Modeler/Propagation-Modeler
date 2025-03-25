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
                double min_x = std::min(obstacle.x1, obstacle.x2) - obstacle.thickness/2;
                double max_x = std::max(obstacle.x1, obstacle.x2) + obstacle.thickness/2;
                double min_y = std::min(obstacle.y1, obstacle.y2) - obstacle.thickness/2;
                double max_y = std::max(obstacle.y1, obstacle.y2) + obstacle.thickness/2;
            
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