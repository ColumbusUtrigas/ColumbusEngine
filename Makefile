TARGET = asd
CC = g++
LIBS = -lSDL2 -lSDL2main -lSDL2_image -lGL -lGLU -lGLEW -ltinyxml2
SRC = ./*.cpp ./Graphics/*.cpp ./GUI/*.cpp ./Impl/*.cpp ./Input/*.cpp ./RenderAPI/*.cpp 
SRC += ./Scene/*.cpp ./System/*.cpp
INCL = -I./ -I/usr/include/SDL2/ -I./Lib/STB/ -I./Lib/JSON/

all:
	$(CC) $(SRC) -o $(TARGET) $(LIBS) $(INCL) -std=c++14






