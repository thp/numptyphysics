/*
 * This file is part of NumptyPhysics <http://thp.io/2015/numptyphysics/>
 * Coyright (c) 2008, 2009, 2010 Tim Edmonds <numptyphysics@gmail.com>
 * Coyright (c) 2008, 2012, 2014, 2015 Thomas Perl <m@thp.io>
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

#include "Common.h"
#include "Config.h"
#include "Game.h"
#include "Path.h"
#include "Canvas.h"
#include "Font.h"
#include "Levels.h"
#include "Os.h"
#include "Scene.h"
#include "Stroke.h"
#include "Script.h"
#include "Dialogs.h"
#include "Ui.h"
#include "Colour.h"

#include "petals_log.h"

#include <cstdio>
#include <iostream>
#include <sstream>
#include <fstream>
#include <memory.h>
#include <errno.h>

using namespace std;


static constexpr const char *JOINT_IND_PATH =
    "282,39 280,38 282,38 285,39 300,39 301,60 303,66 302,64 "
    "301,63 300,48 297,41 296,42 294,43 293,45 291,46 289,48 "
    "287,49 286,52 284,53 283,58 281,62 280,66 282,78 284,82 "
    "287,84 290,85 294,88 297,88 299,89 302,90 308,90 311,89 "
    "314,89 320,85 321,83 323,83 324,81 327,78 328,75 327,63 "
    "326,58 325,55 323,54 321,51 320,49 319,48 316,46 314,44 "
    "312,43 314,43";


static float BUTTON_BORDER() { return SCREEN_WIDTH * 0.02f; }
static float BUTTON_SIZE() { return SCREEN_WIDTH * 0.1f; }

class Game : public GameControl, public Container
{
  Scene   	    m_scene;
  Stroke  	   *m_createStroke;
  Stroke           *m_moveStroke;
  Vec2              m_moveOffset;
  Widget           *m_pauseLabel;
  Widget           *m_editLabel;
  Widget           *m_completedDialog;
  Widget           *m_options;
  Label            *m_clickModeLabel;
  Os               *m_os;
  bool              m_isCompleted;
  Path              m_jointInd;
  Widget           *m_left_button;
  Widget           *m_right_button;
public:
  Game( Levels* levels, int width, int height ) 
  : m_createStroke(NULL),
    m_moveStroke(NULL),
    m_moveOffset(),
    m_pauseLabel( NULL ),
    m_editLabel( NULL ),
    m_completedDialog( NULL ),
    m_options( NULL ),
    m_clickModeLabel(new Label("", nullptr, 0x000000)),
    m_os( Os::get() ),
    m_isCompleted(false),
    m_jointInd(JOINT_IND_PATH)
  , m_left_button(new Button("MENU", Event(Event::OPTION, 1)))
  , m_right_button(new Button("TOOL", Event(Event::OPTION, 2)))
  {
    EVAL_LOCAL(BUTTON_BORDER);
    EVAL_LOCAL(BUTTON_SIZE);

    add(m_left_button, Rect(BUTTON_BORDER, BUTTON_BORDER, BUTTON_BORDER + BUTTON_SIZE, BUTTON_BORDER + BUTTON_SIZE));
    add(m_right_button, Rect(SCREEN_WIDTH - BUTTON_BORDER - BUTTON_SIZE, BUTTON_BORDER, SCREEN_WIDTH - BUTTON_BORDER, BUTTON_BORDER + BUTTON_SIZE));

    m_clickModeLabel->setAlignment(Label::ALIGN_RIGHT | Label::ALIGN_BOTTOM);
    add(m_clickModeLabel, Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT).shrunk(BUTTON_BORDER));

    setEventMap(Os::get()->getEventMap(GAME_MAP));
    sizeTo(Vec2(width,height));
    transparent(true); //don't clear
    m_greedyMouse = true; //get mouse clicks outside the window!

    m_jointInd.scale( 12.0f / (float32)m_jointInd.bbox().width() );
    //m_jointInd.simplify( 2.0f );
    m_jointInd.makeRelative();
    m_levels = levels;
    gotoLevel(0);
    //add( new Button("O",Event::OPTION), Rect(800-32,0,32,32) );
  }


  const char* name() {return "Game";}

  void gotoLevel( int level, bool replay=false )
  {
    bool ok = false;
    m_replaying = replay;

    if ( replay ) {
      // reset scene, delete user strokes, but retain log
      m_scene.reset( NULL, true );
      m_scene.start( true );
      ok = true;
    } else if ( level >= 0 && level < m_levels->numLevels() ) {
        if (m_scene.load(m_levels->load(level))) {
            m_scene.start( m_scene.getLog()->size() > 0 );
            ok = true;
        }
    }

    if (ok) {
      if ( m_edit ) {
	m_scene.protect(0);
      } else {
      }
      m_refresh = true;
      m_level = level;
      if (!m_replaying) {
	m_stats.reset(OS->ticks());
      }
    }
  }


  bool save( const char *file=NULL )
  {	  
    string p;
    if ( file ) {
      p = file;
    } else {
      file = "L99_saved.npsvg";
      p = Config::userLevelFileName(file);
    }
    if ( m_scene.save( p ) ) {
      m_levels->addPath( p.c_str() );
      int l = m_levels->findLevel( p.c_str() );
      if ( l >= 0 ) {
        LOG_DEBUG("Setting level to saved index to %d", l);
	m_level = l;
      }
      showMessage(std::string("<P align=center>saved to<BR>")+file);
      return true;
    }
    return false;
  }

  void saveDemo()
  {
    std::string path = m_levels->demoPath(m_level);
    if (path != "") {
      OS->ensurePath(path);
      path = m_levels->demoName(m_level);
      LOG_INFO("Saving demo of level %d to %s", m_level, path.c_str());
      m_scene.save(path, true);
    } else {
      LOG_INFO("Not daving demo of demo");
    }
  }

  void clickMode(int cm)
  {
    if (cm != m_clickMode) {
      LOG_DEBUG("clickMode = %d", cm);
      m_clickMode = cm;
      switch (cm) {
          case 1:
              setEventMap(Os::get()->getEventMap(GAME_MOVE_MAP));
              m_clickModeLabel->text("Move mode");
              break;
          case 2:
              setEventMap(Os::get()->getEventMap(GAME_ERASE_MAP));
              m_clickModeLabel->text("Erase mode");
              break;
          default:
              setEventMap(Os::get()->getEventMap(GAME_MAP));
              m_clickModeLabel->text("");
              break;
      }
    }
  }

  void setTool( int t )
  {
    m_colour = t;
  }

  void editMode( bool set )
  {
    m_edit = set;
  }

  void showMessage( const std::string& msg )
  {
    LOG_INFO("showMessage \"%s\"", msg.c_str());
    add( new MessageBox(msg) );
  }

  void togglePause()
  {
    if ( !m_paused ) {
      if ( !m_pauseLabel ) {
          m_pauseLabel = new Button("Gameplay paused", Event::PAUSE);
      }
      add( m_pauseLabel, Rect(SCREEN_WIDTH/2-128, 16, SCREEN_WIDTH/2+128, 64));
      m_paused = true;
    } else {
      remove( m_pauseLabel );
      m_pauseLabel = NULL;
      m_paused = false;
    }
  }

  bool isPaused()
  {
    return m_paused;
  }

  void edit( bool doEdit )
  {
    if ( m_edit != doEdit ) {
      m_edit = doEdit;
      if ( m_edit ) {	
	if ( !m_editLabel ) {
            m_editLabel = new Button("Edit mode", Event::DONE);
 	}
	add(m_editLabel, Rect(SCREEN_WIDTH/2-128, SCREEN_HEIGHT-64, SCREEN_WIDTH/2+128, SCREEN_HEIGHT-16));
	m_scene.protect(0);
      } else {
	remove(m_editLabel);
	m_editLabel = NULL;
	m_strokeFixed = false;
	m_strokeSleep = false;
	m_strokeDecor = false;
	if ( m_colour < 2 ) m_colour = 2;
	m_scene.protect();
      }
    }
  }

  Vec2 mousePoint( Event& ev )
  {
      return Vec2(ev.x, ev.y);
  }


  ////////////////////////////////////////////////////////////////
  // layer interface
  ////////////////////////////////////////////////////////////////

  void remove( Widget* w )
  {
    if (w==m_completedDialog) {
      m_completedDialog = NULL;
    }
    Container::remove(w);
  }

  virtual void onTick( int tick ) 
  {
    m_scene.step( isPaused() );

    if ( m_isCompleted && m_completedDialog && m_edit ) {
      remove( m_completedDialog );
      m_completedDialog = NULL;
      m_isCompleted = false;
    }
    if ( m_scene.isCompleted() != m_isCompleted && !m_edit ) {
      m_isCompleted = m_scene.isCompleted();
      if ( m_isCompleted ) {
	if (m_stats.endTime==0) {
	  //don't overwrite time after replay
	  m_stats.endTime = OS->ticks();
	}
        LOG_DEBUG("STATS:");
        LOG_DEBUG("time=%dms", m_stats.endTime-m_stats.startTime);
        LOG_DEBUG("strokes=%d (%d paused, %d undone)", m_stats.strokeCount,
                     m_stats.pausedStrokes, m_stats.undoCount);
	m_completedDialog = createNextLevelDialog(this);
	add( m_completedDialog );
	saveDemo();
      } else if (m_completedDialog) {
	remove( m_completedDialog );
	m_completedDialog = NULL;
      }
    }

    Container::onTick(tick);
  }

  virtual void draw( Canvas& screen, const Rect& area )
  {
      Window *window = dynamic_cast<Window *>(&screen);
      if (window) {
          window->beginOffscreen();
          m_scene.draw(*window);
          window->endOffscreen();
      }

    m_refresh = false;
    m_scene.draw(screen);

    if (m_createStroke) {
        b2Mat22 rot(0.01 * OS->ticks());

        for (auto &candidate: m_scene.getJointCandidates(m_createStroke)) {
            Path joint = m_jointInd;
            joint.translate(-joint.bbox().centroid());
            joint.rotate(rot);
            joint.translate(candidate + joint.bbox().centroid());
            screen.drawPath(joint, 0x606060);
        }
    }

    Container::draw(screen,area);
  }

  virtual bool onEvent( Event& ev )
  {
    bool used = true;

    EVAL_LOCAL(BUTTON_BORDER);
    EVAL_LOCAL(BUTTON_SIZE);

    switch (ev.code) {
    case Event::MENU:
      remove( m_completedDialog );
      add( createMainMenu(this) );
      break;
    case Event::PAUSE:
      LOG_DEBUG("Game pause");
      togglePause();
      break;
    case Event::UNDO:
      if ( !m_replaying ) {
	if ( m_createStroke ) {
	  m_scene.deleteStroke( m_createStroke );
	  m_createStroke = NULL;
	} else if ( m_scene.strokes().size() && m_scene.deleteStroke( m_scene.strokes().at(m_scene.strokes().size()-1) ) ) {
          // FIXME: Make sure undo also works correctly for ropes (delete whole rope at once)
	  m_stats.undoCount++;
	}
      }
      m_refresh = true;
      break;
    case Event::SAVE:
      save();
      break;
    case Event::CANCEL:
      if ( m_edit ) {
	edit(false);
      }
      break;
    case Event::OPTION:
      remove( m_options );
      if (ev.x == 1) {
	//edit menu
	m_options = createEditOpts(this);
        m_left_button->animateTo(Vec2(BUTTON_BORDER, -BUTTON_BORDER-BUTTON_SIZE));
      } else if (ev.x == 2) {
	//play menu
	m_options = createPlayOpts(this);
        m_right_button->animateTo(Vec2(SCREEN_WIDTH-BUTTON_BORDER-BUTTON_SIZE, -BUTTON_BORDER-BUTTON_SIZE));
      }
      if (m_options) {
	add( m_options );
      }
      break;
    case Event::POPUP_CLOSING:
      m_left_button->animateTo(Vec2(BUTTON_BORDER, BUTTON_BORDER));
      m_right_button->animateTo(Vec2(SCREEN_WIDTH-BUTTON_BORDER-BUTTON_SIZE, BUTTON_BORDER));
      used = false;
      break;
    case Event::SELECT:
      switch (ev.x) {
      case 1:
	switch (ev.y) {
	case -1:
	  add(createColourDialog(this, NP::Colour::count, NP::Colour::values)); 
	  break;
	default:
          LOG_DEBUG("SetTool %d", ev.y);
	  setTool(ev.y);
	  break;
	}
	break;
      case 2:
	switch (ev.y) {
	case -1:
	  add( createToolDialog(this) );
	  break;
	}	    
	break;
      }
      break;
    case Event::EDIT:
      edit( !m_edit );
      if (m_edit && !m_paused) {
	togglePause();
      }
      break;
    case Event::DONE:
      if (m_edit) {
	add( createEditDoneDialog(this) );
      }
      break;
    case Event::RESET:
      gotoLevel( m_level );
      break;
    case Event::NEXT:
      if (m_level==0 && m_isCompleted) {
	// from title try to find the first uncompleted level
	while (m_level < m_levels->numLevels()
	       && m_os->exists(m_levels->demoName(m_level))) {
	  m_level++;
	}
	gotoLevel( m_level );	
      } else {
	gotoLevel( m_level+1 );
      }
      break;
    case Event::PREVIOUS:
      gotoLevel( m_level-1 );
      break;
    case Event::REPLAY:
      gotoLevel( ev.x, true );
      break;
    case Event::PLAY:
      gotoLevel( ev.x );
      break;
    case Event::DRAWBEGIN:
      if ( !m_replaying && !m_createStroke ) {
          if (!m_scene.interact(mousePoint(ev))) {
              int attrib = 0;
              if ( m_strokeFixed ) attrib |= ATTRIB_GROUND;
              if ( m_strokeSleep ) attrib |= ATTRIB_SLEEPING;
              if ( m_strokeDecor ) attrib |= ATTRIB_DECOR;
              if ( m_interactiveDraw ) attrib |= ATTRIB_INTERACTIVE;
              m_createStroke = m_scene.newStroke(Path() & mousePoint(ev), m_colour, attrib);
          }
      }
      break;
    case Event::DRAWMORE:
      if ( m_createStroke ) {
	m_scene.extendStroke( m_createStroke, mousePoint(ev) );
      }
      break;
    case Event::DRAWEND:
      if ( m_createStroke ) {
        if (m_strokeRope) {
            m_stats.ropeCount++;
            if (isPaused()) {
                m_stats.pausedRopes++;
            }
            for (auto &stroke: m_createStroke->ropeify(m_scene)) {
                m_scene.activateStroke(stroke);
            }
            m_scene.deleteStroke(m_createStroke);
        } else if ( m_scene.activateStroke( m_createStroke ) ) {
	  m_stats.strokeCount++;
	  if ( isPaused() ) {
	    m_stats.pausedStrokes++; 
	  }
	} else {
	  m_scene.deleteStroke( m_createStroke );
	}
	m_createStroke = NULL;
      }
      break;
    case Event::MOVEBEGIN:
      if ( !m_replaying && !m_moveStroke ) {
          Vec2 point(mousePoint(ev));
          m_moveStroke = m_scene.strokeAtPoint(point, SELECT_TOLERANCE);
          if (m_moveStroke) {
              m_moveOffset = point - m_moveStroke->origin();
          }
      }
      break;
    case Event::MOVEMORE:
      if ( m_moveStroke ) {
	m_scene.moveStroke( m_moveStroke, mousePoint(ev) - m_moveOffset);
      }
      break;
    case Event::MOVEEND:
      m_moveStroke = NULL;
      break;
    case Event::DELETE:
      m_scene.deleteStroke( m_scene.strokeAtPoint( mousePoint(ev),
						   SELECT_TOLERANCE ) );
      m_refresh = true;
      break;
    default:
      used = Container::onEvent(ev);
    }
    return used;
  }

};



Widget* createGameLayer( Levels* levels, int width, int height )
{
  return new Game(levels,width,height);
}
