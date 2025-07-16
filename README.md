# Gulmohar: a RISC-V assembler and simulator

This repository contains an RISC-V assembler written in C. The assembler is capable of translating assembly code into machine code for the RV32I architecture and simulating its execution.

## Project Structure

```plaintext
.
├── main.c               # Entry point for the simulator
├── makefile             # Makefile for building the project
├── README.md            # Project README file
├── report.pdf           # Project report detailing design and implementation
├── simulator            # Directory containing simulator and assembler components
│   ├── assembler.c      # Assembler implementation
│   ├── assembler.h      # Header for assembler
│   ├── cache.c          # Cache simulation functionality
│   ├── cache.h          # Header for cache
│   ├── simulator.c      # Simulator core functionality
│   ├── simulator.h      # Header for simulator
│   ├── utils.c          # Utility functions
│   └── utils.h          # Header for utilities
└── tests                # Assembly test cases
    ├── arithmetic.s     # Tests for arithmetic instructions
    ├── branch.s         # Tests for branch instructions
    ├── fibonacci.s      # Fibonacci sequence implementation
    └── load_store.s     # Tests for load/store instructions

```

## Features

-  **Assembler Support**: Converts RISC-V assembly instructions to machine code.
-  **Cache Simulation**: Simulates a user-configurable D-cache with replacement policies like LRU, FIFO, and RANDOM.
-  **Simulator Functionality**: Supports execution of RISC-V assembly with debug capabilities.
-  **Testing Framework**: A suite of test assembly files to verify the assembler and simulator.

## Usage

### Building the Project

Compile the project using the provided Makefile:

```bash
make
```

This will generate the executable for the simulator.

### Running the Assembler and Simulator

To run the simulator:

```bash
./risc_sim
```

### Using the Simulator

Once running, use the simulator's command-line interface to interact with your assembly programs:

-  **load <filename>**: Load an assembly file for simulation.
-  **run**: Execute the loaded assembly code.
-  **step**: Proceed step-by-step through instructions.
-  **break <line>**: Set a breakpoint at a specific line.
-  **cache_sim enable <filename>**: Enable cache simulation with specified config.
-  **cache_sim status**: Display current cache configuration.
-  **cache_sim stats**: Display cache statistics.

### Cleaning the Project

To remove compiled artifacts:

```bash
make clean
```

## Notes

-  Ensure assembly files comply with RISC-V standards, as improper formatting could lead to errors.
-  The project includes robust caching mechanisms that simulate real-world cache behaviors for performance testing.
