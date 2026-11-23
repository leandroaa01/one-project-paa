#include <algorithm>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include "DadosBestFit.hpp"
#include "DadosBranchAndBound.hpp"
#include "DadosFirstFit.hpp"
#include "DadosLasVegas.hpp"
#include "DadosSA.hpp"
#include "include/Reader.hpp"
#include "include/Runner.hpp"
#include "include/strategies/BestFit.hpp"
#include "include/strategies/BranchAndBound.hpp"
#include "include/strategies/FirstFit.hpp"
#include "include/strategies/FirstFitDecreasingRepack.hpp"
#include "include/strategies/LasVegas.hpp"
#include "include/strategies/SimulatedAnnealing.hpp"

using raw_str = const char*; //> alias para string constante
inline raw_str MENU = R"(MENU PRINCIPAL
  1 - Executar com  data/binpack1.txt
  2 - Executar com  data/binpack2.txt
  3 - Gerar tabela de um algoritmo
  0 - Sair do programa.
Escolha o arquivo de teste:  )";

inline raw_str OPC = R"(
EXECUTAR COM A SOLUCAO:
 1. First-Fit
 2. First-Fit Decreasing com Remanejamento
 3. Best-Fit
 4. Best-Fit  Decreasing com Remanejamento
 5. Branch-and-Bound
 6. Las Vegas (busca exata aleatorizada)
 7. Simulated Annealing
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
    switch (fileOption) {
      case 1:
        path = "data/binpack1.txt";
        break;
      case 2:
        path = "data/binpack2.txt";
        break;
      case 3: {
        std::cout << R"(
Qual tabela deseja gerar?
 1 - Best-Fit e Best-Fit Decreasing com Remanejamento
 2 - First-Fit e First-Fit Decreasing com Remanejamento
 3 - Branch-and-Bound
 4 - Las Vegas (busca exata aleatorizada)
 5 - Simulated Annealing
  6 - Calibrar temperatura do Simulated Annealing
 7 - Simulated Annealing com T0 personalizado
 0 - Voltar
Escolha: )";
        int tableOption;
        if (!(std::cin >> tableOption)) return 0;
        if (tableOption == 0) continue;
        if (tableOption < 1 || tableOption > 7) {
          std::cout << "Opcao de tabela invalida!\n";
          continue;
        }

        std::uint32_t seedBase = simulated_annealing::defaultSeedBase;
        if (tableOption >= 5) {
          std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
          std::cout << "Semente base [Enter=" << seedBase
                    << ", a=aleatoria, ou digite outra]: ";
          std::string seedInput;
          std::getline(std::cin, seedInput);
          if (seedInput == "a" || seedInput == "A" || seedInput == "aleatoria" ||
              seedInput == "aleatória") {
            seedBase = simulated_annealing::randomSeedBase();
          } else if (!seedInput.empty()) {
            try {
              std::size_t consumed = 0;
              const auto parsed = std::stoull(seedInput, &consumed);
              if (consumed != seedInput.size() ||
                  parsed > std::numeric_limits<std::uint32_t>::max()) {
                throw std::out_of_range("seed fora do intervalo");
              }
              seedBase = static_cast<std::uint32_t>(parsed);
            } catch (const std::exception&) {
              std::cerr << "Semente invalida; informe um inteiro entre 0 e "
                        << std::numeric_limits<std::uint32_t>::max() << ".\n";
              return 1;
            }
          }
          std::cout << "Semente base usada: " << seedBase << '\n';
        }
        if (tableOption == 1) {
          std::cout << "Criando tabela de Best-Fit...\n";
          best::runBest();
        } else if (tableOption == 2) {
          std::cout << "Criando tabela de First-Fit...\n";
          first::runFirst();
        } else if (tableOption == 3) {
          std::cout << "Criando tabela de Branch-and-Bound (limite de 600 segundos por instancia)...\n";
          branch_and_bound::runBranchAndBound();
        } else if (tableOption == 4) {
          std::cout << "Criando tabela Las Vegas (limite de "
                    << las_vegas::tableTimeLimit.count()
                    << " segundos por instancia)...\n";
          las_vegas::runLasVegas();
        } else if (tableOption == 5) {
          std::cout << "Criando tabela Simulated Annealing...\n";
          simulated_annealing::runSA(seedBase);
        } else if (tableOption == 6) {
          std::cout << "Executando calibracao de temperatura (10 temperaturas, 3 repeticoes por instancia)...\n";
          simulated_annealing::runTemperatureCalibration(seedBase);
        } else {
          std::cout << "Temperatura inicial T0 personalizada [0.05 a 8.0]: ";
          std::string temperatureInput;
          std::getline(std::cin, temperatureInput);
          std::replace(temperatureInput.begin(), temperatureInput.end(), ',', '.');
          try {
            std::size_t consumed = 0;
            const double initialTemperature = std::stod(temperatureInput, &consumed);
            if (consumed != temperatureInput.size() ||
                !std::isfinite(initialTemperature) || initialTemperature < 0.05 ||
                initialTemperature > 8.0) {
              throw std::out_of_range("temperatura fora do intervalo");
            }
            const auto parameters =
                simulated_annealing::parametersForInitialTemperature(initialTemperature);
            std::cout << "Criando tabela Simulated Annealing com T0="
                      << initialTemperature << "...\n";
            simulated_annealing::runSA(seedBase, parameters);
          } catch (const std::exception&) {
            std::cerr << "Temperatura invalida; informe um valor entre 0.05 e 8.0.\n";
            return 1;
          }
        }
        std::cout << "Tabela salva em data/output/.\n";
        return 0;
      }
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
      case 5:
        Runner::runBenchmark<BranchAndBound>(instances, "Branch-and-Bound");
        break;
      case 6:
        Runner::runBenchmark<LasVegas>(instances, "Las Vegas");
        break;
      case 7:
        Runner::runBenchmark<SimulatedAnnealing>(instances, "Simulated Annealing");
        break;
      default:
        std::cout << "Opcao de solucao invalida!\n";
        break;
    }
  }

  return 0;
}
