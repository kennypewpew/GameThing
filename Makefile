CC=g++

SDL_LINK_FLAGS= -lSDL2 -lGLESv2 -I/usr/include/SDL2
GL_LINK_FLAGS= -lEGL

IFLAGS= -I ./include
CFLAGS= -O2 -g -Wall -Wno-reorder
LFLAGS= $(GLFW_LINK_FLAGS) $(SDL_LINK_FLAGS) $(GL_LINK_FLAGS)

SRC=$(wildcard src/*.cpp)
OBJ_TMP=$(subst src,obj,$(SRC))
OBJ=$(subst .cpp,.o,$(OBJ_TMP))

DEV=$(wildcard dev/*.cpp)
DEV_TMP=$(subst .cpp,,$(DEV))
ALL_DEV=$(subst dev,bin,$(DEV_TMP))

.SECONDARY: $(OBJ)

#all: run_sdlMain
#all: run_char
#all: run_mapTest
#all: run_world
#all: run_WorldState
#all: run_BattleInfo
all: run_MenuBase

compile_all: $(ALL_DEV)

run_%: bin/%
	./$<

bin/%: dev/%.cpp $(OBJ)
	$(CC) $^ -o $@ $(CFLAGS) $(IFLAGS) $(LFLAGS)

obj/%.o: src/%.cpp include/%.h
	$(CC) $< -c -o $@ $(CFLAGS) $(IFLAGS) $(LFLAGS)

clean:
	rm -f obj/* bin/*
