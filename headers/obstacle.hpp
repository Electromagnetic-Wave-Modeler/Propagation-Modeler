#ifndef OBSTACLE_HPP
#define OBSTACLE_HPP

// Mur / obstacle avec épaisseur
class Obstacle {
    public:
        double x1, y1, x2, y2;
        double thickness; // Épaisseur du mur en unités de grille
        double attenuation; // dB de perte
    
        Obstacle(double x1, double y1, double x2, double y2, double thickness, double attenuation)
            : x1(x1), y1(y1), x2(x2), y2(y2), thickness(thickness), attenuation(attenuation) {
            // Normaliser les coordonnées pour que (x1,y1) soit toujours le coin inférieur gauche
            if (x1 > x2) std::swap(x1, x2);
            if (y1 > y2) std::swap(y1, y2);
        }
    
        // Vérifier si un point est à l'intérieur de l'obstacle (avec épaisseur)
        bool isPointInside(double px, double py) const {
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
        }
    
        // Optimisation: intersection entre ligne (émetteur-point) et obstacle avec épaisseur
        bool isBlocking(double x, double y, double emitter_x, double emitter_y) const {
            // Vérification rapide: si l'émetteur ou le point est à l'intérieur de l'obstacle
            if (isPointInside(x, y) || isPointInside(emitter_x, emitter_y)) {
                return true;
            }
    
            // Cas vertical optimisé (avec épaisseur)
            if (std::abs(x1 - x2) < 0.001) {
                double wall_x = x1;
                
                // Si l'émetteur et le point cible sont du même côté du mur, pas d'intersection
                if ((emitter_x < wall_x - thickness/2 && x < wall_x - thickness/2) || 
                    (emitter_x > wall_x + thickness/2 && x > wall_x + thickness/2)) {
                    return false;
                }
                
                // Si le mur est entre l'émetteur et le point cible
                if ((emitter_x <= wall_x - thickness/2 && x >= wall_x - thickness/2) ||
                    (emitter_x >= wall_x + thickness/2 && x <= wall_x + thickness/2)) {
                    // Calcul du point d'intersection
                    if (std::abs(x - emitter_x) < 0.001) {
                        // Ligne verticale - vérifier si elle traverse le mur
                        if (std::min(y, emitter_y) <= y2 && std::max(y, emitter_y) >= y1) {
                            return true;
                        }
                        return false;
                    }
                    
                    double slope = (y - emitter_y) / (x - emitter_x);
                    double b = emitter_y - slope * emitter_x;
                    
                    // Calcul des points d'intersection avec les deux faces du mur
                    double y_intersect1 = slope * (wall_x - thickness/2) + b;
                    double y_intersect2 = slope * (wall_x + thickness/2) + b;
                    
                    // Vérifier si l'un des points d'intersection est dans la plage du mur avec une marge de sécurité
                    return ((y_intersect1 >= y1 - 0.001 && y_intersect1 <= y2 + 0.001) || 
                            (y_intersect2 >= y1 - 0.001 && y_intersect2 <= y2 + 0.001));
                }
                
                return false;
            }
            
            // Cas horizontal optimisé (avec épaisseur)
            if (std::abs(y1 - y2) < 0.001) {
                double wall_y = y1;
                
                // Si l'émetteur et le point cible sont du même côté du mur, pas d'intersection
                if ((emitter_y < wall_y - thickness/2 && y < wall_y - thickness/2) || 
                    (emitter_y > wall_y + thickness/2 && y > wall_y + thickness/2)) {
                    return false;
                }
                
                // Si le mur est entre l'émetteur et le point cible
                if ((emitter_y <= wall_y - thickness/2 && y >= wall_y - thickness/2) ||
                    (emitter_y >= wall_y + thickness/2 && y <= wall_y + thickness/2)) {
                    // Calcul du point d'intersection
                    if (std::abs(y - emitter_y) < 0.001) {
                        // Ligne horizontale - vérifier si elle traverse le mur
                        if (std::min(x, emitter_x) <= x2 && std::max(x, emitter_x) >= x1) {
                            return true;
                        }
                        return false;
                    }
                    
                    double slope = (x - emitter_x) / (y - emitter_y);
                    double b = emitter_x - slope * emitter_y;
                    
                    // Calcul des points d'intersection avec les deux faces du mur
                    double x_intersect1 = slope * (wall_y - thickness/2) + b;
                    double x_intersect2 = slope * (wall_y + thickness/2) + b;
                    
                    // Vérifier si l'un des points d'intersection est dans la plage du mur avec une marge de sécurité
                    return ((x_intersect1 >= x1 - 0.001 && x_intersect1 <= x2 + 0.001) || 
                            (x_intersect2 >= x1 - 0.001 && x_intersect2 <= x2 + 0.001));
                }
                
                return false;
            }
            
            // Pour les obstacles rectangulaires généraux avec épaisseur
            // Créer un rectangle plus grand pour inclure l'épaisseur
            double expanded_x1 = x1 - thickness/2;
            double expanded_y1 = y1 - thickness/2;
            double expanded_x2 = x2 + thickness/2;
            double expanded_y2 = y2 + thickness/2;
            
            // Segments qui composent les bords du rectangle élargi
            std::pair<std::pair<double, double>, std::pair<double, double>> segments[4] = {
                {{expanded_x1, expanded_y1}, {expanded_x2, expanded_y1}}, // segment bas
                {{expanded_x2, expanded_y1}, {expanded_x2, expanded_y2}}, // segment droit
                {{expanded_x2, expanded_y2}, {expanded_x1, expanded_y2}}, // segment haut
                {{expanded_x1, expanded_y2}, {expanded_x1, expanded_y1}}  // segment gauche
            };
            
            // Définir la ligne entre l'émetteur et le point cible
            double dx = x - emitter_x;
            double dy = y - emitter_y;
            
            // Si les points sont trop proches, considérer qu'il n'y a pas d'obstacle
            if (std::abs(dx) < 0.001 && std::abs(dy) < 0.001) {
                return false;
            }
            
            // Calcul des intersections avec chacun des segments du rectangle
            for (int i = 0; i < 4; i++) {
                double x1s = segments[i].first.first;
                double y1s = segments[i].first.second;
                double x2s = segments[i].second.first;
                double y2s = segments[i].second.second;
                
                double dxs = x2s - x1s;
                double dys = y2s - y1s;
                
                // Calcul du déterminant
                double det = dx * dys - dy * dxs;
                
                if (std::abs(det) < 0.001) continue; // Lignes parallèles
                
                double t = (dxs * (emitter_y - y1s) - dys * (emitter_x - x1s)) / det;
                double u = (dx * (emitter_y - y1s) - dy * (emitter_x - x1s)) / det;
                
                // Ajouter une petite marge pour éviter les erreurs d'arrondi
                if (t >= -0.001 && t <= 1.001 && u >= -0.001 && u <= 1.001) {
                    // Calculer les coordonnées du point d'intersection
                    double intersect_x = emitter_x + t * dx;
                    double intersect_y = emitter_y + t * dy;
                    
                    // Vérifier si le point d'intersection est entre l'émetteur et la cible
                    if (t >= 0 && t <= 1) {
                        return true;
                    }
                }
            }
            
            return false;
        }
    };

#endif // OBSTACLE_HPP