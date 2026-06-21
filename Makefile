GXX_FLAGS = -Wall -Wextra
COMPILER = g++
TARGET = img2nfp

all: build
	./$(TARGET)

build:
	$(COMPILER) $(GXX_FLAGS) main.cpp -o $(TARGET)

clean:
	rm -f *.o
	rm -f $(TARGET)