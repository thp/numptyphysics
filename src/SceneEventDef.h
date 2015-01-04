
/**
 * Definition of possible values for scene event.
 *
 * Macro syntax:
 *  SCENE_EVENT_DEFINE_OPERATION(name, has_pos, data_fields)
 *   - name: Symbolic name of operation
 *   - has_pos: If the event has/needs position information
 *   - data_fields: Number of additional userdata fields
 *
 * Used in:
 *  - SceneEvent.h for defining the symbolic enum constants
 *  - SceneEvent.cpp for defining the metadata fields
 **/

#if !defined(SCENE_EVENT_DEFINE_OPERATION)
#error "Define SCENE_EVENT_DEFINE_OPERATION before including this file"
#endif

// Pause / unpause physics
SCENE_EVENT_DEFINE_OPERATION(PAUSE, false, 0)
SCENE_EVENT_DEFINE_OPERATION(UNPAUSE, false, 0)

// Interaction
SCENE_EVENT_DEFINE_OPERATION(INTERACT_AT, true, 0)

// Create stroke
SCENE_EVENT_DEFINE_OPERATION(BEGIN_CREATE_STROKE_AT, true, 2)
SCENE_EVENT_DEFINE_OPERATION(EXTEND_CREATE_STROKE_AT, true, 0)
SCENE_EVENT_DEFINE_OPERATION(ACTIVATE_CREATE_STROKE, false, 0)

// Create stroke - advanced
SCENE_EVENT_DEFINE_OPERATION(ROPEIFY_CREATE_STROKE, false, 0)

// Move stroke
SCENE_EVENT_DEFINE_OPERATION(BEGIN_MOVE_STROKE_AT, true, 0)
SCENE_EVENT_DEFINE_OPERATION(CONTINUE_MOVE_STROKE_AT, true, 0)
SCENE_EVENT_DEFINE_OPERATION(FINISH_MOVE_STROKE, false, 0)

// Delete stroke
SCENE_EVENT_DEFINE_OPERATION(DELETE_LAST_STROKE, false, 0)
SCENE_EVENT_DEFINE_OPERATION(DELETE_STROKE_AT, true, 0)

// Create jet stream
SCENE_EVENT_DEFINE_OPERATION(BEGIN_CREATE_JETSTREAM_AT, true, 0)
SCENE_EVENT_DEFINE_OPERATION(RESIZE_CREATE_JETSTREAM_AT, true, 0)
SCENE_EVENT_DEFINE_OPERATION(ACTIVATE_CREATE_JETSTREAM, false, 0)

// Delete jet stream
SCENE_EVENT_DEFINE_OPERATION(DELETE_LAST_JETSTREAM, false, 0)
