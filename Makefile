clean: # deletes all files in bin folder
	rm -rf bin/*;
	rm -rf tmp/*;
	clear;

# Detect the operating system
OS := $(shell uname)

# Run target with conditional behavior
run:
ifeq ($(OS), Linux)
	@echo "Running on Linux\n"
	gcc src/main.c -lm -o bin/tmp_main && ./bin/tmp_main;
else
	@echo "Running on non-Linux OS"
	gcc src/main.c -o bin/tmp_main && ./bin/tmp_main;
endif


plot: # compiles and runs the main.c file together with the plot.py file
ifeq ($(OS), Linux)
	@echo "Running on Linux\n"
	make clean;
	gcc src/main.c -lm -o bin/tmp_main && ./bin/tmp_main;
	python3 src/plot.py;
else
	@echo "Running on non-Linux OS"
	make clean;
	gcc src/main.c -o bin/tmp_main && ./bin/tmp_main;
	python3 src/plot.py;
endif

leaks: # runs the valgrind tool to check for memory leaks
ifeq ($(OS), Linux)
	@echo "Running on Linux\n"
	gcc src/main.c -lm -o bin/tmp_main && valgrind --leak-check=full ./bin/tmp_main;
else
	@echo "Running on non-Linux OS\n"
	gcc src/main.c -o bin/tmp_main && valgrind --leak-check=full ./bin/tmp_main;
endif