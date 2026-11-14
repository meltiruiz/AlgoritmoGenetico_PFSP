#ifndef ALGORITMO_GENETICO_H
#define ALGORITMO_GENETICO_H

#include <random>
#include <vector>

#include "Instancia.h"
#include "PFSP.h"

// Parametros del algoritmo genetico (ver seccion 3 y 4 del informe)
struct ParametrosGA {
    int tamPoblacion = 100;        // P
    int generaciones = 500;        // G
    int tamTorneo = 3;             // k
    double probCruce = 0.90;       // pc
    double probMutacion = 0.20;    // pm
    int elite = 2;                 // e
    unsigned int semilla = 2026001;
};

// Un individuo: cromosoma (permutacion) y su makespan
struct Individuo {
    Permutacion cromosoma;
    int makespan = 0;
};

// Resultado de una corrida
struct ResultadoGA {
    Permutacion mejorSecuencia;
    int mejorMakespan = 0;
    int mejorInicial = 0;              // mejor makespan de la poblacion inicial
    int generacionHallazgo = 0;        // generacion en la que aparecio el mejor valor
    double tiempoMs = 0.0;             // tiempo solo de optimizacion
    std::vector<int> convergencia;     // mejor Cmax acumulado; [0] = poblacion inicial
};

class AlgoritmoGenetico {
public:
    AlgoritmoGenetico(const Instancia& inst, const ParametrosGA& params);

    // Ejecuta el GA completo y devuelve la mejor solucion encontrada
    ResultadoGA ejecutar();

private:
    const Instancia& inst_;
    ParametrosGA params_;
    std::mt19937 rng_;

    Individuo crearIndividuo(const Permutacion& p);
    std::vector<Individuo> poblacionInicial();
    const Individuo& torneo(const std::vector<Individuo>& poblacion);
    Permutacion cruceOX(const Permutacion& padre1, const Permutacion& padre2);
    void mutacionInsercion(Permutacion& p);
    double aleatorio();  // numero uniforme en [0, 1)
};

#endif
