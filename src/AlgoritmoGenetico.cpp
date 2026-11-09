#include "AlgoritmoGenetico.h"

#include <algorithm>
#include <chrono>
#include <numeric>
#include <set>

AlgoritmoGenetico::AlgoritmoGenetico(const Instancia& inst, const ParametrosGA& params)
    : inst_(inst), params_(params), rng_(params.semilla) {}

double AlgoritmoGenetico::aleatorio() {
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng_);
}

Individuo AlgoritmoGenetico::crearIndividuo(const Permutacion& p) {
    Individuo ind;
    ind.cromosoma = p;
    ind.makespan = calcularMakespan(inst_, p);
    return ind;
}

// Poblacion inicial: 1 individuo construido con NEH + (P - 1) permutaciones aleatorias
std::vector<Individuo> AlgoritmoGenetico::poblacionInicial(int& makespanNEH) {
    std::vector<Individuo> poblacion;
    poblacion.reserve(params_.tamPoblacion);

    Individuo neh = crearIndividuo(heuristicaNEH(inst_));
    makespanNEH = neh.makespan;
    poblacion.push_back(neh);

    Permutacion base(inst_.n);
    std::iota(base.begin(), base.end(), 0);
    while (static_cast<int>(poblacion.size()) < params_.tamPoblacion) {
        Permutacion p = base;
        std::shuffle(p.begin(), p.end(), rng_);
        poblacion.push_back(crearIndividuo(p));
    }
    return poblacion;
}

// Seleccion por torneo: se eligen k individuos al azar y gana el de menor makespan
const Individuo& AlgoritmoGenetico::torneo(const std::vector<Individuo>& poblacion) {
    std::uniform_int_distribution<int> dist(0, static_cast<int>(poblacion.size()) - 1);
    int mejor = dist(rng_);
    for (int i = 1; i < params_.tamTorneo; ++i) {
        int rival = dist(rng_);
        if (poblacion[rival].makespan < poblacion[mejor].makespan) {
            mejor = rival;
        }
    }
    return poblacion[mejor];
}

// Cruce de orden (OX):
//  1) se copia al hijo el segmento [a, b] del padre 1;
//  2) las posiciones restantes se llenan, empezando despues de b, con los trabajos
//     del padre 2 en su orden relativo (tambien desde despues de b) que aun no estan.
// El hijo siempre es una permutacion valida.
Permutacion AlgoritmoGenetico::cruceOX(const Permutacion& padre1, const Permutacion& padre2) {
    int n = static_cast<int>(padre1.size());
    if (n < 2) return padre1;

    std::uniform_int_distribution<int> dist(0, n - 1);
    int a = dist(rng_), b = dist(rng_);
    if (a > b) std::swap(a, b);

    Permutacion hijo(n, -1);
    std::vector<bool> usado(n, false);
    for (int i = a; i <= b; ++i) {
        hijo[i] = padre1[i];
        usado[padre1[i]] = true;
    }

    int posHijo = (b + 1) % n;
    for (int k = 0; k < n; ++k) {
        int trabajo = padre2[(b + 1 + k) % n];
        if (!usado[trabajo]) {
            hijo[posHijo] = trabajo;
            usado[trabajo] = true;
            posHijo = (posHijo + 1) % n;
        }
    }
    return hijo;
}

// Mutacion por insercion: se extrae un trabajo y se inserta en otra posicion
void AlgoritmoGenetico::mutacionInsercion(Permutacion& p) {
    int n = static_cast<int>(p.size());
    if (n < 2) return;
    std::uniform_int_distribution<int> dist(0, n - 1);
    int origen = dist(rng_);
    int destino = dist(rng_);
    while (destino == origen) destino = dist(rng_);

    int trabajo = p[origen];
    p.erase(p.begin() + origen);
    p.insert(p.begin() + destino, trabajo);
}

ResultadoGA AlgoritmoGenetico::ejecutar() {
    ResultadoGA res;
    auto inicio = std::chrono::high_resolution_clock::now();

    auto porMakespan = [](const Individuo& x, const Individuo& y) {
        return x.makespan < y.makespan;
    };

    // 1) Poblacion inicial y mejor solucion global
    std::vector<Individuo> poblacion = poblacionInicial(res.makespanNEH);
    Individuo mejor = *std::min_element(poblacion.begin(), poblacion.end(), porMakespan);
    res.mejorInicial = mejor.makespan;
    res.convergencia.push_back(mejor.makespan);  // generacion 0

    // 2) Ciclo evolutivo
    for (int g = 1; g <= params_.generaciones; ++g) {
        // Elitismo: los e mejores pasan sin cambios
        std::sort(poblacion.begin(), poblacion.end(), porMakespan);
        std::vector<Individuo> nueva(poblacion.begin(),
                                     poblacion.begin() + std::min(params_.elite,
                                                                  params_.tamPoblacion));
        std::set<Permutacion> presentes;  // para evitar duplicados cuando sea posible
        for (const Individuo& e : nueva) presentes.insert(e.cromosoma);

        while (static_cast<int>(nueva.size()) < params_.tamPoblacion) {
            // Seleccion de padres por torneo
            const Individuo& padre1 = torneo(poblacion);
            const Individuo& padre2 = torneo(poblacion);

            // Cruce OX con probabilidad pc; en otro caso se copian los padres
            Permutacion hijos[2];
            if (aleatorio() < params_.probCruce) {
                hijos[0] = cruceOX(padre1.cromosoma, padre2.cromosoma);
                hijos[1] = cruceOX(padre2.cromosoma, padre1.cromosoma);
            } else {
                hijos[0] = padre1.cromosoma;
                hijos[1] = padre2.cromosoma;
            }

            for (Permutacion& hijo : hijos) {
                if (static_cast<int>(nueva.size()) >= params_.tamPoblacion) break;

                // Mutacion por insercion con probabilidad pm
                if (aleatorio() < params_.probMutacion) {
                    mutacionInsercion(hijo);
                }
                // Si el hijo ya existe, se intenta diversificarlo con mas inserciones
                for (int intento = 0; intento < 3 && presentes.count(hijo); ++intento) {
                    mutacionInsercion(hijo);
                }
                presentes.insert(hijo);
                nueva.push_back(crearIndividuo(hijo));
            }
        }

        // Reemplazo de la poblacion y actualizacion de la mejor solucion global
        poblacion = std::move(nueva);
        const Individuo& mejorGen =
            *std::min_element(poblacion.begin(), poblacion.end(), porMakespan);
        if (mejorGen.makespan < mejor.makespan) {
            mejor = mejorGen;
            res.generacionHallazgo = g;
        }
        res.convergencia.push_back(mejor.makespan);
    }

    auto fin = std::chrono::high_resolution_clock::now();
    res.tiempoMs = std::chrono::duration<double, std::milli>(fin - inicio).count();
    res.mejorSecuencia = mejor.cromosoma;
    res.mejorMakespan = mejor.makespan;
    return res;
}
