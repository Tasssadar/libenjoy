program=libenjoy_test
OBJ=test.o libenjoy.o libenjoy_linux.o
OPT=-g

.PHONY: build
.PHONY: install
.PHONY: uninstall
.PHONY: clean
.PHONY: distrib

build: ${program}


clean:
	rm -f *.o ${program}

${program}: ${OBJ}
	gcc ${OBJ} -o ${program} ${OPT}

test.o: test.c
	gcc test.c -c ${OPT}

libenjoy.o: ../src/libenjoy.c
	gcc ../src/libenjoy.c -c ${OPT}

libenjoy_linux.o: ../src/libenjoy_linux.c
	gcc ../src/libenjoy_linux.c -c ${OPT}