#ifndef ROOM_HPP
#define ROOM_HPP

#include <vector>
#include <fstream>
#include <iostream>
#include "emitter.hpp"
#include "obstacle.hpp"

/**
 * Classe représentant une salle de simulation de propagation de signaux
 * Gère une grille 2D avec des émetteurs et des obstacles
 */
class Room {
public:
    int width;      // Largeur de la salle en unités de grille
    int height;     // Hauteur de la salle en unités de grille
    std::vector<Emitter> emitters;  // Liste des émetteurs
    std::vector<Obstacle*> obstacles; // Liste des obstacles
    std::vector<std::vector<double>> powerMap; // Carte des puissances reçues

    /**
     * Constructeur initialisant la grille avec une puissance par défaut
     * @param width Largeur de la grille
     * @param height Hauteur de la grille
     */
    Room(int width, int height);

    /**
     * Ajoute un émetteur à la simulation
     * @param e Émetteur à ajouter
     */
    void addEmitter(Emitter e) {
        emitters.push_back(e);
    }

    /**
     * Ajoute un obstacle à la simulation
     * @param o Obstacle à ajouter
     */
    void addObstacle(Obstacle* o) {
        obstacles.push_back(o);
    }

    /**
     * Calcule la carte de puissance pour chaque point de la grille
     * Combine les contributions de tous les émetteurs en tenant compte des obstacles
     */
    // Optimisation: calcul parallèle des signaux ou pré-calcul des obstacles
    void computeSignalMap(void);

    /**
     * Marque les zones occupées par les obstacles sur la carte de puissance
     * Utilise la valeur spéciale -555 pour identifier les obstacles
     */
    // Marquer les obstacles sur la heatmap
    void markObstaclesOnPowerMap(void);


    void exportToCSV(const std::string& filename);

    bool deleteEmitter(double x, double y);

    bool deleteObstacle(double x1, double y1, double x2, double y2);

private:
    /**
     * Marque les bords de la salle comme zones obstacles
     * Ajoute une bordure de sécurité de 2 unités
     */
    void markRoomBoundaries();
};

#endif // ROOM_HPP