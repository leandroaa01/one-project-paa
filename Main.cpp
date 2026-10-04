#include <iostream>
#include <string>

#include "include/strategies/FirstFit.hpp"
#include "include/strategies/BestFit.hpp"
#include "include/strategies/FirstFitDecreasingRepack.hpp"
#include "include/Reader.hpp"
#include "include/Runner.hpp"

int main()
{
    while (true) {
        std::cout << "\nMENU PRINCIPAL\n";
        std::cout << "1. data/binpack1.txt\n";
        std::cout << "2. data/binpack2.txt\n";
        std::cout << "0. Sair\n";
        std::cout << "Escolha o arquivo de testes: ";

        int fileOption;
        if (!(std::cin >> fileOption)) break;

        if (fileOption == 0) {
            std::cout << "Encerrando o programa...\n";
            break;
        }

        std::string path;
        bool invalidOption = false;

        switch (fileOption) {
            case 1:
                path = "data/binpack1.txt";
                break;
            case 2:
                path = "data/binpack2.txt";
                break;
            default:
                std::cout << "Opcao invalida!\n";
                invalidOption = true;
                break;
        }

        if (invalidOption) {
            continue;
        }

        auto instances = Reader::readInstances(path);
        if (instances.empty()) {
            std::cout << "Nenhuma instancia carregada!\n";
            continue;
        }

        std::cout << "\nEscolha a solucao a ser usada:\n";
        std::cout << "1. First-Fit\n";
        std::cout << "2. First-Fit Decreasing com Remanejamento\n";
        std::cout << "3. Best-Fit\n";
        std::cout << "0. Voltar\n";
        std::cout << "Opcao: ";

        int strategyOption;
        if (!(std::cin >> strategyOption)) break;

        switch (strategyOption) {
            case 0:
                break;
            case 1:
                Runner::runBenchmark<FirstFit>(instances, "First-Fit");
                break;
            case 2:
                Runner::runBenchmark<FirstFitDecreasingRepack>(instances, "First-Fit Decreasing + Repack");
                break;
            case 3:
                Runner::runBenchmark<BestFit>(instances,"Best-Fit");
                break;
            default:
                std::cout << "Opcao de solucao invalida!\n";
                break;
        }
    }

    return 0;
}