#ifndef NUMPTYPHYSICS_SCENEEVENT_H
#define NUMPTYPHYSICS_SCENEEVENT_H

#include "Common.h"

#include <string>

struct OperatorMetadata {
    const char *name;
    bool has_pos;
    int data_fields;

    OperatorMetadata(const char *name, bool has_pos, int data_fields)
        : name(name)
        , has_pos(has_pos)
        , data_fields(data_fields)
    {
    }
};

struct SceneEvent {
    enum Op {
#define SCENE_EVENT_DEFINE_OPERATION(name, has_pos, data_fields) name,
#include "SceneEventDef.h"
#undef SCENE_EVENT_DEFINE_OPERATION
    };

    SceneEvent(const std::string &op, const Vec2 &pos, int userdata1, int userdata2);
    SceneEvent(enum Op op, int x=0, int y=0, int userdata1=0, int userdata2=0) : op(op), pos(x, y), userdata1(userdata1), userdata2(userdata2) {}
    SceneEvent(enum Op op, const Vec2 &pos, int userdata1=0, int userdata2=0) : op(op), pos(pos), userdata1(userdata1), userdata2(userdata2) {}

    const OperatorMetadata *meta() const {
        return meta(op);
    }

    std::string repr() const;

    int arg(int i) const {
        switch (i) {
            case 0:
                return userdata1;
            case 1:
                return userdata2;
            default:
                return 0;
        }
    }

    static const OperatorMetadata *meta(enum Op op);

    enum Op op;
    Vec2 pos;
    int userdata1;
    int userdata2;
};

#endif /* NUMPTYPHYSICS_SCENEEVENT_H */
