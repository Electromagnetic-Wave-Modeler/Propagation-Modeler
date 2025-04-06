#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <ostream>

#include "../headers/obstacle.hpp"

MurDroit::MurDroit(double x1, double y1, double x2, double y2, double thickness, double attenuation)
: Mur(x1, y1, x2, y2, thickness, attenuation) {
    // Normaliser les coordonnées pour que (x1,y1) soit toujours le coin inférieur gauche
    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);
}

bool MurDroit::isPointInside(double px, double py) const {
    // Pour un mur vertical
    if (std::abs(x1 - x2) < 0.001) {
        return (px >= (x1 - thickness/2) && px <= (x1 + thickness/2) &&
                py >= y1 && py <= y2);
    }
    // Pour un mur horizontal
    else if (std::abs(y1 - y2) < 0.001) {
        return (py >= (y1 - thickness/2) && py <= (y1 + thickness/2) &&
                px >= x1 && px <= x2);
    }
    std::cout << "Erreur : Mur ni vertical ni horizontal" << std::endl;
    return false;
}

bool MurDroit::isBlocking(double x, double y, double emitter_x, double emitter_y) const {
    // Vérification rapide: si l'émetteur ou le point est à l'intérieur de l'obstacle
    if (isPointInside(x, y) || isPointInside(emitter_x, emitter_y)) return true;
    else return false;
    

    // Cas vertical optimisé (avec épaisseur)
    if (std::abs(x1 - x2) < EPSILON) {

        double wall_x = x1;
        double left = wall_x - thickness / 2;
        double right = wall_x + thickness / 2;

        // Si l'émetteur et le point cible sont du même côté du mur, pas d'intersection
        if ((emitter_x < left && x < left) || (emitter_x > right && x > right)) {
            return false;
        }

        // Si le mur est entre l'émetteur et le point cible
        if ((emitter_x <= left && x >= left) || (emitter_x >= right && x <= right)) {
            // Calcul du point d'intersection
            double dx = x - emitter_x;
            if (std::abs(dx) < EPSILON) {
                // Ligne verticale - vérifier si elle traverse le mur
                return (std::min(y, emitter_y) <= y2 && std::max(y, emitter_y) >= y1);
            }

            double slope = (y - emitter_y) / dx;
            double b = emitter_y - slope * emitter_x;

            // Calcul des points d'intersection avec les deux faces du mur
            double t_left = (left - emitter_x) / dx;
            double y_left = slope * left + b;
            bool valid_left = (t_left >= 0 && t_left <= 1) && 
                            (y_left >= y1 - EPSILON && y_left <= y2 + EPSILON);

            double t_right = (right - emitter_x) / dx;
            double y_right = slope * right + b;
            bool valid_right = (t_right >= 0 && t_right <= 1) && 
                            (y_right >= y1 - EPSILON && y_right <= y2 + EPSILON);

            return valid_left || valid_right;
        }

        return false;
    }

    
    // Cas horizontal optimisé (avec épaisseur)
    if (std::abs(y1 - y2) < EPSILON) {

        double wall_y = y1;
        double bottom = wall_y - thickness / 2;
        double top = wall_y + thickness / 2;

        // Si l'émetteur et le point cible sont du même côté du mur, pas d'intersection
        if ((emitter_y < bottom && y < bottom) || (emitter_y > top && y > top)) {
            return false;
        }

        // Si le mur est entre l'émetteur et le point cible
        if ((emitter_y <= bottom && y >= bottom) || (emitter_y >= top && y <= top)) {
            // Calcul du point d'intersection
            double dy = y - emitter_y;
            if (std::abs(dy) < EPSILON) {
                // Ligne horizontale - vérifier si elle traverse le mur
                return (std::min(x, emitter_x) <= x2 && std::max(x, emitter_x) >= x1);
            }

            double slope = (x - emitter_x) / dy;
            double b = emitter_x - slope * emitter_y;

            // Calcul des points d'intersection avec les deux faces du mur
            double t_bottom = (bottom - emitter_y) / dy;
            double x_bottom = slope * bottom + b;
            bool valid_bottom = (t_bottom >= 0 && t_bottom <= 1) && 
                            (x_bottom >= x1 - EPSILON && x_bottom <= x2 + EPSILON);

            double t_top = (top - emitter_y) / dy;
            double x_top = slope * top + b;
            bool valid_top = (t_top >= 0 && t_top <= 1) && 
                            (x_top >= x1 - EPSILON && x_top <= x2 + EPSILON);

            return valid_bottom || valid_top;
        }

        return false;
    }
} 