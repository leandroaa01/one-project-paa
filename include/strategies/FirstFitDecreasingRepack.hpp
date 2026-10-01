#pragma once

#include <algorithm>
#include <functional>
#include <numeric>
#include <stdexcept>
#include <vector>

#include "Bin.hpp"

class FirstFitDecreasingRepack {
    std::vector<Bin> bins;
    int binCapacity;

    // Adiciona um item a primeira caixa que couber, ou cria uma nova caixa se não couber em nenhuma.
    // Estratégia First Fit clássica.
    void addItemFirstFit(int item) {
        for (auto& bin : bins) {
            if (bin.capacity >= item) {
                bin.list.push_back(item);
                bin.capacity -= item;
                return;
            }
        }
        bins.push_back(Bin{binCapacity - item, {item}});
    }

    // Tenta eliminar caixas, realocando os itens para outras caixas.
    void repackBins() {

        while (bins.size() > 1) {
            // Cria uma lista de índices das caixas. 
            // Permite definir a ordem das tentativas sem reorganizar o próprio vetor bins.
            std::vector<std::size_t> candidates(bins.size());
            std::iota(candidates.begin(), candidates.end(), 0);

            // Ordena colocando primeiro caixas que tem mais espaço sobrando (menor ocupação).
            std::stable_sort(candidates.begin(), candidates.end(), [this](auto a, auto b) {
                return bins[a].capacity > bins[b].capacity;
            });

            // Registra se alguma caixa foi eliminada nesse ciclo.
            bool eliminated = false;

            for (auto candidate : candidates) {
                // Copia os itens da caixa candidata e os ordena do maior para o menor.
                auto items = bins[candidate].list;
                std::sort(items.begin(), items.end(), std::greater<int>());
                
                // Cria uma solução temporária (uma cópia da solução original), mas sem a caixa candidata.
                auto trial = bins;
                trial.erase(trial.begin() + candidate);

                // Procura, entre as outras caixas existentes, a primeira onde cada item caiba.
                // Segue usando a estratégia First Fit.
                bool allFit = true;
                for (int item : items) {
                    auto destination = std::find_if(trial.begin(), trial.end(), [item](const Bin& bin) {
                        return bin.capacity >= item;
                    });

                    // Se não encontrar uma caixa adequada, abandona essa solução temporária.
                    if (destination == trial.end()) {
                        allFit = false;
                        break;
                    }

                    // Adiciona o item à caixa encontrada e reduz sua capacidade restante.
                    destination->list.push_back(item);
                    destination->capacity -= item;
                }

                // Confirma se todos os itens da caixa candidata couberam em outras caixas.
                if (allFit) {

                    // Confirma a solução temporária, que preserva todos os itens usando uma caixa a menos.
                    bins.swap(trial);
                    
                    eliminated = true;
                    // Sai do laço de candidatas para recalcular índices e ocupações no próximo ciclo.
                    break;
                }
            }

            // Para o loop se nenhuma tentativa funcionou.
            if (!eliminated) return;
        }
    }

public:
    explicit FirstFitDecreasingRepack(int capacity) : binCapacity(capacity) {
        if (capacity <= 0) throw std::invalid_argument("Capacidade deve ser positiva");
    }

    // Essa função trabalha com uma cópia dos itens, não alterando o vetor original.
    // Com isso esse método não atrapalha outras estratégias que usarão o mesmo vetor de itens.
    void pack(std::vector<int> items) {
        for (int item : items) {
            if (item <= 0 || item > binCapacity) {
                throw std::invalid_argument("Item deve estar entre 1 e a capacidade do bin");
            }
        }

        // Limpa as bins anteriores.
        bins.clear();

        // Ordena do maior pro menor (Decreasing);
        std::sort(items.begin(), items.end(), std::greater<int>());
        
        // Adiciona os itens usando a estratégia First Fit e só depois começa o remanejamento.
        for (int item : items) {
            addItemFirstFit(item);
        }
        repackBins(); 
    }

    const std::vector<Bin>& getBins() const { return bins; }
    std::size_t size() const { return bins.size(); }
    int getBinCapacity() const { return binCapacity; }
};
