/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * Coyright (c) 2009, 2010 Tim Edmonds <numptyphysics@gmail.com>
 * Coyright (c) 2012, 2014, 2015 Thomas Perl <m@thp.io>
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
 */

#include "Dialogs.h"
#include "Ui.h"
#include "Canvas.h"
#include "Font.h"
#include "Config.h"
#include "Game.h"
#include "Scene.h"
#include "Colour.h"
#include "I18n.h"

#include "petals_log.h"
#include "thp_format.h"

#include <vector>
#include <algorithm>


////////////////////////////////////////////////////////////////


struct MenuPage : public Panel
{
  MenuPage(bool closeable=false)
  {
    alpha(100);
    Box *vbox = new VBox();
    m_content = new Panel();
    vbox->add( m_content, 100, 1 );
    add(vbox);
    fitToParent(true);
  }
  const char* name() {return "MenuPage";}
  Panel *m_content;
};

class LevelLauncher : public Dialog
{
public:
  LevelLauncher(int l, Image *image)
      : Dialog(Tr(nullptr))
  {
    Box *vbox1 = new VBox();
    vbox1->add( new Spacer(),  100, 1 );
    Box *hbox = new HBox();
    hbox->add( new Spacer(),  10, 2 );
    IconButton *icon = new IconButton(Tr("level"), "", Event::NOP);
    if (image) {
        icon->image(image, false);
    }
    hbox->add( icon, 300, 0 );
    hbox->add( new Spacer(),  10, 1 );
    Box *vbox = new VBox();
    vbox->add( new Spacer(),  10, 1 );
    vbox->add( new IconButton(Tr("Review"),"",
			      Event(Event::REPLAY,l)),
			      BUTTON_HEIGHT, 1 );
    vbox->add( new Spacer(),  10, 0 );
    vbox->add( new IconButton(Tr("Play"),"",
			      Event(Event::PLAY,l)),
			      BUTTON_HEIGHT, 1 );
    vbox->add( new Spacer(),  10, 1 );
    hbox->add( vbox, BUTTON_WIDTH, 0 );
    hbox->add( new Spacer(),  10, 2 );
    vbox1->add(hbox, 200, 0);
    vbox1->add( new Spacer(),  100, 1 );
    content()->add(vbox1);
    sizeTo(Vec2(WORLD_WIDTH,WORLD_HEIGHT));
    moveTo(Vec2(0,0));
    animateTo(Vec2(0,0));
  }
};

class LevelSelector : public MenuPage
{
  static const int THUMB_COUNT = 32;
  GameControl* m_game;
  Levels* m_levels;
  int m_collection;
  int m_dispbase;
  int m_dispcount;
  IconButton* m_thumbs[THUMB_COUNT];
  ScrollArea* m_scroll;
public:
  LevelSelector(GameControl* game, int initialLevel)
    : m_game(game),
      m_levels(game->m_levels),
      m_collection(0),
      m_dispbase(0),
      m_dispcount(0)
  {
    m_scroll = new ScrollArea();
    m_scroll->fitToParent(true);
    m_scroll->virtualSize(Vec2(WORLD_WIDTH,WORLD_HEIGHT));

    m_content->add(m_scroll,0,0);
    fitToParent(true);

    m_collection = m_levels->collectionFromLevel(initialLevel);
    setCollection(m_collection);
  }
  void setCollection(int c)
  {
    if (c < 0 || c >=m_levels->numCollections()) {
      return;
    }    
    m_collection = c;
    m_dispbase = 0;
    m_dispcount = m_levels->collectionSize(c);
    m_scroll->virtualSize(Vec2(WORLD_WIDTH,150+(WORLD_HEIGHT/ICON_SCALE_FACTOR+40)*((m_dispcount+2)/3)));

    m_scroll->empty();
    Box *vbox = new VBox();
    vbox->add( new Spacer(),  10, 0 );
    Box *hbox = new HBox();
    Widget *w = new Button(Tr::copy("<<"),Event::PREVIOUS);
    w->border(false);
    hbox->add( w, BUTTON_WIDTH, 0 );
    hbox->add( new Spacer(), 10, 0 );
    Label *title = new Label(Tr::copy(m_levels->collectionName(c)));
    title->font(Font::headingFont());
    title->alpha(100);
    hbox->add( title, BUTTON_WIDTH, 4 );
    w= new Button(Tr::copy(">>"),Event::NEXT);
    w->border(false);    
    hbox->add( new Spacer(), 10, 0 );
    hbox->add( w, BUTTON_WIDTH, 0 );
    vbox->add( hbox, 64, 0 );
    vbox->add( new Spacer(),  10, 0 );

    hbox = new HBox();
    hbox->add( new Spacer(),  0, 1 );
    int accumw = 0;
    for (int i=0; i<m_dispcount; i++) {
      accumw += WORLD_WIDTH / ICON_SCALE_FACTOR + 10;
      if (accumw >= WORLD_WIDTH) {
	vbox->add(hbox, WORLD_HEIGHT/ICON_SCALE_FACTOR+30, 4);
	vbox->add( new Spacer(),  10, 0 );
	hbox = new HBox();
	hbox->add( new Spacer(),  0, 1 );
	accumw = WORLD_WIDTH / ICON_SCALE_FACTOR;
      }
      m_thumbs[i] = new IconButton(Tr::copy("--"),"",Event(Event::PLAY, //SELECT,
						 m_levels->collectionLevel(c,i)));
      m_thumbs[i]->font(Font::blurbFont());
      m_thumbs[i]->setBg(NP::Colour::SELECTED_BG);
      m_thumbs[i]->border(false);
      hbox->add( m_thumbs[i],  WORLD_WIDTH / ICON_SCALE_FACTOR, 0 );
      hbox->add( new Spacer(), 0, 1 );
    }
    vbox->add(hbox, WORLD_HEIGHT/ICON_SCALE_FACTOR+30, 4);
    vbox->add( new Spacer(), 110, 10 );
    m_scroll->add(vbox,0,0);

    for (int i=0; i<THUMB_COUNT && i+m_dispbase<m_dispcount; i++) {
      Scene scene( true );
      int level = m_levels->collectionLevel(c,i);
      if (scene.load(m_levels->load(level))) {
          auto size = Vec2(WORLD_WIDTH, WORLD_HEIGHT);
          RenderTarget temp(size, Rect(Vec2(0, 0), size));

          temp.begin();
          scene.draw(temp, true);
          temp.end();

          m_thumbs[i]->text( Tr::copy(m_levels->levelName(level)) );
          Image *image = new Image(temp.contents());
          image->scale(1. / ICON_SCALE_FACTOR);
          m_thumbs[i]->image(image);
      }
    }
  }
  bool onEvent(Event& ev)
  {
    switch (ev.code) {
    case Event::PREVIOUS:
      setCollection(m_collection-1);
      return true;
    case Event::NEXT:
      setCollection(m_collection+1);
      return true;
//     case Event::SELECT:
//       for (int i=0; i<THUMB_COUNT && i+m_dispbase<m_dispcount; i++) {
// 	m_thumbs[i]->transparent(true);
//       }
//       m_thumbs[m_dispbase+ev.x]->transparent(false);
//       m_game->gotoLevel(m_levels->collectionLevel(m_collection,m_dispbase+ev.x));
//       add( new LevelLauncher(m_levels->collectionLevel(m_collection,m_dispbase+ev.x), m_thumbs[m_dispbase+ev.x]->image()) );
//       //Event closeEvent(Event::CLOSE);
//       //m_parent->dispatchEvent(closeEvent);
//       return true;
       default:
      /* do nothing */
        break;
    }
    return MenuPage::onEvent(ev);
  }
};

class HelpPage : public MenuPage
{
public:
  HelpPage()
  {
    Box *vbox = new VBox();
    ScrollArea* scroll = new ScrollArea();
    scroll->fitToParent(true);
    // TODO: Make help_text.html translatable
    RichText *text = new RichText(Tr::copy(Config::readFile("help_text.html")));
    scroll->virtualSize(Vec2(WORLD_WIDTH,text->layout(WORLD_WIDTH)));
    text->fitToParent(true);
    text->alpha(100);
    scroll->add(text,0,0);
    vbox->add( scroll, 0, 1 );
    vbox->add( new Button(Tr::copy(PROJECT_HOMEPAGE),Event::SELECT), 36, 0 );
    m_content->add(vbox,0,0);
  }
  bool onEvent(Event& ev)
  {
    if (ev.code == Event::SELECT) {
      OS->openBrowser(PROJECT_HOMEPAGE);
      return true;
    }
    return Panel::onEvent(ev);
  }
};



struct FrontPage : public MenuPage
{
  FrontPage() : MenuPage(true)
  {
    m_content->add( new StockIconButton(Tr("CHOOSE"), StockIcon::CHOOSE, Event(Event::MENU,1)),
		    Rect(125,100,275,300) );
    m_content->add( new StockIconButton(Tr("PLAY"), StockIcon::PLAY, Event(Event::MENU,2)),
		    Rect(325,100,475,300) );
    m_content->add( new StockIconButton(Tr("HELP"), StockIcon::HELP, Event(Event::MENU,3)),
		    Rect(525,100,675,300) );
    fitToParent(true);
  }
};


class MainMenu : public Dialog
{
  GameControl* m_game;
  int          m_chosenLevel;
public:
  MainMenu(GameControl* game)
    : Dialog(Tr("NUMPTY PHYSICS"),Event::NOP,Event::QUIT),
      m_game(game),
      m_chosenLevel(game->m_level)
  {
    //Swipe::lock(false);
    content()->add(new FrontPage());
    sizeTo(Vec2(WORLD_WIDTH,WORLD_HEIGHT));
  }
  ~MainMenu() {
    //Swipe::lock(true);
  }
  bool onEvent( Event& ev )
  {
    switch (ev.code) {
    case Event::MENU:
      switch(ev.x) {
      case 1:
	content()->empty();	
	content()->add(new LevelSelector(m_game, m_chosenLevel));
        if (rightControl()) {
            rightControl()->text(Tr::copy(""));
            rightControl()->event(Event::CANCEL);
        }
	break;
      case 2:
	close();
	break;
      case 3: 
	content()->empty();
	content()->add(new HelpPage());
        if (rightControl()) {
            rightControl()->text(Tr::copy(""));
            rightControl()->event(Event::CANCEL);
        }
	break;
      }
      return true;
    case Event::SELECT:
      LOG_INFO("Select level %d", ev.x);
      m_chosenLevel = ev.x;
      content()->empty();
      content()->add(new LevelLauncher(m_chosenLevel, NULL));
      if (rightControl()) {
          rightControl()->text(Tr::copy(""));
          rightControl()->event(Event(Event::MENU,1));
      }
      return true;
    case Event::CANCEL:
      content()->empty();
      content()->add(new FrontPage());
      if (rightControl()) {
          rightControl()->event(Event::QUIT);
      }
      return true;
    case Event::PLAY:
    case Event::REPLAY:
      close();
      break;
    default:
      break;
    }
    return Dialog::onEvent(ev);
  }
};


Widget* createMainMenu(GameControl* game)
{
  return new MainMenu(game);
}


////////////////////////////////////////////////////////////////


static const MenuItem playNormalOpts[] = {
  MenuItem(Tr("pen"), StockIcon::PEN, Event(Event::SELECT,1,-1)),
  MenuItem(Tr("tools"), StockIcon::CHOOSE, Event(Event::SELECT,2,-1)),
  MenuItem(Tr("pause"), StockIcon::PAUSE, Event::PAUSE),
  MenuItem(Tr("undo"), StockIcon::UNDO, Event::UNDO),
  MenuItem(Tr::copy(""), StockIcon::NONE, Event::NOP)
};

static const MenuItem playPausedOpts[] = {
  MenuItem(Tr("pen"), StockIcon::PEN, Event(Event::SELECT,1,-1)),
  MenuItem(Tr("tools"), StockIcon::CHOOSE, Event(Event::SELECT,2,-1)),
  MenuItem(Tr("resume"), StockIcon::PLAY, Event::PAUSE),
  MenuItem(Tr("undo"), StockIcon::UNDO, Event::UNDO),
  MenuItem(Tr::copy(""), StockIcon::NONE, Event::NOP)
};


class OptsPopup : public MenuDialog
{
protected:
  Vec2 m_closeTarget;
public:
  OptsPopup() : MenuDialog(this, Tr::copy(""), NULL)
  {
    m_buttonDim = Vec2(90,90);
    m_closeTarget = Vec2(-10, 0);
  }

  virtual Widget* makeButton( MenuItem* item, const Event& ev )
  {
    return new StockIconButton(item->tr, item->icon, ev);
  }

  virtual bool onEvent( Event& ev ) {
      if (ev.code == Event::CLOSE) {
          if (m_closeTarget != m_pos.tl) {
              animateTo(m_closeTarget, [this] () {
                  Event closeForReal(Event::CLOSE);
                  onEvent(closeForReal);
              });
              return true;
          }

          Event closingEvent(Event::POPUP_CLOSING);
          if (dispatchEvent(closingEvent)) {
              return true;
          }
      }
      return MenuDialog::onEvent(ev);
  }
};


class PlayOpts : public OptsPopup
{
public:
  PlayOpts(GameControl* game )
  {
    addItems(game->m_paused ? playPausedOpts : playNormalOpts);
    sizeTo(Vec2(140,480));
    moveTo(Vec2(WORLD_WIDTH,0));
    animateTo(Vec2(WORLD_WIDTH-140,0));
    m_closeTarget = Vec2(WORLD_WIDTH, 0);
  }
};

Widget* createPlayOpts(GameControl* game )
{
  return new PlayOpts(game);
}


////////////////////////////////////////////////////////////////


static const MenuItem editNormalOpts[] = {
  MenuItem(Tr("menu"), StockIcon::CLOSE, Event::MENU),
  MenuItem(Tr("rewind"), StockIcon::RESET, Event::RESET),
  MenuItem(Tr("skip"), StockIcon::FORWARD, Event::NEXT),
  MenuItem(Tr("edit"), StockIcon::SHARE, Event::EDIT),
  MenuItem(Tr::copy(""), StockIcon::NONE, Event::NOP)
};

static const MenuItem editDoneOpts[] = {
  MenuItem(Tr("menu"), StockIcon::CLOSE, Event::MENU),
  MenuItem(Tr("rewind"), StockIcon::RESET, Event::RESET),
  MenuItem(Tr("done"), StockIcon::SHARE, Event::DONE),
  MenuItem(Tr::copy(""), StockIcon::NONE, Event::NOP)
};


class EditOpts : public OptsPopup
{
public:
  EditOpts(GameControl* game )
  {
    addItems(game->m_edit ? editDoneOpts : editNormalOpts);
    sizeTo(Vec2(140,480));
    moveTo(Vec2(-140,0));
    animateTo(Vec2(0,0));
    m_closeTarget = Vec2(-width(), 0);
  }
};

Widget* createEditOpts(GameControl* game )
{
  return new EditOpts(game);
}


////////////////////////////////////////////////////////////////


class ColourButton : public Button
{
public:
  ColourButton(const Tr& s, int c, const Event& ev)
    : Button(s,ev)
  {
    m_bg = c;
    transparent(false);
  }
};

class ColourDialog : public MenuDialog
{
public:
  ColourDialog( int num, const int* cols ) 
    : MenuDialog(this, Tr("pen")),
      m_colours(cols)					     
  {
    m_columns = 4;
    m_buttonDim = Vec2(BUTTON_HEIGHT, BUTTON_HEIGHT);
    for (int i=0; i<num; i++) {
      switch(i) {
          case 0: addItem( MenuItem(Tr::copy("O"), StockIcon::NONE, Event(Event::SELECT,1,i)) ); break;
          case 1: addItem( MenuItem(Tr::copy("X"), StockIcon::NONE, Event(Event::SELECT,1,i)) ); break;
          default: addItem( MenuItem(Tr::copy("/"), StockIcon::NONE, Event(Event::SELECT,1,i)) ); break;
      }
    }
    Vec2 size = m_buttonDim*5;
    sizeTo(size);
    animateTo(Vec2(WORLD_WIDTH - m_pos.width(),0));
  }
  Widget* makeButton( MenuItem* item, const Event& ev )
  {
    Button *w = new ColourButton(item->tr,m_colours[item->event.y],ev);
    w->font(Font::titleFont());
    return w;
  }
  const int* m_colours;
};

Widget* createColourDialog(GameControl* game, int n, const int* cols)
{
  return new ColourDialog(n,cols);
}


////////////////////////////////////////////////////////////////

struct ToggleMenuItem {
    ToggleMenuItem(const Tr &tr,
                   std::function<bool(GameControl *)> toggled,
                   std::function<bool(GameControl *)> clicked)
        : menuitem(MenuItem(tr, StockIcon::TICK, Event(Event::SELECT)))
        , toggled(toggled)
        , clicked(clicked)
    {
    }

    MenuItem menuitem;

    // Status query - returns "true" if the check in the menu should be on
    std::function<bool(GameControl *)> toggled;

    // Click handler - returns "true" if the click event was handled
    std::function<bool(GameControl *)> clicked;
};

static const ToggleMenuItem toolOpts[] = {
  ToggleMenuItem(Tr("ground"), [] (GameControl *game) {
      return game->m_strokeFixed;
  }, [] (GameControl *game) {
      game->m_strokeFixed = !game->m_strokeFixed;
      game->m_strokeSleep = false;
      game->m_strokeDecor = false;
      return true;
  }),

  ToggleMenuItem(Tr("sleepy"), [] (GameControl *game) {
      return game->m_strokeSleep;
  }, [] (GameControl *game) {
      game->m_strokeFixed = false;
      game->m_strokeSleep = !game->m_strokeSleep;
      game->m_strokeDecor = false;
      return true;
  }),

  ToggleMenuItem(Tr("decor"), [] (GameControl *game) {
      return game->m_strokeDecor;
  }, [] (GameControl *game) {
      game->m_strokeFixed = false;
      game->m_strokeSleep = false;
      game->m_strokeDecor = !game->m_strokeDecor;
      return true;
  }),

  ToggleMenuItem(Tr("move"), [] (GameControl *game) {
      return game->m_clickMode == CLICK_MODE_MOVE;
  }, [] (GameControl *game) {
      game->toggleClickMode(CLICK_MODE_MOVE);
      return true;
  }),

  ToggleMenuItem(Tr("erase"), [] (GameControl *game) {
      return game->m_clickMode == CLICK_MODE_ERASE;
  }, [] (GameControl *game) {
      game->toggleClickMode(CLICK_MODE_ERASE);
      return true;
  }),

  ToggleMenuItem(Tr("jetstream"), [] (GameControl *game) {
      return game->m_clickMode == CLICK_MODE_DRAW_JETSTREAM;
  }, [] (GameControl *game) {
      game->toggleClickMode(CLICK_MODE_DRAW_JETSTREAM);
      return true;
  }),

  ToggleMenuItem(Tr("rope"), [] (GameControl *game) {
      return game->m_strokeRope;
  }, [] (GameControl *game) {
      game->m_strokeRope = !game->m_strokeRope;
      return true;
  }),

  ToggleMenuItem(Tr("interactive"), [] (GameControl *game) {
      return game->m_interactiveDraw;
  }, [] (GameControl *game) {
      game->m_interactiveDraw = !game->m_interactiveDraw;
      return true;
  }),
};


class ToolDialog : public MenuDialog
{
public:
  ToolDialog(GameControl* game) : MenuDialog(this, Tr("tools"),NULL),
				  m_game(game)
  {
    m_buttonDim = Vec2(200, 40);
    std::vector<MenuItem> items;
    for (auto &o: toolOpts) {
        items.push_back(o.menuitem);
    }
    addItems(items);
    updateTicks();
  }
  Widget* makeButton( MenuItem* item, const Event& ev )
  {
    StockIconButton *w = new StockIconButton(item->tr, item->icon, ev);
    w->align(1);
    m_opts.push_back(w);
    return w;
  }
  void empty()
  {
    m_opts.clear();
  }
  void remove( Widget* w )
  {
      if (!w) {
          return;
      }

      auto it = std::find(m_opts.begin(), m_opts.end(), static_cast<StockIconButton *>(w));
      if (it != m_opts.end()) {
          m_opts.erase(it);
      }
  }
  void updateTicks()
  {
    for (int i=0; i<m_opts.size(); i++) {
      bool tick = false;

      if (i < ARRAY_SIZE(toolOpts)) {
          tick = toolOpts[i].toggled(m_game);
      } else {
          LOG_WARNING("Option not in toolOpts: %s", m_opts[i]->tr().c_str());
      }

      m_opts[i]->set(tick ? StockIcon::TICK : StockIcon::BLANK);
    }
  }

  bool onEvent( Event& ev )
  {
      if (ev.code == Event::SELECT) {
          if (ev.x < ARRAY_SIZE(toolOpts)) {
              if (toolOpts[ev.x].clicked(m_game)) {
                  updateTicks();
                  return true;
              }
          } else {
              LOG_WARNING("Ignoring select event for invalid index %d", ev.x);
          }
      }

      return MenuDialog::onEvent(ev);
  }

private:
  GameControl *m_game;
  std::vector<StockIconButton*> m_opts;
};



Widget* createToolDialog(GameControl* game)
{
  return new ToolDialog(game);
}


////////////////////////////////////////////////////////////////



class NextLevelDialog : public Dialog
{
  GameControl* m_game;
public:
  NextLevelDialog(GameControl* game)
    : Dialog(Tr("BRAVO!!!"),Event::NOP,Event::MENU),
      m_game(game)
  {
    rightControl()->text(Tr::copy(""));
    char buf[32];
    const GameStats& stats = m_game->stats();
    int time = (stats.endTime - stats.startTime)/1000;
    int h = time/60/60;
    int m = time/60 - h*60;
    int s = time - m*60;

    Box *vbox = new VBox();
    vbox->add(new Spacer(),10,1);

    auto tr_time = Tr::defer([=] () {
        if (h > 0) {
            return thp::format(Tr("time: %dh %dm %ds"), m, h, s);
        } else if (m > 0) {
            int m = time/60/1000;
            return thp::format(Tr("time: %dm %ds"), m, s);
        } else {
            return thp::format(Tr("time: %ds"), s);
        }
    });
    vbox->add(new Label(tr_time, nullptr, 0x000000),20,0);

    vbox->add(new Label(TR_DEFERRED(thp::format(Tr("%d stroke(s)"), stats.strokeCount)), nullptr, 0x000000),20,0);

    if (stats.pausedStrokes) {
      auto tr = Tr::defer([=] () {
          std::string msg = thp::format(Tr("%d while paused"), stats.pausedStrokes);
          return thp::format("     (%s)", msg.c_str());
      });
      vbox->add(new Label(tr, nullptr, 0x000000),20,0);
    }

    vbox->add(new Label(TR_DEFERRED(thp::format(Tr("%d undo(s)"), stats.undoCount)), nullptr, 0x000000),20,0);
    vbox->add(new Spacer(),10,1);
 
    Box *hbox2 = new HBox();
    hbox2->add(new Spacer(),20,0);
    hbox2->add(new Button(Tr("review"),Event(Event::REPLAY,game->m_level)),BUTTON_WIDTH,0);
    hbox2->add(new Spacer(),1,1);
    hbox2->add(new Button(Tr("again"),Event::RESET),BUTTON_WIDTH,0);
    hbox2->add(new Spacer(),1,1);
    hbox2->add(new Button(Tr("next"),Event::NEXT),BUTTON_WIDTH,0);
    hbox2->add(new Spacer(),20,0);
    vbox->add(hbox2,BUTTON_HEIGHT,0);

    vbox->add(new Spacer(),10,0);
    content()->add(vbox,0,0);
    moveTo(Vec2(150, WORLD_HEIGHT));
    animateTo(Vec2(150, 120));
    sizeTo(Vec2(500,240));
  }
};


Widget *createNextLevelDialog( GameControl* game )
{
  return new NextLevelDialog(game);
}


////////////////////////////////////////////////////////////////

class EditDoneDialog : public Dialog
{
  GameControl* m_game;
public:
  EditDoneDialog(GameControl* game)
    : Dialog(Tr("Exit Editor"),Event::NOP,Event::CLOSE),
      m_game(game)
  {
    Box *vbox = new VBox();
    vbox->add(new Spacer(),10,1);
    vbox->add(new Label(Tr("Save level?")),20,0);
    vbox->add(new Spacer(),10,1);
 
    Box *hbox2 = new HBox();
    hbox2->add(new Spacer(),20,0);
    hbox2->add(new Button(Tr("cancel"),Event::CLOSE),BUTTON_WIDTH,0);
    hbox2->add(new Spacer(),1,1);
    hbox2->add(new Button(Tr("exit"),Event::EDIT),BUTTON_WIDTH,0);
    hbox2->add(new Spacer(),1,1);
    hbox2->add(new Button(Tr("save"),Event::SAVE),BUTTON_WIDTH,0);
    hbox2->add(new Spacer(),20,0);
    vbox->add(hbox2,BUTTON_HEIGHT,0);

    vbox->add(new Spacer(),10,0);
    content()->add(vbox,0,0);
    animateTo(Vec2(150, 70));
    sizeTo(Vec2(500,240));
  }
  bool onEvent( Event& ev )
  {
    close();
    return false;
  }
};


Widget *createEditDoneDialog( GameControl* game )
{
  return new EditDoneDialog(game);
}

