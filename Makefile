run: # compiles and runs the main.c file
	gcc src/main.c -o bin/tmp_main && ./bin/tmp_main;
	python3 src/plot.py;

clean: # deletes all files in bin folder
	rm -rf bin/*;