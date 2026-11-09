#include "Instancia.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

Instancia leerInstancia(const std::string& ruta) {
    std::ifstream archivo(ruta);
    if (!archivo) {
        throw std::runtime_error("No se pudo abrir el archivo: " + ruta);
    }

    Instancia inst;
    // Nombre del archivo sin la carpeta, para usarlo en los reportes
    std::size_t pos = ruta.find_last_of("/\\");
    inst.nombre = (pos == std::string::npos) ? ruta : ruta.substr(pos + 1);

    // Busca la primera linea que tenga exactamente dos enteros (n y m).
    // Asi se tolera una linea de descripcion opcional al inicio del archivo.
    std::string linea;
    bool encabezado = false;
    while (std::getline(archivo, linea)) {
        std::istringstream ss(linea);
        int a, b;
        std::string resto;
        if (ss >> a >> b && !(ss >> resto)) {
            inst.n = a;
            inst.m = b;
            encabezado = true;
            break;
        }
    }
    if (!encabezado || inst.n <= 0 || inst.m <= 0) {
        throw std::runtime_error("No se encontro la linea con n y m en: " + ruta);
    }

    // Cada trabajo tiene m pares (maquina, tiempo)
    inst.p.assign(inst.n, std::vector<int>(inst.m, -1));
    for (int i = 0; i < inst.n; ++i) {
        for (int k = 0; k < inst.m; ++k) {
            int maquina, tiempo;
            if (!(archivo >> maquina >> tiempo)) {
                throw std::runtime_error("Faltan datos del trabajo " + std::to_string(i + 1));
            }
            if (maquina < 0 || maquina >= inst.m) {
                throw std::runtime_error("Indice de maquina invalido en el trabajo " +
                                         std::to_string(i + 1));
            }
            inst.p[i][maquina] = tiempo;
        }
    }
    return inst;
}
