/**
 * @file LasVegas.hpp
 * @author Leandro Andrade (leandro.andrade.401@ufrn.edu.br)
 * @author Daniel Coelho (daniel.coelho.708@ufrn.edu.br)
 * @brief Implementação da busca Las Vegas para Bin Packing.
 * @details
 * A implementação parte da busca Branch and Bound e aleatoriza a ordem de
 * exploração dos bins candidatos. A aleatoriedade altera o tempo de busca,
 * mas não elimina candidatos: quando a busca termina, a solução ótima é
 * comprovada. Se o limite de tempo for atingido, retorna a melhor solução
 * viável encontrada e indica que a otimalidade não foi comprovada.
 * @version 0.1
 * @date 2026-11-11
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <functional>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Bin.hpp"
#include "BestFit.hpp"

class LasVegas {
    // Estrutura auxiliar para armazenar informações sobre candidatos a bins durante a busca.
    struct Candidate {
        std::size_t binIndex; // < Índice do bin candidato.
        int remainingAfter; // < Capacidade restante do bin após adicionar o item.
        int previousCapacity; // < Capacidade do bin antes de adicionar o item, usado para evitar duplicatas.
    };

private:
    int binCapacity; //<! Capacidade do bin.
    std::vector<int> items; //<! Itens a serem empacotados, ordenados em ordem decrescente.
    std::vector<long long> suffix; //<! suffix[i] = soma dos itens de items[i] até o final, usado para calcular limites inferiores.
    std::vector<Bin> currentBins; //<! Bins atualmente abertos durante a busca.
    std::vector<Bin> bestBins; //<! Melhor solução encontrada até agora.
    int bestCount = 0; //<! Número de bins na melhor solução encontrada até agora.
    int initialUpperBound = 0; //<! Limite superior inicial obtido a partir de uma heurística (Best-Fit Decreasing com Remanejamento).
    long long totalWeight = 0; //<! Soma total dos pesos dos itens, usado para calcular limites inferiores.
    long long globalLowerBound = 0; //<! Limite inferior global calculado a partir do peso total e da capacidade do bin.
    std::uint64_t nodesVisited = 0; //<! Contador de nós visitados durante a busca.
    std::uint64_t nodesPruned = 0; //<! Contador de nós podados durante a busca.
    std::uint64_t seed; //<! Semente que controla a ordem aleatória da busca.
    std::mt19937_64 generator; //<! Gerador usado para embaralhar as tentativas.
    bool optimal = false; //<! Indica se a solução encontrada é ótima.
    std::function<void(std::uint64_t, std::uint64_t, int)> progressCallback; //<! Callback para relatar progresso durante a busca.
    std::function<void(const std::string&)> traceCallback; //<! Callback para relatar eventos de trace durante a busca.
    std::size_t traceLimit = 0; //<! Limite de eventos de trace a serem emitidos; eventos adicionais são omitidos.
    std::size_t traceEvents = 0; //<! Contador de eventos de trace emitidos.
    std::uint64_t traceOmitted = 0; //<! Contador de eventos de trace omitidos devido ao limite.
    std::vector<std::vector<int>> lowerBoundCapacityBuffers; //<! Buffers para armazenar capacidades disponíveis durante o cálculo de limites inferiores.
    std::vector<std::vector<Candidate>> candidateBuffers; //<! Buffers para armazenar candidatos a bins durante a busca.
    std::chrono::milliseconds timeLimit = std::chrono::milliseconds::max(); //<! Limite de tempo para a busca
    std::chrono::steady_clock::time_point searchStart; //<! Marca o início da busca para verificar o limite de tempo.
    bool interrupted = false; //<! Indica se a busca foi interrompida devido ao limite de tempo.

    // Função auxiliar para verificar se o limite de tempo foi atingido.
    bool timeLimitReached() const {
        if (timeLimit == std::chrono::milliseconds::max()) return false;
        return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - searchStart) >= timeLimit;
    }

    // Função auxiliar para verificar se o trace deve ser emitido.
    bool shouldTrace() {
        if (!traceCallback) return false;
        if (traceEvents >= traceLimit) {
            ++traceOmitted;
            return false;
        }
        return true;
    }

    // Função auxiliar para emitir eventos de trace.
    void emitTrace(const std::string& message) {
        ++traceEvents;
        traceCallback(message);
    }

    // Função auxiliar para calcular o teto da divisão inteira.
    static long long ceilDiv(long long value, long long divisor) {return value == 0 ? 0 : 1 + (value - 1) / divisor; }

    long long calculateLowerBound(std::size_t index) {
        long long freeCapacity = 0;
        for (const Bin& bin : currentBins) {freeCapacity += bin.capacity; }
        // O peso restante dos itens a serem empacotados é dado por suffix[index].
        const long long remainingWeight = suffix[index]; 
        // O peso que não pode ser acomodado nos bins abertos precisa de novos bins.
        const long long unusedWeight = std::max(0LL, remainingWeight - freeCapacity);
        // O limite inferior inicial é dado pelo número de bins abertos mais o número mínimo de bins necessários para acomodar o peso restante.
        long long lowerBound = static_cast<long long>(currentBins.size()) +ceilDiv(unusedWeight, binCapacity);

        // Itens maiores que metade da capacidade precisam de bins distintos.
        // Fazemos o pareamento máximo possível com os bins já abertos; os
        // itens sem bin compatível obrigam a abrir novos bins.
        const int largeItemThreshold = binCapacity / 2;
        // Encontrar o primeiro item que não é grande (menor ou igual a largeItemThreshold).
        auto firstNonLarge =std::lower_bound(items.begin() + static_cast<std::ptrdiff_t>(index), items.end(),largeItemThreshold, std::greater<int>());

        // Reaproveitar o buffer do nível atual para evitar alocações em cada nó.
        auto& availableCapacities = lowerBoundCapacityBuffers[index];
        availableCapacities.clear();
        for (const Bin& bin : currentBins) availableCapacities.push_back(bin.capacity);
        // Ordenar as capacidades disponíveis para facilitar o pareamento.
        std::sort(availableCapacities.begin(), availableCapacities.end());

        auto item = items.begin() + static_cast<std::ptrdiff_t>(index);
        std::size_t capacityIndex = availableCapacities.size();
        long long largeItemsNeedingBins = 0;
        while (item != firstNonLarge) {
            // Encontrar o maior bin disponível que pode acomodar o item grande atual.
            if (capacityIndex > 0 && availableCapacities[capacityIndex - 1] >= *item) {
                ++item;
                --capacityIndex;
            } else {
                ++item;
                ++largeItemsNeedingBins;
            }
        }
        // Atualizar o limite inferior considerando os itens grandes que precisam de novos bins.
        lowerBound = std::max(lowerBound, static_cast<long long>(currentBins.size()) + largeItemsNeedingBins);

        // Para cada caixa, cada bin comporta no máximo capacity/(threshold+1)
        // itens maiores que threshold. Isso fornece limites inferiores
        // adicionais sem assumir como os itens serão combinados.
        for (int divisor = 2; divisor <= 8; ++divisor) {
            const int threshold = binCapacity / divisor;
            if (threshold < 1) break;
            const int slotsPerNewBin = binCapacity / (threshold + 1);
            
            const auto firstSmallItem = std::lower_bound(   items.begin() + static_cast<std::ptrdiff_t>(index), items.end(),threshold, std::greater<int>());
          
            const long long remainingLargeItems = std::distance(items.begin() + static_cast<std::ptrdiff_t>(index), firstSmallItem);

            long long existingSlots = 0;

            for (const Bin& bin : currentBins) {
                existingSlots += bin.capacity / (threshold + 1);
            }
            const long long slotsStillNeeded = std::max(0LL, remainingLargeItems - existingSlots);
            const long long extraBins = ceilDiv(slotsStillNeeded, slotsPerNewBin);
            lowerBound = std::max(lowerBound,  static_cast<long long>(currentBins.size()) + extraBins);
        }

        return std::max(lowerBound, globalLowerBound);
    }

    // Função para melhorar a solução inicial usando uma heurística de Best-Fit com remanejamento.
    void improveInitialSolution() {
        // Tenta 500 permutações diferentes dos itens para encontrar uma solução melhor.
        for (int trial = 0; trial < 500 && bestCount > globalLowerBound; ++trial) {
            if (timeLimitReached()) {
                interrupted = true;
                return;
            }
            std::vector<int> candidateItems = items;
            const std::size_t window = static_cast<std::size_t>(2 + trial % 20); 
            for (std::size_t first = 0; first < candidateItems.size(); first += window) {
                const std::size_t last = std::min(candidateItems.size(), first + window);
                std::shuffle(candidateItems.begin() + static_cast<std::ptrdiff_t>(first), candidateItems.begin() + static_cast<std::ptrdiff_t>(last), generator);
            }
            // Avalia a solução candidata usando a heurística Best-Fit.
            BestFit candidate(binCapacity);
            // Adiciona os itens permutados ao candidato.
            for (int item : candidateItems) candidate.addItem(item);

            if (candidate.size() < static_cast<std::size_t>(bestCount)) {
                const int previousBest = bestCount;
                bestCount = static_cast<int>(candidate.size());
                bestBins = candidate.getBins();
                if (shouldTrace()) {
                    emitTrace("Tentativa de ordem " + std::to_string(trial + 1) +
                              " melhorou a solucao de " + std::to_string(previousBest) +
                              " para " + std::to_string(bestCount) + " bins.");
                }
            }
        }
    }

    // Função recursiva de busca para explorar todas as combinações possíveis de empacotamento.
    void search(std::size_t index) {
        if (interrupted) return;
        ++nodesVisited; 
        if (nodesVisited % 1024 == 0 && timeLimitReached()) {
            interrupted = true;
            return;
        }
        if (shouldTrace()) {
            const std::string nextStep = index == items.size()
                                             ? "todos os itens atribuidos"
                                             : "proximo item " + std::to_string(index + 1) + "/" +
                                                   std::to_string(items.size());
            emitTrace("Visita no " + std::to_string(nodesVisited) + ": " + nextStep +
                      ", bins abertos = " + std::to_string(currentBins.size()) +
                      ", melhor solucao = " + std::to_string(bestCount) + ".");
        }
        // Relata progresso a cada 100.000 nós visitados.
        if (progressCallback && nodesVisited % 100000 == 0) {
            progressCallback(nodesVisited, nodesPruned, bestCount);
        }
        // Se todos os itens foram processados, verifica se a solução atual é melhor que a melhor encontrada.
        if (index == items.size()) {
            if (currentBins.size() < static_cast<std::size_t>(bestCount)) {
                const int previousBest = bestCount;
                bestCount = static_cast<int>(currentBins.size());
                bestBins = currentBins;
                if (shouldTrace()) {
                    emitTrace("Solucao completa melhorou de " + std::to_string(previousBest) +
                              " para " + std::to_string(bestCount) + " bins.");
                }
            }
            return;
        }
        // Poda: se o número de bins atuais já é maior ou igual à melhor solução encontrada, não há necessidade de continuar.
        if (currentBins.size() >= static_cast<std::size_t>(bestCount)) {
            ++nodesPruned;
            if (shouldTrace()) emitTrace("Poda: a ramificacao ja usa tantos bins quanto a melhor solucao.");
            return;
        }
        // Poda: se o limite inferior calculado para os itens restantes é maior ou igual à melhor solução encontrada, não há necessidade de continuar.
        const long long lowerBound = calculateLowerBound(index);
        if (lowerBound >= bestCount) {
            ++nodesPruned;
            if (shouldTrace()) {
                emitTrace("Poda pelo limite inferior: LB = " + std::to_string(lowerBound) +
                          " e melhor solucao = " + std::to_string(bestCount) + ".");
            }
            return;
        }

        const int item = items[index];
        auto& candidates = candidateBuffers[index];
        candidates.clear();

        for (std::size_t i = 0; i < currentBins.size(); ++i) {
            const int remaining = currentBins[i].capacity;
            if (remaining >= item) {
                candidates.push_back({i, remaining - item, remaining});
            }
        }
        // Ordena os candidatos com base na capacidade restante após adicionar o item, priorizando bins que deixam menos espaço livre.
        std::sort(candidates.begin(), candidates.end(), [](const Candidate& left, const Candidate& right) {
            return left.remainingAfter < right.remainingAfter;
        });
        const auto uniqueEnd = std::unique(candidates.begin(), candidates.end(),
                                           [](const Candidate& left, const Candidate& right) {
                                               return left.previousCapacity == right.previousCapacity;
                                           });
        candidates.erase(uniqueEnd, candidates.end());

        // A ordem aleatória afeta o tempo de busca, mas todas as alternativas
        // continuam sendo exploradas para preservar a prova de otimalidade.
        std::shuffle(candidates.begin(), candidates.end(), generator);

        // Tenta colocar o item em cada bin candidato.
        for (const Candidate& candidate : candidates) {
            Bin& bin = currentBins[candidate.binIndex];
            const int previousCapacity = bin.capacity;
            if (shouldTrace()) {
                emitTrace("Tenta item " + std::to_string(item) + " no bin " +
                          std::to_string(candidate.binIndex + 1) + "; sobra " +
                          std::to_string(candidate.remainingAfter) + ".");
            }
            bin.capacity -= item;
            bin.list.push_back(item);
            search(index + 1); // Chamada recursiva para o próximo item.
            bin.list.pop_back();
            bin.capacity = previousCapacity;
            if (interrupted) return;
            if (shouldTrace()) {
                emitTrace("Backtracking: remove item " + std::to_string(item) +
                          " do bin " + std::to_string(candidate.binIndex + 1) + ".");
            }
        }
        // Tenta colocar o item em um novo bin, se isso não exceder a melhor solução encontrada.
        if (currentBins.size() + 1 < static_cast<std::size_t>(bestCount)) {
            if (shouldTrace()) emitTrace("Abre novo bin para o item " + std::to_string(item) + ".");
            currentBins.push_back(Bin{binCapacity - item, {item}});
            search(index + 1);
            currentBins.pop_back();
            if (interrupted) return;
        }
    }

public:
/**
     * @brief Construtor da classe LasVegas.
     *
     * @param binCapacity Capacidade máxima de cada bin.
     *
     * Lança uma exceção std::invalid_argument se a capacidade do bin for menor ou igual a zero.
     */
    explicit LasVegas(int binCapacity)
        : binCapacity(binCapacity), seed(std::random_device{}()), generator(seed) {
        if (binCapacity <= 0) {
            throw std::invalid_argument("A capacidade do bin deve ser positiva");
        }
    }

    /**
     * @brief Empacota os itens usando a busca Las Vegas.
     *
     * @param inputItems Itens que serão empacotados.
     *
     * Lança uma exceção std::invalid_argument se algum item for menor ou igual a zero ou maior que a capacidade do bin.
     */
    void pack(const std::vector<int>& inputItems) {
        items = inputItems;
        currentBins.clear();
        bestBins.clear();
        suffix.clear();
        totalWeight = 0;
        nodesVisited = 0;
        nodesPruned = 0;
        optimal = false;
        interrupted = false;
        traceEvents = 0;
        traceOmitted = 0;
        searchStart = std::chrono::steady_clock::now();
        generator.seed(seed);

        for (int item : items) {
            if (item <= 0 || item > binCapacity) {
                throw std::invalid_argument("Item deve estar entre 1 e a capacidade do bin");
            }
            totalWeight += item;
        }
        // Ordena os itens em ordem decrescente para melhorar a eficiência da busca.
        std::sort(items.begin(), items.end(), std::greater<int>());

        BestFit heuristic(binCapacity); // Best-Fit Decreasing com Remanejamento para obter uma solução inicial.
        heuristic.packDecreasingRepack(items); 
        bestBins = heuristic.getBins(); // Inicializa a melhor solução com a heurística.
        bestCount = static_cast<int>(heuristic.size());// Inicializa o número de bins na melhor solução com a heurística.
        initialUpperBound = bestCount; // Armazena o limite superior inicial obtido a partir da heurística.
        globalLowerBound = ceilDiv(totalWeight, binCapacity); // Calcula o limite inferior global com base no peso total e na capacidade do bin.
        if (shouldTrace()) {
            emitTrace("Upper Bound inicial do Best Fit + Repack: " +
                      std::to_string(initialUpperBound) + " bins.");
        }
        improveInitialSolution(); // Tenta melhorar a solução inicial usando permutações aleatórias dos itens e a heurística Best-Fit.
        // Prepara o vetor suffix para calcular limites inferiores durante a busca.
        suffix.assign(items.size() + 1, 0);
        lowerBoundCapacityBuffers.resize(items.size() + 1);
        candidateBuffers.resize(items.size() + 1);
        for (std::size_t i = items.size(); i > 0; --i) {
            suffix[i - 1] = suffix[i] + items[i - 1]; // Calcula a soma dos itens restantes a partir do índice i-1.
        }

        search(0); // Inicia a busca recursiva a partir do primeiro item.
        optimal = !interrupted;
        if (shouldTrace()) {
            emitTrace(interrupted
                          ? "Tempo limite atingido; melhor solucao viavel tem " +
                                std::to_string(bestCount) + " bins; otimo nao comprovado."
                          : "Busca concluida: " + std::to_string(bestCount) +
                                " bins; otimo comprovado.");
        }
        if (traceCallback && traceOmitted > 0) {
            traceCallback("\n" + std::to_string(traceOmitted) +
                          " eventos adicionais omitidos; limite de passos alcancado.");
        }
    }

    const std::vector<Bin>& getBins() const { return bestBins; } //< retorna a melhor solução encontrada.
    std::size_t size() const { return bestBins.size(); } //< retorna o número de bins na melhor solução encontrada.
    int getBinCapacity() const { return binCapacity; } //< retorna a capacidade máxima de cada bin.
    int getInitialUpperBound() const { return initialUpperBound; } //< retorna o limite superior inicial obtido a partir da heurística.
    long long getLowerBound() const { return globalLowerBound; } //< retorna o limite inferior global calculado a partir do peso total e da capacidade do bin.
    std::uint64_t getNodesVisited() const { return nodesVisited; } //< retorna o número de nós visitados durante a busca.
    std::uint64_t getNodesPruned() const { return nodesPruned; } //< retorna o número de nós podados durante a busca.
    std::uint64_t getSeed() const { return seed; } //< retorna a semente usada para aleatorizar a busca.
    bool isOptimal() const { return optimal; } //< retorna true se a solução encontrada for ótima, false caso contrário.
    void setSeed(std::uint64_t value) {
        seed = value;
        generator.seed(seed);
    }
    void setTimeLimit(std::chrono::milliseconds limit) {
        timeLimit = limit.count() > 0 ? limit : std::chrono::milliseconds(1);
    }
    void setProgressCallback(std::function<void(std::uint64_t, std::uint64_t, int)> callback) {progressCallback = std::move(callback); } //<! Define o callback para relatar progresso durante a busca.
    void setTraceCallback(std::function<void(const std::string&)> callback, std::size_t maxEvents = 150) {
        traceCallback = std::move(callback);
        traceLimit = maxEvents;
    }
};
