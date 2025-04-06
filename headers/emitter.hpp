#ifndef EMMITTER_HPP
#define EMMITTER_HPP


#define RESOLUTION_FACTOR 100


// Émetteur Wi-Fi
class Emitter {
    public:
        double x, y, power, frequency;
    
        Emitter(double x, double y, double power, double frequency);
    
        // Calcule la puissance reçue à une distance donnée (sans obstacles)
        double computePower(double x_target, double y_target) const;
    
        // Getters
        double getX() const;
        double getY() const;
        
};

#endif // EMMITTER_HPP