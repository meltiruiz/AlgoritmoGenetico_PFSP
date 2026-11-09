# Algoritmo Genético para el PFSP

Implementación en C++ de un **algoritmo genético (GA)** para el *Permutation Flow Shop Scheduling Problem* (PFSP), minimizando el **makespan**.
Proyecto del curso Análisis y Diseño de Algoritmos — Universidad de Lima, 2026-2.

## Diseño del algoritmo

| Componente | Implementación |
|---|---|
| Representación | Permutación de trabajos, por ejemplo `[J5, J2, J1, J4, J3]` |
| Evaluación | `C[i][j] = max(C[i-1][j], C[i][j-1]) + p[i][j]`, makespan = `C[n][m]` (O(nm)) |
| Población inicial | 1 solución NEH + (P − 1) permutaciones aleatorias |
| Selección | Torneo de tamaño k (gana el de menor makespan) |
| Cruce | Cruce de orden (OX) con probabilidad pc |
| Mutación | Inserción (extrae un trabajo y lo inserta en otra posición) con probabilidad pm |
| Reemplazo | Elitismo: los e mejores pasan directo; se evitan duplicados cuando es posible |
| Parada | Número fijo de generaciones G |

## Estructura

```
include/              cabeceras (.h)
src/
  main.cpp            lectura de argumentos, corridas, estadísticas y archivos CSV
  Instancia.cpp       lectura del archivo .txt
  PFSP.cpp            makespan, tabla de tiempos, heurística NEH
  AlgoritmoGenetico.cpp  GA: población inicial, torneo, OX, inserción, elitismo
instancias/           las 3 instancias del curso
scripts/
  ejecutar_todo.bat   compila y ejecuta las 3 instancias (Windows)
  ejecutar_todo.sh    compila y ejecuta las 3 instancias (Linux/macOS)
  graficar.py         genera las curvas de convergencia y los diagramas de Gantt
```

## Compilación

Con g++ (C++17):

```bash
g++ -std=c++17 -O2 -Iinclude src/*.cpp -o ga_pfsp
```

En Windows (CMD) hay que listar los archivos: `g++ -std=c++17 -O2 -Iinclude src\main.cpp src\Instancia.cpp src\PFSP.cpp src\AlgoritmoGenetico.cpp -o ga_pfsp.exe`

También se puede usar CMake:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Ejecución

```bash
./ga_pfsp instancias/instancia1_bas1.txt
```

Opciones (valores por defecto entre paréntesis):

| Opción | Significado |
|---|---|
| `--corridas R` | ejecuciones independientes (10) |
| `--poblacion P` | tamaño de la población (100) |
| `--generaciones G` | número de generaciones (500) |
| `--torneo k` | tamaño del torneo (3) |
| `--pc x` | probabilidad de cruce (0.90) |
| `--pm x` | probabilidad de mutación (0.20) |
| `--elite e` | individuos que pasan por elitismo (2) |
| `--semilla s` | semilla de la corrida 1; la corrida r usa `s + r − 1` (2026001) |
| `--salida dir` | carpeta de resultados (`resultados`) |
| `--evaluar 5,2,1,4,3` | solo calcula el makespan de esa secuencia |

Ejemplo de verificación del evaluador: `./ga_pfsp instancias/instancia1_bas1.txt --evaluar 5,2,1,4,3` debe dar makespan 52.

## Archivos generados

Por cada instancia se crea `resultados/<instancia>/` con:

- `corridas.csv`: semilla, mejor makespan, makespan NEH, generación del hallazgo, tiempo (ms) y secuencia de cada corrida.
- `convergencia.csv`: mejor makespan acumulado por generación (generación 0 = población inicial) en cada corrida, más el promedio.
- `gantt.csv`: inicio y fin de cada operación de la mejor solución.
- `resumen.csv`: mejor, peor, promedio, desviación estándar muestral y tiempo promedio.

El tiempo se mide con `std::chrono::high_resolution_clock` y solo incluye la optimización (no la lectura del archivo ni la escritura de los CSV).

## Gráficos

```bash
pip install pandas matplotlib
python scripts/graficar.py
```

Genera `convergencia.png` y `gantt.png` dentro de cada carpeta de resultados.
