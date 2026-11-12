#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

#include "Reader.hpp"

using raw_str = const char*; //> alias para string constante
inline raw_str MENU1 = R"(  1. Executar nesta instancia
  2. Pular para a proxima instancia
  3. Calcular tamanho
  0. Voltar ao menu inicial
 )";

class Runner {
private:
    template <typename BinPackType>
    static void packInstance(BinPackType& binPack, const std::vector<int>& items,
                             bool useDecreasingRepack) {
        if constexpr (requires { binPack.packDecreasingRepack(items); }) {
            if (useDecreasingRepack) {
                binPack.packDecreasingRepack(items);
                return;
            }
        }

        if constexpr (requires { binPack.pack(items); }) {
            binPack.pack(items);
        } else {
            for (int item : items) {
                binPack.addItem(item);
            }
        }
    }

public:
    template <typename BinPackType>
    static void runBenchmark(const std::vector<Instance>& instances, const std::string& strategyName,
                             bool useDecreasingRepack = false) {
        for (std::size_t current = 0; current < instances.size(); ++current) {
            const auto& inst = instances[current];

            std::cout << "\n------------------------------------------\n";
            std::cout << " Instancia [" << current + 1 << "/" << instances.size() << "]: " << inst.id << "\n";
            std::cout << MENU1;
            std::cout << "Opcao: ";

            int actionOption;
            if (!(std::cin >> actionOption)) return;

            if (actionOption == 0) {
                std::cout << "Retornando ao menu principal...\n";
                return;
            }

            if (actionOption == 2) {
                continue;
            }

            if(actionOption == 3){
                std::cout << "Lista de itens da instancia em ordem Decrescente:\n ";
                auto list = inst.items;
                std::sort(list.begin(), list.end(), std::greater<int>());
                auto menor = list.back();
                auto maior = list.front();
                auto mediana = list[list.size()/2];
                auto media = std::round((std::accumulate(list.begin(), list.end(), 0.0) / list.size()));
                auto totalItens{0};
                for (const auto& item : list) {
                    std::cout << item << " ";   
                    totalItens+= item;
                }

                std::cout << "\nTotal de todos os item "<< totalItens << " e " << totalItens / 176 ;
               

                std::cout << "\n";
                std::cout << "Maior item:"<< maior << "\n";
                std::cout << "Menor item:"<< menor << "\n";
                std::cout << "Mediana item:"<< mediana << "\n";
                std::cout << "Media item:"<< media << "\n";
                std::cout << "Tamanho do Bin vindo com a instancia: " << inst.binCapacity << "\n";
                std::cout << "Melhor solucao conhecida (Otimo): " << inst.bestKnown << "\n";


                std:: cout << "Possiveis combinações de capacidade do bin: \n";
                auto c1 = maior + mediana + menor;
                auto c2 = 2 * maior + mediana ;
                auto c3 = 2 * maior + menor;
                auto c4 = 2 * mediana + maior;
                
                std::cout << "Maior + Mediana + Menor: " << c1 << "\n";
                std::cout << "2 * Maior + Mediana: " << c2 << "\n";
                std::cout << "2 * Maior + Menor: " << c3 << "\n";
                std::cout << "2 * Mediana + Maior: " << c4 << "\n";
                std::cout << "----------------------------------------\n";

                return;
            }

            BinPackType binPack(inst.binCapacity);

            // limite de tempo de 10 minutos para a busca exata (Branch-and-Bound)
            if constexpr (requires { binPack.setTimeLimit(std::chrono::milliseconds(60000)); }) {
                binPack.setTimeLimit(std::chrono::seconds(600));
            }

            if constexpr (requires { binPack.setTraceCallback(std::function<void(const std::string&)>{}, std::size_t{}); }) {
                std::cout << "\nPasso a passo da busca (ate 150 eventos):\n";
                binPack.setTraceCallback([](const std::string& step) {
                    std::cout << "  - " << step << '\n';
                }, 150);
            }

            if constexpr (requires { binPack.setProgressCallback(std::function<void(std::uint64_t, std::uint64_t, int)>{}); }) {
                std::cout << "\nBusca exata Branch-and-Bound em andamento:\n";
                binPack.setProgressCallback([](std::uint64_t visited, std::uint64_t pruned, int upperBound) {
                    std::cout << "\r  Nos visitados: " << visited
                              << " | podados: " << pruned
                              << " | melhor limite superior: " << upperBound << "   " << std::flush;
                });
            }

            std::cout << "\nTeste do BinPack (" << inst.id << ")\n";
            std::cout << "Capacidade do bin: " << binPack.getBinCapacity() << "\n";
            std::cout << "Total de itens a processar: " << inst.items.size() << "\n\n";

            packInstance(binPack, inst.items, useDecreasingRepack);

            if constexpr (requires { binPack.setProgressCallback(std::function<void(std::uint64_t, std::uint64_t, int)>{}); }) {
                std::cout << "\n\n";
            }

            const auto& bins = binPack.getBins();

            std::ostringstream result;

            for (std::size_t i = 0; i < bins.size(); ++i) {
                result << "Bin " << i + 1 << " -> capacidade restante: " << bins[i].capacity
                       << " | itens: ";

                if (bins[i].list.empty()) {
                    result << "nenhum";
                } else {
                    for (std::size_t j = 0; j < bins[i].list.size(); ++j) {
                        result << bins[i].list[j];
                        if (j + 1 < bins[i].list.size()) {
                            result << ", ";
                        }
                    }
                }

                result << '\n';
            }

            int binsEncontrados = static_cast<int>(binPack.size());
            int diferenca = binsEncontrados - inst.bestKnown;

            result << "\n----------------------------------------\n";
            result << "RESULTADO FINAL (" << inst.id << "):\n";
            result << "Estrategia: " << strategyName << "\n";
            result << "Caixas utilizadas: " << binsEncontrados << "\n";
            result << "Melhor solucao conhecida (Otimo): " << inst.bestKnown << "\n";
            result << "Diferenca para o otimo: " << (diferenca > 0 ? "+" : "") << diferenca << " caixas\n";
            if constexpr (requires {
                              binPack.getInitialUpperBound();
                              binPack.getLowerBound();
                              binPack.getNodesVisited();
                              binPack.getNodesPruned();
                              binPack.isOptimal();
                          }) {
                result << "Upper Bound inicial: " << binPack.getInitialUpperBound() << "\n"
                       << "Lower Bound global: " << binPack.getLowerBound() << "\n"
                       << "Nos visitados: " << binPack.getNodesVisited() << "\n"
                       << "Nos podados: " << binPack.getNodesPruned() << "\n"
                       << "Otimo comprovado: " << (binPack.isOptimal() ? "sim" : "nao") << "\n";
            }
            result << "----------------------------------------\n";

            std::cout << result.str();
            std::ofstream output("data/output/resultados_execucoes.txt", std::ios::app);
            if (!output) {
                std::cerr << "Nao foi possivel salvar os resultados em data/output/resultados_execucoes.txt\n";
            } else {
                output << "Instancia: " << inst.id << '\n'
                       << "Capacidade do bin: " << binPack.getBinCapacity() << '\n'
                       << "Total de itens: " << inst.items.size() << '\n'
                       << result.str() << '\n';
                if (!output) {
                    std::cerr << "Falha ao escrever em data/output/resultados_execucoes.txt\n";
                } else {
                    std::cout << "Resultado salvo em data/output/resultados_execucoes.txt\n";
                }
            }

            // Só para melhorar a visualização do resultado final
            std::cout << "\nPressione [Enter] para continuar...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
        }

        std::cout << "\nFim de todas as instancias do arquivo!\n";
    }
};
