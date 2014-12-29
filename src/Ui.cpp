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

#include "Ui.h"
#include "Font.h"
#include "Canvas.h"
#include "Os.h"
#include "Config.h"
#include "Colour.h"

#include <iostream>
#include <algorithm>


static int indent = 0;

////////////////////////////////////////////////////////////////


Widget::Widget(WidgetParent *p) 
  : m_parent(p),
    m_eventMap(NULL),
    m_pos(0,0,2,2),
    m_focussed(false),
    m_alpha(0),
    m_fitToParent(false),
    m_greedyMouse(false),
    m_bg(NP::Colour::DEFAULT_BG),
    m_fg(NP::Colour::DEFAULT_FG),
    m_border(0),
    m_targetPos(0, 0),
    m_animating(false),
    m_animation_done([](){}),
    m_visible(true)
{}

std::string Widget::toString()
{
  char buf[32];
  char ind[] = "                                                         "
    "                                                         ";
  //fprintf(stderr,"[%d,%d-%d,%d]\n",m_pos.tl.x,m_pos.tl.y,m_pos.br.x,m_pos.br.y);
  sprintf(buf,"[%d,%d-%d,%d]\n",m_pos.tl.x,m_pos.tl.y,m_pos.br.x,m_pos.br.y);
  ind[indent] = '\0';
  std::string s(ind);
  s += name();
  s += buf;
  return s;
}

void Widget::move( const Vec2& by )
{
  m_pos.tl+=by;
  m_pos.br+=by;
}

void Widget::sizeTo( const Vec2& size )
{
  m_pos.br=m_pos.tl+size;  
  onResize();
}

bool Widget::processEvent(ToolkitEvent &ev)
{
  if (m_eventMap) { 
    Event e = m_eventMap->process(ev);
    if (e.code != Event::NOP) {
      return dispatchEvent(e);
    }
  }
  return false;
}

bool Widget::dispatchEvent( Event& ev )
{
  if (m_visible && onEvent(ev)) {
    //fprintf(stderr,"event %d consumed by %s\n", ev.code, name());    
    return true;
  } else if (m_parent) {
    //fprintf(stderr,"toparent %s event %d\n", m_parent->name(), ev.code);
    return m_parent->dispatchEvent(ev);
  }
  return false;
}

void Widget::setEventMap(EventMapType map)
{
  setEventMap( OS->getEventMap(map) );
}


void Widget::draw( Canvas& screen, const Rect& area )
{
  if (!m_visible) {
      return;
  }

  if ( m_alpha > 0 ) {
    Rect r = m_pos;
    if (!r.isEmpty()) {
      if ( m_focussed ) {
	screen.drawRect(m_pos,screen.makeColour(NP::Colour::SELECTED_BG));
      } else {
	screen.drawRect(m_pos,screen.makeColour(m_bg), true, m_alpha);
      }
    }
  }
  if (m_border) {
    screen.drawRect(m_pos,screen.makeColour(NP::Colour::TL_BORDER),false);
  }
}

void Widget::onTick(int tick)
{
    if (m_animating && m_pos.tl != m_targetPos) {
        const int RATE = 3;
        Vec2 diff = m_targetPos - m_pos.tl;
        if (Abs(diff.x) <= RATE && Abs(diff.y) <= RATE) {
            moveTo(m_targetPos);
            m_animating = false;
            m_animation_done();
        } else {
            moveTo((m_pos.tl*RATE+m_targetPos)/(RATE+1));
        }
    }
}


WidgetParent* Widget::topLevel()
{
  WidgetParent* p = parent();
  while (p && p->parent()) {
    p = p->parent();
  }
  return p;
}


////////////////////////////////////////////////////////////////


Label::Label()
    : Widget()
    , m_text()
    , m_font(nullptr)
    , m_alignment(Label::ALIGN_CENTER)
{
}


Label::Label(const std::string& s, const Font* f, int color)
  : m_text(s)
  , m_font(f?f:Font::blurbFont())
  , m_alignment(Label::ALIGN_CENTER)
{
    setFg(color);
}

void Label::text( const std::string& s )
{
    if (m_text != s) {
        m_text = s;
    }
}

void Label::draw( Canvas& screen, const Rect& area )
{
    if (!m_visible) {
        return;
    }

    Widget::draw(screen,area);

    bool align_h_center = ((m_alignment & (ALIGN_LEFT | ALIGN_RIGHT)) == 0);
    bool align_v_center = ((m_alignment & (ALIGN_TOP | ALIGN_BOTTOM)) == 0);

    if (align_h_center && align_v_center) {
        m_font->drawCenter( &screen, m_pos.centroid(), m_text, m_fg);
        return;
    }

    auto func = &Font::drawCenter;

    Vec2 pos = m_pos.centroid();
    Vec2 metrics = m_font->metrics(m_text);

    if ((m_alignment & ALIGN_TOP) != 0) {
        pos.y = m_pos.tl.y;
        if (align_h_center) {
            // Will use drawCenter for drawing, need to adjust vertical
            pos.y += metrics.y / 2;
        }
    }
    if ((m_alignment & ALIGN_BOTTOM) != 0) {
        pos.y = m_pos.br.y - metrics.y;
        if (align_h_center) {
            // Will use drawCenter for drawing, need to adjust vertical
            pos.y += metrics.y / 2;
        }
    }
    if ((m_alignment & ALIGN_LEFT) != 0) {
        pos.x = m_pos.tl.x;
        func = &Font::drawLeft;
        if (align_v_center) {
            // TODO: Adjustments needed?
        }
    }
    if ((m_alignment & ALIGN_RIGHT) != 0) {
        pos.x = m_pos.br.x;
        func = &Font::drawRight;
        if (align_v_center) {
            // TODO: Adjustments needed?
        }
    }

    (m_font->*func)(&screen, pos, m_text, m_fg);
}

////////////////////////////////////////////////////////////////


Button::Button(const std::string& s, Event selEvent)
  : Label(s),
    m_selEvent(selEvent)
{
  border(false);
  alpha(100);
  font(Font::headingFont());
  setEventMap(UI_BUTTON_MAP);
}


void Button::draw( Canvas& screen, const Rect& area )
{
    if (!m_visible) {
        return;
    }

    Label::draw(screen, m_pos);
    if (m_focussed) {
        //screen.drawRect(m_pos,screen.makeColour(NP::Colour::TL_BORDER),false);
    }
}

bool Button::onEvent( Event& ev )
{
  //fprintf(stderr,"Button::onEvent %d\n",ev.code);
  switch (ev.code) {
  case Event::SELECT:
    if (m_focussed) {
      // fprintf(stderr,"button press translate %d -> %d/%d,%d\n",
      //    ev.code,m_selEvent.code,m_selEvent.x,m_selEvent.y);
      m_focussed = false;
      onSelect();
      if (m_parent && m_selEvent.code != Event::NOP) {
	//fprintf(stderr,"button press event dispatch %d\n",m_selEvent.code);
	m_parent->dispatchEvent(m_selEvent);
      }
    }
    return true;
    break;
  case Event::FOCUS:
    if (m_pos.contains(Vec2(ev.x,ev.y))) {
      if (!m_focussed) {
	m_focussed = true;
	return true;
      }
    } else if (m_focussed) {
      m_focussed = false;
    }
  default: break;
  }
  return false;
}


////////////////////////////////////////////////////////////////


Icon::Icon(Image *image)
    : m_image(image)
{
    if (image) {
        sizeTo(Vec2(image->width(), image->height()));
    }
}

Icon::~Icon()
{
    delete m_image;
}

void Icon::image(Image *image)
{
    delete m_image;
    m_image = image;
}

void Icon::draw(Canvas &screen, const Rect &area)
{
    Widget::draw(screen, area);
    if (m_image) {
        screen.drawImage(*m_image, m_pos.tl.x, m_pos.tl.y);
    }
}


////////////////////////////////////////////////////////////////


StockIconButton::StockIconButton(const std::string &label, enum StockIcon::Kind icon, const Event &ev)
    : Button(label, ev)
    , m_icon(icon)
    , m_vertical(true)
{
}

StockIconButton::~StockIconButton()
{
}

void
StockIconButton::draw(Canvas &screen, const Rect &area)
{
    Widget::draw(screen, area);

    int spacing = 10;
    Vec2 textsize = m_font->metrics(m_text);
    Vec2 offset;

    if (m_text.empty()) {
        StockIcon::draw(screen, area, m_icon, m_pos.centroid());
        return;
    }

    if (m_vertical) {
        if (textsize.y + spacing + StockIcon::size() > m_pos.height()) {
            offset = Vec2(0, -textsize.y / 4);
            StockIcon::draw(screen, area, m_icon, m_pos.centroid() + offset);

            offset = Vec2(0, (StockIcon::size()) / 4);
            m_font->drawCenter(&screen, m_pos.centroid() + offset, m_text, m_fg);
        } else {
            offset = Vec2(0, -(textsize.y + spacing) / 2);
            StockIcon::draw(screen, area, m_icon, m_pos.centroid() + offset);

            offset = Vec2(0, (StockIcon::size() + spacing) / 2);
            m_font->drawCenter(&screen, m_pos.centroid() + offset, m_text, m_fg);
        }
    } else {
        Vec2 contentSize(StockIcon::size() + textsize.x,
                         std::max(StockIcon::size(), textsize.y));

        offset = Vec2(m_pos.height() / 2 - m_pos.width() / 2, 0);
        StockIcon::draw(screen, area, m_icon, m_pos.centroid() + offset);

        offset = Vec2(m_pos.height() - m_pos.width() / 2 + spacing, -textsize.y / 2);
        m_font->drawLeft(&screen, m_pos.centroid() + offset, m_text, m_fg);
    }
}


////////////////////////////////////////////////////////////////


IconButton::IconButton(const std::string& s, const std::string& icon, const Event& ev)
  : Button(s,ev)
  , m_vertical(true)
  , m_icon(icon.size() ? new Image(icon) : nullptr)
{
}

IconButton::~IconButton()
{
    delete m_icon;
}

void IconButton::image(Image *image, bool takeOwnership)
{
    delete m_icon;

    if (takeOwnership) {
        m_icon = image;
    } else {
        m_icon = new Image(*image);
    }
}

Image* IconButton::image()
{
    return m_icon;
}

void IconButton::icon(const std::string& icon)
{
    delete m_icon;
    m_icon = new Image(icon);
}

void IconButton::draw( Canvas& screen, const Rect& area )
{
  if (m_icon) {
    Widget::draw(screen,area);
    if (m_focussed) {
      screen.drawRect(m_pos,screen.makeColour(NP::Colour::SELECTED_BG),true);
    }
    Vec2 textsize = m_font->metrics(m_text);    
    if (m_vertical) {
      int x = m_pos.centroid().x - m_icon->width()/2; 
      int y = m_pos.centroid().y - m_icon->height()/2 - textsize.y/2; 
      if (y<m_pos.tl.y) y = m_pos.tl.y;
      screen.drawImage(*m_icon,x,y);
      x = m_pos.centroid().x;
      y += m_icon->height() + m_pos.height()/10;
      m_font->drawCenter( &screen, Vec2(x,y), m_text, m_fg);
    } else {
      int x = m_pos.tl.x + 10;
      int y = m_pos.centroid().y - m_icon->height()/2;
      screen.drawImage(*m_icon,x,y);
      x += m_icon->width() + 10;
      y = m_pos.centroid().y - textsize.y/2;
      m_font->drawLeft( &screen, Vec2(x,y), m_text, m_fg);
    }
  } else {
    Button::draw(screen,area);
  }
}


////////////////////////////////////////////////////////////////


RichText::RichText(const std::string& s, const Font* f)
  : Label(s,f),
    m_layoutRequired(true)
{}

RichText::RichText(unsigned char *s, size_t len, const Font* f)
  : Label(std::string((const char*)s, len),f),
    m_layoutRequired(true)
{}

void RichText::text( const std::string& s )
{
  Label::text(s);
  m_layoutRequired = true;
}

void RichText::draw( Canvas& screen, const Rect& area )
{
  Widget::draw(screen,area);
  if (m_layoutRequired) {
    layout(m_pos.width()-20);
    m_layoutRequired = false;
  }
  for (int l=0; l<m_snippets.size(); l++) {
    if (m_snippets[l].textlen > 0) {
      Vec2 pos = m_pos.tl + m_snippets[l].pos;
      Vec2 posnext = l==m_snippets.size()-1 ? pos:m_pos.tl+m_snippets[l+1].pos;
      if (pos.y < area.br.y && posnext.y >= area.tl.y ) {
	std::string sniptext = m_text.substr(m_snippets[l].textoff,
					     m_snippets[l].textlen);
	switch (m_snippets[l].align) {
	case 1:
	  pos.x += (m_pos.width()-20-m_snippets[l].font->metrics(sniptext).x)/2;
	  break;
	case 2:
	  pos.x += m_pos.width()-20-m_snippets[l].font->metrics(sniptext).x;
	  break;
	}
	m_snippets[l].font->drawLeft( &screen, pos, sniptext, m_fg);
      }
    }
  }
}

int RichText::layout(int w)
{
  struct Tag {
    Tag(const std::string& str, size_t begin, size_t end)
      : m_str(str), m_closed(false), m_begin(begin), m_end(end)
    {
      if (str[m_begin]=='<') {m_begin++;}
      if (str[m_begin]=='/') {m_closed=true; m_begin++;}
      if (str[m_end]=='/') {m_closed=true; m_end--;}
      m_tag = m_str.substr(m_begin,m_str.find_first_of(" \t/>",m_begin)-m_begin);
    }
    const std::string& tag() {
      return m_tag;
    }
    std::string prop(const std::string& name) {
      std::string mark(" ");
      std::string value;
      mark += name + '=';
      if (m_str.find(mark) != std::string::npos) {
	char term = ' ';
	const char *p = m_str.c_str() + m_str.find(mark) + mark.length();
	if (term==' ' && (*p=='\'' || *p=='"')) { term = *p++; }
	while (term!=' ' ? (*p != term) : (*p!='/' && *p!='>')) {
	  value += *p++;
	}
      }
      return value;
    }
    bool closed() { return m_closed; }
    const std::string& m_str;
    bool m_closed;
    size_t m_begin, m_end;
    std::string m_tag;
  };

  const int margin = 10;
  w -= margin*2;
  int x = margin, y = 0, l = 0, h = 0;
  size_t p=0;
  int spacewidth = m_font->metrics(" ").x;
  Snippet snippet = {Vec2(x,y),0,0,0,m_font};
  Vec2 wordmetrics;
  m_snippets.clear();
  m_snippets.push_back(snippet);
  //fprintf(stderr,"layout w=%d \"%s\"\n",w,m_text.c_str());

  while (p != std::string::npos) {
    bool newline = false;
    size_t e = m_text.find_first_of(" \t\n\r<>", p); 

    if (e==std::string::npos) {
      wordmetrics = snippet.font->metrics(m_text.substr(p,e));
    } else {
      wordmetrics = snippet.font->metrics(m_text.substr(p,e-p));
    }
    //fprintf(stderr,"word \"%s\" w=%d\n",m_text.substr(p,e-p).c_str(),wordmetrics.x);
    if (x!=margin) {
      // space
      wordmetrics.x += spacewidth;
    }
    if (x==margin || x+wordmetrics.x < w) {
      // add to snippet
      p = e;
      x += wordmetrics.x;
    } else {
      e = p;
      newline = true;
    }

    if (e!=std::string::npos && m_text[e]=='<') {
      size_t f = m_text.find('>',e);
      Tag tag(m_text,e,f);
      //fprintf(stderr,"got tag \"%s\"\n",tag.tag().c_str());
      if (tag.tag() == "H1") {
	newline = true;
	if (tag.closed()) {
	  snippet.font = m_font;
	  y += snippet.font->height()/2;
	} else {
	  snippet.font = Font::titleFont();
	  h += snippet.font->height();
	}
      } else if (tag.tag() == "H2") {
	newline = true;
	if (tag.closed()) {
	  snippet.font = m_font;
	  y += snippet.font->height()/2;
	} else {
	  snippet.font = Font::headingFont();
	  h += snippet.font->height();
	}
      } else if (tag.tag() == "BR") {
	newline = true;
      } else if (tag.tag() == "P") {
	newline = true;
	std::string align = tag.prop("align");
	if (align=="center") snippet.align=1;
	else if (align=="right") snippet.align=2;
	else snippet.align=0;
      } else if (tag.tag() == "LI") {
	newline = true;
	x += margin;
      } else if (tag.tag() == "IMG") {
      }
      //fprintf(stderr,"skip %d chars \n",f+1-e);
      e = f + 1;
    }

    if (newline) {
      //line break;
      int len = p - m_snippets[l].textoff;
      if (len > 0) {
	m_snippets[l].textlen = len > 0 ? len : 0;
	m_snippets[l].pos.y = y;
	snippet.textoff = e;
	m_snippets.push_back(snippet);
	//fprintf(stderr,"new line %d w=%d, \"%s\"\n", y, x+wordmetrics.x,
	// m_text.substr(m_snippets[l].textoff,m_snippets[l].textlen).c_str());
	y += m_snippets[l].font->height();
	l++;
	x = margin + indent;
	h = 0;
      } else {
	m_snippets[l] = snippet;
	m_snippets[l].textoff = e;
	m_snippets[l].pos.x = x;
      }
    }

    p=e;
    if (p==std::string::npos) {
      m_snippets[l].textlen = m_text.length() - m_snippets[l].textoff;
      m_snippets[l].pos = Vec2(0,y);
      y += m_snippets[l].font->height();
      //fprintf(stderr,"last line %d \"%s\"\n", y,
      //      m_text.substr(m_snippets[l].textoff,m_snippets[l].textlen).c_str());
    } else {
      while (m_text[p] == ' '
	     || m_text[p] == '\n'
	     || m_text[p] == '\r'
	     || m_text[p] == '\t') {
	//eat whitespace
	if (p==m_snippets[l].textoff) {
	  m_snippets[l].textoff++;
	}
	p++;
      }
    }
    
  }
  return y;
}

////////////////////////////////////////////////////////////////


Draggable::Draggable()
  : m_dragMaybe(false),
    m_dragging(false),
    m_step(2,2),
    m_delta(0,0),
    m_internalEvent(false)
{
  setEventMap(UI_DRAGGABLE_MAP);
}

bool Draggable::processEvent(ToolkitEvent &ev)
{
  // get in before our children
  if (!m_internalEvent && m_eventMap) { 
    Event e = m_eventMap->process(ev);
    if (e.code != Event::NOP) {
      if (onPreEvent(e)) {
          return true;
      }
    }
  }
  // normal processing
  return Container::processEvent(ev);
}

bool Draggable::onPreEvent( Event& ev )
{
  //fprintf(stderr,"draggable event %d %d,%d\n",ev.code,ev.x,ev.y);      
  switch (ev.code) {
  case Event::MOVEBEGIN:
    m_dragMaybe = true;
    m_dragOrg = Vec2(ev.x,ev.y);
    return true;
  case Event::MOVEMORE:
    if (m_dragMaybe && !m_dragging
	&& ( Abs(ev.x-m_dragOrg.x) >= CLICK_TOLERANCE
	     || Abs(ev.y-m_dragOrg.y) >= CLICK_TOLERANCE ) ) {
      m_dragging = true;
    }
    if (m_dragging) {
      m_delta = Vec2(ev.x,ev.y)-m_dragOrg;
      m_delta.x = m_step.x ? m_delta.x : 0;
      m_delta.y = m_step.y ? m_delta.y : 0;
      move( m_delta );
      m_dragOrg = Vec2(ev.x,ev.y);
      return true;
    }
    break;
  case Event::MOVEEND:
    if (m_dragging) {
      m_dragMaybe = false;
      m_dragging = false;
      return true;
    } else {
      //translate into a raw click
      int finger = 0; // primary finger
      int button = 1; // left button
      ToolkitEvent e(ToolkitEvent::PRESS, ev.x, ev.y, finger, button);
      m_internalEvent = true;
      processEvent(e);
      e.type = ToolkitEvent::RELEASE;
      bool result = processEvent(e);
      m_internalEvent = false;
      return result;
    }
    break;
  default:
    /* do nothing */
    break;
  }
  return false;
}

bool Draggable::onEvent( Event& ev )
{
  //fprintf(stderr,"draggable event %d %d,%d\n",ev.code,ev.x,ev.y);      
  switch (ev.code) {
  case Event::UP:
    m_dragging = m_dragMaybe = false;
    move(Vec2(0,m_step.y));
    return true;
  case Event::DOWN:
    m_dragging = m_dragMaybe = false;
    move(Vec2(0,-m_step.y));
    return true;
  case Event::LEFT:
    m_dragging = m_dragMaybe = false;
    move(Vec2(-m_step.y,0));
    return true;
  case Event::RIGHT:
    m_dragging = m_dragMaybe = false;
    move(Vec2(m_step.x,0));
    return true;
  default: break;
  }
  return Panel::onEvent(ev);
}


void Draggable::onTick( int tick )
{
  if (!m_dragging && (m_delta.x != 0 || m_delta.y != 0)) {
    //fprintf(stderr, "Draggable::onTick glide %d, %d\n",m_delta.x,m_delta.y);
    move(m_delta);
    m_delta = m_delta * 50 / 51;
  }
}


////////////////////////////////////////////////////////////////


class ScrollContents : public Draggable
{
public:
  ScrollContents()
  {
  }
};

ScrollArea::ScrollArea()
  : m_canvas(NULL)
{
  m_contents = new ScrollContents();
  m_contents->step(Vec2(0,5));
  Container::add(m_contents,0,0);
}
  

bool ScrollArea::onEvent( Event& ev )
{
  return Panel::onEvent(ev);
}

void ScrollArea::onResize()
{
  delete m_canvas;
  m_canvas = new Canvas(m_pos.size().x, m_pos.size().y);
}

void ScrollArea::virtualSize( const Vec2& size )
{
  m_contents->sizeTo(size);
}

void ScrollArea::draw( Canvas& screen, const Rect& area )
{
  Rect cpos = m_contents->position();
  if (cpos.tl.y > m_pos.tl.y) {
    m_contents->moveTo(Vec2(cpos.tl.x,m_pos.tl.y));
  }
  if (cpos.br.y < m_pos.br.y && cpos.height() > m_pos.height()) {
    m_contents->moveTo(Vec2(cpos.tl.x,m_pos.br.y - cpos.size().y));
  }
  Container::draw(screen,area);
}

void ScrollArea::add( Widget* w, int x, int y )
{
  m_contents->add(w,x,y);
}

void ScrollArea::remove( Widget* w )
{
  m_contents->remove(w);
}

void ScrollArea::empty()
{
  m_contents->empty();
}



////////////////////////////////////////////////////////////////


Container::Container()
{}

Container::~Container()
{
  for (int i=0; i<m_children.size(); ++i) {
    delete m_children[i];
  }
}
  
std::string Container::toString()
{
  std::string s = Widget::toString();
  indent++;
  for (int i=0; i<m_children.size(); ++i) {
    s += m_children[i]->toString();
  }
  indent--;
  return s;
}

void Container::move( const Vec2& by )
{
  WidgetParent::move(by);
  for (int i=0; i<m_children.size(); ++i) {
    m_children[i]->move(by);
  }  
}

void Container::onTick( int tick )
{
  for (int i=0; i<m_children.size(); ++i) {
    m_children[i]->onTick(tick);
  }
  WidgetParent::onTick(tick);
}

void Container::draw( Canvas& screen, const Rect& area )
{
  WidgetParent::draw(screen,area);
  for (int i=0; i<m_children.size(); ++i) {
    if (m_children[i]->position().intersects(area)) {
      Rect relArea = area;
      relArea.clipTo(m_children[i]->position());
      m_children[i]->draw(screen, relArea);
    }
  }
}

bool Container::processEvent(ToolkitEvent &ev)
{
  if (ev.type == ToolkitEvent::PRESS ||
          ev.type == ToolkitEvent::RELEASE ||
          ev.type == ToolkitEvent::MOVE) {
    if (ev.finger != 0) {
        /**
         * Ignore button events for multi-touch fingers if it
         * isn't the first finger (which == 0). This prevents
         * "cool" drawing effects on Harmattan.
         *
         * TODO: Make it cool by allowing multiple lines to be
         * drawn simultaneously (like NuMTyPYsics ;)
         **/
        return true;
    }
  }

  switch (ev.type) {
      case ToolkitEvent::PRESS:
      case ToolkitEvent::RELEASE:
          for (int i=m_children.size()-1; i>=0; --i) {
              Widget *c = m_children[i];
              if (c->greedyMouse() || c->position().contains(ev.pos())) {
                  if (c->processEvent(ev)) {
                      return true;
                  }
              }
          }
          break;
      default:
          for (int i=m_children.size()-1; i>=0; --i) {
              //fprintf(stderr," ev to: %s\n",m_children[i]->toString().c_str());
              if (m_children[i]->processEvent(ev)) {
                  return true;
              }
          }
          break;
  }
  return Widget::processEvent(ev);
}

void Container::onResize()
{
  for (int i=0; i<m_children.size(); ++i) {
    if (m_children[i]->fitToParent()) {
      m_children[i]->moveTo(m_pos.tl);
      m_children[i]->sizeTo(m_pos.size());
    }
  }
}

void Container::add( Widget* w, int x, int y )
{
  if (x!=-9999 && y!=-9999) {
    Vec2 pos = m_pos.tl+Vec2(x,y);
    w->moveTo(pos);
  }
  w->setParent(this);
  m_children.push_back(w);
  onResize();
}

void Container::remove( Widget* w )
{
    if (!w) {
        return;
    }

    auto it = std::find(m_children.begin(), m_children.end(), w);
    if (it != m_children.end()) {
        w->setParent(nullptr);
        m_children.erase(it);
        delete w;
    }
}

void Container::empty()
{
    for (int i=m_children.size()-1; i>=0; i--) {
        remove(m_children[i]);
    }
}

////////////////////////////////////////////////////////////////


Box::Box(int spacing, bool vertical)
  : m_spacing(spacing),
    m_vertical(vertical)
{
  fitToParent(true);
}

void Box::onResize()
{
  int totalw=-m_spacing, totalg=0;
  for (int i=0; i<m_sizes.size(); ++i) {
    totalw += m_sizes[i] + m_spacing;
    totalg += m_growths[i];
  }

  int extra = (m_vertical ? m_pos.height() : m_pos.width()) - totalw;
  extra = extra > 0 ? extra : 0;
  Vec2 org(m_pos.tl);

  for (int i=0; i<m_sizes.size(); ++i) {
    //fprintf(stderr,"box hild %d at %d,%d\n",i,org.x,org.y);
    m_children[i]->moveTo(org);
    int incr = totalg>0 ? m_growths[i] * extra / totalg : 0;
    if (m_vertical) {
      m_children[i]->sizeTo(Vec2(m_pos.width(),m_sizes[i]+incr));
      org.y += m_sizes[i] + incr + m_spacing;
    } else {
      m_children[i]->sizeTo(Vec2(m_sizes[i]+incr,m_pos.height()));
      org.x += m_sizes[i] + incr + m_spacing;
    }
  }
}

void Box::add( Widget* w, int dim, int grow )
{
  Panel::add(w,0,0);
  m_sizes.push_back(dim);
  m_growths.push_back(grow);
  onResize();
}

void Box::remove( Widget* w )
{
    int index = indexOf(m_children, w);
    m_sizes.erase(m_sizes.begin() + index);
    m_growths.erase(m_growths.begin() + index);
    Panel::remove(w);
    onResize();
}


////////////////////////////////////////////////////////////////


void Menu::addItems(const MenuItem* item)
{
  while (item && item->event.code != Event::NOP) {
    m_items.push_back(new MenuItem(item->text, item->icon, item->event));
    item++;
  }
  layout();
}

void Menu::addItem(const MenuItem& item)
{
  m_items.push_back(new MenuItem(item.text, item.icon, item.event));
  layout();
}

void Menu::addItem(const std::string& s, Event event)
{
  addItem(MenuItem(s, StockIcon::NONE, event));
}


////////////////////////////////////////////////////////////////


TabBook::TabBook()
  : m_count(0),
    m_selected(-1),
    m_contents(NULL)
{}

TabBook::~TabBook()
{
  selectTab(-1);
  for (int i=0; i<m_panels.size(); ++i) {
    delete m_panels[i];
  }
}

void TabBook::onResize()
{
  for (int i=0; i<m_count; i++) {
    m_tabs[i]->moveTo(Vec2((2*i-m_count)*TAB_WIDTH/2+m_pos.centroid().x,m_pos.tl.y));
  }
}

bool TabBook::onEvent( Event& ev )
{
  if (ev.code == Event::SELECT && ev.y==-1) {
    selectTab(ev.x);
    return true;
  }
  return Panel::onEvent(ev);
}

void TabBook::draw( Canvas& screen, const Rect& area )
{
  Panel::draw(screen,area);
  Rect border = m_pos;
  border.tl.y += TAB_HEIGHT;
  screen.drawRect(border,screen.makeColour(NP::Colour::TL_BORDER),false);
}

void TabBook::addTab( const std::string &s, Widget* w )
{
  Widget* tab = new Button(s,Event(Event::SELECT,m_count,-1));
  add( tab, Rect(m_count*TAB_WIDTH,0,
		 (m_count+1)*TAB_WIDTH,TAB_HEIGHT) );
  m_tabs.push_back(tab);
  m_panels.push_back(w);
  m_count++;
  if ( m_selected < 0 ) {
    selectTab(m_count-1);
  }
}

void TabBook::selectTab( int t )
{
  if (m_contents) {
    m_tabs[m_selected]->setBg(NP::Colour::DEFAULT_BG);
    m_children.erase(std::find(m_children.begin(), m_children.end(), m_contents));
    m_contents = NULL;
  }
  if ( t>=0 && t<m_count ) {
    m_selected = t;
    m_tabs[m_selected]->setBg(NP::Colour::SELECTED_BG);
    m_contents = m_panels[t];
    Rect area(Vec2(1,TAB_HEIGHT+1),m_pos.size()-Vec2(1,1));
    add(m_contents,area);
  }
}


////////////////////////////////////////////////////////////////


Dialog::Dialog( const std::string &title, Event left, Event right )
{
  setEventMap(UI_DIALOG_MAP);
  setFg(0x000000);
  alpha(100);
  m_greedyMouse = true;
  m_title = new Label(title,Font::titleFont());
  m_title->alpha(0);
  m_content = new Panel();
  m_left = m_right = NULL;
  m_closeRequested = false;
  VBox *all = new VBox();
  if (title.length() > 0) {
    HBox *bar = new HBox();
    if (left.code!=Event::NOP) {
      m_left = new Button("<>",left);
      bar->add(m_left, 100, 0);
    }
    bar->add(m_title, 64, 1);
    if (right.code!=Event::NOP) {
      m_right = new StockIconButton("", StockIcon::CLOSE, right);
      bar->add(m_right, 100, 0);
    }
    all->add(bar, DIALOG_TITLE_HEIGHT, 0);
  }
  all->add(m_content, 40, 1);
  add(all,0,0);
}


void Dialog::onTick( int tick )
{
  if (m_closeRequested) {
    m_parent->remove(this);
    return;
  }

  Panel::onTick(tick);
}

bool Dialog::processEvent(ToolkitEvent &ev)
{
  if (ev.type == ToolkitEvent::RELEASE
      && !m_pos.contains(ev.pos())) {
    // click outside the dialog to close
    Event closeEvent(Event::CLOSE);
    return onEvent(closeEvent);
  }
  // dialogs eat all raw events...
  Panel::processEvent(ev);
  return true;
}

bool Dialog::onEvent( Event& ev )
{
  //fprintf(stderr,"dialog event %d\n",ev.code);      
  if ( ev.code == Event::CLOSE ) {
    close();
    return true;
  }
  return Panel::onEvent(ev);
}


void Dialog::draw(Canvas &screen, const Rect &area)
{
    Window *window = dynamic_cast<Window *>(&screen);
    if (window) {
        RenderTarget tmp(m_pos.width() / 2, m_pos.height() / 2);
        tmp.begin();

        Image *offscreen = window->offscreen();
        Rect r2(0, 0, screen.width(), screen.height());
        tmp.drawBlur(*offscreen, m_pos, r2, 0.0, 0.7);

        tmp.end();

        Image image(tmp.contents());
        Rect r(0, 0, tmp.width(), tmp.height());
        screen.drawBlur(image, r, m_pos, 0.7, 0.0);
    } else {
        screen.drawRect(area, 0xff0000);
    }
    screen.drawRect(m_pos, 0xffffff, true, 100);
    Panel::draw(screen, area);
}

bool Dialog::close()
{
  if (m_parent) {
    //fprintf(stderr,"close dialog\n");    
    m_closeRequested = true;
  }
  return false; // ??
}


////////////////////////////////////////////////////////////////


MenuDialog::MenuDialog( Widget* evtarget, const std::string &title, const MenuItem* items )
  : Dialog(title),
    m_target(evtarget),
    m_columns(MENU_COLUMNS),
    m_buttonDim(BUTTON_WIDTH,BUTTON_HEIGHT)
{
  m_box = new VBox(BUTTON_SPACING);
  m_box->fitToParent(true);
  content()->add( m_box, 0, 0 );
  moveTo(Vec2(SCREEN_WIDTH-24, 0));
  sizeTo(Vec2(m_buttonDim.x*MENU_COLUMNS+24,200));
  animateTo(Vec2(SCREEN_WIDTH-m_buttonDim.x*MENU_COLUMNS-24, 0));
  if (items) {
    addItems(items);
  }
}

bool MenuDialog::onEvent( Event& ev )
{
  if (ev.code == Event::SELECT
      && ev.x >= 0 && ev.x < m_items.size()
      && ev.y == -777
      && m_target
      && m_target->dispatchEvent(m_items[ev.x]->event)) {
    //fprintf(stderr,"MenuDialog event translate[%d] -> %d\n",ev.x,m_items[ev.x]->event.code);    
    Event closeEvent(Event::CLOSE);
    onEvent(closeEvent);
    return true;
  }
  return Dialog::onEvent(ev);
}

Widget* MenuDialog::makeButton( MenuItem* item, const Event& ev )
{
  //fprintf(stderr,"MenuDialog::makeButton %s\n",item->text.c_str());
  return new Button(item->text,ev);
}

void MenuDialog::layout()
{
  m_box->empty();
  m_box->add( new Spacer(), 1, 4 );
  Box *row = NULL;
  for (int i=0; i<m_items.size(); i++) {
    if ((i%m_columns)==0) {
      if (row) row->add( new Spacer(), 1, 1 );
      row = new HBox(BUTTON_SPACING);
      row->add( new Spacer(), 1, 1 );
      m_box->add(row,m_buttonDim.y,0);
    }
    row->add( makeButton(m_items[i], Event(Event::SELECT,i,-777)), m_buttonDim.x,0);
  }  
  if (row) {
    if (m_items.size()%m_columns > 0) {
      for (int i=0; i<m_columns-(m_items.size()%m_columns); i++) {
	row->add( new Spacer(), m_buttonDim.x, 0 );
      }
    }
    row->add( new Spacer(), 1, 1 );
  }
  m_box->add( new Spacer(), 1, 1 );
  int h = (m_items.size()+m_columns-1)/m_columns
    * (m_buttonDim.y+BUTTON_SPACING) + DIALOG_TITLE_HEIGHT + 20;
  sizeTo( Vec2(m_pos.width(), h) );
}



////////////////////////////////////////////////////////////////


MessageBox::MessageBox( const std::string& text )
{
  Box * vbox = new VBox();
  RichText *rt = new RichText(text);
  vbox->add( rt, 100, 100 );
  content()->add( vbox, 0, 0 );
  sizeTo(Vec2(300,150));
  animateTo(Vec2(250,100));
}


////////////////////////////////////////////////////////////////


void
StockIcon::draw(Canvas &screen, const Rect &area, enum Kind kind, const Vec2 &pos)
{
    if (kind < 0 || kind >= ICON_COUNT) {
        std::cerr << "Invalid icon kind!" << std::endl;
        return;
    }

    constexpr int COLUMNS = 4;
    constexpr int ROWS = 3;

    constexpr int BORDER = 10;
    constexpr int SIZE = 90;

    int column = kind % COLUMNS;
    int row = kind / COLUMNS;

    assert(row < ROWS);

    int x = BORDER + column * (SIZE + BORDER);
    int y = BORDER + row * (SIZE + BORDER);

    Rect src(x, y, x+SIZE, y+SIZE);
    Rect dst(pos.x - size() / 2, pos.y - size() / 2,
             pos.x + size() / 2, pos.y + size() / 2);

    Image icons("icons.png", true);
    screen.drawAtlas(icons, src, dst);
}

int
StockIcon::size()
{
    return 80;
}
