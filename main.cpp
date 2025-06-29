//Código para la generación procedural de minas para Stardew Valley basado en Cellular automata con Room templates y sonido.
//Félix Fuentes e Ignacio Alfaro 

#include <iostream>
#include <vector>
#include <random>   // For random number generation
#include <chrono>   // For seeding the random number generator
using Map = std::vector<std::vector<char>>;

const char pared = '#';
const char piso = ' ';
const char minerales = '.';
const char entrada = 'E';

//inicialización de los layouts que se van a interponer
// 1. Área de Inicio/Escalera (Start/Stairwell Area)
const Map layout_start_area = {
    {pared, pared, pared, pared, pared},
    {pared, piso,  entrada,  piso,  pared}, // 'S' se convierte en 'piso' para el CA
    {pared, piso,  piso,  piso,  pared},
    {pared, pared, pared, pared, pared}
};

// 2. Pasillo Básico (Basic Corridor)
const Map layout_basic_corridor = {
    {pared, pared, pared},
    {pared, piso,  pared},
    {pared, pared, pared}
};

// 3. Pasillo Curvo (Curved Corridor)
const Map layout_curved_corridor = {
    {pared, pared, pared, pared},
    {pared, piso,  piso,  pared},
    {pared, piso,  pared, pared},
    {pared, piso,  piso,  pared},
    {pared, pared, pared, pared}
};

// 4. Cruce de Cuatro Caminos (Four-Way Crossroads)
const Map layout_four_way_crossroads = {
    {pared, pared, pared, pared, pared},
    {pared, piso,  pared, piso,  pared},
    {pared, piso,  piso,  piso,  pared},
    {pared, piso,  pared, piso,  pared},
    {pared, pared, pared, pared, pared}
};

// 5. Cruce en T (T-Junction)
const Map layout_t_junction = {
    {pared, pared, pared, pared, pared},
    {pared, piso,  pared, piso,  pared},
    {pared, piso,  piso,  piso,  pared},
    {pared, pared, pared, pared, pared}
};

// 6. Sala de Minerales Pequeña (Small Mineral Room)
const Map layout_small_mineral_room = {
    {pared, pared, pared, pared, pared, pared, pared},
    {pared, piso,  piso,  piso,  piso,  piso,  pared}, // 'M' se convierte en 'piso'
    {pared, piso,  piso,  piso,  piso,  piso,  pared},
    {pared, piso,  piso,  piso,  piso,  piso,  pared},
    {pared, pared, pared, pared, pared, pared, pared}
};

// 7. Sala Abierta (Open Chamber)
const Map layout_open_chamber = {
    {piso,  piso,  piso,  piso,  piso,  piso,  piso,  piso,  piso},
    {piso,  pared, pared, pared, pared, pared, pared, pared, piso},
    {piso,  pared, pared, pared, pared, pared, pared, pared, piso},
    {piso,  pared, pared, pared, pared, pared, pared, pared, piso},
    {piso,  pared, pared, pared, pared, pared, pared, pared, piso},
    {piso,  pared, pared, pared, pared, pared, pared, pared, piso},
    {piso,  piso,  piso,  piso,  piso,  piso,  piso,  piso,  piso}
};

// 8. Cuarto del Tesoro/Recurso Clave (Treasure/Key Resource Room)
const Map layout_treasure_room = {
    {pared, pared, pared, pared, pared, pared, pared},
    {pared, piso,  piso,  piso,  piso,  piso,  pared},
    {pared, piso,  pared, pared, pared, piso,  pared},
    {pared, piso,  piso,  piso,  piso,  piso,  pared}, // 'M' se convierte en 'piso'
    {pared, piso,  pared, pared, pared, piso,  pared},
    {pared, piso,  piso,  piso,  piso,  piso,  pared},
    {pared, pared, pared, pared, pared, pared, pared}
};

// 9. Túnel Estrecho (Narrow Tunnel)
const Map layout_narrow_tunnel = {
    {pared, pared, pared},
    {pared, piso,  pared},
    {pared, piso,  pared},
    {pared, piso,  pared},
    {pared, pared, pared}
};

// 10. Bloqueo de Roca (Rock Blockage)
const Map layout_rock_blockage = {
    {pared, pared, pared, pared, pared},
    {pared, pared, piso,  pared, pared},
    {pared, pared, pared, pared, pared}
};




void insertLayout(Map& mainMap, const Map& layout, int startX, int startY) { //inserta algun layout en un lugar específico
    int layoutRows = layout.size();
    if (layoutRows == 0) return;
    int layoutCols = layout[0].size();

    for (int y = 0; y < layoutRows; ++y) {
        for (int x = 0; x < layoutCols; ++x) {
            int currentMapX = startX + x;
            int currentMapY = startY + y;

            // Asegurarse de no salirse de los límites del mapa principal
            if (currentMapX >= 0 && currentMapX < mainMap[0].size() &&
                currentMapY >= 0 && currentMapY < mainMap.size()) {
                mainMap[currentMapY][currentMapX] = layout[y][x];
            }
        }
    }
}
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
            if(noisyMap[y][x] == pared){
                noisyMap[y][x] = ((rand() % 30) < 2) ? pared : piso;
            }
             
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
            //si es caracter especial
             if (currentMap[y][x] == entrada || currentMap[y][x] == minerales) {
                newMap[y][x] = currentMap[y][x]; // Mantiene el caracter especial
                continue; // Pasa a la siguiente celda sin aplicar las reglas del CA
            }
            //una vez voy recorriendo el mapa debo recorrer el area al rededor del punto para revisar si tiene los valores necesarios para difuminar
            int count = 0;
            for(int radY = - R; radY <= R; radY++){
                for (int radX = - R; radX <= R; radX++){
                    int nx = x + radX;
                    int ny = y + radY;
                    // Si la celda es un carácter especial de layout, NO LA MODIFICAMOS
           
                    
                     if (nx < 0 +1 || nx >= W -1 || ny < 0 +1 || ny >= H -1) {//los limites del mapa se establecen en 2 al lado del borde
            
                        count += 1;
                    }else if (currentMap[ny][nx] == pared){
                        count += 1;
                    }
                    
                }
            }
            noisyMap[y][x] = (count>= U) ? pared:piso;//si en el punto especifico la cantidad de casillas que la rodea es mayor o igual a lo que pide u, entonces se convierte en parte del mapa
        }
    }

    return noisyMap;
}


int main(){
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);
    std::cout << "--- PCG OF STARDEW VALLEY MINES SIMULATION ---" << std::endl;
    std::uniform_int_distribution<int> largoDistribucion(10, 40);
    std::uniform_int_distribution<int> altoDistribucion(10, 40);

    int mapCols = altoDistribucion(generator); //Limite de alto
    int mapRows = largoDistribucion(generator); //Limite de largo
    float playerLuck = 0; //Suerte del jugador
    int idMap = 0; //ID del mapa
    int radiusNoise = 1; //
    double limitNoise = 0.5; //



    Map myMap(mapCols, std::vector<char>(mapRows, piso));int startAreaX = (mapRows / 2) - (layout_start_area[0].size() / 2); // Centra horizontalmente
    int startAreaY = 1; // Un poco separado del borde superior para visibilidad

    // Asegurarse de que no se salga de los límites
    if (startAreaX < 1) startAreaX = 1;
    if (startAreaY < 1) startAreaY = 1;
    if (startAreaX + layout_start_area[0].size() + 1 > mapRows) {
        startAreaX = mapRows - layout_start_area[0].size() - 1;
    }
    if (startAreaY + layout_start_area.size() + 1 > mapCols) {
        startAreaY = mapCols - layout_start_area.size() - 1;
    }

    insertLayout(myMap, layout_start_area, startAreaX, startAreaY);
    std::cout << std::endl;
    //std::cout << "\nInitial map state:" << std::endl;

    // --- Main Simulation Loop ---
    for (int iteration = 0; iteration < 1; ++iteration) {
        //std::cout << "\n--- Iteration " << iteration + 1 << " ---" << std::endl;
        myMap = GenerarRuido(myMap, mapRows, mapCols);
        myMap = cellularAutomata(myMap, mapRows, mapCols, radiusNoise, limitNoise);
        printMap(myMap);
    }
    
    insertLayout(myMap, layout_start_area, 2, 2); // Siempre se coloca el area de inicio

    std::cout << "\n--- Simulation Finished ---" << std::endl;


    return 0;
}