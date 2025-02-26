#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>

const double SPEED_OF_LIGHT = 3e8;  // en m/s

// Émetteur Wi-Fi
class Emitter {

    private:

        double x, y, power, frequency;

    public:

        Emitter(double x, double y, double power, double frequency) : x(x), y(y), power(power), frequency(frequency) {
            // Appel initialisé
        }

        Emitter(double x, double y, double power, double frequency) {
            // Appel par défaut
            this->x = x;
            this->y = y;
            this->power = power;
            this->frequency = frequency;
        }

        // Calcule la puissance reçue à une distance donnée (sans obstacles)
        double computePower(double x_target, double y_target) const {
            double d = std::sqrt(std::pow(x_target - x, 2) + std::pow(y_target - y, 2));
            if (d == 0) return power; // Éviter la division par zéro

            double wavelength = SPEED_OF_LIGHT / frequency;
            double fspl = 20 * std::log10(d) + 20 * std::log10(frequency) + 20 * std::log10(4 * M_PI / SPEED_OF_LIGHT);
            
            return power - fspl; // En dB
        }

        // Getters
        double getX() const { return x; }
        double getY() const { return y; }

};

// Mur / obstacle
class Obstacle {
public:
    double x1, y1, x2, y2;
    double attenuation; // dB de perte

    Obstacle(double x1, double y1, double x2, double y2, double attenuation)
        : x1(x1), y1(y1), x2(x2), y2(y2), attenuation(attenuation) {}

    bool isBlocking(double x, double y, double emitter_x, double emitter_y) const {
        // Approximation : vérifie si le segment passe par l'obstacle
        return (x >= x1 && x <= x2 && y >= y1 && y <= y2);
    }
};

// Simulation avec grille
class Room {
public:
    int width, height;
    std::vector<Emitter> emitters;
    std::vector<Obstacle> obstacles;
    std::vector<std::vector<double>> powerMap;

    Room(int width, int height) : width(width), height(height) {
        powerMap.resize(height, std::vector<double>(width, -100.0)); // -100 dB par défaut
    }

    void addEmitter(Emitter e) {
        emitters.push_back(e);
    }

    void addObstacle(Obstacle o) {
        obstacles.push_back(o);
    }

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

int main() {
    Room room(50, 50);

    // Ajout d'un émetteur Wi-Fi
    room.addEmitter(Emitter(10, 10, 30, 2.4e9)); // 30 dBm, 2.4 GHz

    // Ajout d'un mur qui bloque partiellement
    room.addObstacle(Obstacle(20, 0, 20, 50, 10)); // Mur vertical, 10 dB d'atténuation

    // Calcul de la puissance en chaque point
    room.computeSignalMap();

    // Export en CSV
    room.exportToCSV("heatmap.csv");

    return 0;
}