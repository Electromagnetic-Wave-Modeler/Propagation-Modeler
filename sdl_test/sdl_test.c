#include <stdio.h>
#include <time.h>
#include <SDL.h>
#include <stdbool.h>

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) return -1;

    const int rows = 10;
    const int cols = 50;
    const int cellSize = 10;

    SDL_Window* window = SDL_CreateWindow("Representation SDL",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        cols * cellSize, rows * cellSize, 0);
    if (!window) return -1;

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) return -1;

    // Matrice initialement à 0
    int salle[rows][cols] ;
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            salle[i][j] = 0;
        }
    }

    // Placement des objets (coordonnées (2,2), (2,3), (2,5) => ligne = 1, colonne = 1/2/4 en 0-based)
    salle[1][1] = 1;
    salle[1][2] = 1;
    salle[1][3] = 1;

    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
        }

        // Efface l’écran
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Dessine la matrice
        for (int r = 0; r < rows; r++) {
            for (int c = 0; c < cols; c++) {
                SDL_Rect cell = { c * cellSize, r * cellSize, cellSize, cellSize };

                if (salle[r][c] == 1) {
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Rouge = objet
                } else {
                    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);   // Vert = vide
                }
                SDL_RenderFillRect(renderer, &cell);

                // Bordures noires
                // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                // SDL_RenderDrawRect(renderer, &cell);
            }
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}