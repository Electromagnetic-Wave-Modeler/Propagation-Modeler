TARGET = dist/main

SRC_FILES = $(wildcard src/*.cpp)
SRC_FILES2 = $(wildcard src/obstacle/*.cpp)
SOURCES = main.cpp $(SRC_FILES) $(SRC_FILES2)
OBJS = ${SOURCES:.cpp=.o}
SDL2_PATH = lib/SDL2
SDL2_ttf_PATH = lib/SDL2_ttf



all: $(TARGET) run

linux: $(TARGET) runlinux clean

$(TARGET): main.cpp
	@g++ ${SOURCES} -o $(TARGET) -Iheaders/ -I${SDL2_PATH}/include -I${SDL2_TTF_PATH}/include -L${SDL2_PATH}/lib -L${SDL2_TTF_PATH}/lib -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf

run: $(TARGET)
	@./$(TARGET).exe

runlinux: $(TARGET)
	@./$(TARGET)

clean:
	@rm -f $(TARGET)

