#include "SceneEvent.h"

#include "petals_log.h"
#include "thp_format.h"


static const OperatorMetadata
META[] = {
#define SCENE_EVENT_DEFINE_OPERATION(name, has_pos, data_fields) OperatorMetadata(#name, has_pos, data_fields),
#include "SceneEventDef.h"
#undef SCENE_EVENT_DEFINE_OPERATION
};


static enum SceneEvent::Op
findOp(const std::string &op)
{
    for (int i=0; i<ARRAY_SIZE(META); i++) {
        if (op == META[i].name) {
            return SceneEvent::Op(i);
        }
    }

    LOG_FATAL("Invalid operator: '%s'", op.c_str());
    return SceneEvent::Op(0);
}


SceneEvent::SceneEvent(const std::string &op, const Vec2 &pos, int userdata1, int userdata2)
    : op(findOp(op))
    , pos(pos)
    , userdata1(userdata1)
    , userdata2(userdata2)
{
}

std::string
SceneEvent::repr() const
{
    auto m = meta();

    std::string posinfo(m->has_pos ? thp::format(" (pos=%d,%d)", pos.x, pos.y) : "");
    std::string arginfo;

    for (int i=0; i<m->data_fields; i++) {
        if (arginfo.length()) {
            arginfo += ", ";
        }
        arginfo += thp::format("%d", arg(i));
    }

    if (arginfo.length()) {
        arginfo = " (args=" + arginfo + ")";
    }

    return std::string(m->name) + posinfo + arginfo;
}

const OperatorMetadata *
SceneEvent::meta(enum Op op)
{
    return &META[op];
}
