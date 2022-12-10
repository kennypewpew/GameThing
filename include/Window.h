#pragma once

#include <string>

#include <SDL.h>

int InitializeSDL(const std::string &,const int&, const int&);
void FinalizeSDL();
void SwapWindows();
SDL_Window* GetWindow();

class ScreenDimensions {
 private:
  unsigned int DISP_W;
  unsigned int DISP_H;
  bool DIMS_CHANGED;
  bool changedPrev;

 public:
  ScreenDimensions(unsigned int w , unsigned int h);
  void SetDimensions(unsigned int w , unsigned int h);
  unsigned int W();
  unsigned int H();
  bool Changed();
  void Update();
};

