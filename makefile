all: main exe clean

main:
	@g++ -o test test.cpp
exe:
	@./test
clean:
	@rm test