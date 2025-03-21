#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>

#define RESOLUTION_FACTOR 100
#define PRECISION 0.005
#define PRECISION_INCREMENT 0.05

const double SPEED_OF_LIGHT = 3e8;  // en m/s

// Émetteur Wi-Fi
class Emitter {

    public:

        double x, y, power, frequency;

        Emitter(double x, double y, double power, double frequency) : x(x), y(y), power(power), frequency(frequency) {
            // Appel initialisé
        }

        // Calcule la puissance reçue à une distance donnée (sans obstacles)
        double computePower(double x_target, double y_target) const {
            double d = std::sqrt(std::pow((x_target - x)/RESOLUTION_FACTOR, 2) + std::pow((y_target - y)/RESOLUTION_FACTOR, 2));
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
    int x1, y1, x2, y2;
    double attenuation; // dB de perte

    Obstacle(double x1, double y1, double x2, double y2, double attenuation)
        : x1(x1), y1(y1), x2(x2), y2(y2), attenuation(attenuation) {}

    bool isBlocking(double x, double y, double emitter_x, double emitter_y) const {

        // Formule de l'équation de la droite entre le point courant et la source
        double a = x - emitter_x;
        double b = y - emitter_y;
        double c = b / a;
        double d = emitter_y - c * emitter_x;
        // std::cout << c << std::endl;

        double distance_emitter = std::pow(y - emitter_y,2) + std::pow(x - emitter_x,2);
        if (x < emitter_x) distance_emitter = -distance_emitter;

        //l'origine est le point (x1, y1), donc droite d'équation y = c * x

        //si l'obstacle passe par la droite, alors est bloquant
        for(double i = x1; i <= x2; i += PRECISION_INCREMENT){
            for(double j = y1; j < y2; j += PRECISION_INCREMENT){

                double distance = std::pow(j - emitter_y, 2) + std::pow(i - emitter_x, 2);

                if (((c * i + d) >= (j - j * PRECISION) ) && ((c * i + d) <=  (j + j * PRECISION)) && (distance <= distance_emitter)) {
                    return true;
                }
            }
        }

        return false;
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
        powerMap.resize(height, std::vector<double>(width, -90.0)); // -90 dB par défaut
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
                        if (obstacle.isBlocking(x, y, emitter.x, emitter.y)) {
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
    Room room(300, 400);

    // Ajout d'un émetteur Wi-Fi
    room.addEmitter(Emitter(150, 150, -30, 2.4e9)); // 30 dBm, 2.4 GHz

    // Ajout d'un mur qui bloque partiellement
    room.addObstacle(Obstacle(200, 0, 200, 300, 10)); // Mur vertical, 1 dB d'atténuation

    // Calcul de la puissance en chaque point
    room.computeSignalMap();

    // Export en CSV
    room.exportToCSV("heatmap.csv");

    std::cout << "Done!" << std::endl;

    return 0;
}
