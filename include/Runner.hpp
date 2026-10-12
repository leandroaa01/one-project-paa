#pragma once

#include <algorithm>
#include <cmath>
#include <fstream>
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

            std::cout << "\nTeste do BinPack (" << inst.id << ")\n";
            std::cout << "Capacidade do bin: " << binPack.getBinCapacity() << "\n";
            std::cout << "Total de itens a processar: " << inst.items.size() << "\n\n";

            packInstance(binPack, inst.items, useDecreasingRepack);

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
