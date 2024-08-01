run: # compiles and runs the main.c file
	gcc src/main.c -o bin/tmp_main && ./bin/tmp_main;
	
plot: # compiles and runs the main.c file together with the plot.py file
	rm -rf bin/*;
	gcc src/main.c -o bin/tmp_main && ./bin/tmp_main;
	python3 src/plot.py;


clean: # deletes all files in bin folder
	rm -rf bin/*;
	clear;