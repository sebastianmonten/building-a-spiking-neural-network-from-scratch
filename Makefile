

clean: # deletes all files in bin folder
	rm -rf bin/*;
	clear;



# Detect the operating system
OS := $(shell uname)

# Default target
all: run

# Run target with conditional behavior
run:
ifeq ($(OS), Linux)
	@echo "Running on Linux"
	gcc src/main.c -lm -o bin/tmp_main && ./bin/tmp_main;
else
	@echo "Running on non-Linux OS"
	gcc src/main.c -o bin/tmp_main && ./bin/tmp_main;
endif


plot: # compiles and runs the main.c file together with the plot.py file
ifeq ($(OS), Linux)
	@echo "Running on Linux"
	rm -rf bin/*;
	gcc src/main.c -lm -o bin/tmp_main && ./bin/tmp_main;
	python3 src/plot.py;
else
	@echo "Running on non-Linux OS"
	rm -rf bin/*;
	gcc src/main.c -o bin/tmp_main && ./bin/tmp_main;
	python3 src/plot.py;
endif
