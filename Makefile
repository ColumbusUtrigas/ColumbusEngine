TARGET = asd
CC = g++
LIBS = -lSDL2 -lSDL2main -lGL -lGLU -lGLEW -ltinyxml2 -lpng -ltiff -ljpeg -lLinearMath -lBulletCollision -lBulletDynamics
LIBS += -lopenal
SRC = ./Common/Image/*.cpp ./Common/Model/*.cpp ./Common/Sound/*.cpp ./*.cpp ./Graphics/*.cpp
SRC += ./Graphics/OpenGL/*.cpp ./Graphics/Particles/*.cpp ./Audio/*.cpp ./Audio/OpenAL/*.cpp
SRC += ./GUI/*.cpp ./Impl/*.cpp ./Input/*.cpp ./RenderAPI/*.cpp 
SRC += ./Scene/*.cpp ./System/*.cpp ./Physics/*.cpp
INCL = -I./ -I/usr/include/SDL2/ -I./Lib/JSON/ -I/usr/include/bullet/

all:
	$(CC) $(SRC) -o $(TARGET) $(LIBS) $(INCL) -std=c++14 -O3 -Wall -Wno-reorder -Wno-sign-compare -Wno-strict-aliasing






