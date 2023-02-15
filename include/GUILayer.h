#pragma once

#include "Containers.hpp"
#include "Text.h"

#include <GLES3/gl3.h>

#include <vector>

enum ClickType {
  NONE,
  LCLICK,
  RCLICK,
  MCLICK
};

//template<typename T>
//void FunctionHandler(void *v, void *a) {
//  T* obj = (T*)v;
//  T::FunctionArgs *arg = (T::FunctionArgs*)a;
//  obj->FunctionHandler(arg);
//}

template<typename T>
void ClickHandler(int x, int y, ClickType c, void *v) {
  T* obj = (T*)v;
  obj->ClickHandler(x,y,c);
}

template<typename T>
void MenuSelectHandler(void *v, void *args) {
  typedef typename T::MenuArgs MA;
  T* obj = (T*)v;
  MA* a = (MA*)args;
  obj->MenuSelectHandler(*a);
}

class ScreenRegion {
 public:
  int x0, y0, x1, y1;
  //int z; // Allow for positioning some things on top
  void (*handler)(int, int, ClickType, void*);
  void (*display)(void*);
  void *obj;
  UuidMapper<ScreenRegion> subRegions;

  ScreenRegion(int _x0, int _y0, int _x1, int _y1);
  ScreenRegion();
  ~ScreenRegion();

  uint32_t InsertRegion( ScreenRegion s );

  void RemoveRegion( uint32_t id );

  void SetObjDispAndHandle( void (*h)(int,int,ClickType,void*) , void (*d)(void*), void* o );

  void HandleClick(int x, int y, ClickType c);

  void Render();
};

class MessageSequence {
 public:
  std::vector<std::string> seq;
  int cnt;
  TextWriter *writer;
  uint64_t onDisplay;
  bool loop;
  Pos2Df pos;
  float scale;

  void CommonInit();

  MessageSequence();
  MessageSequence(std::vector<std::string> &s);
  MessageSequence(TextWriter* w);
  MessageSequence(TextWriter* w, std::vector<std::string> &s);
  MessageSequence(TextWriter* w, std::vector<std::string> &s, Pos2Df p);
  MessageSequence(TextWriter* w, std::vector<std::string> &s, Pos2Df p, float sc);

  bool AdvanceText();

  bool ReverseText();

  void ClickHandler(int x, int y, ClickType c);
};

class MenuItem {
 public:
  std::string text;
  std::string hover;
  std::vector<MenuItem> subItems;
  uint64_t screenId, writerId;
  bool attached, displayed;
  void (*clickAction)(void*,void*);
  void *clickObj, *clickArgs;

  MenuItem();
  MenuItem( const std::string &t
          , const std::string &h = std::string()
          );
  MenuItem( const std::string &t
          , const std::string &h
          , void (*ca)(void*,void*)
          , void *co
          , void *args
          );

  void AddItem( const std::string &text, const std::string &hover = std::string() );
  void AddItem( const std::string &text, const std::string &hover , void (*ca)(void*,void*) , void *co, void *args );

  void RemoveFrom( TextWriter *writer, ScreenRegion *screen);

  void ClickHandler(int x, int y, ClickType c);
};

class MenuCommon {
 public:
  std::vector<MenuItem> items;
  TextWriter *writer;
  ScreenRegion *screen;
  TextBackground *bg;
  //IconWriter *icons;
  float scale;
  uint32_t bgBoxId;
  bool displayed, changed;

  MenuCommon();

  MenuCommon( TextWriter *w
            , TextBackground *b
            , ScreenRegion *r
            , float s
            );

  MenuCommon( TextWriter *w
            , TextBackground *b
            , ScreenRegion *r
            , float s
            , std::vector<MenuItem> &mi
            );

  void AddItem( const MenuItem &m );

  void AddItem( const std::string &t , const std::string h = std::string() );

  void AddItem( const std::string &t , const std::string &h , void (*ca)(void*,void*) , void *co, void *args );

  virtual void Display() = 0;

  virtual void Remove() = 0;

  void DisplayItemOnScreen( MenuItem &it , float x, float y );

  void AttachItemToScreen( MenuItem &it, float x0, float y0, float x1, float y1 );
};

class VerticalMenu : public MenuCommon {
 public:
  Box2Df bounds;

  VerticalMenu();

  VerticalMenu( Box2Df bound
              , TextWriter *w
              , TextBackground *b
              , ScreenRegion *r
              , float s
              );

  VerticalMenu( Box2Df bound
              , TextWriter *w
              , TextBackground *b
              , ScreenRegion *r
              , float s
              , std::vector<MenuItem> &mi
              );

  ~VerticalMenu();

  void Display();

  void Remove();
};

class PopupMenu : public VerticalMenu {
 public:
  int activeSub;

  PopupMenu();

  PopupMenu( Pos2Df pos
           , TextWriter *w
           , TextBackground *b
           , ScreenRegion *r
           , float s
           );

  PopupMenu( Pos2Df pos
           , TextWriter *w
           , TextBackground *b
           , ScreenRegion *r
           , float s
           , std::vector<MenuItem> &mi
           );

  ~PopupMenu();

  void Display();
};

class Button : public PopupMenu {
 public:
  Button( Pos2Df pos
        , TextWriter *w
        , TextBackground *b
        , ScreenRegion *r
        , float s
        , MenuItem m
        );
};

template<int N>
class MulticolumnMenu {
 public:
  std::array<VerticalMenu,N> cols;
  std::array<float,N> widths;
  Box2Df bounds;

  MulticolumnMenu();

  MulticolumnMenu( Box2Df bound
                 , std::array<float,N> ws
                 , TextWriter *w
                 , TextBackground *b
                 , ScreenRegion *r
                 , float s
                 );

  MulticolumnMenu( Box2Df bound
                 , std::array<float,N> ws
                 , TextWriter *w
                 , TextBackground *b
                 , ScreenRegion *r
                 , float s
                 , std::array<std::vector<MenuItem>,N> &mi
                 );

  ~MulticolumnMenu();

  void Display();

  void Remove();

  void AddRow(std::array<std::string,N> r);

  void AddRow(std::array<MenuItem,N> r);
};


class HorizontalMenu : public MenuCommon {
 public:
  Box2Df bounds;

  HorizontalMenu();

  HorizontalMenu( Box2Df bound
              , TextWriter *w
              , TextBackground *b
              , ScreenRegion *r
              , float s
              );

  HorizontalMenu( Box2Df bound
              , TextWriter *w
              , TextBackground *b
              , ScreenRegion *r
              , float s
              , std::vector<MenuItem> &mi
              );

  ~HorizontalMenu();

  void Display();

  void Remove();
};

