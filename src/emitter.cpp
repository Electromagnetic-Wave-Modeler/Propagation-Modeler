#include "../headers/emitter.hpp"
#include <cmath>

const double SPEED_OF_LIGHT = 3e8;  // en m/s

Emitter::Emitter(double x, double y, double power, double frequency) 
: x(x), y(y), power(power), frequency(frequency) {}

double Emitter::computePower(double x_target, double y_target) const {
    double d = std::sqrt(std::pow((x_target - x)/RESOLUTION_FACTOR, 2) + 
                         std::pow((y_target - y)/RESOLUTION_FACTOR, 2));
    if (d < 0.001) return power; // Éviter la division par zéro

    // Pas besoin de calculer la longueur d'onde si on ne l'utilise pas directement
    double fspl = 20 * std::log10(d) + 20 * std::log10(frequency) + 
                  20 * std::log10(4 * M_PI / SPEED_OF_LIGHT);
    
    return power - fspl; // En dB
}

double Emitter::getX() const { return x; }

double Emitter::getY() const { return y; }