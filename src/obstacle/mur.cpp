#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <ostream>

#include "../../headers/obstacle.hpp"


void Mur::precalculerParametresGeometriques() {
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

bool Mur::satTest(double e_axe, double e_perp, double p_axe, double p_perp, double min_a, double max_a, double min_p, double max_p) const {
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

Mur::Mur(double x1, double y1, double x2, double y2, double thickness, double attenuation)
: Obstacle(attenuation), x1(x1), y1(y1), x2(x2), y2(y2), thickness(thickness) {
    // Normaliser les coordonnées pour que (x1,y1) soit toujours le coin inférieur gauche
    if (x1 > x2) std::swap(x1, x2);
    if (y1 > y2) std::swap(y1, y2);
    precalculerParametresGeometriques();  // Pré-calcul des paramètres géométriques
}

void Mur::getExpandedBounds(double& min_x, double& min_y, double& max_x, double& max_y) const {
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

bool Mur::isPointInside(double px, double py) const {
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

bool Mur::segmentIntersectsRectangle(double x0, double y0, double x1, double y1, double rect_x1, double rect_y1, double rect_x2, double rect_y2) const {
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

bool Mur::isBlocking(double x, double y, double emitter_x, double emitter_y) const {
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