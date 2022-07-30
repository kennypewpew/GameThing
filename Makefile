CC=g++

#GLFW_LINK_FLAGS= -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl
#GLEW_LINK_FLAGS= -lGLEW
GLEW_LINK_FLAGS= -lSDL2 -lGLESv2 -I/usr/include/SDL2
GL_LINK_FLAGS= -lEGL

IFLAGS= -I ./include
CFLAGS= -O2 -g -Wall
LFLAGS= $(GLFW_LINK_FLAGS) $(GLEW_LINK_FLAGS) $(GL_LINK_FLAGS)

SRC=src/Maps.cpp src/Window.cpp src/MapGen.cpp src/GlLayer.cpp src/Actor.cpp src/Physics.cpp
#SRC=src/Maps.cpp src/Window.cpp src/MapGen.cpp
OBJ_TMP=$(subst src,obj,$(SRC))
OBJ=$(subst .cpp,.o,$(OBJ_TMP))

.SECONDARY: $(OBJ)

#all: run_pureSDL
all: run_sdlMain
#all: run_char
#all: run_MapGen

run_mapTest: bin/mapTest
	./$<

run_char: bin/char
	./$<

run_displayMap: bin/displayMap
	./$<

run_sdlMain: bin/sdlMain
	echo $(OBJ)
	./$<

run_pureSDL: bin/pureSDL
	./$<

run_%: bin/%

bin/%: obj/%.o $(OBJ)
	$(CC) $^ -o $@ $(CFLAGS) $(IFLAGS) $(LFLAGS)

obj/%.o: src/%.cpp
	$(CC) $^ -c -o $@ $(CFLAGS) $(IFLAGS) $(LFLAGS)
