#ifndef INSTANCIA_H
#define INSTANCIA_H

#include <string>
#include <vector>

// Datos de una instancia del PFSP:
//   n trabajos, m maquinas y la matriz p[trabajo][maquina] con los tiempos.
struct Instancia {
    std::string nombre;                 // nombre del archivo (sin ruta)
    int n = 0;                          // numero de trabajos
    int m = 0;                          // numero de maquinas
    std::vector<std::vector<int>> p;    // p[i][j]: tiempo del trabajo i en la maquina j
};

// Lee un archivo .txt con el formato del curso:
//   primera linea: n m
//   luego una linea por trabajo con pares "maquina tiempo" (maquinas desde 0).
// Lanza std::runtime_error si el archivo no existe o tiene un formato invalido.
Instancia leerInstancia(const std::string& ruta);

#endif
