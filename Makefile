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
EXE = ifj26
#Nazev output souboru
SRC = $(wildcard *.c)
#vezme všechny .c soubory
HDR = $(wildcard *.h)
#vezme všechny .h soubory
OBJ = $(SRC:.c=.o)
DEP = $(OBJ:.o=.d)
#Vygeneruje závislost pro .c a .h soubory pro kompilaci
IC26INT = ic26int
#testovaci output file
TESTDIR = tests
#testovaci dir

.PHONY: all build check build-check run memtest format clean test testSimple testComplex

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
	rm -f tests/CODESimple.code
	rm -f tests/CODESimple.in
	rm -f tests/CODE.code
	rm -f tests/CODS.in
	rm -f tests/CODEComplex.code
	rm -f tests/CODEComplex.in
	@echo "[INFO] Temporary files removed."

test: $(EXE)
	./$(EXE) < $(TESTDIR)/CODE.ifj26 > $(TESTDIR)/CODE.code
	docker run --rm -i -v $(CURDIR)/tests/CODE.code:/app/prog docker.io/leondryaso/ic26int:latest prog < tests/CODE.code; echo "exit: $$?"

testSimple: $(EXE)
	./$(EXE) < $(TESTDIR)/CODESimple.ifj26 > $(TESTDIR)/CODESimple.code
	docker run --rm -i -v $(CURDIR)/tests/CODESimple.code:/app/prog docker.io/leondryaso/ic26int:latest prog < tests/CODESimple.code; echo "exit: $$?"

testComplex: $(EXE)
	./$(EXE) < $(TESTDIR)/CODEComplex.ifj26 > $(TESTDIR)/CODEComplex.code
	docker run --rm -i -v $(CURDIR)/tests/CODEComplex.code:/app/prog docker.io/leondryaso/ic26int:latest prog < tests/CODEComplex.code; echo "exit: $$?"

help:
	@echo === COMPILATION INSTRUCTIONS ===
	@echo 					!LINUX ONLY!
	@echo make         	    - Compile program
	@echo make all    	    - Compile program
	@echo make check   	    - Check code formatting
	@echo make build-check  - Check and compile program
	@echo make run      	- Compile and run program
	@echo make memtest  	- Run Valgrind memory test
	@echo make format   	- Format source files with clang-format
	@echo make clean    	- Remove temporary files
	@echo make testSimple 	- Runs a simple ifj2026 to ifjcode2026 file and runs ic26int
	@echo make test    		- Runs a ifj2026 to ifjcode2026 file and runs ic26int
	@echo make testComplex  - Runs a complex ifj2026 to ifjcode2026 file and runs ic26int
	@echo make help         - Shows help

-include $(DEP)