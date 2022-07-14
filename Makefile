CC=g++

#GLFW_LINK_FLAGS= -lglfw -lGL -lX11 -lpthread -lXrandr -lXi -ldl
#GLEW_LINK_FLAGS= -lGLEW
GLEW_LINK_FLAGS= -lSDL2 -lGLESv2

IFLAGS= -I ./include
CFLAGS= -O2 -g
LFLAGS= $(GLFW_LINK_FLAGS) $(GLEW_LINK_FLAGS)

all: run_sdlMain

run_displayMap: bin/displayMap
	./$<

run_sdlMain: bin/sdlMain
	./$<

run_%: bin/%

bin/%: src/%.cpp
	$(CC) $^ -o $@ $(CFLAGS) $(IFLAGS) $(LFLAGS)
