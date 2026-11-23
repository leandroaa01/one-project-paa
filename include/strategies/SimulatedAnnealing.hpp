/**
 * @file SimulatedAnnealing.hpp
 * @author Leandro Andrade (leandro.andrade.401@ufrn.edu.br)
 * @author Daniel Coelho (daniel.coelho.708@ufrn.edu.br)
 * @brief Implementação do algoritmo de Simulated Annealing para o problema de bin packing.
 *
 * Este arquivo contém a definição da classe SimulatedAnnealing, que implementa o algoritmo de
 * Simulated Annealing para resolver o problema de bin packing. O algoritmo busca uma solução
 * aproximada para o problema, tentando minimizar o número de bins utilizados.
 *
 * @date 2026-11-21
 */

#pragma once

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

#include "BestFit.hpp"

struct SimulatedAnnealingParameters {
    double initialTemperature = 0.3;
    double minimumTemperature = 0.003;
    std::size_t temperatureLevels = 30;
    std::size_t minimumIterationsPerTemperature = 50;
};

/**
 * @class SimulatedAnnealing
 * @brief Implementa o algoritmo de Simulated Annealing para o problema de bin packing.
 *
 * A classe SimulatedAnnealing utiliza o algoritmo de Simulated Annealing para encontrar uma
 * solução aproximada para o problema de bin packing. O algoritmo busca minimizar o número de
 * bins utilizados, explorando diferentes permutações dos itens e aceitando soluções piores com
 * base em uma função de probabilidade que depende da temperatura atual.
 */
class SimulatedAnnealing {
private:
    std::vector<Bin> bins; // <! Vetor de bins utilizados na solução atual.
    int binCapacity; // <! Capacidade máxima de cada bin.
    unsigned int seed; // <! Semente para o gerador de números aleatórios.
    std::mt19937 generator; // <! Gerador de números aleatórios.
    SimulatedAnnealingParameters parameters;

    /**
     * @brief Decodifica uma permutação de índices em uma solução de bins.
     *
     * @param permutation Vetor de índices representando a permutação dos itens.
     * @param items Vetor contendo os tamanhos dos itens.
     * @return Vetor de bins resultante da decodificação da permutação.
     *
     * A função utiliza o algoritmo Best Fit para empacotar os itens na ordem especificada
     * pela permutação. Cada índice na permutação corresponde a um item no vetor items.
     */
    std::vector<Bin> decode(const std::vector<std::size_t>& permutation,const std::vector<int>& items) const {
        BestFit decoder(binCapacity);
        for (std::size_t index : permutation) { decoder.addItem(items[index]); }
        return decoder.getBins();
    }

public:
    /**
     * @brief Construtor da classe SimulatedAnnealing.
     *
     * @param binCapacity Capacidade máxima de cada bin.
     *
     * O construtor inicializa a capacidade dos bins e configura o gerador de números aleatórios
     * com uma semente baseada em um dispositivo aleatório. Lança uma exceção se a capacidade
     * do bin for menor ou igual a zero.
     */
    explicit SimulatedAnnealing(
        int binCapacity,
        SimulatedAnnealingParameters parameters = {})
        : binCapacity(binCapacity), seed(std::random_device{}()), generator(seed),
          parameters(parameters) {
        if (binCapacity <= 0) {
            throw std::invalid_argument("A capacidade do bin deve ser positiva");
        }
        if (!std::isfinite(parameters.initialTemperature) ||
            !std::isfinite(parameters.minimumTemperature) ||
            parameters.initialTemperature <= parameters.minimumTemperature ||
            parameters.minimumTemperature <= 0.0 || parameters.temperatureLevels == 0 ||
            parameters.minimumIterationsPerTemperature == 0) {
            throw std::invalid_argument("Parametros de temperatura invalidos");
        }
    }

    /**
     * @brief Define a semente para o gerador de números aleatórios.
     *
     * @param value Valor da semente.
     *
     * Esta função permite definir uma semente específica para o gerador de números aleatórios,
     * garantindo que os resultados do algoritmo possam ser reproduzidos.
     */
    void setSeed(unsigned int value) {
        seed = value;
        generator.seed(seed);
    }
    
    /**
     * @brief Retorna a semente atual do gerador de números aleatórios.
     *
     * @return Valor da semente atual.
     *
     * Esta função permite acessar a semente atualmente utilizada pelo gerador de números aleatórios,
     * útil para depuração ou reprodução de resultados.
     */
    unsigned int getSeed() const {
        return seed;
    }

    /**
     * @brief Empacota os itens utilizando o algoritmo de Simulated Annealing.
     *
     * @param items Vetor contendo os tamanhos dos itens a serem empacotados.
     *
     * A função realiza o empacotamento dos itens utilizando o algoritmo de Simulated Annealing,
     * buscando minimizar o número de bins utilizados. Lança uma exceção se algum item for
     * inválido (menor ou igual a zero ou maior que a capacidade do bin).
     */
    void pack(const std::vector<int>& items) {
        bins.clear(); // Limpa os bins antes de iniciar o empacotamento.
        // Valida os itens para garantir que todos estão dentro dos limites aceitáveis.
        for (int item : items) {
            if (item <= 0 || item > binCapacity) {
                throw std::invalid_argument("Item deve estar entre 1 e a capacidade do bin");
            }
        }
        if (items.empty()) {
            return;
        }

        generator.seed(seed); // Reinicia o gerador de números aleatórios com a semente definida.
        std::vector<std::size_t> currentPermutation(items.size()); // Vetor para armazenar a permutação atual dos índices dos itens.
        std::iota(currentPermutation.begin(), currentPermutation.end(), 0); // Inicializa a permutação com índices sequenciais.
        std::shuffle(currentPermutation.begin(), currentPermutation.end(), generator); // Embaralha a permutação inicial para começar com uma configuração aleatória.

        std::vector<Bin> currentBins = decode(currentPermutation, items); // Decodifica a permutação inicial para obter os bins correspondentes.
        std::vector<Bin> bestBins = currentBins; // Inicializa os melhores bins encontrados com a configuração atual.
        std::size_t currentFitness = currentBins.size(); // Calcula a "fitness" atual, que é o número de bins utilizados.
        std::size_t bestFitness = currentFitness; // Inicializa a melhor "fitness" encontrada com a configuração atual.

        if (items.size() >= 2) {
            // Configurações para a busca de vizinhança e aceitação de soluções piores.
            std::uniform_int_distribution<std::size_t> position(0, items.size() - 1);
            // Distribuição uniforme para decidir se uma solução pior será aceita com base na temperatura atual.
            std::uniform_real_distribution<double> probability(0.0, 1.0);
            // Calcula o número de iterações por temperatura, garantindo um mínimo definido.
            const std::size_t iterationsPerTemperature =
                std::max(parameters.minimumIterationsPerTemperature, items.size());
            const double coolingRate = std::pow(
                parameters.minimumTemperature / parameters.initialTemperature,
                1.0 / static_cast<double>(parameters.temperatureLevels));
            double temperature = parameters.initialTemperature;

            for (std::size_t level = 0; level < parameters.temperatureLevels; ++level) {
                for (std::size_t iteration = 0; iteration < iterationsPerTemperature; ++iteration) {
                    std::vector<std::size_t> neighborPermutation = currentPermutation; // Cria uma cópia da permutação atual para gerar um vizinho.
                    std::size_t first = position(generator); // Seleciona aleatoriamente o primeiro índice para troca.
                    std::size_t second = position(generator); // Seleciona aleatoriamente o segundo índice para troca.
                    while (second == first) {
                        second = position(generator);
                    }
                    // Troca os dois índices selecionados para gerar uma nova permutação vizinha.
                    std::swap(neighborPermutation[first], neighborPermutation[second]); 

                    // Decodifica a permutação vizinha para obter os bins correspondentes e calcula sua "fitness".
                    std::vector<Bin> neighborBins = decode(neighborPermutation, items);
                    // Calcula a "fitness" da solução vizinha, que é o número de bins utilizados.
                    std::size_t neighborFitness = neighborBins.size();
                    // Calcula a diferença de "fitness" entre a solução vizinha e a atual.
                    const auto delta = static_cast<long long>(neighborFitness) - static_cast<long long>(currentFitness);
                    //usar a formula = e^(-delta/temperatura) para calcular a probabilidade de aceitar uma solução pior.
                    const bool accept = delta <= 0 || probability(generator) < std::exp(-static_cast<double>(delta) / temperature);

                    // Se a solução vizinha for melhor ou for aceita com base na probabilidade, atualiza a solução atual.
                    if (accept) {
                        currentPermutation = std::move(neighborPermutation);
                        currentBins = std::move(neighborBins);
                        currentFitness = neighborFitness;
                        if (currentFitness < bestFitness) {
                            bestBins = currentBins;
                            bestFitness = currentFitness;
                        }
                    }
                }
                temperature *= coolingRate;
            }
        }
        // Atualiza os bins com a melhor solução encontrada após o processo de Simulated Annealing.
        bins = std::move(bestBins);
    }

    const std::vector<Bin>& getBins() const {  return bins;
    }

    std::size_t size() const { return bins.size();}

    int getBinCapacity() const {return binCapacity;}
};
