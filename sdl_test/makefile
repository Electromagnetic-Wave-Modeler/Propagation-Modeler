CC = g++

SOURCES = sdl_test.cpp
OBJS = ${SOURCES:.cpp=.o}

all: build run

build: ${OBJS}
	@${CC} -o dist/programm.exe ${OBJS} -LSDL2/ -lSDL2 

%.o: %.cpp
	@${CC} -c $< -o $@ -ISDL2/include/

run: dist/programm.exe
	@./dist/programm.exe

clean:
	@del ${OBJS}