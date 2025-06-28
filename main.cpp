//Código para la generación procedural de minas para Stardew Valley basado en Cellular automata con Room templates y sonido.
//Félix Fuentes e Ignacio Alfaro 

#include <iostream>
#include <vector>
#include <random>   // For random number generation
#include <chrono>   // For seeding the random number generator

const char pared = '#';
const char piso = '0';
using Map = std::vector<std::vector<char>>;

void printMap(const Map& map) {
    std::cout << "--- Current Map ---" << std::endl;
    for (const auto& row : map) {
        for (char cell : row) {
            
            std::cout << cell << " ";
        }
        std::cout << std::endl;
    }
    std::cout << "-------------------" << std::endl;
}

Map GenerarRuido(const Map& currentMap, int W, int H){
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    srand(seed);
    Map noisyMap = currentMap;
     for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            noisyMap[x][y] = ((rand() % 30) < 2) ? pared : piso;
        }
    }
    return noisyMap;
}

//cellular automata con modificacion
//a este se le agregan limites de las paredes, que el tamaño también varie a partir de un rango
//Tambien se le agrega un modificador para que no queden espacios intransitables
Map cellularAutomata(const Map& currentMap, int W, int H, int R, double U) {
    Map newMap = currentMap; // Initially, the new map is a copy of the current one

   
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
                    }else if (newMap[nx][ny] == pared){
                        count += 1;
                    }
                    
                }
            }
            noisyMap[x][y] = (count>= U) ? pared:piso;//si en el punto especifico la cantidad de casillas que la rodea es mayor o igual a lo que pide u, entonces se convierte en parte del mapa
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



    Map myMap(mapCols, std::vector<char>(mapRows, 0));



    std::cout << std::endl;
    //std::cout << "\nInitial map state:" << std::endl;

    // --- Main Simulation Loop ---
    for (int iteration = 0; iteration < 1; ++iteration) {
        //std::cout << "\n--- Iteration " << iteration + 1 << " ---" << std::endl;
        myMap = GenerarRuido(myMap, mapRows, mapCols);
        myMap = cellularAutomata(myMap, mapRows, mapCols, radiusNoise, limitNoise);
        printMap(myMap);
    }

    std::cout << "\n--- Simulation Finished ---" << std::endl;


    return 0;
}