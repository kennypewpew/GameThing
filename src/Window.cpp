#include <SDL.h>
#include <SDL_opengles2.h>
#include <GLES3/gl3.h>
#include <EGL/egl.h>

#include <string>

#include "Window.h"
#include "Globals.h"

// The window
SDL_Window *window = NULL;
// The OpenGL context
SDL_GLContext context = NULL;

SDL_Window* GetWindow() {
  return window;
}

PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOES;
PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOES;
PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOES;
PFNGLISVERTEXARRAYOESPROC glIsVertexArrayOES;

ScreenDimensions::ScreenDimensions(unsigned int w , unsigned int h) {
  this->DISP_W = w;
  this->DISP_H = h;
  this->DIMS_CHANGED = true;
  this->changedPrev = true;
}

void ScreenDimensions::SetDimensions(unsigned int w , unsigned int h) {
  this->DISP_W = w;
  this->DISP_H = h;
  this->DIMS_CHANGED = true;
  this->changedPrev = true;
}

unsigned int ScreenDimensions::W() {
  return DISP_W;
}

unsigned int ScreenDimensions::H() {
  return DISP_H;
}

void ScreenDimensions::Update() {
  this->changedPrev = true;
  this->DIMS_CHANGED = false;
}

bool ScreenDimensions::Changed() {
  return this->changedPrev | this->DIMS_CHANGED;
}

int InitializeSDL(const std::string &title,const int&WindowWidth, const int &WindowHeight) {
  glGenVertexArraysOES = (PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress ( "glGenVertexArraysOES" );
  glBindVertexArrayOES = (PFNGLBINDVERTEXARRAYOESPROC)eglGetProcAddress ( "glBindVertexArrayOES" );
  glDeleteVertexArraysOES = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress ( "glDeleteVertexArraysOES" );
  glIsVertexArrayOES = (PFNGLISVERTEXARRAYOESPROC)eglGetProcAddress ( "glIsVertexArrayOES" );

  if ( NULL != window || NULL != context ) {
    SDL_Log("Failed to initialize : Detected existing window/context\n");
  }

  // Init SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }
  // Setup the exit hook
  atexit(SDL_Quit);

  // Request OpenGL ES 3.0
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

  // Want double-buffering
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // Create the window
  window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED, WindowWidth, WindowHeight,
      SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
  if (!window) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
        "Couldn't create the main window.", NULL);
    return EXIT_FAILURE;
  }

  context = SDL_GL_CreateContext(window);
  if (!context) {
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
        "Couldn't create an OpenGL context.", NULL);
    return EXIT_FAILURE;
  }

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  SCREEN.SetDimensions( WindowWidth, WindowHeight );

  return 0;
}

void FinalizeSDL() {
  SDL_DestroyWindow(window);
  SDL_Quit();
}

void SwapWindows() {
  SCREEN.Update();
  SDL_GL_SwapWindow(window);
}

