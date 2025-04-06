#ifndef OBSTACLE_HPP
#define OBSTACLE_HPP

#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <ostream>


/**
 * Classe représentant un obstacle dans une simulation de propagation de signal.
 * Gère deux types d'obstacles : rectangles avec épaisseur et cercles.
 */
class Obstacle {
    protected : 

        double attenuation;

    public:

        Obstacle(double attenuation);

        // Constante pour les comparaisons de précision flottante
        static constexpr double EPSILON = 1e-6;

        // Destructeur virtuel pour permettre une destruction des classes dérivées
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
        void precalculerParametresGeometriques();

        bool satTest(double e_axe, double e_perp, double p_axe, double p_perp, double min_a, double max_a, double min_p, double max_p) const;


    public :
        Mur(double x1, double y1, double x2, double y2, double thickness, double attenuation);

        double getX1() const { return x1; }
        double getY1() const { return y1; }
        double getX2() const { return x2; }
        double getY2() const { return y2; }

        
        void getExpandedBounds(double& min_x, double& min_y, double& max_x, double& max_y) const override;  

        // Vérifier si un point est à l'intérieur de l'obstacle (avec épaisseur)
        bool isPointInside(double px, double py) const override;

        /**
        * Algorithme de Liang-Barsky pour l'intersection segment-rectangle
        * @param rect_x1,rect_y1 Coin inférieur gauche
        * @param rect_x2,rect_y2 Coin supérieur droit
        * @return true si intersection détectée
        */
        bool segmentIntersectsRectangle(double x0, double y0, double x1, double y1, double rect_x1, double rect_y1, double rect_x2, double rect_y2) const;

        bool isBlocking(double x, double y, double emitter_x, double emitter_y) const;

};


//classe pour les murs verticaux et horizontaux
class MurDroit:public Mur{
    
    public:

        MurDroit(double x1, double y1, double x2, double y2, double thickness, double attenuation);

    
        // Vérifier si un point est à l'intérieur de l'obstacle (avec épaisseur)
        bool isPointInside(double px, double py) const override;

        // Optimisation: intersection entre ligne (émetteur-point) et obstacle avec épaisseur
        bool isBlocking(double x, double y, double emitter_x, double emitter_y) const override;
};



class obstacleCirculaire : public Obstacle {
    protected :
        double cx, cy;      // Centre du cercle
        double radius;      // Rayon du cercle

    public :
        obstacleCirculaire(double cx, double cy, double radius, double attenuation);


        bool isPointInside(double px, double py) const;

        void getExpandedBounds(double& min_x, double& min_y, double& max_x, double& max_y) const;

        /*
        * Vérifie l'intersection avec un obstacle circulaire (logique interne)
        */
        // Optimisation: intersection entre ligne (émetteur-point) et obstacle avec épaisseur
        bool isBlocking(double x, double y, double emitter_x, double emitter_y) const override;

        double getCenterX() const { return cx; }
        double getCenterY() const { return cy; }
        double getRadius() const { return radius; }
        
};


#endif // OBSTACLE_HPP