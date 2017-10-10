TARGET = asd
CC = g++
LIBS = -lSDL2 -lSDL2main -lGL -lGLU -lGLEW -ltinyxml2 -lfreeimage
SRC = ./*.cpp ./Graphics/*.cpp ./Impl/*.cpp ./RenderAPI/*.cpp ./System/*.cpp
INCL = -I./ -I/usr/include/SDL2/

all:
	$(CC) $(SRC) -o $(TARGET) $(LIBS) $(INCL) -std=c++14






