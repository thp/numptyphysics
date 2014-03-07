#ifndef NUMPTYPHYSICS_RENDERER_H
#define NUMPTYPHYSICS_RENDERER_H

#include <memory>

#include "Common.h"
#include "Path.h"

namespace NP {

class TextureData {
public:
    TextureData(int w, int h) : w(w), h(h) {}
    virtual ~TextureData() {}

    int w;
    int h;
};

typedef std::shared_ptr<TextureData> Texture;

class FontData {
public:
    FontData(int size) : size(size) {}
    virtual ~FontData() {}

    int size;
};

typedef std::shared_ptr<FontData> Font;

class FramebufferData {
public:
    FramebufferData(int w, int h) : w(w), h(h) {}
    virtual ~FramebufferData() {}

    int w;
    int h;
};

typedef std::shared_ptr<FramebufferData> Framebuffer;

class Renderer {
public:
    virtual void init() = 0;

    virtual void size(int *width, int *height) = 0;

    virtual Texture load(const char *filename) = 0;

    virtual Framebuffer framebuffer(int width, int height) = 0;
    virtual void begin(Framebuffer &rendertarget) = 0;
    virtual void end(Framebuffer &rendertarget) = 0;
    virtual Texture retrieve(Framebuffer &rendertarget) = 0;

    virtual void image(const Texture &texture, int x, int y, int w, int h) = 0;
    virtual void rectangle(const Rect &rect, int rgba, bool fill) = 0;
    virtual void path(const Path &path, int rgba) = 0;

    virtual Font load(const char *filename, int size) = 0;

    virtual void metrics(const Font &font, const char *text, int *width, int *height) = 0;
    virtual Texture text(const Font &font, const char *text, int rgb) = 0;

    virtual void clear() = 0;
    virtual void flush() = 0;
    virtual void swap() = 0;
};

};

#endif /* NUMPTYPHYSICS_RENDERER_H */
