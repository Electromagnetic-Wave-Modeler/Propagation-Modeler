#ifndef OBSTACLE_HPP
#define OBSTACLE_HPP

#include <cmath>
#include <algorithm>
#include <stdexcept>



/**
 * Classe représentant un obstacle dans une simulation de propagation de signal.
 * Gère deux types d'obstacles : rectangles avec épaisseur et cercles.
 * Fournit des méthodes pour détecter les collisions et calculer les zones d'influence.
 */
class Obstacle {
    protected : 

        double attenuation;

    public:

        Obstacle(double attenuation) : attenuation(attenuation) {}

        // Constante pour les comparaisons de précision flottante
        static constexpr double EPSILON = 1e-6;

        // Destructeur virtuel pour permettre une destruction correcte des classes dérivées
        virtual ~Obstacle() {};
        
        /**
        * Vérifie si un point (x,y) se trouve dans la zone d'influence de l'obstacle
        * @param px Coordonnée X du point à tester
        * @param py Coordonnée Y du point à tester
        * @return true si le point est dans l'obstacle ou sur son bord
        */
        // Vérifie si un point est à l'intérieur de l'obstacle
        virtual bool isPointInside(double px, double py) const = 0;
        
        // Vérifie si l'obstacle bloque la ligne entre un émetteur et un point
        virtual bool isBlocking(double x, double y, double emitter_x, double emitter_y) const = 0;

        /**
        * Calcule la boîte englobante étendue de l'obstacle
        * @param[out] min_x,min_y Coin inférieur gauche de la zone d'influence
        * @param[out] max_x,max_y Coin supérieur droit de la zone d'influence
        */
        virtual void getExpandedBounds(double& min_x, double& min_y, double& max_x, double& max_y) const = 0;

        double getAttenuation() const { return attenuation; }
        
};


/**
* Constructeur pour les obstacles rectangulaires
* @param x1,y1 Coordonnées du premier point définissant le rectangle
* @param x2,y2 Coordonnées du deuxième point définissant le rectangle
* @param thickness Épaisseur de l'obstacle perpendiculairement au segment
* @param attenuation Perte de signal en dB lors de la traversée
*/
class Mur : public Obstacle{
    protected :
        double x1, y1;      // Point de départ du segment
        double x2, y2;      // Point d'arrivée du segment
        double thickness;   // Épaisseur perpendiculaire au segment


    private:
        /**
        * Structure stockant les paramètres géométriques pré-calculés
        * Optimise les calculs répétitifs de collision et de projection
        */
        struct ParametresGeometriques {
            double mid_x, mid_y;       ///< Centre géométrique du segment de base
            double dir_x, dir_y;       ///< Vecteur directionnel du segment (non normalisé)
            double longueur_sq;        ///< Longueur au carré du segment (optimisation)
            double longueur;           ///< Longueur réelle du segment
            double dir_unit_x;         ///< Vecteur directionnel unitaire
            double dir_unit_y;
            double perp_dir_x;         ///< Vecteur perpendiculaire unitaire (rotation de 90°)
            double perp_dir_y;
            double demi_longueur;      ///< Demi-longueur du segment
            double demi_epaisseur;     ///< Demi-épaisseur de l'obstacle
        };

        ParametresGeometriques params_geo; ///< Cache des paramètres géométriques

        /**
        * Prépare les paramètres géométriques essentiels
        * Calculs effectués une seule fois lors de la construction
        */
        void precalculerParametresGeometriques() {
            // Calcul du point médian
            params_geo.mid_x = (x1 + x2) * 0.5;
            params_geo.mid_y = (y1 + y2) * 0.5;

            // Vecteur directionnel primaire
            params_geo.dir_x = x2 - x1;
            params_geo.dir_y = y2 - y1;
            params_geo.longueur_sq = params_geo.dir_x*params_geo.dir_x + params_geo.dir_y*params_geo.dir_y;

            // Gestion des segments dégénérés
            if (params_geo.longueur_sq < EPSILON * EPSILON) {
                params_geo.longueur = 0;
                return;
            }

            // Normalisation des vecteurs
            params_geo.longueur = std::sqrt(params_geo.longueur_sq);
            params_geo.dir_unit_x = params_geo.dir_x / params_geo.longueur;
            params_geo.dir_unit_y = params_geo.dir_y / params_geo.longueur;

            // Calcul du vecteur perpendiculaire (sens trigonométrique)
            params_geo.perp_dir_x = -params_geo.dir_unit_y;
            params_geo.perp_dir_y = params_geo.dir_unit_x;

            // Pré-calcul des valeurs fréquemment utilisées
            params_geo.demi_longueur = params_geo.longueur * 0.5;
            params_geo.demi_epaisseur = thickness * 0.5;
        }

        /**
        * Implémente le théorème de l'axe séparateur (SAT)
        * @param e_axe,e_perp Projection de l'émetteur
        * @param p_axe,p_perp Projection du récepteur
        * @param min_a,max_a Plage de l'axe principal
        * @param min_p,max_p Plage de l'axe perpendiculaire
        * @return true si les projections se chevauchent sur tous les axes
        */
        bool satTest(double e_axe, double e_perp, double p_axe, double p_perp, double min_a, double max_a, double min_p, double max_p) const {
            // Calcul des intervalles du segment
            const double seg_min_axe = std::min(e_axe, p_axe);
            const double seg_max_axe = std::max(e_axe, p_axe);
            const double seg_min_perp = std::min(e_perp, p_perp);
            const double seg_max_perp = std::max(e_perp, p_perp);

            // Exclusion rapide par AABB
            if (seg_max_axe < min_a - EPSILON || seg_min_axe > max_a + EPSILON) return false;
            if (seg_max_perp < min_p - EPSILON || seg_min_perp > max_p + EPSILON) return false;

            // Calcul des paramètres de ligne
            const double delta_axe = p_axe - e_axe;
            const double delta_perp = p_perp - e_perp;
            double t_enter = 0.0, t_exit = 1.0;

            // Test de l'axe principal
            if (std::abs(delta_axe) > EPSILON) {
            double t1 = (min_a - e_axe) / delta_axe;
            double t2 = (max_a - e_axe) / delta_axe;
            if (t1 > t2) std::swap(t1, t2);
            t_enter = std::max(t_enter, t1);
            t_exit = std::min(t_exit, t2);
            if (t_enter > t_exit) return false;
            }

            // Test de l'axe perpendiculaire
            if (std::abs(delta_perp) > EPSILON) {
            double t1 = (min_p - e_perp) / delta_perp;
            double t2 = (max_p - e_perp) / delta_perp;
            if (t1 > t2) std::swap(t1, t2);
            t_enter = std::max(t_enter, t1);
            t_exit = std::min(t_exit, t2);
            if (t_enter > t_exit) return false;
            }

            return (t_enter <= t_exit) && (t_exit >= 0.0) && (t_enter <= 1.0);
        }


    public :
        Mur(double x1, double y1, double x2, double y2, double thickness, double attenuation)
        : Obstacle(attenuation), x1(x1), y1(y1), x2(x2), y2(y2), thickness(thickness) {
            // Normaliser les coordonnées pour que (x1,y1) soit toujours le coin inférieur gauche
            if (x1 > x2) std::swap(x1, x2);
            if (y1 > y2) std::swap(y1, y2);
            precalculerParametresGeometriques();  // Pré-calcul des paramètres géométriques
        }

        double getX1() const { return x1; }
        double getY1() const { return y1; }
        double getX2() const { return x2; }
        double getY2() const { return y2; }

        
        void getExpandedBounds(double& min_x, double& min_y, double& max_x, double& max_y) const override {
            // Utilisation des paramètres pré-calculés
            const auto& pg = params_geo;

            // Cas dégénéré (segment de longueur nulle)
            if (pg.longueur_sq < EPSILON * EPSILON) {
                min_x = pg.mid_x - pg.demi_epaisseur;
                min_y = pg.mid_y - pg.demi_epaisseur;
                max_x = pg.mid_x + pg.demi_epaisseur;
                max_y = pg.mid_y + pg.demi_epaisseur;
                return;
            }

            // Calcul des extensions perpendiculaires
            const double extend_x = pg.perp_dir_x * pg.demi_epaisseur;
            const double extend_y = pg.perp_dir_y * pg.demi_epaisseur;

            // Génération des 4 sommets étendus
            const double sommets[4][2] = {
                {x1 + extend_x, y1 + extend_y},  // Extension positive sur le premier point
                {x1 - extend_x, y1 - extend_y},  // Extension négative sur le premier point
                {x2 + extend_x, y2 + extend_y},  // Extension positive sur le deuxième point
                {x2 - extend_x, y2 - extend_y}   // Extension négative sur le deuxième point
            };

            // Calcul des limites de la boîte englobante
            min_x = std::min({sommets[0][0], sommets[1][0], sommets[2][0], sommets[3][0]});
            max_x = std::max({sommets[0][0], sommets[1][0], sommets[2][0], sommets[3][0]});
            min_y = std::min({sommets[0][1], sommets[1][1], sommets[2][1], sommets[3][1]});
            max_y = std::max({sommets[0][1], sommets[1][1], sommets[2][1], sommets[3][1]});
        }    

        // Vérifier si un point est à l'intérieur de l'obstacle (avec épaisseur)
        bool isPointInside(double px, double py) const override {
            const auto& pg = params_geo;

            // Cas dégénéré traité comme un cercle
            if (pg.longueur_sq < EPSILON * EPSILON) {
                const double dx = px - pg.mid_x;
                const double dy = py - pg.mid_y;
                return (dx*dx + dy*dy) <= (pg.demi_epaisseur * pg.demi_epaisseur) + EPSILON;
            }

            // Calcul du vecteur relatif au milieu
            const double dx = px - pg.mid_x;
            const double dy = py - pg.mid_y;

            // Projections sur les axes local et perpendiculaire
            const double proj_axe = dx * pg.dir_unit_x + dy * pg.dir_unit_y;
            const double proj_perp = dx * pg.perp_dir_x + dy * pg.perp_dir_y;

            // Vérification des limites
            return (std::abs(proj_axe) <= pg.demi_longueur + EPSILON) && 
                   (std::abs(proj_perp) <= pg.demi_epaisseur + EPSILON);
        }

        /**
        * Algorithme de Liang-Barsky pour l'intersection segment-rectangle
        * @param rect_x1,rect_y1 Coin inférieur gauche
        * @param rect_x2,rect_y2 Coin supérieur droit
        * @return true si intersection détectée
        */
        bool segmentIntersectsRectangle(double x0, double y0, double x1, double y1, double rect_x1, double rect_y1, double rect_x2, double rect_y2) const {
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

        bool isBlocking(double x, double y, double emitter_x, double emitter_y) const {
            // Vérification rapide: si l'émetteur ou le point est à l'intérieur de l'obstacle
            if (isPointInside(x, y) || isPointInside(emitter_x, emitter_y)) {
                return true;
            }

            const auto& pg = params_geo;
            if (pg.longueur_sq < EPSILON * EPSILON) return false;

            // Conversion vers le repère local
            const double local_em_x = emitter_x - pg.mid_x;
            const double local_em_y = emitter_y - pg.mid_y;
            const double local_pt_x = x - pg.mid_x;
            const double local_pt_y = y - pg.mid_y;

            // Projections sur les axes
            const double proj_em_axe = local_em_x * pg.dir_unit_x + local_em_y * pg.dir_unit_y;
            const double proj_em_perp = local_em_x * pg.perp_dir_x + local_em_y * pg.perp_dir_y;
            const double proj_pt_axe = local_pt_x * pg.dir_unit_x + local_pt_y * pg.dir_unit_y;
            const double proj_pt_perp = local_pt_x * pg.perp_dir_x + local_pt_y * pg.perp_dir_y;

            // Application du théorème de l'axe séparateur
            return satTest(
                proj_em_axe, proj_em_perp,   // Projections émetteur
                proj_pt_axe, proj_pt_perp,   // Projections récepteur
                -pg.demi_longueur, pg.demi_longueur,  // Plage axe principal
                -pg.demi_epaisseur, pg.demi_epaisseur // Plage axe perpendiculaire
            );
        }

};


//classe pour les murs verticaux et horizontaux
class MurDroit:public Mur{
    
    public:

        MurDroit(double x1, double y1, double x2, double y2, double thickness, double attenuation)
        : Mur(x1, y1, x2, y2, thickness, attenuation) {
            // Normaliser les coordonnées pour que (x1,y1) soit toujours le coin inférieur gauche
            if (x1 > x2) std::swap(x1, x2);
            if (y1 > y2) std::swap(y1, y2);
        }

    
        // Vérifier si un point est à l'intérieur de l'obstacle (avec épaisseur)
        bool isPointInside(double px, double py) const override {
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

        // Optimisation: intersection entre ligne (émetteur-point) et obstacle avec épaisseur
        bool isBlocking(double x, double y, double emitter_x, double emitter_y) const override {
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
};


/**
* Constructeur pour les obstacles circulaires
* @param cx,cy Coordonnées du centre du cercle
* @param radius Rayon du cercle
* @param attenuation Perte de signal en dB lors de la traversée
*/
class MeubleRond : public Obstacle {
    protected :
        double cx, cy;      // Centre du cercle
        double radius;      // Rayon du cercle

    public :
        MeubleRond(double cx, double cy, double radius, double attenuation)
            : Obstacle(attenuation), cx(cx), cy(cy), radius(radius) {}


        bool isPointInside(double px, double py) const {
             // Formule de distance au carré pour éviter la racine
             const double dx = px - cx;
             const double dy = py - cy;
             return (dx*dx + dy*dy) <= (radius * radius) + EPSILON;
        }

        void getExpandedBounds(double& min_x, double& min_y, double& max_x, double& max_y) const override {
            min_x = cx - radius;
            min_y = cy - radius;
            max_x = cx + radius;
            max_y = cy + radius;
        }

        /*
        * Vérifie l'intersection avec un obstacle circulaire (logique interne)
        */
        // Optimisation: intersection entre ligne (émetteur-point) et obstacle avec épaisseur
        bool isBlocking(double x, double y, double emitter_x, double emitter_y) const override {
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

        double getCenterX() const { return cx; }
        double getCenterY() const { return cy; }
        double getRadius() const { return radius; }
        
};


#endif // OBSTACLE_HPP