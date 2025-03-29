#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <SDL.h>
#include <algorithm>
#include <limits>
#include <cmath>

#include "../headers/display.hpp"
#include "../headers/room.hpp"

// Fonction pour charger les données de puissance WiFi depuis un CSV
std::vector<std::vector<double>> loadCSV(const std::string& filename) {
    std::vector<std::vector<double>> grid;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Impossible d'ouvrir le fichier: " << filename << std::endl;
        return grid;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::vector<double> row;
        std::stringstream ss(line);
        std::string value;
        
        while (std::getline(ss, value, ',')) {
            try {
                row.push_back(std::stod(value));
            } catch (const std::exception& e) {
                row.push_back(std::numeric_limits<double>::quiet_NaN());
            }
        }
        
        if (!row.empty()) {
            grid.push_back(row);
        }
    }
    
    std::cout << "Fichier CSV charge avec succes. Dimensions: " 
              << grid.size() << "x" << (grid.empty() ? 0 : grid[0].size()) << std::endl;
    return grid;
}

// Conversion dBm vers couleur RGB
SDL_Color dBmToColor(double power, double min_power, double max_power) {
    // Normaliser la valeur entre 0 et 1
    double normalized = (power - min_power) / (max_power - min_power);
    // normalized = std::max(0.0, std::min(1.0, normalized));
    
    // Palettes de couleurs RdYlGn (Rouge-Jaune-Vert)
    SDL_Color color;

    if(normalized < 0.5) {
        // Rouge (faible) à Jaune (moyen)
        double t = normalized * 2;
        color.r = 255;
        color.g = static_cast<Uint8>(255 * t);
        color.b = 0;
    } else {
        double t = (normalized - 0.5) * 2;
        color.r = static_cast<Uint8>(255 * (1.0 - t));
        color.g = 255;
        color.b = 0;
    }
    
    //transparence
    color.a = 255;
    return color;
}

int displaying(Room* room) {
    // const std::string csvFile = "heatmap.csv";
    const int cellSize = 1;  // Taille du carré en pixels
    
    // Charger les données
    // std::vector<std::vector<double>> powerMap = room.powerMap;//loadCSV(csvFile);
    
    if ((*room).powerMap.empty()) {
        std::cerr << "Aucune donnee n'a ete chargee depuis le fichier CSV" << std::endl;
        return 1;
    }
    
    int gridHeight = (*room).powerMap.size();
    int gridWidth = (*room).powerMap[0].size();
    
    // Trouver les valeurs min et max
    double minPower = std::numeric_limits<double>::max();
    double maxPower = std::numeric_limits<double>::lowest();
    
    for (const auto& row : (*room).powerMap) {
        for (double val : row) {
            if (!std::isnan(val) && val != -555) {
                minPower = std::min(minPower, val);
                maxPower = std::max(maxPower, val);
            }
        }
    }
    
    std::cout << "Puissance min: " << minPower << " dBm, max: " << maxPower << " dBm" << std::endl;
    
    // Remplacer les NaN par la valeur minimale
    for (auto& row : (*room).powerMap) {
        for (auto& val : row) {
            if (std::isnan(val)) {
                val = minPower;
            }
        }
    }
    
    // Initialiser SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Erreur d'initialisation SDL: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    // Créer la fenêtre
    SDL_Window* window = SDL_CreateWindow(
        "Carte thermique du signal WiFi (dBm)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        gridWidth * cellSize, gridHeight * cellSize,
        SDL_WINDOW_SHOWN
    );
    
    if (!window) {
        std::cerr << "Erreur de creation de fenêtre: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    
    // Créer le renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    if (!renderer) {
        std::cerr << "Erreur de creation du renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Dessiner la heatmap
    for (int y = 0; y < gridHeight; y++) {
        for (int x = 0; x < gridWidth; x++) {
            //SDL_Color color = dBmToColor((*powerMap)[y][x], minPower, maxPower);
            double val = (*room).powerMap[y][x];
            SDL_Color color;
            if (val == -555) {
                color = {0, 0, 0, 255}; // noir
            } else {
                color = dBmToColor(val, minPower, maxPower);
            }
            SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            
            SDL_Rect rect = {
                x * cellSize,
                y * cellSize,
                cellSize,
                cellSize
            };
            
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    SDL_RenderPresent(renderer);

    // Variables pour suivre le dernier clic
    int lastClickX = -1;
    int lastClickY = -1;
    bool showClickInfo = false;
    SDL_Rect infoBox = {10, 10, 180, 60};
    
    // Attendre que l'utilisateur ferme la fenêtre
    bool running = true;
    SDL_Event event;
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                if (event.button.button == SDL_BUTTON_LEFT) {
                    // Récupérer les coordonnées du clic
                    lastClickX = event.button.x / cellSize;  // Convertir en coordonnées de la grille
                    lastClickY = event.button.y / cellSize;
                    
                    // Vérifier que les coordonnées sont dans la grille
                    if (lastClickX >= 0 && lastClickX < gridWidth && 
                        lastClickY >= 0 && lastClickY < gridHeight) {
                        
                        double signalPower = (*room).powerMap[lastClickY][lastClickX];
                        
                        std::cout << "Clic a la position: (" << lastClickX << ", " 
                                  << lastClickY << ")" << std::endl;
                        std::cout << "Puissance du signal: " << signalPower << " dBm" << std::endl;
                        
                        showClickInfo = true;
                        
                        // Mettre à jour l'affichage pour montrer l'info du clic
                        // Redessiner la heatmap
                        for (int y = 0; y < gridHeight; y++) {
                            for (int x = 0; x < gridWidth; x++) {
                                double val = (*room).powerMap[y][x];
                                SDL_Color color;
                                if (val == -555) {
                                    color = {0, 0, 0, 255}; // noir
                                } else {
                                    color = dBmToColor(val, minPower, maxPower);
                                }
                                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
                                
                                SDL_Rect rect = {
                                    x * cellSize,
                                    y * cellSize,
                                    cellSize,
                                    cellSize
                                };
                                
                                SDL_RenderFillRect(renderer, &rect);
                            }
                        }
                        
                        // Dessiner un marqueur sur la position du clic
                        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // blanc
                        SDL_Rect marker = {
                            lastClickX * cellSize - 2,
                            lastClickY * cellSize - 2,
                            5, 
                            5
                        };
                        SDL_RenderFillRect(renderer, &marker);
                        
                        SDL_RenderPresent(renderer);
                    }
                }
            }
        }
    }
    
    // Libérer les ressources
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    std::cout << "Visualization completed" << std::endl;

    return 0;
}
