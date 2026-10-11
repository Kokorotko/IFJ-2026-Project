#IFJ-2026-Překladač
#xfranek00 - Kamil Franek
#xhrusko00 - Ondřej Hruška
#xkonvim00 - Martin Konvička
#xklimat00 - Tomáš Klíma

# Compiler
CC = clang

# Compiler flags (-MMD -MP generuje .d soubory, takže se po změně .h přeloží závislé .c)
CFLAGS = -Wall -Wextra -std=c17 -g -MMD -MP

# Soubory
EXE = ifj26 #Nazev output souboru
SRC = $(wildcard *.c) #vezme všechny .c soubory
HDR = $(wildcard *.h) #vezme všechny .h soubory
OBJ = $(SRC:.c=.o)
DEP = $(OBJ:.o=.d) #Vygeneruje závislost pro .c a .h soubory pro kompilaci

.PHONY: all build check build-check run memtest format clean

all: build

build: $(EXE)

#Kompilator
$(EXE): $(OBJ)
	@echo "[INFO] Linking program..."
	$(CC) -o $@ $^
	@echo "[INFO] Compilation finished."

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


check:
	@echo "[INFO] Checking code formatting..."
	clang-format --dry-run --Werror $(SRC) $(HDR)
	@echo "[INFO] Code check finished."

build-check: check build

run: build
	@echo "[INFO] Starting program..."
	./$(EXE)

memtest: build
	@echo "[INFO] Running Valgrind memory test..."
	valgrind --leak-check=full --show-leak-kinds=all ./$(EXE)

format:
	@echo "[INFO] Formatting source files..."
	clang-format -i $(SRC) $(HDR)
	@echo "[INFO] Formatting complete."

clean:
	@echo "[INFO] Removing temporary files..."
	rm -f $(EXE) $(OBJ) $(DEP)
	@echo "[INFO] Temporary files removed."

help:
	@echo === COMPILATION INSTRUCTIONS ===
	@echo 			!LINUX ONLY!
	@echo make          - Compile program
	@echo make all    - Compile program
	@echo make check    - Check code formatting
	@echo make build-check - Check and compile program
	@echo make run      - Compile and run program
	@echo make memtest  - Run Valgrind memory test
	@echo make format   - Format source files with clang-format
	@echo make clean    - Remove compiled files
	@echo make help     - Shows help

-include $(DEP)