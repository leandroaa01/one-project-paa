#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include "Reader.hpp"
#include "strategies/BestFit.hpp"
#include "strategies/FirstFit.hpp"
#include "strategies/FirstFitDecreasingRepack.hpp"


// Execute a partir da raiz do projeto.
template<class Strategy>
std::size_t run(const Instance& instance, bool useDecreasingRepack = false) {
    Strategy strategy(instance.binCapacity);
    if constexpr (requires { strategy.pack(instance.items); }) {
        strategy.pack(instance.items);
    } else {
        for (int item : instance.items) strategy.addItem(item);
    }
    //for BestFit
    // if(useDecreasingRepack){
    //     strategy.packDecreasingRepack(instance.items);
    // }
    std::vector<int> packed;
    for (const auto& bin : strategy.getBins()) {
        int used = 0;
        for (int item : bin.list) { used += item; packed.push_back(item); }
        if (used > instance.binCapacity || bin.capacity != instance.binCapacity - used)
            throw std::runtime_error("Capacidade invalida: " + instance.id);
    }
    auto original = instance.items;
    std::sort(original.begin(), original.end());
    std::sort(packed.begin(), packed.end());
    if (original != packed) throw std::runtime_error("Itens divergentes: " + instance.id);

    return strategy.size();
}

int main() {
    std::ofstream output("data/output/resultados_FirstFit.csv");
if (!output) {
    throw std::runtime_error("Nao foi possivel abrir a saida");
}

output << "arquivo,instancia,itens,capacidade,melhor_conhecida,"
          "first_fit,ffd_repack,excesso_ff,excesso_ffd_repack\n";

for (const auto* path : {"data/binpack1.txt", "data/binpack2.txt"}) {
    auto instances = Reader::readInstances(path);

    if (instances.empty()) {
        throw std::runtime_error("Dataset vazio ou indisponivel");
    }

    const auto count = instances.size() / 2;

    for (std::size_t i = 0; i < count; ++i) {
        const auto& inst = instances[i];

        if (inst.items.size() != static_cast<std::size_t>(inst.numItems)) {
            throw std::runtime_error("Quantidade de itens invalida");
        }

        const auto ff = run<FirstFit>(inst);
        const auto ffd = run<FirstFitDecreasingRepack>(inst);
        const auto bf = run<BestFit>(inst);
        const auto bfd = run<BestFit>(inst, true);

        output << path << ','
               << inst.id << ','
               << inst.numItems << ','
               << inst.binCapacity << ','
               << inst.bestKnown << ','
               << ff << ','
               << ffd << ','
               << static_cast<int>(ff) - inst.bestKnown << ','
               << static_cast<int>(ffd) - inst.bestKnown << '\n';
    }

    std::cout << path << ": "
              << count << '/' << instances.size()
              << " instancias validadas\n";
}

if (!output) {
    throw std::runtime_error("Falha ao escrever resultados");
}
}
