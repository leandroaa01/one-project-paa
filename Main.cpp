#include <iostream>
#include <string>

#include "include/BinPack.hpp"
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
        std::cin >> fileOption;

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
        std::cout << "1. BinPack (Original)\n";
        std::cout << "2. Outra solução (Ainda não implementada kkk)\n";
        std::cout << "0. Voltar\n";
        std::cout << "Opcao: ";

        int strategyOption;
        std::cin >> strategyOption;

        switch (strategyOption) {
            case 0:
                break;
            case 1:
                Runner::runBenchmark<BinPack>(instances, "BinPack (Original)");
                break;
            case 2:
                std::cout << "Ainda não implementada, man\n";
                break;
            default:
                std::cout << "Opcao de solucao invalida!\n";
                break;
        }
    }

    return 0;
}