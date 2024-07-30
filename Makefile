run: # compiles and runs the main.c file
	gcc src/main.c -o bin/tmp_main && ./bin/tmp_main;
	
# python3 src/plot.py;


tmp:
	gcc src/tmp_queue.c -o bin/tmp_queue && ./bin/tmp_queue;

clean: # deletes all files in bin folder
	rm -rf bin/*;