// #include <stdio.h>
// #include <time.h>
// #include <SDL.h>
// #include <stdbool.h>
// #include <iostream>

// int main(int argc, char* argv[]) {
//     if (SDL_Init(SDL_INIT_VIDEO) != 0) return -1;

//     const int rows = 10;
//     const int cols = 50;
//     const int cellSize = 10;

//     SDL_Window* window = SDL_CreateWindow("Representation SDL",
//         SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
//         cols * cellSize, rows * cellSize, 0);
//     if (!window) return -1;

//     SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
//     if (!renderer) return -1;

//     // Matrice initialement à 0
//     int salle[rows][cols] ;
//     for(int i = 0; i < rows; i++){
//         for(int j = 0; j < cols; j++){
//             salle[i][j] = 0;
//         }
//     }

//     // Placement des objets (coordonnées (2,2), (2,3), (2,5) => ligne = 1, colonne = 1/2/4 en 0-based)
//     salle[1][1] = 1;
//     salle[1][2] = 1;
//     salle[1][3] = 1;

//     bool running = true;
//     SDL_Event e;
//     while (running) {
//         while (SDL_PollEvent(&e)) {
//             if (e.type == SDL_QUIT) running = false;
//         }

//         // Efface l’écran
//         SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
//         SDL_RenderClear(renderer);

//         // Dessine la matrice
//         for (int r = 0; r < rows; r++) {
//             for (int c = 0; c < cols; c++) {
//                 SDL_Rect cell = { c * cellSize, r * cellSize, cellSize, cellSize };

//                 if (salle[r][c] == 1) {
//                     SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Rouge = objet
//                 } else {
//                     SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);   // Vert = vide
//                 }
//                 SDL_RenderFillRect(renderer, &cell);

//                 // Bordures noires
//                 // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
//                 // SDL_RenderDrawRect(renderer, &cell);
//             }
//         }

//         SDL_RenderPresent(renderer);
//     }

//     SDL_DestroyRenderer(renderer);
//     SDL_DestroyWindow(window);
//     SDL_Quit();
//     std::cout << "SDL_Quit" << std::endl;
//     return 0;
// }

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <SDL.h>
#include <algorithm>
#include <limits>
#include <cmath>

// Fonction pour charger les données de puissance WiFi depuis un CSV
std::vector<std::vector<double>> loadCSV(const std::string& filename) {
    std::vector<std::vector<double>> grid;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "❌ Impossible d'ouvrir le fichier: " << filename << std::endl;
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
    
    std::cout << "✅ Fichier CSV chargé avec succès. Dimensions: " 
              << grid.size() << "x" << (grid.empty() ? 0 : grid[0].size()) << std::endl;
    return grid;
}

// Conversion dBm vers couleur RGB
SDL_Color dBmToColor(double power, double min_power, double max_power) {
    // Normaliser la valeur entre 0 et 1
    double normalized = (power - min_power) / (max_power - min_power);
    normalized = std::max(0.0, std::min(1.0, normalized));
    
    // Palettes de couleurs RdYlGn (Rouge-Jaune-Vert)
    SDL_Color color;
    
    if (normalized < 0.5) {
        // Rouge (faible) à Jaune (moyen)
        double t = normalized * 2;
        color.r = 255;
        color.g = static_cast<Uint8>(255 * t);
        color.b = 0;
    } else {
        // Jaune (moyen) à Vert (fort)
        double t = (normalized - 0.5) * 2;
        color.r = static_cast<Uint8>(255 * (1.0 - t));
        color.g = 255;
        color.b = 0;
    }
    
    color.a = 255;
    return color;
}

int main(int argc, char* argv[]) {
    const std::string csvFile = "../heatmap.csv";
    const int cellSize = 1;  // Taille du carré en pixels
    
    // Charger les données
    std::vector<std::vector<double>> powerGrid = loadCSV(csvFile);
    
    if (powerGrid.empty()) {
        std::cerr << "❌ Aucune donnée n'a été chargée depuis le fichier CSV" << std::endl;
        return 1;
    }
    
    int gridHeight = powerGrid.size();
    int gridWidth = powerGrid[0].size();
    
    // Trouver les valeurs min et max
    double minPower = std::numeric_limits<double>::max();
    double maxPower = std::numeric_limits<double>::lowest();
    
    for (const auto& row : powerGrid) {
        for (double val : row) {
            if (!std::isnan(val)) {
                minPower = std::min(minPower, val);
                maxPower = std::max(maxPower, val);
            }
        }
    }
    
    std::cout << "Puissance min: " << minPower << " dBm, max: " << maxPower << " dBm" << std::endl;
    
    // Remplacer les NaN par la valeur minimale
    for (auto& row : powerGrid) {
        for (auto& val : row) {
            if (std::isnan(val)) {
                val = minPower;
            }
        }
    }
    
    // Initialiser SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "❌ Erreur d'initialisation SDL: " << SDL_GetError() << std::endl;
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
        std::cerr << "❌ Erreur de création de fenêtre: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    
    // Créer le renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    if (!renderer) {
        std::cerr << "❌ Erreur de création du renderer: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    
    // Dessiner la heatmap
    for (int y = 0; y < gridHeight; y++) {
        for (int x = 0; x < gridWidth; x++) {
            SDL_Color color = dBmToColor(powerGrid[y][x], minPower, maxPower);
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
    
    // Afficher le résultat
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
    
    std::cout << "✅ Visualisation terminée" << std::endl;
    return 0;
}