#pragma once

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "BranchAndBoundConfig.hpp"
#include "Reader.hpp"
#include "strategies/BranchAndBound.hpp"

namespace branch_and_bound {

inline void runBranchAndBound(const std::string& use) {
    const bool staticCapacity = use == "s" || use == "sim";
    const std::string fileStem = staticCapacity
                                     ? "data/output/resultados_BranchAndBound_176"
                                     : "data/output/resultados_BranchAndBound";
    const std::string fileName = fileStem + "-" +
                                 std::to_string(defaultTimeLimit.count()) + "s.csv";

    std::ofstream output(fileName);
    if (!output) {
        throw std::runtime_error("Nao foi possivel abrir " + fileName);
    }

    output << "arquivo,instancia,itens,capacidade,minimo_de_bins,branch_and_bound,"
              "upper_bound_inicial,lower_bound_global,nos_visitados,nos_podados,"
              "excesso,tempo_limite_segundos,status\n";

    for (const auto* path : {"data/binpack1.txt", "data/binpack2.txt"}) {
        const auto instances = Reader::readInstances(path, staticCapacity);
        if (instances.empty()) {
            throw std::runtime_error("Dataset vazio ou indisponivel: " + std::string(path));
        }

        for (const auto& instance : instances) {
            if (instance.items.size() != static_cast<std::size_t>(instance.numItems)) {
                throw std::runtime_error("Quantidade de itens invalida: " + instance.id);
            }

            BranchAndBound solver(instance.binCapacity);
            constexpr int timeLimitSeconds = static_cast<int>(defaultTimeLimit.count());
            solver.setTimeLimit(defaultTimeLimit);
            solver.setProgressCallback([](std::uint64_t visited, std::uint64_t pruned, int upperBound) {
                std::cout << "\r  nos: " << visited << " | podados: " << pruned
                          << " | melhor UB: " << upperBound << std::flush;
            });
            solver.pack(instance.items);
            std::cout << '\n';

            std::vector<int> packed;
            for (const Bin& bin : solver.getBins()) {
                long long used = 0;
                for (int item : bin.list) {
                    used += item;
                    packed.push_back(item);
                }
                if (used > instance.binCapacity ||
                    bin.capacity != instance.binCapacity - used) {
                    throw std::runtime_error("Capacidade invalida: " + instance.id);
                }
            }

            auto original = instance.items;
            std::sort(original.begin(), original.end());
            std::sort(packed.begin(), packed.end());
            if (original != packed) {
                throw std::runtime_error("Itens divergentes: " + instance.id);
            }

            output << path << ',' << instance.id << ',' << instance.numItems << ','
                   << instance.binCapacity << ',' << instance.bestKnown << ','
                   << solver.size() << ',' << solver.getInitialUpperBound() << ','
                   << solver.getLowerBound() << ',' << solver.getNodesVisited() << ','
                   << solver.getNodesPruned() << ','
                   << static_cast<long long>(solver.size()) - instance.bestKnown << ','
                   << timeLimitSeconds << ','
                   << (solver.isOptimal() ? "otimo_comprovado" : "limite_de_tempo") << '\n';
            output.flush();

            std::cout << "Branch-and-Bound " << instance.id << ": " << solver.size()
                      << " bins; nos visitados: " << solver.getNodesVisited() << '\n';
        }

        std::cout << path << ": " << instances.size() << " instancias processadas\n";
    }

    if (!output) {
        throw std::runtime_error("Falha ao escrever " + fileName);
    }
    std::cout << "Tabela salva em " << fileName << '\n';
}

} // namespace branch_and_bound
