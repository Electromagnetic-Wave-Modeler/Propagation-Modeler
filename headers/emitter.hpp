#ifndef EMMITTER_HPP
#define EMMITTER_HPP

const double SPEED_OF_LIGHT = 3e8;  // en m/s

#define RESOLUTION_FACTOR 100


// Émetteur Wi-Fi
class Emitter {
    public:
        double x, y, power, frequency;
    
        Emitter(double x, double y, double power, double frequency) 
            : x(x), y(y), power(power), frequency(frequency) {}
    
        // Calcule la puissance reçue à une distance donnée (sans obstacles)
        double computePower(double x_target, double y_target) const {
            double d = std::sqrt(std::pow((x_target - x)/RESOLUTION_FACTOR, 2) + 
                                 std::pow((y_target - y)/RESOLUTION_FACTOR, 2));
            if (d < 0.001) return power; // Éviter la division par zéro
    
            // Pas besoin de calculer la longueur d'onde si on ne l'utilise pas directement
            double fspl = 20 * std::log10(d) + 20 * std::log10(frequency) + 
                          20 * std::log10(4 * M_PI / SPEED_OF_LIGHT);
            
            return power - fspl; // En dB
        }
    
        // Getters
        double getX() const { return x; }
        double getY() const { return y; }
};

#endif // EMMITTER_HPP