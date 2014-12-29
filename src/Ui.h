/*
 * This file is part of NumptyPhysics
 * Copyright (C) 2008 Tim Edmonds
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 */

#ifndef UI_H
#define UI_H

#include "Common.h"
#include "Event.h"

#include <string>
#include <functional>
#include <vector>

class Canvas;
class Image;
class Widget;
class Font;

class WidgetParent;

class StockIcon {
 public:
  enum Kind {
   NONE = -1,

   SHARE = 0,
   UNDO,
   PEN,
   RESET,
   FORWARD,
   HELP,
   PLAY,
   CLOSE,
   CHOOSE,
   PAUSE,
   BLANK,
   TICK,

   ICON_COUNT,
  };

  static void draw(Canvas &screen, const Rect &area, enum Kind kind, const Vec2 &pos);
  static int size();
};


class Widget
{
 public:
  virtual ~Widget() {}
  virtual const char* name() {return "Widget";}
  virtual std::string toString();
  virtual void move( const Vec2& by );
  virtual void moveTo( const Vec2& to ) {move(to-m_pos.tl);}
  virtual void sizeTo( const Vec2& size );
  virtual void animateTo(const Vec2 &to, std::function<void()> done=[](){}) { m_targetPos = to; m_animating = true; m_animation_done = done; }
  virtual const Rect& position() const { return m_pos; }
  virtual void onTick(int tick);
  virtual void draw( Canvas& screen, const Rect& area );
  virtual bool processEvent(ToolkitEvent &ev);
  bool dispatchEvent( Event& ev );

  virtual void onResize() {}
  virtual bool onEvent( Event& ev ) {return false;}

  void setParent(WidgetParent* p) {m_parent = p;}
  WidgetParent* parent() { return m_parent; }
  WidgetParent* topLevel();
  void setEventMap(EventMap* em) {m_eventMap = em;}
  void setEventMap(EventMapType map);

  Rect& position() { return m_pos; }
  void setBg(int bg) {m_bg=bg;}
  void setFg(int fg) {m_fg=fg;}
  void fitToParent(bool fit) { m_fitToParent=fit;}
  bool fitToParent() {return m_fitToParent;}
  bool greedyMouse() {return m_greedyMouse;}
  void transparent(bool t) {m_alpha=t?0:255;}
  void alpha(int a) {m_alpha=a;}
  void border(bool drawBorder) {m_border = drawBorder?1:0;}
  void show() { m_visible = true; }
  void hide() { m_visible = false; }

  int width() { return m_pos.width(); }
  int height() { return m_pos.height(); }
 protected:
  Widget(WidgetParent *p=NULL);
  WidgetParent* m_parent;
  EventMap*     m_eventMap;
  Rect          m_pos;
  bool          m_focussed;
  int           m_alpha;
  bool          m_fitToParent;
  bool          m_greedyMouse;
  int           m_bg;
  int           m_fg;
  int           m_border;
  Vec2          m_targetPos;
  bool          m_animating;
  std::function<void()> m_animation_done;
  bool          m_visible;
};

class Spacer : public Widget {
  const char* name() {return "Spacer";}
};

class Label : public Widget
{
 public:
  Label();
  Label(const std::string& s, const Font* f=NULL, int color=0xffffff);
  const char* name() {return "Label";}
  virtual void text( const std::string& s );
  const std::string& text() const { return m_text; }
  virtual void draw( Canvas& screen, const Rect& area );
  void font( const Font* f ) { m_font = f; }

  enum Alignment {
      ALIGN_CENTER = 0,

      ALIGN_LEFT = 1 << 0,
      ALIGN_RIGHT = 1 << 1,

      ALIGN_TOP = 1 << 2,
      ALIGN_BOTTOM = 1 << 3,
  };

  int alignment() { return m_alignment; }
  void setAlignment(int alignment) { m_alignment = alignment; }

 protected:
  std::string m_text;
  const Font *m_font;
  int m_alignment;
};


class Button : public Label
{
 public:
  Button(const std::string& s, Event event=Event::NOP);
  const char* name() {return "Button";}
  void event(Event ev) {m_selEvent = ev;}
  void draw( Canvas& screen, const Rect& area );
  virtual bool onEvent( Event& ev );
  virtual void onSelect() {};
 protected:
  Event m_selEvent;
};


class Icon : public Label
{
public:
    Icon(Image *image=nullptr);
    ~Icon();

    const char *name() { return "Icon"; }

    void image(Image *image);
    void draw(Canvas &screen, const Rect &area);

protected:
    Image *m_image;
};


class IconButton : public Button
{
 public:
  IconButton(const std::string& s, const std::string& icon, const Event& ev);
  ~IconButton();
  const char* name() {return "IconButton";}
  void image(Image *image, bool takeOwnership=true);
  Image* image();
  void icon(const std::string& icon);
  void draw( Canvas& screen, const Rect& area );
  void align(int dir) { m_vertical=(dir==0); }
 protected:
  bool m_vertical;
  Image *m_icon;
};


class StockIconButton : public Button {
 public:
  StockIconButton(const std::string &label, enum StockIcon::Kind icon, const Event &ev);
  ~StockIconButton();
  void align(int dir) { m_vertical = (dir == 0); }
  void set(enum StockIcon::Kind icon) { m_icon = icon; }

  const char *name() { return "StockIconButton"; }
  void draw(Canvas &screen, const Rect &area);
 protected:
  enum StockIcon::Kind m_icon;
  bool m_vertical;
};


class RichText : public Label
{
 public:
  RichText(const std::string& s, const Font* f=NULL);
  RichText(unsigned char *s, size_t len, const Font* f=NULL);
  virtual void text( const std::string& s );
  virtual void draw( Canvas& screen, const Rect& area );
  int layout(int w);
 protected:
  struct Snippet {
    Vec2 pos;
    int textoff;
    int textlen;
    int align;
    const Font* font;
  };
  std::vector<Snippet> m_snippets;
  bool m_layoutRequired;
};

class WidgetParent : public Widget
{
 public:
  const char* name() {return "WidgetParent";}
  virtual void add( Widget* w, int x=-9999, int y=-9999 )=0;
  void add( Widget* w, const Vec2& pos ) {add(w,pos.x,pos.y);}
  void add( Widget* w, const Rect& pos ) {w->sizeTo(pos.size());add(w,pos.tl.x,pos.tl.y);}
  virtual void remove( Widget* w )=0;
};


class Container : public WidgetParent
{
 public:
  Container();
  ~Container();

  const char* name() {return "Container";}
  virtual std::string toString();
  virtual void move( const Vec2& by );
  virtual void onTick( int tick );
  virtual void draw( Canvas& screen, const Rect& area );
  virtual bool processEvent(ToolkitEvent &ev);
  virtual void onResize();

  virtual void add( Widget* w, int x=-9999, int y=-9999 );
  using WidgetParent::add;
  virtual void remove( Widget* w );
  virtual void empty();
 protected:
  std::vector<Widget*> m_children;
};

class Panel : public Container
{
  const char* name() {return "Panel";}
};

class Box : public Panel
{
 public:
  Box(int spacing=0, bool vertical=false);
  const char* name() {return "Box";}
  virtual void onResize();
  virtual void add( Widget* w, int wh, int grow );
  virtual void remove( Widget* w );
 protected:
  std::vector<int> m_sizes;
  std::vector<int> m_growths;
  int  m_spacing;
  bool m_vertical;
};

class VBox : public Box 
{
 public: 
 VBox(int spacing=0) : Box(spacing,true) {}
 const char* name() {return "VBox";}
};

class HBox : public Box
{
 public:
 HBox(int spacing=0) : Box(spacing,false) {}
 const char* name() {return "HBox";}
};

class Draggable : public Panel
{
 public:
  Draggable();
  const char* name() {return "Draggable";}
  bool processEvent(ToolkitEvent &ev);
  bool onPreEvent( Event& ev );
  bool onEvent( Event& ev );
  void onTick( int tick );
  void step( const Vec2& s ) { m_step = s; }
 protected:
  bool m_dragMaybe;
  bool m_dragging;
  Vec2 m_dragOrg;
  Vec2 m_step;
  Vec2 m_delta;
  bool m_internalEvent;
};

class ScrollArea : public Panel
{
 public:
  ScrollArea();
  const char* name() {return "ScrollArea";}
  bool onEvent( Event& ev );
  virtual void onResize();
  virtual void draw( Canvas& screen, const Rect& area );
  virtual void add( Widget* w, int x=-9999, int y=-9999 );
  using WidgetParent::add;
  virtual void remove( Widget* w );
  virtual void empty();

  virtual void virtualSize( const Vec2& size );
 protected:
  Canvas* m_canvas;
  Draggable* m_contents;
};

class MenuItem
{
public:
  MenuItem(const std::string& s, enum StockIcon::Kind icon=StockIcon::NONE, Event ev=Event::NOP)
  : text(s), icon(icon), event(ev) {}
  std::string text;
  enum StockIcon::Kind icon;
  Event event;
};

class Menu
{
 public:
  void addItems(const MenuItem* item);
  void addItem(const MenuItem& item);
  void addItem(const std::string& s, Event event=Event::NOP);
 protected:
  virtual void layout() =0;
  std::vector<MenuItem*> m_items;
};

class TabBook : public Panel
{
 public:
  TabBook();
  ~TabBook();
  const char* name() {return "TabBook";}
  virtual void onResize();
  virtual bool onEvent( Event& ev );
  virtual void draw( Canvas& screen, const Rect& area );

  virtual void addTab( const std::string &s, Widget* w );
  void selectTab( int t );
 private:
  int m_count, m_selected;
  std::vector<Widget*> m_tabs;
  std::vector<Widget*> m_panels;
  Widget* m_contents;
};

class Dialog : public Panel
{
 public:
  Dialog( const std::string &title="", Event left=Event::NOP, Event right=Event::NOP );
  const char* name() {return "Dialog";}
  void onTick( int tick );
  bool processEvent(ToolkitEvent &ev);
  bool onEvent( Event& ev );
  virtual void draw(Canvas &screen, const Rect &area);
  bool close();
  virtual Container* content() { return m_content; }
  Button* leftControl() { return m_left; }
  Button* rightControl() { return m_right; }
 protected:
  Label *m_title;
  Button *m_left, *m_right;
  Container *m_content;
  bool m_closeRequested;
};

class MenuDialog : public Dialog, public virtual Menu
{
 public:
  MenuDialog( Widget* evtarget, const std::string &title, const MenuItem* items=NULL );
  const char* name() {return "MenuDialog";}
  virtual bool onEvent( Event& ev );
 protected:
  virtual Widget* makeButton( MenuItem* item, const Event& ev );
  virtual void layout();
  Widget *m_target;
  Box *m_box;
  int m_columns;
  Vec2 m_buttonDim;
};

class MessageBox : public Dialog
{
 public:
  MessageBox( const std::string& text );
};

class Layer : public Dialog
{
 public:
  const char* name() {return "Layer";}
  virtual void onShow() {}
  virtual void onHide() {}
};



#endif //UI_H
