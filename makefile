TARGET = test

all: $(TARGET) run

$(TARGET): draft.cpp
	@g++ draft.cpp -o $(TARGET)

run: $(TARGET)
	@./$(TARGET).exe

linux: $(TARGET) runlinux clean

runlinux: $(TARGET)
	@./$(TARGET)

clean:
	@rm -f $(TARGET)

