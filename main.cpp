//Código para la generación procedural de minas para Stardew Valley basado en Cellular automata con Room templates y sonido.
//Félix Fuentes e Ignacio Alfaro 

#include <iostream>
#include <vector>
#include <random>   // For random number generation
#include <chrono>   // For seeding the random number generator


using Map = std::vector<std::vector<int>>;

void printMap(const Map& map) {
    std::cout << "--- Current Map ---" << std::endl;
    for (const auto& row : map) {
        for (int cell : row) {
            
            std::cout << cell << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "-------------------" << std::endl;
}


Map cellularAutomata(const Map& currentMap, int W, int H, int R, double U) {
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    srand(seed);
    Map newMap = currentMap; // Initially, the new map is a copy of the current one
   
    //se genera el ruido
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            newMap[x][y] = ((rand() % 30) < 2) ? 1 : 0;
        }
    }
    Map noisyMap = newMap;
    //se recorre el mapa nuevo
    for (int y = 0; y < H ; y++){
        for (int x = 0; x < W; x ++){
            //una vez voy recorriendo el mapa debo recorrer el area al rededor del punto para revisar si tiene los valores necesarios para difuminar
            int count = 0;
            for(int radY = - R; radY <= R; radY++){
                for (int radX = - R; radX <= R; radX++){
                    int nx = x + radX;
                    int ny = y + radY;

                     if (nx < 0 || nx >= W || ny < 0 || ny >= H) {//si sale del mapa, salta ese paso
            
                        count += 1;
                    }else{
                        count += newMap[nx][ny];
                    }
                    
                }
            }
            noisyMap[x][y] = (count>= U) ? 0:1;
        }
    }

    return noisyMap;
}


int main(){
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);
    std::cout << "--- PCG OF STARDEW VALLEY MINES SIMULATION ---" << std::endl;

    int mapCols = 10; //Limite de alto
    int mapRows = 10; //Limite de largo
    float playerLuck = 0; //Suerte del jugador
    int idMap = 0; //ID del mapa
    int radiusNoise = 1; //
    double limitNoise = 0.5; //



    Map myMap(mapCols, std::vector<int>(mapRows, 0));



    std::cout << std::endl;
    //std::cout << "\nInitial map state:" << std::endl;

    // --- Main Simulation Loop ---
    for (int iteration = 0; iteration < 1; ++iteration) {
        //std::cout << "\n--- Iteration " << iteration + 1 << " ---" << std::endl;
        myMap = cellularAutomata(myMap, mapRows, mapCols, radiusNoise, limitNoise);
        printMap(myMap);
    }

    std::cout << "\n--- Simulation Finished ---" << std::endl;


    return 0;
}