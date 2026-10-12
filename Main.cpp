#include <iostream>
#include <string>

#include "DadosBestFit.hpp"
#include "DadosFirstFit.hpp"
#include "include/Reader.hpp"
#include "include/Runner.hpp"
#include "include/strategies/BestFit.hpp"
#include "include/strategies/FirstFit.hpp"
#include "include/strategies/FirstFitDecreasingRepack.hpp"

using raw_str = const char*; //> alias para string constante
inline raw_str MENU = R"(MENU PRINCIPAL
  1 - Executar com  data/binpack1.txt
  2 - Executar com  data/binpack2.txt
  3 - Gera tabela de dados dos Algoritmos
  0 - Sair do programa.
Escolha o arquivo de teste:  )";

inline raw_str OPC = R"(
EXECUTAR COM A SOLUCAO:
 1. First-Fit
 2. First-Fit Decreasing com Remanejamento
 3. Best-Fit
 4. Best-Fit  Decreasing com Remanejamento
 0. Mudar arquivo de teste
Escolha o Solucao: )";

int main()
{
  while (true) {
    std::cout << MENU;

    int fileOption;
    if (!(std::cin >> fileOption)) break;

    if (fileOption == 0) {
      std::cout << "Encerrando o programa...\n";
      break;
    }

    std::string path;
    bool invalidOption = false;
    std::string op{ "n" };
    switch (fileOption) {
      case 1:
        path = "data/binpack1.txt";
        break;
      case 2:
        path = "data/binpack2.txt";
        break;
      case 3:
        std::cout << "Quer usar um tamanho estatico dos dados, N/s? ";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::getline(std::cin, op);
        if (op.empty()) op = "n";
        std::cout << "criando tabela do Best Fit\n";
        best::runBest(op);
        std::cout << "criando tabela do Firt Fit\n";
        first::runFirst(op);
        std::cout << "tabelas em /data/output/\n";
        std::cout << "Encerrando o programa...\n";
        return 0;
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

    std::cout << OPC;

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
        Runner::runBenchmark<BestFit>(instances, "Best-Fit");
        break;
      case 4:
        Runner::runBenchmark<BestFit>(instances, "Best-Fit Decreasing + Repack", true);
        break;
      default:
        std::cout << "Opcao de solucao invalida!\n";
        break;
    }
  }

  return 0;
}