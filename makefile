TARGET = test

all: $(TARGET) run

$(TARGET): draft.cpp
	g++ draft.cpp -o $(TARGET)

run: $(TARGET)
	./$(TARGET).exe
