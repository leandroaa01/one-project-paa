# Algorithm for Bin Packing

Projeto desenvolvido por Daniel Coelho dos Santos (<daniel.coelho.708@ufrn.edu.br>) e Leandro Andrade de Araujo (<leandro.andrade.401@ufrn.edu.br>) para a disciplina Projeto e Analise de Algoritmos (PAA) da UFRN.

## Sobre o projeto

O projeto implementa heuristicas e metodos de busca para o problema de empacotamento em caixas (Bin Packing). A primeira unidade aborda algoritmos aproximativos, a segunda trata de buscas exatas e aleatorizadas, e a terceira implementa uma meta-heuristica.

O programa le instancias dos arquivos `data/binpack1.txt` e `data/binpack2.txt`, permite executar cada instancia individualmente e gera tabelas CSV para comparacao dos resultados.

## Estrategias implementadas

### Primeira unidade

- **First-Fit:** percorre os bins na ordem existente e coloca o item no primeiro bin com capacidade suficiente.
- **First-Fit Decreasing com Remanejamento:** organiza os itens em ordem decrescente, aplica First-Fit e tenta remover bins realocando seus itens.
- **Best-Fit:** coloca o item no bin que deixar a menor capacidade restante depois da insercao.
- **Best-Fit Decreasing com Remanejamento:** organiza os itens em ordem decrescente, aplica Best-Fit e repete a remocao de bins quando todos os itens cabem em outros bins.

### Segunda unidade

- **Branch-and-Bound:** realiza busca exata em arvore, ordena os itens em ordem decrescente e utiliza limites inferiores, limites superiores e podas para encontrar a solucao otima. A busca usa um limite de 600 segundos por instancia.
- **Las Vegas (busca exata aleatorizada):** reutiliza a estrutura de Branch-and-Bound, aleatoriza a ordem de exploracao dos candidatos e ainda comprova a otimalidade quando a busca termina sem interrupcao.

### Terceira unidade

- **Simulated Annealing:** representa uma solucao como uma permutacao dos itens, usa Best-Fit para decodifica-la em bins e troca pares de itens para explorar vizinhos. Solucoes piores podem ser aceitas conforme a temperatura; o algoritmo guarda a melhor solucao encontrada.

## Menu principal

O programa possui tres areas principais:

1. Executar o arquivo `data/binpack1.txt`.
2. Executar o arquivo `data/binpack2.txt`.
3. Gerar uma tabela CSV de desempenho de um dos metodos.

Ao executar uma instancia, o usuario pode:

- executar a instancia atual;
- pular para a instancia seguinte;
- calcular estatisticas dos itens em ordem decrescente;
- voltar ao menu principal.

## Geracao de tabelas

O menu de tabelas oferece:

1. Best-Fit e Best-Fit Decreasing com Remanejamento.
2. First-Fit e First-Fit Decreasing com Remanejamento.
3. Branch-and-Bound.
4. Las Vegas.
5. Simulated Annealing, com semente informada pelo usuario.
6. Calibracao da temperatura do Simulated Annealing.
7. Simulated Annealing com temperatura inicial personalizada.

As tabelas usam a capacidade registrada em cada instancia.

Os CSV sao escritos em `data/output/`. Os nomes evitam sobrescrever execucoes anteriores quando o mesmo tipo de tabela ja existe.

A tabela comum do Simulated Annealing grava a semente base e a semente de cada instancia, os parametros usados, o numero de bins e o tempo de execucao. Pressione Enter para usar a semente fixa `20261123`, digite `a` para sortear uma nova ou informe outro numero. A semente base escolhida aparece na tela e no CSV; informar a mesma novamente reproduz as mesmas sementes por instancia.

A calibracao testa temperaturas iniciais de 0,05; 0,1; 0,2; 0,3; 0,4; 0,5; 0,75; 1; 1,5; e 3. Para cada uma, executa tres repeticoes em todas as 40 instancias, com sementes pareadas entre temperaturas. A temperatura minima e 1% da temperatura inicial, com 30 niveis e pelo menos 50 iteracoes por nivel. Sao gravados um CSV com cada execucao e outro com as medias por temperatura. A semente base padrao tambem e `20261123`. A melhor temperatura inicial e escolhida pela menor media de bins; em empate, vence a menor temperatura inicial.

Na calibracao atual, a temperatura inicial selecionada foi 0,3. A execucao normal usa esse valor, temperatura minima 0,003, 30 niveis e pelo menos 50 iteracoes por nivel.

A opcao 7 gera uma tabela normal usando o T0 informado pelo usuario, entre 0,05 e 8,0. A temperatura minima sera 1% do T0, com os mesmos 30 niveis e pelo menos 50 iteracoes por nivel. O CSV registra o T0 escolhido e as sementes usadas.

## Estrutura do repositorio

```text
.
├── CMakeLists.txt
├── install.sh
├── Main.cpp
├── README.md
├── data/
│   ├── binpack1.txt
│   ├── binpack2.txt
│   └── output/
│       ├── resultados_BestFit.csv
│       ├── resultados_FirstFit.csv
│       ├── resultados_BranchAndBound-600s.csv
│       ├── resultados_LasVegas-1200s.csv
│       ├── calibracao_temperatura_SA.csv
│       └── resumo_calibracao_temperatura_SA.csv
├── docs/
│   └── relatorio/
├── include/
│   ├── Bin.hpp
│   ├── Reader.hpp
│   ├── Runner.hpp
│   ├── BranchAndBoundConfig.hpp
│   ├── LasVegasConfig.hpp
│   ├── DadosBestFit.hpp
│   ├── DadosFirstFit.hpp
│   ├── DadosBranchAndBound.hpp
│   ├── DadosLasVegas.hpp
│   └── strategies/
│       ├── FirstFit.hpp
│       ├── FirstFitDecreasingRepack.hpp
│       ├── BestFit.hpp
│       ├── BranchAndBound.hpp
│       ├── LasVegas.hpp
│       └── SimulatedAnnealing.hpp
└── src/
    ├── Reader.cpp
    ├── Runner.cpp
    └── strategies/
        ├── FirstFit.cpp
        └── FirstFitDecreasingRepack.cpp
```

## Compilacao e execucao

### Requisitos

- CMake 3.28 ou superior.
- Compilador C++ com suporte a C++23.

No Debian ou Ubuntu, por exemplo:

```bash
sudo apt install cmake g++
```

### Compilar

Clone o repositorio e entre no diretorio:

```bash
git clone https://github.com/leandroaa01/one-project-paa.git
cd one-project-paa
```

Compile com CMake:

```bash
cmake -S . -B build
cmake --build build
```

Ou execute o script fornecido pelo projeto:

```bash
./install.sh
```

### Executar

```bash
./bin/BinPack
```

Durante a execucao interativa, selecione o arquivo de teste e depois uma das sete estrategias disponiveis. As buscas exatas podem imprimir o progresso de nos visitados e podados.
