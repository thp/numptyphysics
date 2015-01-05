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


static const char *
clickModeName(enum ClickMode cm)
{
    switch (cm) {
        case CLICK_MODE_DRAW_STROKE: return "draw stroke";
        case CLICK_MODE_MOVE: return "move";
        case CLICK_MODE_ERASE: return "erase";
        case CLICK_MODE_DRAW_JETSTREAM: return "draw jetstream";
    }

    LOG_NOTREACHED;
    return nullptr;
}


static float BUTTON_BORDER() { return WORLD_WIDTH * 0.02f; }
static float BUTTON_SIZE() { return WORLD_WIDTH * 0.1f; }

class Game : public GameControl, public Container
{
  Scene   	    m_scene;
  Widget           *m_pauseLabel;
  Widget           *m_editLabel;
  Widget           *m_completedDialog;
  Widget           *m_options;
  Label            *m_clickModeLabel;
  Os               *m_os;
  bool              m_isCompleted;
  Widget           *m_left_button;
  Widget           *m_right_button;
  int               m_reset_countdown;
public:
  Game( Levels* levels, int width, int height ) 
  : m_pauseLabel( NULL ),
    m_editLabel( NULL ),
    m_completedDialog( NULL ),
    m_options( NULL ),
    m_clickModeLabel(new Label("", nullptr, 0x000000)),
    m_os( Os::get() ),
    m_isCompleted(false)
  , m_left_button(new Button("MENU", Event(Event::OPTION, 1)))
  , m_right_button(new Button("TOOL", Event(Event::OPTION, 2)))
  , m_reset_countdown(0)
  {
    EVAL_LOCAL(BUTTON_BORDER);
    EVAL_LOCAL(BUTTON_SIZE);

    add(m_left_button, Rect(BUTTON_BORDER, BUTTON_BORDER, BUTTON_BORDER + BUTTON_SIZE, BUTTON_BORDER + BUTTON_SIZE));
    add(m_right_button, Rect(WORLD_WIDTH - BUTTON_BORDER - BUTTON_SIZE, BUTTON_BORDER, WORLD_WIDTH - BUTTON_BORDER, BUTTON_BORDER + BUTTON_SIZE));

    m_clickModeLabel->setAlignment(Label::ALIGN_RIGHT | Label::ALIGN_BOTTOM);
    add(m_clickModeLabel, Rect(0, 0, WORLD_WIDTH, WORLD_HEIGHT).shrunk(BUTTON_BORDER));

    setEventMap(Os::get()->getEventMap(GAME_MAP));
    sizeTo(Vec2(width,height));
    transparent(true); //don't clear
    m_greedyMouse = true; //get mouse clicks outside the window!

    m_levels = levels;
    gotoLevel(0);
    //add( new Button("O",Event::OPTION), Rect(800-32,0,32,32) );
  }


  const char* name() {return "Game";}

  void replayLevel() {
      // reset scene, delete user strokes, but retain log
      m_replaying = m_scene.replay();
  }

  void gotoLevel(int level) {
      if (level < 0 || level >= m_levels->numLevels()) {
          LOG_WARNING("Level does not exist: %d", level);
          return;
      }

      if (m_scene.load(m_levels->load(level))) {
          m_replaying = m_scene.start();

          if (m_edit) {
              // Unprotect all strokes
              m_scene.protect(0);
          }

          m_level = level;
          m_stats.reset(OS->ticks());
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
      LOG_INFO("Not saving demo of demo");
    }
  }

  void toggleClickMode(enum ClickMode cm)
  {
      if (cm == m_clickMode) {
          clickMode(CLICK_MODE_DRAW_STROKE);
      } else {
          clickMode(cm);
      }
  }

  void clickMode(enum ClickMode cm)
  {
    if (cm != m_clickMode) {
      LOG_DEBUG("clickMode = %s", clickModeName(cm));
      m_clickMode = cm;
      switch (cm) {
          case CLICK_MODE_DRAW_STROKE:
              setEventMap(Os::get()->getEventMap(GAME_MAP));
              m_clickModeLabel->text("");
              break;
          case CLICK_MODE_MOVE:
              setEventMap(Os::get()->getEventMap(GAME_MOVE_MAP));
              m_clickModeLabel->text("Move mode");
              break;
          case CLICK_MODE_ERASE:
              setEventMap(Os::get()->getEventMap(GAME_ERASE_MAP));
              m_clickModeLabel->text("Erase mode");
              break;
          case CLICK_MODE_DRAW_JETSTREAM:
              setEventMap(Os::get()->getEventMap(GAME_JETSTREAM_MAP));
              m_clickModeLabel->text("Create jet stream");
              break;
          default:
              LOG_NOTREACHED;
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
      add( m_pauseLabel, Rect(WORLD_WIDTH/2-128, 16, WORLD_WIDTH/2+128, 64));
      m_paused = true;
      m_scene.onSceneEvent(SceneEvent(SceneEvent::PAUSE));
    } else {
      remove( m_pauseLabel );
      m_pauseLabel = NULL;
      m_paused = false;
      m_scene.onSceneEvent(SceneEvent(SceneEvent::UNPAUSE));
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
	add(m_editLabel, Rect(WORLD_WIDTH/2-128, WORLD_HEIGHT-64, WORLD_WIDTH/2+128, WORLD_HEIGHT-16));
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
    m_scene.step();

    if (m_reset_countdown > 0) {
        m_reset_countdown--;
        if (m_reset_countdown == REWIND_ANIMATION_TICKS / 2) {
            if (m_scene.isCompleted()) {
                // From the finish screen, we always start the level fresh
                gotoLevel(m_level);
            } else if (!m_replaying) {
                // Store all events up to now, so that we can playback those
                // events after reloading the level up to a given point in
                // the past (REWIND_JUMP_LENGTH ticks before now)
                ScriptLog events = *(m_scene.getLog());
                int ticks = m_scene.getTicks();
                int target = std::max(0, ticks - REWIND_JUMP_LENGTH);

                LOG_DEBUG("Rewinding: %d -> %d", ticks, target);

                // Reload level and replay history until the target point
                gotoLevel(m_level);
                m_scene.playbackUntil(events, target);
            } else {
                // FIXME: Implement step-wise rewind for playback as well?
                gotoLevel(m_level);
            }
        }
    }

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

      auto rect = OS->renderer()->rect();

      if (window) {
          // If we draw an effect
          std::function<void(Image *, const Rect &src, const Rect &dst)> effect;

          if (m_reset_countdown > 0) {
              effect = [this, window] (Image *img, const Rect &src, const Rect &dst) {
                  float alpha = powf(1.f - fabsf(2.f * (float(m_reset_countdown) / float(REWIND_ANIMATION_TICKS) - 0.5f)), 0.4f);
                  window->drawRewind(*img, src, dst, OS->ticks(), alpha);
              };
          } else if (isPaused()) {
              effect = [window] (Image *img, const Rect &src, const Rect &dst) {
                  window->drawSaturation(*img, src, dst, 0.7f);
              };
          }

          std::unique_ptr<Image> img = nullptr;

          if (effect) {
              // If we want to draw an effect, render to a texture as input for the effect
              RenderTarget target(rect.w(), rect.h());
              target.begin();
              m_scene.draw(target);
              target.end();
              img.reset(new Image(target.contents()));
          } else {
              // Default "effect" is drawing the scene directly to the window's offscreen
              effect = [this, window] (Image *img, const Rect &src, const Rect &dst) {
                  m_scene.draw(*window);
              };
          }

          // Now we can draw the sceen into the offscreen buffer
          window->beginOffscreen();
          effect(img.get(), rect, rect);
          window->endOffscreen();
      }

      // Draw the whole backbuffer to the screen
      screen.drawImage(*window->offscreen(), 0, 0);

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
          if (m_clickMode == CLICK_MODE_DRAW_JETSTREAM) {
              if (m_scene.onSceneEvent(SceneEvent(SceneEvent::DELETE_LAST_JETSTREAM))) {
                  m_stats.undoCount++;
              }
          } else {
              if (m_scene.onSceneEvent(SceneEvent(SceneEvent::DELETE_LAST_STROKE))) {
                  m_stats.undoCount++;
              }
          }
      }
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
        m_right_button->animateTo(Vec2(WORLD_WIDTH-BUTTON_BORDER-BUTTON_SIZE, -BUTTON_BORDER-BUTTON_SIZE));
      }
      if (m_options) {
	add( m_options );
      }
      break;
    case Event::POPUP_CLOSING:
      m_left_button->animateTo(Vec2(BUTTON_BORDER, BUTTON_BORDER));
      m_right_button->animateTo(Vec2(WORLD_WIDTH-BUTTON_BORDER-BUTTON_SIZE, BUTTON_BORDER));
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
      if (m_edit && !isPaused()) {
	togglePause();
      }
      break;
    case Event::DONE:
      if (m_edit) {
	add( createEditDoneDialog(this) );
      }
      break;
    case Event::RESET:
      m_reset_countdown = REWIND_ANIMATION_TICKS;
      break;
    case Event::NEXT:
      if (m_level==0 && m_isCompleted) {
	// from title try to find the first uncompleted level
	while (m_level < m_levels->numLevels()
	       && m_os->exists(m_levels->demoName(m_level))) {
	  m_level++;
	}
	gotoLevel(m_level);
      } else {
	gotoLevel(m_level+1);
      }
      break;
    case Event::PREVIOUS:
      gotoLevel(m_level-1);
      break;
    case Event::REPLAY:
      replayLevel();
      break;
    case Event::PLAY:
      gotoLevel(ev.x);
      break;
    case Event::DRAWBEGIN:
      if (!m_replaying) {
          if (m_scene.canInteractAt(mousePoint(ev))) {
              m_scene.onSceneEvent(SceneEvent(SceneEvent::INTERACT_AT, mousePoint(ev)));
          } else {
              int attrib = 0;
              if ( m_strokeFixed ) attrib |= ATTRIB_GROUND;
              if ( m_strokeSleep ) attrib |= ATTRIB_SLEEPING;
              if ( m_strokeDecor ) attrib |= ATTRIB_DECOR;
              if ( m_interactiveDraw ) attrib |= ATTRIB_INTERACTIVE;
              m_scene.onSceneEvent(SceneEvent(SceneEvent::BEGIN_CREATE_STROKE_AT, mousePoint(ev), m_colour, attrib));
          }
      }
      break;
    case Event::DRAWMORE:
      if (!m_replaying) {
          m_scene.onSceneEvent(SceneEvent(SceneEvent::EXTEND_CREATE_STROKE_AT, mousePoint(ev)));
      }
      break;
    case Event::DRAWEND:
      if (!m_replaying) {
          if (m_strokeRope) {
              if (m_scene.onSceneEvent(SceneEvent(SceneEvent::ROPEIFY_CREATE_STROKE))) {
                  m_stats.ropeCount++;
                  if (isPaused()) {
                      m_stats.pausedRopes++;
                  }
              }
          } else {
              if (m_scene.onSceneEvent(SceneEvent(SceneEvent::ACTIVATE_CREATE_STROKE))) {
                  m_stats.strokeCount++;
                  if (isPaused()) {
                      m_stats.pausedStrokes++;
                  }
              }
          }
      }
      break;
    case Event::MOVEBEGIN:
      if (!m_replaying) {
          m_scene.onSceneEvent(SceneEvent(SceneEvent::BEGIN_MOVE_STROKE_AT, mousePoint(ev)));
      }
      break;
    case Event::MOVEMORE:
      if (!m_replaying) {
          m_scene.onSceneEvent(SceneEvent(SceneEvent::CONTINUE_MOVE_STROKE_AT, mousePoint(ev)));
      }
      break;
    case Event::MOVEEND:
      if (!m_replaying) {
          m_scene.onSceneEvent(SceneEvent(SceneEvent::FINISH_MOVE_STROKE));
      }
      break;
    case Event::JETSTREAMBEGIN:
      if (!m_replaying) {
          m_scene.onSceneEvent(SceneEvent(SceneEvent::BEGIN_CREATE_JETSTREAM_AT, mousePoint(ev)));
      }
      break;
    case Event::JETSTREAMMORE:
      if (!m_replaying) {
          m_scene.onSceneEvent(SceneEvent(SceneEvent::RESIZE_CREATE_JETSTREAM_AT, mousePoint(ev)));
      }
      break;
    case Event::JETSTREAMEND:
      if (!m_replaying) {
          m_scene.onSceneEvent(SceneEvent(SceneEvent::ACTIVATE_CREATE_JETSTREAM));
      }
      break;
    case Event::DELETE:
      if (!m_replaying) {
          m_scene.onSceneEvent(SceneEvent(SceneEvent::DELETE_STROKE_AT, mousePoint(ev)));
      }
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
