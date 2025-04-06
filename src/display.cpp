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
#include "../headers/emitter.hpp"
#include "../headers/obstacle.hpp"
#include "../lib/SDL2_ttf/include/SDL_ttf.h"

#define CELL_SIZE 1 // Taille de la cellule de la grille
#define CLICK_THRESHOLD 30 // Seuil de distance pour détecter un clic sur un émetteur

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

int handlepowerMap(Room* room, SDL_Renderer* renderer){
    if ((*room).powerMap.empty()) {
        std::cerr << "Aucune donnee n'a ete chargee" << std::endl;
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
                x * CELL_SIZE,
                y * CELL_SIZE,
                CELL_SIZE,
                CELL_SIZE
            };
            
            SDL_RenderFillRect(renderer, &rect);
        }
    }
    return 0;
}

SDL_Texture* renderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, SDL_Color textColor) {
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text, textColor);
    if (!textSurface) {
        std::cerr << "Impossible de créer la surface de texte: " << TTF_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FreeSurface(textSurface);
    
    if (!textTexture) {
        std::cerr << "Impossible de créer la texture du texte: " << SDL_GetError() << std::endl;
        return nullptr;
    }
    
    return textTexture;
}

int displaying(Room* room) {

    if (TTF_Init() != 0) {
        std::cerr << "Erreur d'initialisation SDL_ttf: " << TTF_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }
    
    // Charger une police
    TTF_Font* font = TTF_OpenFont("assets/fonts/DejaVuSans.ttf", 16); // Remplacez par le chemin de votre police
    if (!font) {
        std::cerr << "Erreur de chargement de police: " << TTF_GetError() << std::endl;
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    

    int gridHeight = (*room).powerMap.size();
    int gridWidth = (*room).powerMap[0].size();
    
    // Initialiser SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Erreur d'initialisation SDL: " << SDL_GetError() << std::endl;
        return 1;
    }
    
    // Créer la fenêtre
    SDL_Window* window = SDL_CreateWindow(
        "Carte thermique du signal WiFi (dBm)",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        gridWidth * CELL_SIZE, gridHeight * CELL_SIZE,
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

    handlepowerMap(room, renderer); 

    SDL_RenderPresent(renderer);

    // Variables pour suivre le dernier clic
    int lastClickX = -1;
    int lastClickY = -1;
    bool showClickInfo = false;
    SDL_Rect infoBox = {10, 10, 180, 60};

    // Bouton mur ///////////////////////////
    bool addingWall = false;        // Mode d'ajout de mur
    int wallStartX = -1;            // Premier point du mur (X)
    int wallStartY = -1;            // Premier point du mur (Y)
    bool waitingForSecondPoint = false; // En attente du second point
    
    // Attendre que l'utilisateur ferme la fenêtre
    bool running = true;
    SDL_Event event;
    Emitter* selectedEmitter = new Emitter(0, 0, 0, 0); // Émetteur sélectionné 
    bool emitterSelected = false;

    // Définition du bouton
    SDL_Rect addWallButton = {10, gridHeight * CELL_SIZE - 40, 120, 30}; // Position en bas à gauche
    bool buttonHovered = false;

    //fenetre d'affichage des valeurs de puissance, en bas à droite
    SDL_Rect powerInfoBox = {gridWidth * CELL_SIZE - 200, gridHeight * CELL_SIZE - 100, 180, 80};
    SDL_Color powerInfoColor = {255, 255, 255, 255}; // Couleur blanche




    
    while (running) {

    //dessiner la zone d'information, en bas à droite, en blanc
    SDL_SetRenderDrawColor(renderer, powerInfoColor.r, powerInfoColor.g, powerInfoColor.b, powerInfoColor.a);
    SDL_RenderFillRect(renderer, &powerInfoBox);
    // Récupérer la position de la souris pour le survol du bouton
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    buttonHovered = (mouseX >= addWallButton.x && mouseX <= addWallButton.x + addWallButton.w &&
                     mouseY >= addWallButton.y && mouseY <= addWallButton.y + addWallButton.h);       
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
                        // Vérifier si le clic est sur le bouton d'ajout de mur
                    if (mouseX >= addWallButton.x && mouseX <= addWallButton.x + addWallButton.w &&
                        mouseY >= addWallButton.y && mouseY <= addWallButton.y + addWallButton.h) {
                        
                        addingWall = true;
                        waitingForSecondPoint = false;
                        wallStartX = -1;
                        wallStartY = -1;
                        std::cout << "Mode ajout de mur activé. Cliquez pour définir le premier point." << std::endl;
                    }
                    // Sinon, traiter le clic en fonction du mode actuel
                    else if (addingWall) {
                        // Récupérer les coordonnées du clic
                        int clickX = event.button.x / CELL_SIZE;
                        int clickY = event.button.y / CELL_SIZE;
                        
                        // Vérifier que les coordonnées sont dans la grille
                        if (clickX >= 0 && clickX < gridWidth && 
                            clickY >= 0 && clickY < gridHeight) {
                            
                            if (!waitingForSecondPoint) {
                                // Premier point du mur
                                wallStartX = clickX;
                                wallStartY = clickY;
                                waitingForSecondPoint = true;
                                std::cout << "Premier point défini: (" << wallStartX << ", " << wallStartY 
                                        << "). Cliquez pour définir le second point." << std::endl;
                            } else {
                                // Deuxième point du mur
                                int wallEndX = clickX;
                                int wallEndY = clickY;
                                
                                // Ajouter le mur
                                (*room).addObstacle(new Mur(wallStartX, wallStartY, wallEndX, wallEndY, 10, 5));
                                std::cout << "Mur ajouté de (" << wallStartX << ", " << wallStartY << ") à (" 
                                            << wallEndX << ", " << wallEndY << ")" << std::endl;
                                
                                // Recalculer la carte
                                (*room).computeSignalMap();
                                (*room).markObstaclesOnPowerMap();
                                
                                // Réinitialiser le mode d'ajout de mur
                                addingWall = false;
                                waitingForSecondPoint = false;
                                
                                // mise à jour de la power map
                                handlepowerMap(room, renderer); 
                            }
                        }
                    }
                    else {
                        // Récupérer les coordonnées du clic
                        lastClickX = event.button.x / CELL_SIZE;  // Convertir en coordonnées de la grille
                        lastClickY = event.button.y / CELL_SIZE;
                        
                        // Vérifier que les coordonnées sont dans la grille
                        if (lastClickX >= 0 && lastClickX < gridWidth && 
                            lastClickY >= 0 && lastClickY < gridHeight) {
                            
                            double signalPower = (*room).powerMap[lastClickY][lastClickX];
                            
                            std::cout << "Clic a la position: (" << lastClickX << ", " 
                                    << lastClickY << ")" << std::endl;
                            std::cout << "Puissance du signal: " << signalPower << " dBm" << std::endl;


                            
                            showClickInfo = true;
                            
                            if(emitterSelected){
                                // Déplacer l'émetteur à la nouvelle position
                                (*selectedEmitter).x = lastClickX;
                                (*selectedEmitter).y = lastClickY;

                                std::cout << "Emetteur deplace a la position: (" << (*selectedEmitter).getX() << ", " 
                                        << (*selectedEmitter).getY() << ")" << std::endl;
                                
                                // Mettre à jour la carte de puissance
                                (*room).computeSignalMap(); // Recalculer la carte de puissance
                                (*room).markObstaclesOnPowerMap();
                                
                                emitterSelected = false; // Réinitialiser l'état de sélection
                                showClickInfo = true;
                            } 
                            else {
                                // Vérifier si le clic est sur un émetteur
                                bool foundEmitter = false;
                                for (auto& emitter : (*room).emitters) {
                                    // Correction de la condition
                                    if ((emitter.getX() - CLICK_THRESHOLD < lastClickX) && (emitter.getX() + CLICK_THRESHOLD > lastClickX) && 
                                        (emitter.getY() - CLICK_THRESHOLD < lastClickY) && (emitter.getY() + CLICK_THRESHOLD > lastClickY)) {
                                        selectedEmitter = &emitter;
                                        emitterSelected = true;
                                        foundEmitter = true;
                                        std::cout << "Emetteur selectionne a la position: (" << (*selectedEmitter).getX() << ", " 
                                                << (*selectedEmitter).getY() << ")" << std::endl;
                                        break;
                                    }
                                }
                                
                                // Si on n'a pas trouvé d'émetteur, affiche simplement les infos
                                if (!foundEmitter) {
                                    showClickInfo = true;
                                }
                            }
                            
                            handlepowerMap(room, renderer); 

                            // Dessiner un marqueur sur la position du clic
                            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // blanc
                            SDL_Rect marker = {
                                lastClickX * CELL_SIZE - 2,
                                lastClickY * CELL_SIZE - 2,
                                5, 
                                5
                            };
                            SDL_RenderFillRect(renderer, &marker);

                            // Dessiner le bouton
                            if (buttonHovered) {
                                SDL_SetRenderDrawColor(renderer, 100, 150, 255, 255); // Bleu plus clair en survol
                            } else {
                                SDL_SetRenderDrawColor(renderer, 70, 130, 230, 255); // Bleu normal
                            }
                            SDL_RenderFillRect(renderer, &addWallButton);
                            
                            SDL_Color textColor = {255, 255, 255, 255}; // Blanc
                            SDL_Texture* buttonTextTexture = renderText(renderer, font, "ADD WALL", textColor);

                            if (buttonTextTexture) {
                                int textWidth, textHeight;
                                SDL_QueryTexture(buttonTextTexture, nullptr, nullptr, &textWidth, &textHeight);
                                
                                // Centrer le texte sur le bouton
                                SDL_Rect textRect = {
                                    addWallButton.x + (addWallButton.w - textWidth) / 2,
                                    addWallButton.y + (addWallButton.h - textHeight) / 2,
                                    textWidth,
                                    textHeight
                                };
                                
                                SDL_RenderCopy(renderer, buttonTextTexture, nullptr, &textRect);
                                
                                // Libérer la texture après usage
                                SDL_DestroyTexture(buttonTextTexture);
                            }




                            SDL_RenderPresent(renderer);
                        }
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
