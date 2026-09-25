#pragma once

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <string>
#include <vector>

#include "Reader.hpp"

class Runner {
public:
    template <typename BinPackType>
    static void runBenchmark(const std::vector<Instance>& instances, const std::string& strategyName) {
        for (auto current = 0; current < instances.size(); ++current) {
            const auto& inst = instances[current];

            std::cout << "\n----------------------------------------\n";
            std::cout << "Instancia [" << current + 1 << "/" << instances.size() << "]: " << inst.id << "\n";
            std::cout << "1. Executar nesta instancia\n";
            std::cout << "2. Pular para a proxima instancia\n";
            std::cout << "0. Voltar ao menu inicial\n";
            std::cout << "3. Estaticas da instancia\n";
            std::cout << "Opcao: ";

            int actionOption;
            std::cin >> actionOption;

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
                for (const auto& item : list) {
                    std::cout << item << " ";   
                }
               

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

            for (int item : inst.items) {
                binPack.addItem(item);
            }

            const auto& bins = binPack.getBins();

            for (std::size_t i = 0; i < bins.size(); ++i) {
                std::cout << "Bin " << i + 1 << " -> capacidade restante: " << bins[i].capacity
                          << " | itens: ";

                if (bins[i].list.empty()) {
                    std::cout << "nenhum";
                } else {
                    for (std::size_t j = 0; j < bins[i].list.size(); ++j) {
                        std::cout << bins[i].list[j];
                        if (j + 1 < bins[i].list.size()) {
                            std::cout << ", ";
                        }
                    }
                }

                std::cout << '\n';
            }

            int binsEncontrados = static_cast<int>(binPack.size());
            int diferenca = binsEncontrados - inst.bestKnown;

            std::cout << "\n----------------------------------------\n";
            std::cout << "RESULTADO FINAL (" << inst.id << "):\n";
            std::cout << "Estrategia: " << strategyName << "\n";
            std::cout << "Caixas utilizadas: " << binsEncontrados << "\n";
            std::cout << "Melhor solucao conhecida (Otimo): " << inst.bestKnown << "\n";
            std::cout << "Diferenca para o otimo: " << (diferenca > 0 ? "+" : "") << diferenca << " caixas\n";
            std::cout << "----------------------------------------\n";

            // Só para melhorar a visualização do resultado final
            std::cout << "\nPressione [Enter] para continuar...";
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cin.get();
        }

        std::cout << "\nFim de todas as instancias do arquivo!\n";
    }
};