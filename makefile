TARGET = dist/main

SOURCES = main.cpp src/display.cpp
OBJS = ${SOURCES:.cpp=.o}
SDL2_PATH = lib/SDL2

all: $(TARGET) run

$(TARGET): main.cpp
	@g++ ${SOURCES} -o $(TARGET) -L${SDL2_PATH} -I${SDL2_PATH}/include/ -lSDL2 -Iheaders/

run: $(TARGET)
	@./$(TARGET).exe

linux: $(TARGET) runlinux clean

runlinux: $(TARGET)
	@./$(TARGET)

clean:
	@rm -f $(TARGET)

