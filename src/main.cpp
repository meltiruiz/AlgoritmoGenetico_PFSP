// =============================================================================
//  Algoritmo Genetico para el Permutation Flow Shop Scheduling Problem (PFSP)
//  Curso: Analisis y Diseno de Algoritmos - Universidad de Lima (2026-2)
//
//  Uso:
//    ga_pfsp <instancia.txt> [opciones]
//
//  Opciones (valores por defecto entre parentesis):
//    --corridas R      numero de ejecuciones independientes (10)
//    --poblacion P     tamano de la poblacion (100)
//    --generaciones G  numero de generaciones (500)
//    --torneo k        tamano del torneo (3)
//    --pc x            probabilidad de cruce OX (0.90)
//    --pm x            probabilidad de mutacion por insercion (0.20)
//    --elite e         individuos que pasan directo por elitismo (2)
//    --semilla s       semilla de la primera corrida; la corrida r usa s + r - 1 (2026001)
//    --salida dir      carpeta donde se guardan los CSV (resultados)
//    --evaluar 5,2,1,4,3   solo calcula el makespan de esa secuencia y termina
// =============================================================================

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "AlgoritmoGenetico.h"
#include "Instancia.h"
#include "PFSP.h"

namespace fs = std::filesystem;

namespace {

void mostrarAyuda() {
    std::cout << "Uso: ga_pfsp <instancia.txt> [--corridas R] [--poblacion P]\n"
                 "       [--generaciones G] [--torneo k] [--pc x] [--pm x] [--elite e]\n"
                 "       [--semilla s] [--salida dir] [--evaluar 5,2,1,4,3]\n";
}

// Convierte "5,2,1,4,3" (trabajos desde 1) en una permutacion con indices desde 0
Permutacion leerSecuencia(const std::string& texto) {
    Permutacion p;
    std::stringstream ss(texto);
    std::string token;
    while (std::getline(ss, token, ',')) {
        if (!token.empty() && (token[0] == 'J' || token[0] == 'j')) token = token.substr(1);
        p.push_back(std::stoi(token) - 1);
    }
    return p;
}

// Secuencia en formato CSV-amigable: J5 J2 J1 J4 J3
std::string secuenciaCSV(const Permutacion& p) {
    std::string s;
    for (std::size_t i = 0; i < p.size(); ++i) {
        if (i > 0) s += " ";
        s += "J" + std::to_string(p[i] + 1);
    }
    return s;
}

void imprimirMatriz(const Instancia& inst) {
    std::cout << "Matriz de tiempos de procesamiento:\n      ";
    for (int j = 0; j < inst.m; ++j) std::cout << std::setw(6) << ("M" + std::to_string(j + 1));
    std::cout << "\n";
    for (int i = 0; i < inst.n; ++i) {
        std::cout << std::setw(6) << ("J" + std::to_string(i + 1));
        for (int j = 0; j < inst.m; ++j) std::cout << std::setw(6) << inst.p[i][j];
        std::cout << "\n";
    }
    std::cout << "\n";
}

// Guarda inicio y fin de cada operacion de la secuencia (para el diagrama de Gantt)
void guardarGantt(const Instancia& inst, const Permutacion& pi, const std::string& ruta) {
    std::vector<std::vector<int>> C = tablaFinalizacion(inst, pi);
    std::ofstream f(ruta);
    f << "posicion,trabajo,maquina,inicio,fin\n";
    for (std::size_t i = 0; i < pi.size(); ++i) {
        for (int j = 0; j < inst.m; ++j) {
            int fin = C[i][j];
            int inicio = fin - inst.p[pi[i]][j];
            f << (i + 1) << ",J" << (pi[i] + 1) << ",M" << (j + 1) << "," << inicio << ","
              << fin << "\n";
        }
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        mostrarAyuda();
        return 1;
    }

    // ---------------- Lectura de argumentos ----------------
    std::string rutaInstancia = argv[1];
    ParametrosGA params;
    int corridas = 10;
    std::string carpetaSalida = "resultados";
    std::string secuenciaEvaluar;

    try {
        for (int i = 2; i < argc; ++i) {
            std::string op = argv[i];
            auto valor = [&]() -> std::string {
                if (i + 1 >= argc) throw std::runtime_error("Falta el valor de " + op);
                return argv[++i];
            };
            if (op == "--corridas") corridas = std::stoi(valor());
            else if (op == "--poblacion") params.tamPoblacion = std::stoi(valor());
            else if (op == "--generaciones") params.generaciones = std::stoi(valor());
            else if (op == "--torneo") params.tamTorneo = std::stoi(valor());
            else if (op == "--pc") params.probCruce = std::stod(valor());
            else if (op == "--pm") params.probMutacion = std::stod(valor());
            else if (op == "--elite") params.elite = std::stoi(valor());
            else if (op == "--semilla") params.semilla = static_cast<unsigned>(std::stoul(valor()));
            else if (op == "--salida") carpetaSalida = valor();
            else if (op == "--evaluar") secuenciaEvaluar = valor();
            else throw std::runtime_error("Opcion desconocida: " + op);
        }
        if (corridas < 1 || params.tamPoblacion < 2 || params.generaciones < 0 ||
            params.tamTorneo < 1 || params.elite < 0 || params.elite > params.tamPoblacion) {
            throw std::runtime_error("Parametros fuera de rango");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        mostrarAyuda();
        return 1;
    }

    // ---------------- Carga de la instancia ----------------
    Instancia inst;
    try {
        inst = leerInstancia(rutaInstancia);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    std::cout << "=====================================================\n";
    std::cout << " Algoritmo Genetico para PFSP\n";
    std::cout << " Instancia: " << inst.nombre << " (" << inst.n << " trabajos x " << inst.m
              << " maquinas)\n";
    std::cout << "=====================================================\n\n";
    imprimirMatriz(inst);

    // Modo de verificacion: solo evaluar una secuencia dada
    if (!secuenciaEvaluar.empty()) {
        Permutacion p = leerSecuencia(secuenciaEvaluar);
        if (!esPermutacionValida(p, inst.n)) {
            std::cerr << "Error: la secuencia no es una permutacion valida de " << inst.n
                      << " trabajos\n";
            return 1;
        }
        std::cout << "Secuencia " << permutacionATexto(p)
                  << " -> makespan = " << calcularMakespan(inst, p) << "\n";
        return 0;
    }

    std::cout << "Parametros: P=" << params.tamPoblacion << ", G=" << params.generaciones
              << ", k=" << params.tamTorneo << ", pc=" << params.probCruce
              << ", pm=" << params.probMutacion << ", elite=" << params.elite
              << ", corridas=" << corridas << ", semillas=" << params.semilla << "-"
              << (params.semilla + corridas - 1) << "\n\n";

    // ---------------- Corridas independientes ----------------
    std::vector<ResultadoGA> resultados;
    std::vector<unsigned int> semillas;
    for (int r = 1; r <= corridas; ++r) {
        ParametrosGA pr = params;
        pr.semilla = params.semilla + static_cast<unsigned>(r - 1);
        AlgoritmoGenetico ga(inst, pr);
        ResultadoGA res = ga.ejecutar();

        // Control de consistencia: la solucion debe ser una permutacion valida
        if (!esPermutacionValida(res.mejorSecuencia, inst.n) ||
            calcularMakespan(inst, res.mejorSecuencia) != res.mejorMakespan) {
            std::cerr << "Error interno: solucion invalida en la corrida " << r << "\n";
            return 2;
        }

        std::cout << "Corrida " << std::setw(2) << r << " (semilla " << pr.semilla
                  << "): Cmax = " << std::setw(6) << res.mejorMakespan
                  << " | gen. hallazgo = "
                  << std::setw(3) << res.generacionHallazgo << " | tiempo = " << std::fixed
                  << std::setprecision(3) << res.tiempoMs << " ms | "
                  << permutacionATexto(res.mejorSecuencia) << "\n";
        std::cout.unsetf(std::ios::fixed);

        resultados.push_back(res);
        semillas.push_back(pr.semilla);
    }

    // ---------------- Estadisticas ----------------
    int mejorIdx = 0, peor = resultados[0].mejorMakespan;
    double suma = 0.0, sumaTiempo = 0.0;
    for (int r = 0; r < corridas; ++r) {
        const ResultadoGA& res = resultados[r];
        if (res.mejorMakespan < resultados[mejorIdx].mejorMakespan) mejorIdx = r;
        if (res.mejorMakespan > peor) peor = res.mejorMakespan;
        suma += res.mejorMakespan;
        sumaTiempo += res.tiempoMs;
    }
    double promedio = suma / corridas;
    double var = 0.0;
    for (const ResultadoGA& res : resultados) {
        var += (res.mejorMakespan - promedio) * (res.mejorMakespan - promedio);
    }
    double desv = (corridas > 1) ? std::sqrt(var / (corridas - 1)) : 0.0;  // muestral
    double tiempoProm = sumaTiempo / corridas;
    const ResultadoGA& mejor = resultados[mejorIdx];

    std::cout << "\n------------------- RESUMEN -------------------\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Mejor Cmax:      " << mejor.mejorMakespan << " (corrida " << (mejorIdx + 1)
              << ")\n";
    std::cout << "Peor Cmax:       " << peor << "\n";
    std::cout << "Cmax promedio:   " << promedio << "\n";
    std::cout << "Desv. estandar:  " << desv << "\n";
    std::cout << "Tiempo promedio: " << std::setprecision(3) << tiempoProm << " ms\n";
    std::cout << "Mejor secuencia: " << permutacionATexto(mejor.mejorSecuencia) << "\n";

    // ---------------- Archivos de salida ----------------
    std::string base = inst.nombre.substr(0, inst.nombre.find_last_of('.'));
    fs::path carpeta = fs::path(carpetaSalida) / base;
    fs::create_directories(carpeta);

    // 1) Detalle de cada corrida
    {
        std::ofstream f(carpeta / "corridas.csv");
        f << "corrida,semilla,mejor_makespan,mejor_inicial,generacion_hallazgo,"
             "tiempo_ms,secuencia\n";
        f << std::fixed << std::setprecision(3);
        for (int r = 0; r < corridas; ++r) {
            const ResultadoGA& res = resultados[r];
            f << (r + 1) << "," << semillas[r] << "," << res.mejorMakespan << ","
              << res.mejorInicial << "," << res.generacionHallazgo
              << "," << res.tiempoMs << "," << secuenciaCSV(res.mejorSecuencia) << "\n";
        }
    }

    // 2) Convergencia: mejor Cmax acumulado por generacion en cada corrida y su promedio
    {
        std::ofstream f(carpeta / "convergencia.csv");
        f << "generacion";
        for (int r = 1; r <= corridas; ++r) f << ",corrida" << r;
        f << ",promedio\n";
        f << std::fixed << std::setprecision(2);
        for (int g = 0; g <= params.generaciones; ++g) {
            f << g;
            double s = 0.0;
            for (const ResultadoGA& res : resultados) {
                f << "," << res.convergencia[g];
                s += res.convergencia[g];
            }
            f << "," << s / corridas << "\n";
        }
    }

    // 3) Datos del diagrama de Gantt de la mejor solucion
    guardarGantt(inst, mejor.mejorSecuencia, (carpeta / "gantt.csv").string());

    // 4) Resumen de la instancia (una fila, util para la Tabla de resultados)
    {
        std::ofstream f(carpeta / "resumen.csv");
        f << "instancia,n,m,poblacion,generaciones,torneo,pc,pm,elite,corridas,"
             "mejor,peor,promedio,desv_est,tiempo_prom_ms,mejor_corrida,mejor_secuencia\n";
        f << inst.nombre << "," << inst.n << "," << inst.m << "," << params.tamPoblacion << ","
          << params.generaciones << "," << params.tamTorneo << "," << std::fixed
          << std::setprecision(2)
          << params.probCruce << "," << params.probMutacion << "," << params.elite << ","
          << corridas << "," << mejor.mejorMakespan << "," << peor
          << "," << promedio << "," << desv << "," << std::setprecision(3) << tiempoProm << ","
          << (mejorIdx + 1) << "," << secuenciaCSV(mejor.mejorSecuencia) << "\n";
    }

    std::cout << "\nArchivos generados en: " << carpeta.string() << "/\n"
              << "  corridas.csv, convergencia.csv, gantt.csv, resumen.csv\n";
    return 0;
}
