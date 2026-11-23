#pragma once

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <map>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Reader.hpp"
#include "strategies/SimulatedAnnealing.hpp"

namespace simulated_annealing {

inline constexpr std::uint32_t defaultSeedBase = 20261123U;
inline const SimulatedAnnealingParameters defaultParameters{
    0.3, 0.003, 30, 50};
inline constexpr double minimumTemperatureRatio = 0.01;
inline constexpr std::size_t defaultTemperatureLevels = 30;
inline constexpr std::size_t defaultMinimumIterationsPerTemperature = 50;

inline std::uint32_t randomSeedBase() {
    return static_cast<std::uint32_t>(std::random_device{}());
}

inline double coolingRate(const SimulatedAnnealingParameters& parameters) {
    return std::pow(parameters.minimumTemperature / parameters.initialTemperature,
                    1.0 / static_cast<double>(parameters.temperatureLevels));
}

inline SimulatedAnnealingParameters parametersForInitialTemperature(double temperature) {
    return {temperature, temperature * minimumTemperatureRatio,
            defaultTemperatureLevels, defaultMinimumIterationsPerTemperature};
}

inline std::string uniqueOutputPath(const std::string& stem) {
    std::string path = stem + ".csv";
    for (std::size_t suffix = 2; std::filesystem::exists(path); ++suffix) {
        path = stem + "-" + std::to_string(suffix) + ".csv";
    }
    return path;
}

inline std::size_t iterationsPerTemperature(
    const Instance& instance, const SimulatedAnnealingParameters& parameters) {
    return std::max(parameters.minimumIterationsPerTemperature, instance.items.size());
}

inline std::size_t run(const Instance& instance, unsigned int seed,
                       const SimulatedAnnealingParameters& parameters) {
    SimulatedAnnealing strategy(instance.binCapacity, parameters);
    strategy.setSeed(seed);
    strategy.pack(instance.items);

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

inline void runSA(unsigned int baseSeed,
                  const SimulatedAnnealingParameters& parameters = defaultParameters) {
    const std::string stem = "data/output/resultados_SimulatedAnnealing";
    const std::string fileName = uniqueOutputPath(stem);

    std::ofstream output(fileName);
    if (!output) {
        throw std::runtime_error("Nao foi possivel abrir " + fileName);
    }
    output.imbue(std::locale::classic());
    output << "arquivo,instancia,itens,capacidade,referencia_bins,temperatura_inicial,"
              "temperatura_minima,niveis,resfriamento,iteracoes_por_nivel,semente_base,semente,"
              "simulated_annealing,excesso,tempo_ms\n";

    std::size_t instanceNumber = 0;
    for (const auto* path : {"data/binpack1.txt", "data/binpack2.txt"}) {
        const auto instances = Reader::readInstances(path);
        if (instances.empty()) {
            throw std::runtime_error("Dataset vazio ou indisponivel: " + std::string(path));
        }

        for (const auto& instance : instances) {
            if (instance.items.size() != static_cast<std::size_t>(instance.numItems)) {
                throw std::runtime_error("Quantidade de itens invalida: " + instance.id);
            }

            const unsigned int seed = baseSeed + static_cast<unsigned int>(instanceNumber++);
            const auto start = std::chrono::steady_clock::now();
            const auto bins = run(instance, seed, parameters);
            const double elapsedMs = std::chrono::duration<double, std::milli>(
                                         std::chrono::steady_clock::now() - start)
                                         .count();
            output << path << ',' << instance.id << ',' << instance.numItems << ','
                   << instance.binCapacity << ',' << instance.bestKnown << ','
                   << parameters.initialTemperature << ',' << parameters.minimumTemperature << ','
                   << parameters.temperatureLevels << ',' << coolingRate(parameters) << ','
                   << iterationsPerTemperature(instance, parameters) << ',' << baseSeed << ','
                   << seed << ',' << bins
                   << ',' << static_cast<long long>(bins) - instance.bestKnown << ','
                   << elapsedMs << '\n';
            output.flush();
        }

        std::cout << path << ": " << instances.size() << " instancias processadas\n";
    }

    if (!output) {
        throw std::runtime_error("Falha ao escrever " + fileName);
    }
    std::cout << "Tabela salva em " << fileName << '\n';
}

struct TemperatureSummary {
    std::size_t runs = 0;
    std::size_t bestBins = 0;
    std::size_t worstBins = 0;
    std::size_t reachedReference = 0;
    double totalBins = 0.0;
    double totalExcess = 0.0;
    double totalElapsedMs = 0.0;
};

inline void runTemperatureCalibration(unsigned int baseSeed) {
    constexpr std::array<double, 10> temperatures{
        0.05, 0.1, 0.2, 0.3, 0.4, 0.5, 0.75, 1.0, 1.5, 3.0};
    constexpr std::size_t repetitions = 3;

    std::vector<std::pair<std::string, Instance>> sample;
    for (const auto* path : {"data/binpack1.txt", "data/binpack2.txt"}) {
        const auto instances = Reader::readInstances(path);
        if (instances.empty()) {
            throw std::runtime_error("Dataset vazio ou indisponivel: " + std::string(path));
        }

        for (const auto& instance : instances) sample.emplace_back(path, instance);
    }

    const std::string detailFile = uniqueOutputPath("data/output/calibracao_temperatura_SA");
    std::ofstream detail(detailFile);
    if (!detail) {
        throw std::runtime_error("Nao foi possivel abrir " + detailFile);
    }
    detail.imbue(std::locale::classic());
    detail << "arquivo,instancia,itens,capacidade,referencia_bins,temperatura_inicial,"
              "temperatura_minima,niveis,resfriamento,iteracoes_por_nivel,repeticao,semente_base,semente,"
              "bins,excesso,tempo_ms\n";

    std::map<double, TemperatureSummary> summaries;
    std::size_t trialNumber = 0;
    const std::size_t seedsPerInstance = repetitions;
    for (std::size_t sampleIndex = 0; sampleIndex < sample.size(); ++sampleIndex) {
        const auto& [path, instance] = sample[sampleIndex];
        for (std::size_t repetition = 0; repetition < repetitions; ++repetition) {
            const unsigned int seed = baseSeed + static_cast<unsigned int>(
                sampleIndex * seedsPerInstance + repetition);
            for (double initialTemperature : temperatures) {
                const auto parameters = parametersForInitialTemperature(initialTemperature);
                const auto start = std::chrono::steady_clock::now();
                const auto bins = run(instance, seed, parameters);
                const double elapsedMs = std::chrono::duration<double, std::milli>(
                                             std::chrono::steady_clock::now() - start)
                                             .count();
                const long long excess = static_cast<long long>(bins) - instance.bestKnown;
                const auto iterations = iterationsPerTemperature(instance, parameters);
                const double alpha = coolingRate(parameters);

                detail << path << ',' << instance.id << ',' << instance.numItems << ','
                       << instance.binCapacity << ',' << instance.bestKnown << ','
                       << initialTemperature << ',' << parameters.minimumTemperature << ','
                       << parameters.temperatureLevels << ',' << alpha << ',' << iterations << ','
                       << repetition + 1 << ',' << baseSeed << ',' << seed << ',' << bins << ',' << excess << ','
                       << elapsedMs << '\n';
                detail.flush();
                if (!detail) {
                    throw std::runtime_error("Falha ao escrever " + detailFile);
                }

                auto& summary = summaries[initialTemperature];
                if (summary.runs == 0) {
                    summary.bestBins = bins;
                    summary.worstBins = bins;
                } else {
                    summary.bestBins = std::min(summary.bestBins, bins);
                    summary.worstBins = std::max(summary.worstBins, bins);
                }
                ++summary.runs;
                summary.totalBins += static_cast<double>(bins);
                summary.totalExcess += static_cast<double>(excess);
                summary.totalElapsedMs += elapsedMs;
                if (bins == static_cast<std::size_t>(instance.bestKnown)) {
                    ++summary.reachedReference;
                }
                ++trialNumber;
            }
        }
        std::cout << "Calibradas " << instance.id << " (" << trialNumber << " execucoes)\n";
    }

    if (!detail) {
        throw std::runtime_error("Falha ao escrever " + detailFile);
    }

    const std::string summaryFile = uniqueOutputPath("data/output/resumo_calibracao_temperatura_SA");
    std::ofstream summaryOutput(summaryFile);
    if (!summaryOutput) {
        throw std::runtime_error("Nao foi possivel abrir " + summaryFile);
    }
    summaryOutput.imbue(std::locale::classic());
    summaryOutput << "temperatura_inicial,temperatura_minima,niveis,resfriamento,execucoes,semente_base,"
                     "media_bins,media_excesso,melhor_bins,pior_bins,atingiu_referencia,"
                     "tempo_medio_ms\n";
    summaryOutput << std::fixed << std::setprecision(4);

    double selectedTemperature = temperatures.front();
    double selectedMeanBins = std::numeric_limits<double>::infinity();
    for (const auto& [initialTemperature, summary] : summaries) {
        const double meanBins = summary.totalBins / static_cast<double>(summary.runs);
        const double meanExcess = summary.totalExcess / static_cast<double>(summary.runs);
        const double meanTime = summary.totalElapsedMs / static_cast<double>(summary.runs);
        const auto parameters = parametersForInitialTemperature(initialTemperature);
        summaryOutput << initialTemperature << ',' << parameters.minimumTemperature << ','
                      << parameters.temperatureLevels << ',' << coolingRate(parameters) << ','
                      << summary.runs << ',' << baseSeed << ',' << meanBins << ',' << meanExcess << ','
                      << summary.bestBins << ',' << summary.worstBins << ','
                      << summary.reachedReference << ',' << meanTime << '\n';

        if (meanBins < selectedMeanBins ||
            (meanBins == selectedMeanBins && initialTemperature < selectedTemperature)) {
            selectedTemperature = initialTemperature;
            selectedMeanBins = meanBins;
        }
    }

    if (!summaryOutput) {
        throw std::runtime_error("Falha ao escrever " + summaryFile);
    }
    std::cout << "Testes executados: " << trialNumber << '\n'
              << "Testes detalhados: " << detailFile << '\n'
              << "Resumo por temperatura: " << summaryFile << '\n'
              << "Melhor temperatura inicial nesta amostra: " << selectedTemperature
              << " (media de " << selectedMeanBins << " bins)\n";
}

} // namespace simulated_annealing
