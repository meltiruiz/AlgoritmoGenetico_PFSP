#include "PFSP.h"

#include <algorithm>
#include <numeric>

int calcularMakespan(const Instancia& inst, const Permutacion& pi) {
    // fila[j] guarda C[i-1][j] (cuando la maquina j queda libre).
    // Al recorrer j de izquierda a derecha, fila[j-1] ya es C[i][j-1].
    std::vector<int> fila(inst.m, 0);
    for (int trabajo : pi) {
        for (int j = 0; j < inst.m; ++j) {
            int maquinaLibre = fila[j];                    // C[i-1][j]
            int trabajoListo = (j > 0) ? fila[j - 1] : 0;  // C[i][j-1]
            fila[j] = std::max(maquinaLibre, trabajoListo) + inst.p[trabajo][j];
        }
    }
    return pi.empty() ? 0 : fila[inst.m - 1];  // C[n][m]
}

std::vector<std::vector<int>> tablaFinalizacion(const Instancia& inst, const Permutacion& pi) {
    int n = static_cast<int>(pi.size());
    std::vector<std::vector<int>> C(n, std::vector<int>(inst.m, 0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < inst.m; ++j) {
            int arriba = (i > 0) ? C[i - 1][j] : 0;     // maquina libre
            int izquierda = (j > 0) ? C[i][j - 1] : 0;  // trabajo listo
            C[i][j] = std::max(arriba, izquierda) + inst.p[pi[i]][j];
        }
    }
    return C;
}

Permutacion heuristicaNEH(const Instancia& inst) {
    // Paso 1: ordenar los trabajos de mayor a menor tiempo total de procesamiento
    std::vector<int> orden(inst.n);
    std::iota(orden.begin(), orden.end(), 0);
    std::vector<int> total(inst.n, 0);
    for (int i = 0; i < inst.n; ++i) {
        total[i] = std::accumulate(inst.p[i].begin(), inst.p[i].end(), 0);
    }
    std::stable_sort(orden.begin(), orden.end(),
                     [&](int a, int b) { return total[a] > total[b]; });

    // Paso 2: insertar cada trabajo en la posicion que da el menor makespan parcial
    Permutacion parcial;
    for (int trabajo : orden) {
        int mejorPos = 0;
        int mejorValor = -1;
        for (int pos = 0; pos <= static_cast<int>(parcial.size()); ++pos) {
            Permutacion prueba = parcial;
            prueba.insert(prueba.begin() + pos, trabajo);
            int valor = calcularMakespan(inst, prueba);
            if (mejorValor < 0 || valor < mejorValor) {
                mejorValor = valor;
                mejorPos = pos;
            }
        }
        parcial.insert(parcial.begin() + mejorPos, trabajo);
    }
    return parcial;
}

bool esPermutacionValida(const Permutacion& pi, int n) {
    if (static_cast<int>(pi.size()) != n) return false;
    std::vector<bool> visto(n, false);
    for (int t : pi) {
        if (t < 0 || t >= n || visto[t]) return false;
        visto[t] = true;
    }
    return true;
}

std::string permutacionATexto(const Permutacion& pi) {
    std::string s = "[";
    for (std::size_t i = 0; i < pi.size(); ++i) {
        if (i > 0) s += ", ";
        s += "J" + std::to_string(pi[i] + 1);
    }
    return s + "]";
}
