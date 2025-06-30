// Código para la generación procedural de minas para Stardew Valley.
// Basado en Cellular Automata y Room Templates.
// Félix Fuentes e Ignacio Alfaro
// (Aguantando hasta el final del semestre)

#include <iostream>
#include <vector>
#include <random>    // Para generación aleatoria.
#include <chrono>    // Para la semilla del generador.
#include <numeric>   // Para std::iota.
#include <algorithm> // Para std::shuffle.
#include <cmath>     // Para funciones matemáticas.
#include <queue>     // Para el algoritmo BFS.
#include <utility>   // Para std::pair.

using Map = std::vector<std::vector<char>>; // Alias para el tipo de mapa.

const char pared = '#';      // Carácter para las paredes.
const char piso = ' ';       // Carácter para los pisos transitables.
const char minerales = '.';  // Carácter para los minerales.
const char entrada = 'E';    // Carácter para el punto de entrada.

// --- Parámetros y funciones de Perlin Noise ---
// Esenciales para generar texturas orgánicas.
const int N = 256;
unsigned char Permutation[N*2];
const float GRADIENTS[8][2] = {
    {1, 0}, {-1, 0}, {0, 1}, {0, -1},
    {1, 1}, {1, -1}, {-1, 1}, {-1, -1}
};

// Interlación suave.
double Smoothstep(double t){
    return t * t * t * (t * (t * 6 - 15) + 10);
}

// Interpolación lineal.
double Lerp(double t, double a, double b){
    return a + t * (b - a);
}

// Producto punto de vectores.
double DotProduct(double gradX, double gradY, double distValX, double distValY) {
    return (gradX * distValX) + (gradY * distValY);
}

// Inicializa la tabla de permutación para el ruido Perlin.
void InicializarPerlinNoise() {
    std::iota(Permutation, Permutation + N, 0);
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine engine(seed);
    std::shuffle(Permutation, Permutation + N, engine);
    for (int i = 0; i < N; ++i) {
        Permutation[i + N] = Permutation[i];
    }
}

//mapas de layouts predeterminados
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
    {pared, minerales,  piso,  piso,  minerales,  piso,  pared}, // 'M' se convierte en 'piso'
    {pared, piso,  minerales,  minerales,  minerales,  minerales,  pared},
    {pared, piso,  minerales,  piso,  minerales,  piso,  pared},
    {pared, pared, pared, pared, pared, pared, pared}
};

// 7. Sala Abierta (Open Chamber)
const Map layout_open_chamber = {
    {piso,  piso,  piso,  piso,  piso,  piso,  piso,  piso,  piso},
    {piso,  pared, pared, pared, pared, pared, pared, pared, piso},
    {piso,  pared, minerales, pared, pared, pared, pared, pared, piso},
    {piso,  pared, pared, minerales, minerales, pared, pared, pared, piso},
    {piso,  minerales, pared, minerales, pared, pared, pared, pared, piso},
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




// insertLayout: Inserta un layout en el mapa principal en las coordenadas especificadas.
void insertLayout(Map& mainMap, const Map& layout, int startX, int startY) {
    int layoutRows = layout.size();
    if (layoutRows == 0) return;
    int layoutCols = layout[0].size();

    for (int y = 0; y < layoutRows; ++y) {
        for (int x = 0; x < layoutCols; ++x) {
            int currentMapX = startX + x;
            int currentMapY = startY + y;
            // Asegura que no se salga de los límites del mapa.
            if (currentMapX >= 0 && currentMapX < mainMap[0].size() &&
                currentMapY >= 0 && currentMapY < mainMap.size()) {
                mainMap[currentMapY][currentMapX] = layout[y][x];
            }
        }
    }
}

// printMap: Imprime el mapa en la consola para visualización.
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

// GenerarRuido: Aplica un ruido inicial al mapa, convirtiendo paredes en pisos aleatoriamente.
Map GenerarRuido(const Map& currentMap, int W, int H, std::mt19937& generator){
    // Nota: Usar srand/rand aquí es subóptimo con std::mt19937, pero funciona.
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    srand(seed);

    Map noisyMap = currentMap;
    for (int y = 0; y < H; ++y) {
        for (int x = 0; x < W; ++x) {
            if(noisyMap[y][x] == pared){
                noisyMap[y][x] = ((rand() % 100) < 70) ? piso : pared;
            }
        }
    }
    return noisyMap;
}

// cellularAutomata: Aplica las reglas del autómata celular para refinar el mapa.
// Decide si una celda es pared o piso basándose en sus vecinos en un radio 'R' y un umbral 'U'.
Map cellularAutomata(const Map& currentMap, int W, int H, int R, double U) {
    Map newMap = currentMap;

    for (int y = 0; y < H ; y++){
        for (int x = 0; x < W; x ++){
            // Preserva las celdas de entrada y minerales de los layouts.
            if (currentMap[y][x] == entrada || currentMap[y][x] == minerales) {
                newMap[y][x] = currentMap[y][x];
                continue;
            }

            int count = 0; // Contador de vecinos que son pared.
            for(int radY = - R; radY <= R; radY++){
                for (int radX = - R; radX <= R; radX++){
                    int nx = x + radX;
                    int ny = y + radY;
                    // Los bordes del mapa siempre cuentan como pared.
                    if (nx < 1 || nx >= W - 1 || ny < 1 || ny >= H - 1) {
                        count += 1;
                    } else if (currentMap[ny][nx] == pared){
                        count += 1;
                    }
                }
            }
            // Regla del autómata: si el número de vecinos pared es mayor o igual a U, la celda se vuelve pared.
            newMap[y][x] = (count >= U) ? pared : piso;
        }
    }
    return newMap;
}

void Initialize_random_Layout(Map& currentMap,int margenX, int margenY, int mapCols, int mapRows,std::mt19937& generator){
    
     // Layout: small_mineral_room
    int mineralRoomWidth = layout_small_mineral_room[0].size();
    int mineralRoomHeight = layout_small_mineral_room.size();
    if (mapCols - mineralRoomWidth - margenX >= margenX && mapRows - mineralRoomHeight - margenY >= margenY) {
        std::uniform_int_distribution<int> distribucionX(margenX, mapCols - mineralRoomWidth - margenX);
        std::uniform_int_distribution<int> distribucionY(margenY, mapRows - mineralRoomHeight - margenY);
        insertLayout(currentMap, layout_small_mineral_room, distribucionX(generator), distribucionY(generator));
    } else {
        std::cerr << "Advertencia: Mapa demasiado pequeño para layout_small_mineral_room." << std::endl;
    }

    // Layout: four_way_crossroads
    int crossroadsWidth = layout_four_way_crossroads[0].size();
    int crossroadsHeight = layout_four_way_crossroads.size();
    if (mapCols - crossroadsWidth - margenX >= margenX && mapRows - crossroadsHeight - margenY >= margenY) {
        std::uniform_int_distribution<int> distribucionX(margenX, mapCols - crossroadsWidth - margenX);
        std::uniform_int_distribution<int> distribucionY(margenY, mapRows - crossroadsHeight - margenY);
        insertLayout(currentMap, layout_four_way_crossroads, distribucionX(generator), distribucionY(generator));
    } else {
        std::cerr << "Advertencia: Mapa demasiado pequeño para layout_four_way_crossroads." << std::endl;
    }

    // Layout: treasure_room
    int treasureRoomWidth = layout_treasure_room[0].size();
    int treasureRoomHeight = layout_treasure_room.size();
    if (mapCols - treasureRoomWidth - margenX >= margenX && mapRows - treasureRoomHeight - margenY >= margenY) {
        std::uniform_int_distribution<int> distribucionX(margenX, mapCols - treasureRoomWidth - margenX);
        std::uniform_int_distribution<int> distribucionY(margenY, mapRows - treasureRoomHeight - margenY);
        insertLayout(currentMap, layout_treasure_room, distribucionX(generator), distribucionY(generator));
    } else {
        std::cerr << "Advertencia: Mapa demasiado pequeño para layout_treasure_room." << std::endl;
    }

    // Layout: narrow_tunnel 
    int narrowTunnelWidth = layout_narrow_tunnel[0].size();
    int narrowTunnelHeight = layout_narrow_tunnel.size();
    if (mapCols - narrowTunnelWidth - margenX >= margenX && mapRows - narrowTunnelHeight - margenY >= margenY) {
        std::uniform_int_distribution<int> distribucionX(margenX, mapCols - narrowTunnelWidth - margenX); // Usar narrowTunnelWidth
        std::uniform_int_distribution<int> distribucionY(margenY, mapRows - narrowTunnelHeight - margenY); // Usar narrowTunnelHeight
        insertLayout(currentMap, layout_narrow_tunnel, distribucionX(generator), distribucionY(generator));
    } else {
        std::cerr << "Advertencia: Mapa demasiado pequeño para layout_narrow_tunnel." << std::endl;
    }

    // Layout: curved_corridor
    int curvedCorridorWidth = layout_curved_corridor[0].size();
    int curvedCorridorHeight = layout_curved_corridor.size();
    if (mapCols - curvedCorridorWidth - margenX >= margenX && mapRows - curvedCorridorHeight - margenY >= margenY) {
        std::uniform_int_distribution<int> distribucionX(margenX, mapCols - curvedCorridorWidth - margenX);
        std::uniform_int_distribution<int> distribucionY(margenY, mapRows - curvedCorridorHeight - margenY);
        insertLayout(currentMap, layout_curved_corridor, distribucionX(generator), distribucionY(generator));
    } else {
        std::cerr << "Advertencia: Mapa demasiado pequeño para layout_curved_corridor." << std::endl;
    }

    // Layout: basic_corridor
    int basicCorridorWidth = layout_basic_corridor[0].size();
    int basicCorridorHeight = layout_basic_corridor.size();
    if (mapCols - basicCorridorWidth - margenX >= margenX && mapRows - basicCorridorHeight - margenY >= margenY) {
        std::uniform_int_distribution<int> distribucionX(margenX, mapCols - basicCorridorWidth - margenX);
        std::uniform_int_distribution<int> distribucionY(margenY, mapRows - basicCorridorHeight - margenY);
        insertLayout(currentMap, layout_basic_corridor, distribucionX(generator), distribucionY(generator));
    } else {
        std::cerr << "Advertencia: Mapa demasiado pequeño para layout_basic_corridor." << std::endl;
    }

    // Layout: t_junction
    int tJunctionWidth = layout_t_junction[0].size();
    int tJunctionHeight = layout_t_junction.size();
    if (mapCols - tJunctionWidth - margenX >= margenX && mapRows - tJunctionHeight - margenY >= margenY) {
        std::uniform_int_distribution<int> distribucionX(margenX, mapCols - tJunctionWidth - margenX);
        std::uniform_int_distribution<int> distribucionY(margenY, mapRows - tJunctionHeight - margenY);
        insertLayout(currentMap, layout_t_junction, distribucionX(generator), distribucionY(generator));
    } else {
        std::cerr << "Advertencia: Mapa demasiado pequeño para layout_t_junction." << std::endl;
    }

    // Layout: open_chamber
    int openChamberWidth = layout_open_chamber[0].size();
    int openChamberHeight = layout_open_chamber.size();
    if (mapCols - openChamberWidth - margenX >= margenX && mapRows - openChamberHeight - margenY >= margenY) {
        std::uniform_int_distribution<int> distribucionX(margenX, mapCols - openChamberWidth - margenX);
        std::uniform_int_distribution<int> distribucionY(margenY, mapRows - openChamberHeight - margenY);
        insertLayout(currentMap, layout_open_chamber, distribucionX(generator), distribucionY(generator));
    } else {
        std::cerr << "Advertencia: Mapa demasiado pequeño para layout_open_chamber." << std::endl;
    }

    // Layout: rock_blockage
    int rockBlockageWidth = layout_rock_blockage[0].size();
    int rockBlockageHeight = layout_rock_blockage.size();
    if (mapCols - rockBlockageWidth - margenX >= margenX && mapRows - rockBlockageHeight - margenY >= margenY) {
        std::uniform_int_distribution<int> distribucionX(margenX, mapCols - rockBlockageWidth - margenX);
        std::uniform_int_distribution<int> distribucionY(margenY, mapRows - rockBlockageHeight - margenY);
        insertLayout(currentMap, layout_rock_blockage, distribucionX(generator), distribucionY(generator));
    } else {
        std::cerr << "Advertencia: Mapa demasiado pequeño para layout_rock_blockage." << std::endl;
    }
}

// GetPerlinNoise: Calcula el valor de ruido Perlin en un punto (x, y).
double GetPerlinNoise(double x, double y){
    int xi = static_cast<int>(std::floor(x));
    int yi = static_cast<int>(std::floor(y));
    double xf = x - xi;
    double yf = y - yi;

    int p00 = Permutation[ (xi + Permutation[yi & (N-1)]) & (N-1) ];
    int p10 = Permutation[ ( (xi + 1) + Permutation[yi & (N-1)] ) & (N-1) ];
    int p01 = Permutation[ (xi + Permutation[ (yi + 1) & (N-1) ]) & (N-1) ];
    int p11 = Permutation[ ( (xi + 1) + Permutation[ (yi + 1) & (N-1) ]) & (N-1) ];

    double g00_x = GRADIENTS[p00 & 7][0]; double g00_y = GRADIENTS[p00 & 7][1];
    double g10_x = GRADIENTS[p10 & 7][0]; double g10_y = GRADIENTS[p10 & 7][1];
    double g01_x = GRADIENTS[p01 & 7][0]; double g01_y = GRADIENTS[p01 & 7][1];
    double g11_x = GRADIENTS[p11 & 7][0]; double g11_y = GRADIENTS[p11 & 7][1];

    double dot00 = DotProduct(g00_x, g00_y, xf, yf);
    double dot10 = DotProduct(g10_x, g10_y, xf - 1.0, yf);
    double dot01 = DotProduct(g01_x, g01_y, xf, yf - 1.0);
    double dot11 = DotProduct(g11_x, g11_y, xf - 1.0, yf - 1.0);

    double u = Smoothstep(xf);
    double v = Smoothstep(yf);

    double interpX1 = Lerp(u, dot00, dot10);
    double interpX2 = Lerp(u, dot01, dot11);
    double finalNoise = Lerp(v, interpX1, interpX2);

    return finalNoise;
}

// ColocarMinerales: Distribuye minerales en las zonas de piso usando Perlin Noise.
// Se evita colocar minerales cerca del punto de entrada.
void ColocarMinerales(Map& map, int mapCols, int mapRows, double escalaRuidoMineral, double umbralMineral,int entradaX, int entradaY, int radioSeguridadEntrada) {
    for (int y = 0; y < mapRows; ++y) {
        for (int x = 0; x < mapCols; ++x) {
            // Comprueba si la celda está dentro del radio de seguridad de la entrada.
            double distancia_a_entrada = std::sqrt(std::pow(x - entradaX, 2) + std::pow(y - entradaY, 2));
            if (distancia_a_entrada < radioSeguridadEntrada) {
                continue;
            }

            // Si es piso, decide si se convierte en mineral usando ruido Perlin.
            if (map[y][x] == piso) {
                double noiseValue = GetPerlinNoise(static_cast<double>(x) / escalaRuidoMineral,
                                                   static_cast<double>(y) / escalaRuidoMineral);
                double mappedNoise = (noiseValue + 1.0) * 0.5; // Mapea el ruido a [0, 1].

                if (mappedNoise > umbralMineral) {
                    map[y][x] = minerales;
                }
            }
        }
    }
}

// getNeighbors: Devuelve las coordenadas de los vecinos adyacentes (no diagonales).
std::vector<std::pair<int, int>> getNeighbors(int x, int y, int W, int H) {
    std::vector<std::pair<int, int>> neighbors;
    int dx[] = {0, 0, 1, -1};
    int dy[] = {1, -1, 0, 0};
    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (nx >= 0 && nx < W && ny >= 0 && ny < H) {
            neighbors.push_back({nx, ny});
        }
    }
    return neighbors;
}

// findReachable: Realiza un BFS (Breadth-First Search) para encontrar todas las celdas transitables
// (piso, mineral, entrada) que son accesibles desde un punto de inicio.
std::vector<std::vector<bool>> findReachable(const Map& map, int startX, int startY) {
    int H = map.size();
    int W = map[0].size();
    std::vector<std::vector<bool>> visited(H, std::vector<bool>(W, false));
    std::queue<std::pair<int, int>> q;

    if (startX >= 0 && startX < W && startY >= 0 && startY < H &&
        (map[startY][startX] == piso || map[startY][startX] == minerales || map[startY][startX] == entrada)) {
        q.push({startX, startY});
        visited[startY][startX] = true;
    } else {
        std::cerr << "Advertencia: La celda de inicio del Flood Fill (" << startX << ", " << startY << ") no es transitable." << std::endl;
        return visited;
    }

    while (!q.empty()) {
        std::pair<int, int> current = q.front();
        q.pop();
        int x = current.first;
        int y = current.second;

        for (const auto& neighbor : getNeighbors(x, y, W, H)) {
            int nx = neighbor.first;
            int ny = neighbor.second;

            if ((map[ny][nx] == piso || map[ny][nx] == minerales || map[ny][nx] == entrada) &&
                !visited[ny][nx]) {
                visited[ny][nx] = true;
                q.push({nx, ny});
            }
        }
    }
    return visited;
}

// connectPoints: Conecta dos puntos en el mapa creando un camino de 'piso'.
// Usa un algoritmo simple de conexión en L (horizontal, luego vertical).
void connectPoints(Map& map, std::pair<int, int> p1, std::pair<int, int> p2) {
    while (p1.first != p2.first) {
        if (map[p1.second][p1.first] == pared || map[p1.second][p1.first] == minerales) {
            map[p1.second][p1.first] = piso;
        }
        p1.first += (p1.first < p2.first) ? 1 : -1;
    }
    while (p1.second != p2.second) {
        if (map[p1.second][p1.first] == pared || map[p1.second][p1.first] == minerales) {
            map[p1.second][p1.first] = piso;
        }
        p1.second += (p1.second < p2.second) ? 1 : -1;
    }
    if (map[p1.second][p1.first] == pared || map[p1.second][p1.first] == minerales) {
        map[p1.second][p1.first] = piso;
    }
}

int main(){
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);
    InicializarPerlinNoise(); // Inicializa el generador de ruido.
    std::cout << "--- PCG OF STARDEW VALLEY MINES SIMULATION ---" << std::endl;

    int margenX = 10; // Márgenes para la colocación de layouts.
    int margenY = 10;

    std::uniform_int_distribution<int> largoDistribucion(10, 40); // Rango de tamaños del mapa.
    std::uniform_int_distribution<int> altoDistribucion(10, 40);
    
    int mapRows = altoDistribucion(generator);
    int mapCols = largoDistribucion(generator);
    double limitNoise = 4.0; // Umbral para el autómata celular.
    int radiusNoise = 1;     // Radio de vecinos para el autómata.
    double escalaMineral = 11.0; 
    double umbralMineral = 0.7;

    Map myMap(mapRows, std::vector<char>(mapCols, pared)); // Inicializa el mapa lleno de paredes.

    // Coloca la sala de inicio y busca el punto de entrada 'E'.
    int startAreaX = mapCols / 4
     - (layout_start_area[0].size() / 2);
    int startAreaY = 0;
    // Ajustes de límites para la sala de inicio.
    if (startAreaX < 1) startAreaX = 1;
    if (startAreaY < 1) startAreaY = 1;
    if (startAreaX + layout_start_area[0].size() + 1 > mapCols) {
        startAreaX = mapCols - layout_start_area[0].size() - 1;
    }
    if (startAreaY + layout_start_area.size() + 1 > mapRows) {
        startAreaY = mapRows - layout_start_area.size() - 1;
    }
    insertLayout(myMap, layout_start_area, startAreaX, startAreaY);

    int entryPointX = -1, entryPointY = -1; // Coordenadas del punto de entrada.
    for(int y = startAreaY; y < startAreaY + layout_start_area.size(); ++y) {
        for(int x = startAreaX; x < startAreaX + layout_start_area[0].size(); ++x) {
            if(myMap[y][x] == entrada) {
                entryPointX = x;
                entryPointY = y;
                break;
            }
        }
        if (entryPointX != -1) break;
    }
    
    // Inserta otros layouts aleatoriamente.
    Initialize_random_Layout(myMap,margenX, margenY, mapCols, mapRows, generator);
    std::cout << "\n--- Mapa después de insertar layouts ---" << std::endl;
    printMap(myMap);

    // Aplica ruido inicial.
    myMap = GenerarRuido(myMap, mapCols, mapRows, generator);

    // Bucle principal del Autómata Celular.
    for (int iteration = 0; iteration < 3; ++iteration) {
        std::cout << "\n--- Iteración del Autómata " << iteration + 1 << " ---" << std::endl;
        myMap = cellularAutomata(myMap, mapCols, mapRows, radiusNoise, limitNoise);
        printMap(myMap);
    }

    // Comprueba y conecta regiones aisladas usando Flood Fill.
    if (entryPointX != -1 && entryPointY != -1) {
        std::vector<std::vector<bool>> reachableCells = findReachable(myMap, entryPointX, entryPointY);
        std::cout << "\n--- Comprobando y Conectando Regiones ---" << std::endl;
        bool connectedNewRegion = false;
        std::pair<int, int> mainRegionAnchor = {entryPointX, entryPointY};

        for (int y = 0; y < mapRows; ++y) {
            for (int x = 0; x < mapCols; ++x) {
                if ((myMap[y][x] == piso || myMap[y][x] == minerales) && !reachableCells[y][x]) {
                    // Si encuentra una celda transitable no conectada, inicia un BFS para encontrar su región.
                    Map tempMap = myMap;
                    std::vector<std::vector<bool>> isolatedRegionVisited = findReachable(tempMap, x, y);

                    int isolatedRegionAnchorX = -1, isolatedRegionAnchorY = -1;
                    for (int iy = 0; iy < mapRows; ++iy) {
                        for (int ix = 0; ix < mapCols; ++ix) {
                            if (isolatedRegionVisited[iy][ix]) {
                                isolatedRegionAnchorX = ix;
                                isolatedRegionAnchorY = iy;
                                break;
                            }
                        }
                        if (isolatedRegionAnchorX != -1) break;
                    }

                    if (isolatedRegionAnchorX != -1) {
                        std::cout << "  > Conectando región aislada en (" << isolatedRegionAnchorX
                                  << ", " << isolatedRegionAnchorY << ") a (" << mainRegionAnchor.first
                                  << ", " << mainRegionAnchor.second << ")" << std::endl;
                        connectPoints(myMap, {isolatedRegionAnchorX, isolatedRegionAnchorY}, mainRegionAnchor);
                        connectedNewRegion = true;
                        // Vuelve a calcular las celdas alcanzables después de la conexión.
                        reachableCells = findReachable(myMap, entryPointX, entryPointY);
                        if (reachableCells[y][x]) {
                            continue;
                        }
                    }
                }
            }
        }
        if (!connectedNewRegion) {
            std::cout << "  > No se encontraron regiones aisladas para conectar." << std::endl;
        } else {
             std::cout << "\n--- Mapa después de conectar regiones ---" << std::endl;
             printMap(myMap);
        }
    } else {
        std::cerr << "Error: La celda de entrada 'E' no se pudo encontrar en el mapa. La conectividad no se verificó." << std::endl;
    }

    // Coloca los minerales en el mapa final.
    ColocarMinerales(myMap, mapCols, mapRows, escalaMineral, umbralMineral, startAreaX, startAreaY, 5);
    std::cout << "\n--- Mapa Final (con minerales) ---" << std::endl;
    printMap(myMap);
    std::cout << "\n--- Simulación Finalizada ---" << std::endl;

    return 0;
}