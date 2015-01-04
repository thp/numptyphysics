#ifndef NUMPTYPHYSICS_JETSTREAM_H
#define NUMPTYPHYSICS_JETSTREAM_H

#include "Canvas.h"
#include "Common.h"
#include "Stroke.h"

#include <string>

class JetStream {
public:
    JetStream(const Rect &rect, const b2Vec2 &force);

    void draw(Canvas &canvas);
    void tick();
    void update(std::vector<Stroke *> &strokes);
    std::string asString();

    void activate();
    void resize(const Vec2 &mouse);

private:
    bool active;
    Vec2 origin;
    Rect rect;
    b2Vec2 force;
    std::vector<b2Vec2> particles;
};

#endif /* NUMPTYPHYSICS_JETSTREAM_H */
