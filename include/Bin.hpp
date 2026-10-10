#pragma once

#include <vector>

struct Bin {
    int capacity; // Capacidade restante.
    std::vector<int> list;

    bool isFull() const {
        return capacity <= 0;
    }
};
