#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Reader.hpp"
#include "strategies/SimulatedAnnealing.hpp"

namespace simulated_annealing {

inline std::size_t run(const Instance& instance, unsigned int& seed) {
    SimulatedAnnealing strategy(instance.binCapacity);
    strategy.pack(instance.items);
    seed = strategy.getSeed();

    std::vector<int> packed;
    for (const auto& bin : strategy.getBins()) {
        int used = 0;
        for (int item : bin.list) {
            used += item;
            packed.push_back(item);
        }
        if (used > instance.binCapacity || bin.capacity != instance.binCapacity - used) {
            throw std::runtime_error("Capacidade invalida: " + instance.id);
        }
    }

    auto original = instance.items;
    std::sort(original.begin(), original.end());
    std::sort(packed.begin(), packed.end());
    if (original != packed) {
        throw std::runtime_error("Itens divergentes: " + instance.id);
    }

    return strategy.size();
}

inline void runSA() {
    const std::string fileStem = "data/output/resultados_SimulatedAnnealing";
    std::string fileName = fileStem + ".csv";
    for (std::size_t runNumber = 2; std::filesystem::exists(fileName); ++runNumber) {
        fileName = fileStem + "-" + std::to_string(runNumber) + ".csv";
    }

    std::ofstream output(fileName);
    if (!output) {
        throw std::runtime_error("Nao foi possivel abrir " + fileName);
    }

    output << "arquivo,instancia,itens,capacidade,minimo_de_bins,"
              "simulated_annealing,semente,excesso\n";

    for (const auto* path : {"data/binpack1.txt", "data/binpack2.txt"}) {
        const auto instances = Reader::readInstances(path);
        if (instances.empty()) {
            throw std::runtime_error("Dataset vazio ou indisponivel: " + std::string(path));
        }

        for (const auto& instance : instances) {
            if (instance.items.size() != static_cast<std::size_t>(instance.numItems)) {
                throw std::runtime_error("Quantidade de itens invalida: " + instance.id);
            }

            unsigned int seed = 0;
            const auto bins = run(instance, seed);
            output << path << ',' << instance.id << ',' << instance.numItems << ','
                   << instance.binCapacity << ',' << instance.bestKnown << ','
                   << bins << ',' << seed << ','
                   << static_cast<long long>(bins) - instance.bestKnown << '\n';
            output.flush();
        }

        std::cout << path << ": " << instances.size() << " instancias processadas\n";
    }

    if (!output) {
        throw std::runtime_error("Falha ao escrever " + fileName);
    }
    std::cout << "Tabela salva em " << fileName << '\n';
}

} // namespace simulated_annealing
