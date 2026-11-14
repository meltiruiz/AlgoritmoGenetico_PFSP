#include "PFSP.h"

#include <algorithm>

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
