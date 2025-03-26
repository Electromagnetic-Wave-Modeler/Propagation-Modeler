#ifndef OBSTACLE_HPP
#define OBSTACLE_HPP

enum class ObstacleType { RECTANGLE, CIRCLE };

// Mur / obstacle avec épaisseur
class Obstacle {

    public:

        ObstacleType type;

        // Constante pour les comparaisons de précision flottante
        static constexpr double EPSILON = 1e-6;

        // Pour les rectangles
        double x1, y1, x2, y2;
        double thickness; // Épaisseur du mur en unités de grille
        
        // Pour les cercles
        double cx, cy, radius;

        double attenuation; // dB de perte
    
        Obstacle(double x1, double y1, double x2, double y2, double thickness, double attenuation)
            : type(ObstacleType::RECTANGLE), x1(x1), y1(y1), x2(x2), y2(y2), thickness(thickness), attenuation(attenuation) {
            // Normaliser les coordonnées pour que (x1,y1) soit toujours le coin inférieur gauche
            if (x1 > x2) std::swap(x1, x2);
            if (y1 > y2) std::swap(y1, y2);
        }

        Obstacle(double cx, double cy, double radius, double attenuation)
        : type(ObstacleType::CIRCLE), cx(cx), cy(cy), radius(radius), attenuation(attenuation),
          x1(0), y1(0), x2(0), y2(0), thickness(0) {}

        // Obtenir la limite de l'obstacle (en tenant compte de l'épaisseur)
        void getExpandedBounds(double& min_x, double& min_y, double& max_x, double& max_y) const {
            min_x = std::min(x1, x2) - thickness/2;
            max_x = std::max(x1, x2) + thickness/2;
            min_y = std::min(y1, y2) - thickness/2;
            max_y = std::max(y1, y2) + thickness/2;
            //std::cout << "min_x: " << min_x << " min_y: " << min_y << " max_x: " << max_x << " max_y: " << max_y << std::endl;
        }
    
        // Vérifier si un point est à l'intérieur de l'obstacle (avec épaisseur)
        bool isPointInside(double px, double py) const {
            if (type == ObstacleType::RECTANGLE) {
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
                // Pour les autres types d'obstacles (rectangles, etc.)
                else {
                    return (px >= x1 && px <= x2 && py >= y1 && py <= y2);
                }
            } else if (type == ObstacleType::CIRCLE) {
                // Pour un mur circulaire
                double dx = px - cx;
                double dy = py - cy;
                return (dx * dx + dy * dy) <= (radius * radius);
            }
            return false;    
        }

    // Vérifier si un segment (émetteur → point) est bloqué par l'obstacle
    bool isBlocking(double x, double y, double emitter_x, double emitter_y) const {

        // Vérification rapide: si l'émetteur ou le point est à l'intérieur de l'obstacle
        if (isPointInside(x, y) || isPointInside(emitter_x, emitter_y)) {
            return true;
        }

        if (type == ObstacleType::RECTANGLE) {
            return isRectangleBlocking(x, y, emitter_x, emitter_y);
        } 
        else if (type == ObstacleType::CIRCLE) {
            return isCircleBlocking(x, y, emitter_x, emitter_y);
        }
        return false;
    }

    private:

        // Optimisation: intersection entre ligne (émetteur-point) et obstacle avec épaisseur
        bool isRectangleBlocking(double x, double y, double emitter_x, double emitter_y) const {
    
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
            
            // Cas général (rectangle)
            double expanded_x1 = x1 - thickness / 2;
            double expanded_y1 = y1 - thickness / 2;
            double expanded_x2 = x2 + thickness / 2;
            double expanded_y2 = y2 + thickness / 2;

            // Vérifier si le segment émetteur-point intersecte le rectangle élargi
            return segmentIntersectsRectangle(
                emitter_x, emitter_y, x, y,
                expanded_x1, expanded_y1, expanded_x2, expanded_y2
            );
        }

        // Vérifier si un segment intersecte un rectangle (algorithme de Liang-Barsky)
        bool segmentIntersectsRectangle(
            double x0, double y0, double x1, double y1,
            double rect_x1, double rect_y1, double rect_x2, double rect_y2
        ) const {
            double t_min = 0.0;
            double t_max = 1.0;
            double dx = x1 - x0;
            double dy = y1 - y0;

            // Vérifier chaque bord du rectangle
            double p[4] = { -dx, dx, -dy, dy };
            double q[4] = { x0 - rect_x1, rect_x2 - x0, y0 - rect_y1, rect_y2 - y0 };

            for (int i = 0; i < 4; i++) {
                if (std::abs(p[i]) < EPSILON) {
                    // Segment parallèle au bord
                    if (q[i] < 0) return false;
                } else {
                    double t = q[i] / p[i];
                    if (p[i] < 0) {
                        if (t > t_min) t_min = t;
                    } else {
                        if (t < t_max) t_max = t;
                    }
                    if (t_min > t_max) return false;
                }
            }

            return true;
        }

    
        // Vérification de blocage pour un cercle
        bool isCircleBlocking(double x, double y, double emitter_x, double emitter_y) const {
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
    };

#endif // OBSTACLE_HPP