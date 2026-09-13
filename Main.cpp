#include <iostream>
#include <vector>

#include "BinPack.hpp"

int main()
{
    BinPack binPack(10);
    std::vector<int> items = {3, 4, 2, 7, 1, 5, 6, 8, 2, 4};

    std::cout << "Teste do BinPack\n";
    std::cout << "Capacidade do bin: " << binPack.getBinCapacity() << "\n\n";

    for (int item : items) {
        binPack.addItem(item);
        std::cout << "Adicionado item: " << item << "\n";
    }

    const auto& bins = binPack.getBins();
    std::cout << "\nQuantidade de bins utilizados: " << binPack.size() << "\n";

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

    return 0;
}
