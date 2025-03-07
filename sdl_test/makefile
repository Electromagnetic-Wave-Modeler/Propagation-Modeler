CC = gcc

SOURCES = sdl_test.c
OBJS = ${SOURCES:.c=.o}

all: build run

build: ${OBJS}
	@${CC} -o dist/programm.exe ${OBJS} -LSDL2/ -lSDL2

%.o: %.c
	@${CC} -c $< -o $@ -ISDL2/include/

run: dist/programm.exe
	@./dist/programm.exe

clean:
	@del ${OBJS}