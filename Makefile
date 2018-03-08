TARGET = asd
CC = g++
LIBS = -lSDL2 -lSDL2main -lGL -lGLU -lGLEW -ltinyxml2 -lpng -ltiff -ljpeg
SRC = ./Common/Image/*.cpp ./Common/Model/*.cpp ./*.cpp ./Graphics/*.cpp
SRC += ./Graphics/OpenGL/*.cpp ./GUI/*.cpp ./Impl/*.cpp ./Input/*.cpp ./RenderAPI/*.cpp 
SRC += ./Scene/*.cpp ./System/*.cpp
INCL = -I./ -I/usr/include/SDL2/ -I./Lib/JSON/

all:
	$(CC) $(SRC) -o $(TARGET) $(LIBS) $(INCL) -std=c++14






