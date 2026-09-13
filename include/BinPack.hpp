/**
 * @file BinPack.hpp
 * @author  Leandro Andrade(leandro.andrade.401@ufrn.edu.br)
 * @author  Daniel Coelho(daniel.coelho.708@ufrn.edu.br)
 * @brief  TODO
 * @details  TODO
 * @version 0.1
 * @date 2026-09-01
 * @copyright Copyright (c) 2026
 *
 */
#pragma once
#include <vector>

struct Bin {
    int capacity;
    std::vector<int> list;
    bool isFull() const {
        return capacity <= 0;
    }
};

class BinPack {
    private:
    std::vector<Bin> bins;
    int binCapacity;
public:
    BinPack(int binCapacity) : binCapacity(binCapacity) {
        bins.push_back(Bin{binCapacity, {}});
    }

    void addItem(int itemSize) {
        for (auto& bin : bins) {
            //se a capacidade do bin for maior ou igual ao tamanho do item, adiciona o item ao bin
            if (bin.capacity >= itemSize) {
                bin.list.push_back(itemSize);
                bin.capacity -= itemSize;
                return;
            }
        }
        // se o bin não tiver capacidade suficiente, cria um novo bin e adiciona o item a ele
        bins.push_back(Bin{binCapacity - itemSize, {itemSize}});
    }

    const std::vector<Bin>& getBins() const {
        return bins;
    }

    std::size_t size() const {
        return bins.size();
    }

    int getBinCapacity() const {
        return binCapacity;
    }
};

