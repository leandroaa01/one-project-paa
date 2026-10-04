/**
 * @file BestFit.hpp
 * @author Leandro Andrade (leandro.andrade.401@ufrn.edu.br)
 * @author Daniel Coelho (daniel.coelho.708@ufrn.edu.br)
 * @brief Implementação do algoritmo Best Fit para Bin Packing.
 * @details
 * O algoritmo Best Fit tenta colocar cada item no bin que ficará
 * com a menor quantidade de espaço livre após a inserção.
 * Caso nenhum bin tenha espaço suficiente, um novo bin é criado.
 *
 * @version 0.1
 * @date 2026-09-01
 * @copyright Copyright (c) 2026
 *
 */

#pragma once

#include <vector>
#include "Bin.hpp"

class BestFit {
private:
    // Lista de bins utilizados pelo algoritmo.
    std::vector<Bin> bins;

    // Capacidade máxima de cada bin.
    int binCapacity;

public:
    /**
     * @brief Construtor da classe BestFit.
     *
     * @param binCapacity Capacidade máxima de cada bin.
     */
    BestFit(int binCapacity) : binCapacity(binCapacity) {}

    /**
     * @brief Adiciona um item utilizando a estratégia Best Fit.
     *
     * Primeiro procura o bin que, após receber o item,
     * ficará com a menor capacidade restante.
     *
     * Caso nenhum bin tenha espaço suficiente, um novo bin
     * é criado para armazenar o item.
     *
     * @param itemSize Tamanho do item que será adicionado.
     */
    void addItem(int itemSize) {

        // Procura o melhor bin para colocar o item.
        int melhor = bestBin(itemSize);

        // Se encontrou um bin onde o item cabe...
        if (melhor != -1) {

            // Adiciona o item à lista de itens do bin.
            bins[melhor].list.push_back(itemSize);

            // Atualiza a capacidade restante do bin.
            bins[melhor].capacity -= itemSize;

        } else {

            // Se nenhum bin possui espaço suficiente,
            // cria um novo bin com o item dentro dele.
            bins.push_back(
                Bin{
                    binCapacity - itemSize,
                    {itemSize}
                }
            );
        }
        realocar();
    }

    /**
     * @brief Retorna todos os bins utilizados.
     *
     * @return Referência constante para o vetor de bins.
     *
     * O uso de const impede que o código que recebe o retorno
     * altere diretamente os bins.
     */
    const std::vector<Bin>& getBins() const {
        return bins;
    }

    /**
     * @brief Retorna a quantidade de bins utilizados.
     *
     * @return Número de bins.
     */
    std::size_t size() const {
        return bins.size();
    }

    /**
     * @brief Retorna a capacidade máxima dos bins.
     *
     * @return Capacidade de cada bin.
     */
    int getBinCapacity() const {
        return binCapacity;
    }

    /**
     * @brief Encontra o melhor bin para receber um item.
     *
     * A estratégia Best Fit procura o bin que deixa
     * a menor quantidade de espaço sobrando depois
     * da inserção do item.
     *
     * Por exemplo:
     *
     * Capacidade do bin = 10
     * Item = 6
     *
     * Se o bin possui 8 de espaço:
     * 8 - 6 = 2 de espaço restante.
     *
     * Se outro bin possui 10 de espaço:
     * 10 - 6 = 4 de espaço restante.
     *
     * O primeiro bin é escolhido, pois deixa apenas 2
     * unidades de espaço sobrando.
     *
     * @param itemSize Tamanho do item que será inserido.
     * @return Índice do melhor bin ou -1 caso nenhum bin comporte o item.
     */
    int bestBin(int itemSize) const {

        // Começamos com um valor maior que qualquer
        // capacidade possível para encontrar o menor restante.
        int menor = binCapacity + 1;

        // -1 significa que nenhum bin adequado foi encontrado ainda.
        int idxBin = -1;

        // Percorre todos os bins existentes.
        for (std::size_t i = 0; i < bins.size(); i++) {

            // Calcula quanto espaço sobraria no bin
            // depois de colocar o item.
            int restante = bins[i].capacity - itemSize;

            // O item só pode ser colocado se couber no bin.
            //
            // restante >= 0:
            // significa que o bin possui espaço suficiente.
            //
            // restante < menor:
            // significa que encontramos um bin que deixa
            // menos espaço sobrando que o melhor encontrado antes.
            if (restante >= 0 && restante < menor) {

                // Atualiza o menor espaço restante encontrado.
                menor = restante;

                // Guarda o índice do bin escolhido.
                idxBin = i;
            }
        }

        // Retorna o índice do melhor bin.
        //
        // Se nenhum bin comportar o item, retorna -1.
        return idxBin;
    }

    bool realocar() {
    for (int i = static_cast<int>(bins.size()) - 1; i >= 0; i--) {

        // Faz uma cópia dos bins atuais.
        std::vector<Bin> tentativa = bins;

        // Guarda os pacotes do bin que queremos remover.
        std::vector<int> pacotes = tentativa[i].list;

        // Remove temporariamente o bin da tentativa.
        tentativa.erase(tentativa.begin() + i);

        bool conseguiu = true;

        // Tenta realocar cada pacote.
        for (int pacote : pacotes) {

            int melhor = -1;
            int menor = binCapacity + 1;

            for (std::size_t j = 0; j < tentativa.size(); j++) {

                int restante = tentativa[j].capacity - pacote;

                if (restante >= 0 && restante < menor) {
                    menor = restante;
                    melhor = static_cast<int>(j);
                }
            }

            // O pacote não coube em nenhum bin.
            if (melhor == -1) {
                conseguiu = false;
                break;
            }

            // Realoca o pacote.
            tentativa[melhor].list.push_back(pacote);
            tentativa[melhor].capacity -= pacote;
        }

        // Se todos os pacotes couberam...
        if (conseguiu) {

            // Aceita a nova solução.
            bins = tentativa;

            return true;
        }
    }

    return false;
}

};
