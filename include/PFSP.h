#ifndef PFSP_H
#define PFSP_H

#include <string>
#include <vector>

#include "Instancia.h"

// Una solucion es una permutacion de trabajos (indices desde 0).
// Ejemplo: {4, 1, 0, 3, 2} equivale a [J5, J2, J1, J4, J3].
using Permutacion = std::vector<int>;

// Calcula el makespan con la recurrencia del PFSP:
//   C[i][j] = max(C[i-1][j], C[i][j-1]) + p[pi_i][j]
// Usa O(m) memoria y O(nm) tiempo.
int calcularMakespan(const Instancia& inst, const Permutacion& pi);

// Tabla completa de tiempos de finalizacion C[i][j] (para el diagrama de Gantt).
std::vector<std::vector<int>> tablaFinalizacion(const Instancia& inst, const Permutacion& pi);

// Heuristica constructiva NEH (Nawaz, Enscore y Ham, 1983).
Permutacion heuristicaNEH(const Instancia& inst);

// Verifica que pi contenga cada trabajo exactamente una vez.
bool esPermutacionValida(const Permutacion& pi, int n);

// Convierte una permutacion a texto: [J5, J2, J1, J4, J3]
std::string permutacionATexto(const Permutacion& pi);

#endif
