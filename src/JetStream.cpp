#include "JetStream.h"

#include "thp_format.h"
#include "petals_log.h"

JetStream::JetStream(const Rect &rect, const b2Vec2 &force)
    : active(false)
    , origin(rect.tl)
    , rect(rect)
    , force(force)
    , particles()
{
    for (int i=0; i<sqrtf(rect.w() * rect.h())/10; i++) {
        particles.push_back(b2Vec2(rect.tl.x, rect.tl.y) +
                            b2Vec2(rand() % rect.width(), rand() % rect.height()));
    }
}

void
JetStream::draw(Canvas &canvas)
{
    canvas.drawRect(rect, 0x000044, true, 20);

    for (auto &particle: particles) {
        Vec2 pos(particle.x, particle.y);
        Path p;
        p.push_back(pos);
        p.push_back(pos + Vec2(force.x, force.y));
        canvas.drawPath(p, 0x000000, 128);
    }
}

void
JetStream::tick()
{
    if (!active) {
        return;
    }

    for (auto &particle: particles) {
        particle += force;
        if (!rect.contains(particle)) {
            while (particle.x < rect.tl.x) {
                particle.x += rect.width();
            }
            while (particle.x > rect.br.x) {
                particle.x -= rect.width();
            }
            while (particle.y < rect.tl.y) {
                particle.y += rect.height();
            }
            while (particle.y > rect.br.y) {
                particle.y -= rect.height();
            }
        }
    }
}

void
JetStream::update(std::vector<Stroke *> &strokes)
{
    if (!active) {
        return;
    }

    for (auto &stroke: strokes) {
        if (rect.intersects(stroke->screenBbox())) {
            auto body = stroke->body();
            if (body) {
                body->ApplyImpulse(force, body->GetWorldCenter());
            }
        }
    }
}

std::string
JetStream::asString()
{
    return thp::format("<rect class=\"jetstream\" x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" "
                       "np:force=\"%.2f,%.2f\" fill=\"blue\" stroke=\"none\" />",
                       rect.tl.x, rect.tl.y, rect.w(), rect.h(), force.x, force.y);
}

void
JetStream::activate()
{
    active = true;
}

void
JetStream::resize(const Vec2 &mouse)
{
    rect = Rect::order(origin, mouse);

    particles.clear();
    for (int i=0; i<sqrtf(rect.w() * rect.h())/10; i++) {
        particles.push_back(b2Vec2(rect.tl.x, rect.tl.y) +
                            b2Vec2(rand() % rect.width(), rand() % rect.height()));
    }
}
