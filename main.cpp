//Código para la generación procedural de minas para Stardew Valley basado en Cellular automata con Room templates y sonido.
//Félix Fuentes e Ignacio Alfaro 

#include <iostream>
#include <vector>
#include <random>   // For random number generation
#include <chrono>   // For seeding the random number generator
#include <numeric>   // Para std::iota
#include <algorithm> // Para std::shuffle
#include <cmath>     // Para floor, cos, sin, PI

using Map = std::vector<std::vector<char>>;

const char pared = '#';
const char piso = ' ';
const char minerales = '.';
const char entrada = 'E';

const int N = 256;
unsigned char Permutation[N*2];
const float GRADIENTS[8][2] = {
    {1, 0}, {-1, 0}, {0, 1}, {0, -1}, // Cardinales
    {1, 1}, {1, -1}, {-1, 1}, {-1, -1} // Diagonales (normalizados en Init)
};
//interpolacion suave
double Smoothstep(double t){
    return t * t * t * (t * (t * 6 - 15) + 10);
}
//interpolacion lineal
double Lerp(double t, double a, double b){
    return a + t * (b - a);
}
//funcion de producto punto
double DotProduct(double gradX, double gradY, double distValX, double distValY) {
    return (gradX * distValX) + (gradY * distValY);
}

void InicializarPerlinNoise() {
    // 1. Llenar la tabla P con valores de 0 a N-1
    std::iota(Permutation, Permutation + N, 0); // Rellena P[0] a P[N-1] con 0, 1, ..., N-1

    // 2. Barajar la tabla P (Fisher-Yates shuffle)
    // Usamos un generador de números aleatorios para el shuffle
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine engine(seed);
    std::shuffle(Permutation, Permutation + N, engine);

    // 3. Duplicar la tabla P para que los índices puedan envolver fácilmente
    for (int i = 0; i < N; ++i) {
        Permutation[i + N] = Permutation[i];
    }

    // Opcional: Normalizar los vectores diagonales si usaste enteros para GRADIENTS_2D
    // for (int i = 0; i < 8; ++i) {
    //     float magnitude = std::sqrt(GRADIENTS_2D[i][0] * GRADIENTS_2D[i][0] + GRADIENTS_2D[i][1] * GRADIENTS_2D[i][1]);
    //     if (magnitude > 0) {
    //         // Se normalizan las diagonales para que sean vectores unitarios
    //         // Los cardinales (1,0) etc. ya son unitarios
    //         if (GRADIENTS_2D[i][0] != 0 && GRADIENTS_2D[i][1] != 0) { // Si es un vector diagonal
    //              GRADIENTS_2D_NORMALIZED[i][0] = GRADIENTS_2D[i][0] / magnitude;
    //              GRADIENTS_2D_NORMALIZED[i][1] = GRADIENTS_2D[i][1] / magnitude;
    //         } else {
    //              GRADIENTS_2D_NORMALIZED[i][0] = GRADIENTS_2D[i][0];
    //              GRADIENTS_2D_NORMALIZED[i][1] = GRADIENTS_2D[i][1];
    //         }
    //     }
    // }
    // En este ejemplo, el uso directo de GRADIENTS_2D en GetPerlinNoise es con los valores ya unitarios.
    // Si tus vectores GRADIENTS_2D no fueran unitarios, deberías normalizarlos aquí o precalcular una tabla normalizada.
    // Para los que te puse (1,1), (1,-1), etc., su magnitud es sqrt(2), por lo que se dividirían por sqrt(2).
    // Si los dejas así, el ruido solo tendrá un rango un poco diferente, pero seguirá siendo coherente.
}

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
                noisyMap[y][x] = ((rand() % 100) < 70) ? piso : pared;
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



double GetPerlinNoise(double x, double y){
    // 1. Encontrar las coordenadas enteras de la celda de la rejilla.
    // floor() nos da el entero más grande que es menor o igual al número.
    int xi = static_cast<int>(std::floor(x));
    int yi = static_cast<int>(std::floor(y));

    // 2. Calcular la posición fraccional (coordenadas locales dentro de la celda [0, 1]).
    double xf = x - xi;
    double yf = y - yi;

    // 3. Obtener los 4 nodos de la esquina y sus gradientes.
    // El operador `& (N - 1)` es una forma eficiente de hacer ` % N` cuando N es una potencia de 2.
    // Usamos la tabla de permutación (P) para obtener un índice pseudo-aleatorio para los gradientes.
    // La fórmula `P[ (X + P[Y & (N-1)]) & (N-1) ]` es la forma estándar de Perlin para obtener
    // un índice hash coherente para cada esquina.

    // Índices de permutación para las 4 esquinas de la celda
    int p00 = Permutation[ (xi + Permutation[yi & (N-1)]) & (N-1) ];
    int p10 = Permutation[ ( (xi + 1) + Permutation[yi & (N-1)] ) & (N-1) ];
    int p01 = Permutation[ (xi + Permutation[ (yi + 1) & (N-1) ]) & (N-1) ];
    int p11 = Permutation[ ( (xi + 1) + Permutation[ (yi + 1) & (N-1) ]) & (N-1) ];

    // Para obtener el vector gradiente, usamos el resultado de la permutación
    // para indexar en la tabla `GRADIENTS_2D`.
    // El `& 7` (que es `& (8-1)`) es porque `GRADIENTS_2D` tiene 8 vectores.
    // Si cambias `GRADIENTS_2D` a otro tamaño que sea potencia de 2, ajusta este `&`.
    double g00_x = GRADIENTS[p00 & 7][0];
    double g00_y = GRADIENTS[p00 & 7][1];

    double g10_x = GRADIENTS[p10 & 7][0];
    double g10_y = GRADIENTS[p10 & 7][1];

    double g01_x = GRADIENTS[p01 & 7][0];
    double g01_y = GRADIENTS[p01 & 7][1];

    double g11_x = GRADIENTS[p11 & 7][0];
    double g11_y = GRADIENTS[p11 & 7][1];

    // 4. Calcular los productos punto para cada esquina.
    // El vector de distancia desde la esquina al punto (x, y) es (xf - rel_x, yf - rel_y)
    // Donde rel_x y rel_y son 0 o 1 dependiendo de la esquina.
    // Por ejemplo, para la esquina (xi, yi), las coordenadas relativas son (0,0), así que (xf-0, yf-0) = (xf, yf).
    // Para la esquina (xi+1, yi), las coordenadas relativas son (1,0), así que (xf-1, yf-0) = (xf-1, yf).
    // Y así sucesivamente.

    double dot00 = DotProduct(g00_x, g00_y, xf, yf);
    double dot10 = DotProduct(g10_x, g10_y, xf - 1.0, yf);
    double dot01 = DotProduct(g01_x, g01_y, xf, yf - 1.0);
    double dot11 = DotProduct(g11_x, g11_y, xf - 1.0, yf - 1.0);

    // 5. Interpolar suavemente los valores.
    // Primero, aplicar la función smoothstep a las coordenadas fraccionales.
    double u = Smoothstep(xf);
    double v = Smoothstep(yf);

    // Interpolación bilineal:
    // a. Interpolar horizontalmente los dos valores superiores.
    double interpX1 = Lerp(u, dot00, dot10);
    // b. Interpolar horizontalmente los dos valores inferiores.
    double interpX2 = Lerp(u, dot01, dot11);

    // c. Finalmente, interpolar verticalmente esos dos resultados.
    double finalNoise = Lerp(v, interpX1, interpX2);

    return finalNoise; // El valor de ruido estará en el rango [-1, 1]
}
void ColocarMinerales(Map& map, int mapCols, int mapRows, double escalaRuidoMineral, double umbralMineral,int entradaX, int entradaY, int radioSeguridadEntrada) {
    for (int y = 0; y < mapRows; ++y) {
        for (int x = 0; x < mapCols; ++x) {
            // Paso 1: Comprobar la zona de seguridad de la entrada
            // Calcula la distancia euclidiana entre la celda actual y la entrada
            double distancia_a_entrada = std::sqrt(std::pow(x - entradaX, 2) + std::pow(y - entradaY, 2));

            // Si la celda está dentro del radio de seguridad, no se colocan minerales.
            if (distancia_a_entrada < radioSeguridadEntrada) {
                continue; // Pasa a la siguiente celda
            }

            // Paso 2: Solo si la celda es un 'piso' y no es la entrada misma
            if (map[y][x] == piso) {
                double noiseValue = GetPerlinNoise(static_cast<double>(x) / escalaRuidoMineral,
                                                   static_cast<double>(y) / escalaRuidoMineral);
                double mappedNoise = (noiseValue + 1.0) * 0.5;

                if (mappedNoise > umbralMineral) {
                    map[y][x] = minerales;
                }
            }
        }
    }
}

int main(){
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);
    InicializarPerlinNoise();
    std::cout << "--- PCG OF STARDEW VALLEY MINES SIMULATION ---" << std::endl;
    int margenX = 5;
    int margenY = 5;

    std::uniform_int_distribution<int> largoDistribucion(10, 40);
    std::uniform_int_distribution<int> altoDistribucion(10, 40);
    
    int mapRows = altoDistribucion(generator);
    int mapCols = largoDistribucion(generator);;
    double limitNoise = 4.0;
    int radiusNoise = 1;
    double escalaMineral = 11.0; 
    double umbralMineral = 0.7;
    //para generación aleatoria de los layouts
     std::uniform_int_distribution<int> distribucionX(margenX, mapCols - layout_start_area[0].size() - margenX);
    std::uniform_int_distribution<int> distribucionY(margenY, mapRows - layout_start_area.size() - margenY);
    //cosas que voy a colocar
    //1. generacion random de los layouts

    ///////////////////////////////////////////////////////////////////////
    Map myMap(mapRows, std::vector<char>(mapCols, pared));
    int startAreaX = (mapCols / 2) - (layout_start_area[0].size() / 2);
    int startAreaY = 1;

    if (startAreaX < 1) startAreaX = 1;
    if (startAreaY < 1) startAreaY = 1;
    if (startAreaX + layout_start_area[0].size() + 1 > mapCols) {
        startAreaX = mapCols - layout_start_area[0].size() - 1;
    }
    if (startAreaY + layout_start_area.size() + 1 > mapRows) {
        startAreaY = mapRows - layout_start_area.size() - 1;
    }

    insertLayout(myMap, layout_start_area, startAreaX, startAreaY);
    insertLayout(myMap, layout_small_mineral_room, mapCols / 4, mapRows / 4);
    insertLayout(myMap, layout_four_way_crossroads, mapCols / 2, mapRows / 2);
    insertLayout(myMap, layout_treasure_room, mapCols * 3 / 4, mapRows * 3 / 4);
    insertLayout(myMap, layout_narrow_tunnel, mapCols / 6, mapRows / 2);

    std::cout << "\n--- Mapa después de insertar layouts (antes del ruido) ---" << std::endl;
    printMap(myMap);

    myMap = GenerarRuido(myMap, mapCols, mapRows);
    // --- Main Simulation Loop ---
    for (int iteration = 0; iteration < 3; ++iteration) {
        std::cout << "\n--- Iteración del Autómata " << iteration + 1 << " ---" << std::endl;
        
        myMap = cellularAutomata(myMap, mapCols, mapRows, radiusNoise, limitNoise);
        printMap(myMap); // <-- Imprime DESPUÉS de la actualización
    }

    ColocarMinerales(myMap, mapCols, mapRows, escalaMineral, umbralMineral, startAreaX, startAreaY, 5);
    printMap(myMap);
    std::cout << "\n--- Simulación Finalizada ---" << std::endl;

    return 0;
}