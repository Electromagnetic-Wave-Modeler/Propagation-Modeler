#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <ostream>

#include "../headers/obstacle.hpp"

/**
* Constructeur pour les obstacles circulaires
* @param cx,cy Coordonnées du centre du cercle
* @param radius Rayon du cercle
* @param attenuation Perte de signal en dB lors de la traversée
*/
obstacleCirculaire::obstacleCirculaire(double cx, double cy, double radius, double attenuation)
: Obstacle(attenuation), cx(cx), cy(cy), radius(radius) {}

bool obstacleCirculaire::isPointInside(double px, double py) const{
    // Formule de distance au carré pour éviter la racine
    const double dx = px - cx;
    const double dy = py - cy;
    return (dx*dx + dy*dy) <= (radius * radius) + EPSILON;
}

void obstacleCirculaire::getExpandedBounds(double& min_x, double& min_y, double& max_x, double& max_y) const {
    min_x = cx - radius;
    min_y = cy - radius;
    max_x = cx + radius;
    max_y = cy + radius;
}

bool obstacleCirculaire::isBlocking(double x, double y, double emitter_x, double emitter_y) const {
    // Vérification rapide: si l'émetteur ou le point est à l'intérieur de l'obstacle
    if (isPointInside(x, y) || isPointInside(emitter_x, emitter_y)) {
        return true;
    }

    double dx = x - emitter_x;
    double dy = y - emitter_y;
    double fx = emitter_x - cx;
    double fy = emitter_y - cy;

    double a = dx * dx + dy * dy;
    double b = 2 * (fx * dx + fy * dy);
    double c = fx * fx + fy * fy - radius * radius;

    double discriminant = b * b - 4 * a * c;
    if (discriminant < 0) return false; // Pas d'intersection

    double t1 = (-b - std::sqrt(discriminant)) / (2 * a);
    double t2 = (-b + std::sqrt(discriminant)) / (2 * a);

    return (t1 >= 0 && t1 <= 1) || (t2 >= 0 && t2 <= 1);
}