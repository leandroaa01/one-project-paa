#pragma once

#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// Representa os dados de um problema específico do arquivo
struct Instance {
    std::string id;
    int binCapacity;
    int numItems;
    int bestKnown;
    std::vector<int> items;
};

class Reader {
public:
    static std::vector<Instance> readInstances(const std::string& filepath, bool scaticSize = false) {
        std::vector<Instance> instances;
        std::ifstream file(filepath);

        // Valida se o arquivo existe e foi aberto corretamente
        if (!file.is_open()) {
            std::cerr << "Erro ao abrir arquivo: " << filepath << '\n';
            return instances;
        }

        // Lê o total de instâncias contidas no arquivo (primeira linha)
        int numProblems = 0;
        file >> numProblems;

        // Itera sobre cada problema contido no arquivo
        for (int p = 0; p < numProblems; ++p) {
            Instance inst;
            
            file >> inst.id;
            file >> inst.binCapacity >> inst.numItems >> inst.bestKnown;
            auto total = 0;
            inst.items.resize(inst.numItems);
            for (int i = 0; i < inst.numItems; ++i) {
                file >> inst.items[i];
                total+= inst.items[i];
            }
            // para o calculo estatico
            if(scaticSize){
            auto static_value{176};
            inst.binCapacity = static_value;
            inst.bestKnown = static_cast<int>(
    std::ceil(static_cast<double>(total) / static_value)
);
            }
            instances.push_back(inst);
        }


        return instances;
    }
};