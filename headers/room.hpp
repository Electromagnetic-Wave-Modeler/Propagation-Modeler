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