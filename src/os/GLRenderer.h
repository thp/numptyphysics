#ifndef NUMPTYPHYSICS_GLRENDERER_H
#define NUMPTYPHYSICS_GLRENDERER_H

#include "Renderer.h"

class GLTextureData : public NP::TextureData {
public:
    GLTextureData(unsigned char *pixels, int width, int height);
    ~GLTextureData();

    unsigned int m_texture;
    float m_subwidth;
    float m_subheight;
};

class GLRendererPriv;

class GLRenderer : public NP::Renderer {
public:
    GLRenderer(int w, int h);
    ~GLRenderer();

    virtual void init();

    virtual void size(int *width, int *height);

    virtual NP::Texture load(unsigned char *pixels, int w, int h);

    virtual void image(const NP::Texture &texture, int x, int y);
    virtual void rectangle(const Rect &r, int rgba, bool fill);
    virtual void path(const Path &p, int rgba);

    virtual void clear();
    virtual void flush();

private:
    int m_width;
    int m_height;

    GLRendererPriv *priv;
};

#endif /* NUMPTYPHYSICS_GLRENDERER_H */
