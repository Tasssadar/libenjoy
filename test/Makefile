program=libenjoy_test
OPT=-g
LIBS=
CC?=gcc

OBJ=test.o libenjoy.o
ifneq ($(BUILD_WINDOWS),true)
  OBJ += libenjoy_linux.o
else
  OBJ += libenjoy_win32.o
  LIBS += -lwinmm
endif

.PHONY: build
.PHONY: install
.PHONY: uninstall
.PHONY: clean
.PHONY: distrib

build: ${program}

clean:
	rm -f *.o ${program}

${program}: ${OBJ}
	$(CC) ${OBJ} -o ${program} ${OPT} ${LIBS}

test.o: test.c
	$(CC) test.c -c ${OPT}

libenjoy.o: ../src/libenjoy.c
	$(CC) ../src/libenjoy.c -c ${OPT}

ifneq ($(BUILD_WINDOWS),true)
libenjoy_linux.o: ../src/libenjoy_linux.c
	$(CC) ../src/libenjoy_linux.c -c ${OPT}
else
libenjoy_win32.o: ../src/libenjoy_win32.c
	$(CC) ../src/libenjoy_win32.c -c ${OPT}
endif
