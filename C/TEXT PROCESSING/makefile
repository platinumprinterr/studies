# List your *.h files (if you do not have them in your project then leave the variable "headers" empty):
headers = rand_malloc.h

# List your *.c files:
sources = testlib.c rand_malloc.c

# Specify name of your program:
executable = testlib

$(executable): $(sources) $(headers)
	gcc -g -Wall -pedantic -fsanitize=undefined $(sources) -o $(executable)

.PHONY: clean
clean:
	rm -f $(executable) *.o

.PHONY: check
check: $(executable)
	valgrind --leak-check=full -s --show-leak-kinds=all --track-origins=yes ./$(executable)
