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

int displaying(std::vector<std::vector<double>>* powerGrid){
    const std::string csvFile = "heatmap.csv";
    const int cellSize = 1;  // Taille du carré en pixels
    
    // Charger les données
    // std::vector<std::vector<double>> powerGrid = room.powerMap;//loadCSV(csvFile);
    
    if ((*powerGrid).empty()) {
        std::cerr << "Aucune donnee n'a ete chargee depuis le fichier CSV" << std::endl;
        return 1;
    }
    
    int gridHeight = powerGrid->size();
    int gridWidth = (*powerGrid)[0].size();
    
    // Trouver les valeurs min et max
    double minPower = std::numeric_limits<double>::max();
    double maxPower = std::numeric_limits<double>::lowest();
    
    for (const auto& row : *powerGrid) {
        for (double val : row) {
            if (!std::isnan(val) && val != -555) {
                minPower = std::min(minPower, val);
                maxPower = std::max(maxPower, val);
            }
        }
    }
    
    std::cout << "Puissance min: " << minPower << " dBm, max: " << maxPower << " dBm" << std::endl;
    
    // Remplacer les NaN par la valeur minimale
    for (auto& row : *powerGrid) {
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
            SDL_Color color = dBmToColor((*powerGrid)[y][x], minPower, maxPower);
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
        }
    }
    
    // Libérer les ressources
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    std::cout << "Visualisation terminee" << std::endl;

    return 0;
}

// int main(int argc, char* argv[]) {

//     return 0;
// }