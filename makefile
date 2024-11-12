CC = gcc
CFLAGS = -g -Wall -Wextra -std=c11

# Targets
all: final run

final: main.o utils.o assembler.o simulator.o
	@$(CC) $(CFLAGS) -o riscv_sim main.o assembler.o simulator.o utils.o

main.o: main.c ./simulator/utils.h ./simulator/assembler.h ./simulator/simulator.h
	@$(CC) $(CFLAGS) -c main.c

assembler.o: ./simulator/assembler.c utils.o ./simulator/assembler.h
	@$(CC) $(CFLAGS) -c ./simulator/assembler.c

simulator.o: ./simulator/simulator.c ./simulator/simulator.h utils.o
	@$(CC) $(CFLAGS) -c ./simulator/simulator.c

utils.o: ./simulator/utils.c ./simulator/utils.h
	@$(CC) $(CFLAGS) -c ./simulator/utils.c

run: final clean

clean:
	@rm -f *.o assembler
