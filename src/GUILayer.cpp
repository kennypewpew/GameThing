#include "GUILayer.h"
#include "Utility.h"
#include "Globals.h"

#include <GLES3/gl3.h>
#include "SDL.h"

#include <cstddef>


ScreenRegion::ScreenRegion(int _x0, int _y0, int _x1, int _y1)
            : x0(_x0)
            , x1(_x1)
            , y0(_y0)
            , y1(_y1)
            , handler(NULL)
            , display(NULL)
            {}

ScreenRegion::ScreenRegion() : handler(NULL)
               , display(NULL)
               {}

ScreenRegion::~ScreenRegion() {}

uint32_t ScreenRegion::InsertRegion( ScreenRegion s ) {
  return this->subRegions.Insert(s);
}

void ScreenRegion::RemoveRegion( uint32_t id ) {
  this->subRegions.Delete(id);
}

void ScreenRegion::SetObjDispAndHandle( void (*h)(int,int,ClickType,void*) , void (*d)(void*), void* o ){
  this->obj = o;
  this->handler = h;
  this->display = d;
}

void ScreenRegion::HandleClick(int x, int y, ClickType c) {
  for ( auto sr : subRegions.map ) {
    auto r = sr.second;
    if (  x >= r.x0 && x < r.x1
       && y >= r.y0 && y < r.y1 ) {
      r.HandleClick(x,y,c);
      return;
    }
  }
  if ( !this->handler ) return;
  (this->handler)(x,y,c,this->obj);
  return;
}

void ScreenRegion::Render() {
  if ( !this->display ) return;
  (this->display)(this->obj);
}


void MessageSequence::CommonInit() {
  this->cnt = -1;
  this->writer = NULL;
  this->loop = false;
  pos = { .x = 0. , .y = 0. };
  scale = 2.;
}

MessageSequence::MessageSequence() {
  this->CommonInit();
}
MessageSequence::MessageSequence(std::vector<std::string> &s) : seq(s) {
  this->CommonInit();
}
MessageSequence::MessageSequence(TextWriter* w){
  this->CommonInit();
  this->writer = w;
}
MessageSequence::MessageSequence(TextWriter* w, std::vector<std::string> &s) : seq(s) {
  this->CommonInit();
  this->writer = w;
}
MessageSequence::MessageSequence(TextWriter* w, std::vector<std::string> &s, Pos2Df p) : seq(s) {
  this->CommonInit();
  this->writer = w;
  this->pos = p;
}
MessageSequence::MessageSequence(TextWriter* w, std::vector<std::string> &s, Pos2Df p, float sc) : seq(s) {
  this->CommonInit();
  this->writer = w;
  this->pos = p;
  this->scale = sc;
}

bool MessageSequence::AdvanceText() {
  if ( !this->writer ) {
    // TODO: Error handling
    SDL_Log("MessageSequence not attached to a writer");
  }
  if ( this->cnt >= int(seq.size()) ) return true;
  if ( this->cnt > -1 ) {
    this->writer->RemoveText(this->onDisplay);
  }
  ++(this->cnt);
  if ( this->loop ) this->cnt = this->cnt % seq.size();
  if ( this->cnt == int(this->seq.size()) ) return false;
  this->onDisplay = this->writer->AddText(DisplayText(pos.x,pos.y,this->scale,this->seq[this->cnt]));
  return true;
}

bool MessageSequence::ReverseText() {
  if ( !this->writer ) {
    // TODO: Error handling
    SDL_Log("MessageSequence not attached to a writer");
  }
  if ( this->cnt >= 0 )
  {
    this->writer->RemoveText(this->onDisplay);
  }
  if ( this->cnt <= 0 ) {
    return true;
  }
  --(this->cnt);
  if ( this->loop ) this->cnt = this->cnt % seq.size();
  if ( this->cnt == int(this->seq.size()) ) return false;
  this->onDisplay = this->writer->AddText(DisplayText(pos.x,pos.y,this->scale,this->seq[this->cnt]));
  return true;
}

void MessageSequence::ClickHandler(int x, int y, ClickType c) {
  if ( c == LCLICK )
    this->AdvanceText();
  if ( c == RCLICK )
    this->ReverseText();
}

MenuItem::MenuItem() : attached(false)
                     , displayed(false)
                     , clickAction(NULL)
                     , clickObj(NULL)
                     , clickArgs(NULL)
                     {}
MenuItem::MenuItem( const std::string &t
                  , const std::string &h
                  )
                    : attached(false)
                    , displayed(false)
                    , clickAction(NULL)
                    , clickObj(NULL)
                    , clickArgs(NULL)
                    , text(t)
                    , hover(h)
                  {}
MenuItem::MenuItem( const std::string &t
                  , const std::string &h
                  , void (*ca)(void*,void*)
                  , void *co
                  , void *args
                  )
                    : attached(false)
                    , displayed(false)
                    , clickAction(ca)
                    , clickObj(co)
                    , clickArgs(args)
                    , text(t)
                    , hover(h)
                  {}

void MenuItem::AddItem( const std::string &text, const std::string &hover) {
  subItems.push_back( MenuItem( text, hover ) );
}
void MenuItem::AddItem( const std::string &text, const std::string &hover , void (*ca)(void*,void*) , void *co, void *args ) {
  subItems.push_back( MenuItem( text, hover, ca, co, args ) );
}

void MenuItem::RemoveFrom( TextWriter *writer, ScreenRegion *screen ) {
  if ( (NULL != writer) && this->displayed ) writer->RemoveText  (this->writerId);
  if ( (NULL != screen) && this->attached  ) screen->RemoveRegion(this->screenId);
  this->displayed = false;
  this->attached = false;
}

void MenuItem::ClickHandler(int x, int y, ClickType c) {
  if ( clickAction ) clickAction(clickObj,clickArgs);
  else printf("Unassigned menu option : %s\n", this->text.c_str());
}

MenuCommon::MenuCommon() : writer(NULL)
                         , screen(NULL)
                         , bg(NULL)
                         , scale(1.0)
                         //, activeSub(-1)
                         , displayed(false)
                         , changed(false)
                         {}

MenuCommon::MenuCommon( TextWriter *w
                      , TextBackground *b
                      , ScreenRegion *r
                      , float s
                      ) : writer(w)
                        , screen(r)
                        , bg(b)
                        , scale(s)
                        //, activeSub(-1)
                        , displayed(false)
                        , changed(false)
                        {}

MenuCommon::MenuCommon( TextWriter *w
                      , TextBackground *b
                      , ScreenRegion *r
                      , float s
                      , std::vector<MenuItem> &mi
                      ) : writer(w)
                        , screen(r)
                        , bg(b)
                        , scale(s)
                        , items(mi)
                        //, activeSub(-1)
                        , displayed(false)
                        , changed(false)
                        {}

void MenuCommon::AddItem( const MenuItem &m ) {
  items.push_back( m );
}

void MenuCommon::AddItem( const std::string &t , const std::string h) {
  items.push_back( MenuItem( t , h ) );
  this->changed = true;
}

void MenuCommon::AddItem( const std::string &t , const std::string &h , void (*ca)(void*,void*) , void *co, void *args ) {
  items.push_back( MenuItem( t, h, ca, co, args ) );
  this->changed = true;
}

void MenuCommon::DisplayItemOnScreen( MenuItem &it , float x, float y ) {
  if ( !it.displayed ) {
    it.writerId = this->writer->AddText(DisplayText(x,y,this->scale,it.text));
    it.displayed = true;
  }
}

void MenuCommon::AttachItemToScreen( MenuItem &it, float x0, float y0, float x1, float y1 ) {
  if ( !it.attached ) {
    Pos2D p0 = GlPosToPixels( x0 , y0 );
    Pos2D p1 = GlPosToPixels( x1 , y1 );
    ScreenRegion s(p0.x,p0.y,p1.x,p1.y);
    s.SetObjDispAndHandle( ClickHandler<MenuItem> , NULL, &it );
    it.screenId = screen->InsertRegion(s);
    it.attached = true;
  }
}

VerticalMenu::VerticalMenu() : MenuCommon() {}

VerticalMenu::VerticalMenu( Box2Df bound
            , TextWriter *w
            , TextBackground *b
            , ScreenRegion *r
            , float s
            ) : MenuCommon(w,b,r,s)
              , bounds(bound)
            {}

VerticalMenu::VerticalMenu( Box2Df bound
            , TextWriter *w
            , TextBackground *b
            , ScreenRegion *r
            , float s
            , std::vector<MenuItem> &mi
            ) : MenuCommon(w,b,r,s,mi)
              , bounds(bound)
            {}


VerticalMenu::~VerticalMenu() {
  this->Remove();
}

void VerticalMenu::Display() {
  if ( !this->displayed || this->changed || SCREEN.Changed() ) {
    this->Remove();
    if ( this->writer ) {
      float x0 = bounds.x0;
      float y0 = bounds.y0;
      float yf = bounds.y1;
      float xf = bounds.x1;

      float yCurrent = y0;
      for ( auto &it : items ) {
        float yPrev = yCurrent;
        yCurrent -= this->scale * writer->font.lineHeight / SCREEN.H();
        if ( this->screen ) AttachItemToScreen( it, x0, yPrev, xf, yCurrent );
        DisplayItemOnScreen( it , x0 , yCurrent );
      }

      if ( this->bg ) {
        x0 = x0 - this->scale * 0.2 * writer->font.lineHeight / SCREEN.H();
        if ( this->displayed ) {
          bg->RemoveRectangle(bgBoxId);
        }
        bgBoxId = bg->AddRectangle( x0 , y0 , xf , yf );
      }

      this->displayed = true;
    }
  }
}

void VerticalMenu::Remove() {
  for ( auto &it : items ) {
    it.RemoveFrom( this->writer, this->screen );
  }
  if ( this->bg ) bg->RemoveRectangle(bgBoxId);
  this->displayed = false;
}

PopupMenu::PopupMenu() : VerticalMenu() {}

PopupMenu::PopupMenu( Pos2Df pos
         , TextWriter *w
         , TextBackground *b
         , ScreenRegion *r
         , float s
         ) : VerticalMenu({.x0=pos.x,.y0=pos.y},w,b,r,s)
         {}

PopupMenu::PopupMenu( Pos2Df pos
         , TextWriter *w
         , TextBackground *b
         , ScreenRegion *r
         , float s
         , std::vector<MenuItem> &mi
         ) : VerticalMenu({.x0=pos.x,.y0=pos.y},w,b,r,s,mi)
         {}

PopupMenu::~PopupMenu() {
  this->Remove();
}

void PopupMenu::Display() {
  if ( !this->displayed || this->changed || SCREEN.Changed() ) {
    if ( this->writer ) {
      float x0 = bounds.x0;
      float y0 = bounds.y0;
      float maxLength = 0;
      for ( auto it : items ) {
        float l = TextLength( it.text , writer->font , this->scale );
        if ( l > maxLength ) maxLength = l;
      }
      float yf = y0 - items.size() * (this->scale * writer->font.lineHeight / SCREEN.H()) - this->scale * 0.3 * writer->font.lineHeight / SCREEN.H();
      float xf = x0 + maxLength * scale;

      this->bounds.x1 = xf;
      this->bounds.y1 = yf;
      VerticalMenu::Display();
    }
  }
}

Button::Button( Pos2Df pos
              , TextWriter *w
              , TextBackground *b
              , ScreenRegion *r
              , float s
              , MenuItem m
              ) : PopupMenu(pos,w,b,r,s) {
  this->AddItem(m);
  this->Display();
}

HorizontalMenu::HorizontalMenu() : MenuCommon() {}

HorizontalMenu::HorizontalMenu( Box2Df bound
                              , TextWriter *w
                              , TextBackground *b
                              , ScreenRegion *r
                              , float s
                              ) : MenuCommon(w,b,r,s)
                                , bounds(bound)
                              {}

HorizontalMenu::HorizontalMenu( Box2Df bound
                              , TextWriter *w
                              , TextBackground *b
                              , ScreenRegion *r
                              , float s
                              , std::vector<MenuItem> &mi
                              ) : MenuCommon(w,b,r,s,mi)
                                , bounds(bound)
                              {}

HorizontalMenu::~HorizontalMenu() {
  this->Remove();
}

void HorizontalMenu::Display() {
  if ( !this->displayed || this->changed || SCREEN.Changed() ) {
    this->Remove();
    if ( this->writer ) {
      float x0 = bounds.x0;
      float y0 = bounds.y0;
      float yf = bounds.y1;
      float xf = bounds.x1;
      float yShift = (y0 - yf) / 5.;

      float xCurrent = x0;
      for ( auto &it : items ) {
        float xPrev = xCurrent;
        float l = TextLength( it.text + "     " , writer->font , this->scale );
        xCurrent += l;
        if ( this->screen ) AttachItemToScreen( it, xPrev, y0, xCurrent, yf );
        DisplayItemOnScreen( it , xPrev , yf + yShift );
      }

      if ( this->bg ) {
        x0 = x0 - this->scale * 0.2 * writer->font.lineHeight / SCREEN.H();
        if ( this->displayed ) {
          bg->RemoveRectangle(bgBoxId);
        }
        bgBoxId = bg->AddRectangle( x0 , y0 , xf , yf );
      }

      this->displayed = true;
    }
  }
}

void HorizontalMenu::Remove() {
  for ( auto &it : items ) {
    it.RemoveFrom( this->writer, this->screen );
  }
  if ( this->bg ) bg->RemoveRectangle(bgBoxId);
  this->displayed = false;
}


