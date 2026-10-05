#IFJ-2026-Překladač
#xfranek00 - Kamil Franek
#xhrusko00 - Ondřej Hruška
#xkonvim00 - Martin Konvička
#xklimat00 - Tomáš Klíma

# Compiler
CC = clang

# Compiler flags
CFLAGS = -Wall -Wextra -std=c17 -g

# Files
SRC = main.c
EXE_FILE = main.exe
EXE_PATH = .\$(EXE_FILE)

build:
	@echo [INFO] Compiling program...
	$(CC) $(CFLAGS) -o $(EXE_FILE) $(SRC)
	@echo [INFO] Compilation finished.

check:
	@echo [INFO] Checking code formatting...
	clang-format --dry-run --Werror $(SRC)
	@echo [INFO] Code check finished.

build-check: check build

run: build
	@echo [INFO] Starting program...
	@if exist $(EXE_PATH) ( $(EXE_PATH) ) else ( echo [ERROR] File "$(EXE_FILE)" does not exist. )

memtest: build
	@echo [INFO] Running Valgrind memory test...
	valgrind --leak-check=full --show-leak-kinds=all ./$(EXE_FILE)

format:
	@echo [INFO] Formatting source files...
	clang-format -i $(SRC)
	@echo [INFO] Formatting complete.

clean:
	@echo [INFO] Removing temporary files...
	@if exist $(EXE_FILE) del /q $(EXE_FILE)
	@if exist *.o del /q *.o
	@echo [INFO] Temporary files removed.

help:
	@echo === COMPILATION INSTRUCTIONS ===
	@echo make          - Compile program
	@echo make build    - Compile program
	@echo make check    - Check code formatting
	@echo make build-check - Check and compile program
	@echo make run      - Compile and run program
	@echo make memtest  - Run Valgrind memory test
	@echo make format   - Format source files
	@echo make clean    - Remove compiled files
	@echo make help     - Show this help