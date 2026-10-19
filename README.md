#  Algorithm For Bin Packing

## Authors
This project was developed by Daniel Coelho Dos Santos (<daniel.coelho.708@ufrn.edu.br>) and Leandro Andrade de Araujo (<leandro.andrade.401@ufrn.edu.br>) as part of the *PAA* course at UFRN.

## Repository Structure

```text
.
├── CMakeLists.txt
├── install.sh
├── Main.cpp
├── data/
│   ├── binpack1.txt
│   ├── binpack2.txt
│   └── output/
│       ├── resultados_BestFit.csv
│       ├── resultados_BestFit_176.csv
│       ├── resultados_FirstFit.csv
│       └── resultados_FirstFit_176.csv
├── docs/
│   ├── Relatorio_Final.pdf
│   ├── Trabalho_1_-_Algoritmo_Aproximativo_-_Direes.pdf
│   └── relatorio/
│       ├── relatorio_bin_packing.tex
│       ├── referencias.bib
│       ├── sbc-template.sty
│       ├── sbc.bst
│       ├── caption2.sty
│       └── table-*.png
├── include/
│   ├── strategies/
│   │   ├── BestFit.hpp
│   │   ├── FirstFit.hpp
│   │   └── FirstFitDecreasingRepack.hpp
│   ├── Bin.hpp
│   ├── DadosBestFit.hpp
│   ├── DadosFirstFit.hpp
│   ├── Reader.hpp
│   └── Runner.hpp
└── src/
    ├── strategies/
    │   ├── FirstFit.cpp
    │   └── FirstFitDecreasingRepack.cpp
    ├── Reader.cpp
    └── Runner.cpp
```

Os CSVs gerados pelos experimentos ficam em `data/output/`. A execução interativa também pode criar `data/output/resultados_execucoes.txt`.

## Compilling and Running
> [!note]
> This method requires CMake (>= 3.28.3) to be installed on your system.
> CMake is a cross-platform build system generator used to configure and compile the project. If it's not already installed, 
> you can install it using your system’s package manager (e.g., `sudo apt install cmake` on Debian/Ubuntu or `brew install cmake` on macOS).

Once CMake is set up, follow the steps:


1 Clone this repository:

```bash
git clone https://github.com/leandroaa01/one-project-paa.git
```


2. Navigate to the project directory:

```bash
cd one-project-paa
```

3. Compile and build the program using `cmake`:

```bash
cmake -S . -B build 
cmake --build build
```
or

```bash
./install.sh
```

4. Run:

```bash
./bin/BinPack 
```

> [!note]
The program is menu-driven; there are no command-line options. Choose a test file, then choose one of the four algorithms. Within the algorithm menu, execute the current instance, skip it, or return to the main menu. The detailed result is also appended to `data/output/resultados_execucoes.txt`.
